/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/


#include "mitkGizmo.h"
#include "mitkGizmoInteractor3D.h"

// MITK includes
#include <mitkRenderingManager.h>
#include <mitkBaseRenderer.h>
#include <mitkVtkInterpolationProperty.h>
#include <mitkLookupTableProperty.h>

// VTK includes
#include <vtkConeSource.h>
#include <vtkSphereSource.h>
#include <vtkCylinderSource.h>
#include <vtkAppendPolyData.h>
#include <vtkTransformPolyDataFilter.h>
#include <vtkPointData.h>
#include <vtkCharArray.h>
#include <vtkRenderWindow.h>
#include <vtkPoints.h>
#include <vtkCellArray.h>
#include <vtkTubeFilter.h>
#include <vtkMath.h>
#include <vtkPolyDataNormals.h>

// ITK includes
#include <itkCommand.h>

// MicroServices
#include <usGetModuleContext.h>

#define _USE_MATH_DEFINES
#include <math.h>

mitk::DataNode::Pointer mitk::Gizmo::AddGizmoToNode(DataNode* node, DataStorage* storage)
{
  assert(node);
  if (node->GetData() == nullptr ||
      node->GetData()->GetGeometry() == nullptr)
  {
    return nullptr;
  }
  //--------------------------------------------------------------
  // Add visual gizmo that follows the node to be manipulated
  //--------------------------------------------------------------

  auto gizmo = Gizmo::New();
  auto gizmoNode = DataNode::New();
  gizmoNode->SetName("Gizmo");
  gizmoNode->SetData(gizmo);
  //gizmoNode->SetProperty("material.ambientCoefficient", FloatProperty::New(0));
  //gizmoNode->SetProperty("material.diffuseCoefficient", FloatProperty::New(1));
  //gizmoNode->SetProperty("material.specularCoefficient", FloatProperty::New(0.2));
  //gizmoNode->SetProperty("material.interpolation", VtkInterpolationProperty::New(2)); // PHONG
  gizmoNode->SetProperty("scalar visibility", BoolProperty::New(true));
  gizmoNode->SetProperty("ScalarsRangeMinimum", DoubleProperty::New(0));
  gizmoNode->SetProperty("ScalarsRangeMaximum", DoubleProperty::New(6));

  double colorMoveFreely[] = {1,0,0,1}; // RGBA
  double colorAxisX[] = {0.753,0,0,1}; // colors copied from QmitkStdMultiWidget to
  double colorAxisY[] = {0,0.69,0,1};  // look alike
  double colorAxisZ[] = {0,0.502,1,1};

  // build a nice color table
  vtkSmartPointer<vtkLookupTable> lut = vtkSmartPointer<vtkLookupTable>::New();
  lut->SetNumberOfTableValues(7);
  lut->SetTableRange(0,6);
  lut->SetTableValue(MoveFreely, colorMoveFreely);
  lut->SetTableValue(MoveAlongAxisX, colorAxisX);
  lut->SetTableValue(MoveAlongAxisY, colorAxisY);
  lut->SetTableValue(MoveAlongAxisZ, colorAxisZ);
  lut->SetTableValue(RotateAroundAxisX, colorAxisX);
  lut->SetTableValue(RotateAroundAxisY, colorAxisY);
  lut->SetTableValue(RotateAroundAxisZ, colorAxisZ);

  mitk::LookupTable::Pointer mlut = mitk::LookupTable::New();
  mlut->SetVtkLookupTable(lut);

  mitk::LookupTableProperty::Pointer lutProp = mitk::LookupTableProperty::New();
  lutProp->SetLookupTable(mlut);
  gizmoNode->SetProperty("LookupTable", lutProp);


  // Hide by default, show in all 3D windows
  gizmoNode->SetProperty("helper object", BoolProperty::New(true));
  gizmoNode->SetProperty("visible", BoolProperty::New(false));
  auto rwList = RenderingManager::GetInstance()->GetAllRegisteredRenderWindows();
  for (auto& rw : rwList)
  {
    auto renderer = BaseRenderer::GetInstance(rw);
    if (renderer != nullptr)
    {
      if (renderer->GetMapperID() == BaseRenderer::Standard3D)
      {
        gizmoNode->SetProperty("visible", BoolProperty::New(true), renderer);
      }
    }
  }

  gizmo->FollowGeometry(node->GetData()->GetGeometry());

  //--------------------------------------------------------------
  // Add interaction to the gizmo
  //--------------------------------------------------------------

  mitk::GizmoInteractor3D::Pointer interactor = mitk::GizmoInteractor3D::New();
  interactor->LoadStateMachine("Gizmo3DStates.xml", us::GetModuleContext()->GetModule());
  interactor->SetEventConfig("Gizmo3DConfig.xml", us::ModuleRegistry::GetModule("MitkGizmo"));

  interactor->SetGizmoNode(gizmoNode);
  interactor->SetManipulatedObjectNode(node);

  if (storage)
  {
    storage->Add(gizmoNode);
  }

  return gizmoNode;
}

mitk::Gizmo::Gizmo()
: Surface()
{
  m_Center.Fill(0);

  m_AxisX.Fill(0); m_AxisX[0] = 1;
  m_AxisY.Fill(0); m_AxisY[1] = 1;
  m_AxisZ.Fill(0); m_AxisZ[2] = 1;

  m_Radius.Fill(1);

  UpdateRepresentation();
}

void mitk::Gizmo::UpdateRepresentation()
{
  /* bounding box around the unscaled bounding object */
  ScalarType bounds[6]={-m_Radius[0]*1.2,+m_Radius[0]*1.2,
                        -m_Radius[1]*1.2,+m_Radius[1]*1.2,
                        -m_Radius[2]*1.2,+m_Radius[2]*1.2};
  GetGeometry()->SetBounds(bounds);
  GetTimeGeometry()->Update();

  SetVtkPolyData( BuildGizmo() );
}

mitk::Gizmo::~Gizmo()
{
}

namespace {

void AssignScalarValueTo(vtkPolyData* polydata, char value)
{
  vtkSmartPointer<vtkCharArray> pointData = vtkSmartPointer<vtkCharArray>::New();

  int numberOfPoints = polydata->GetNumberOfPoints();
  pointData->SetNumberOfComponents(1);
  pointData->SetNumberOfTuples(numberOfPoints);
  pointData->FillComponent(0,value);
  polydata->GetPointData()->SetScalars(pointData);
}

vtkSmartPointer<vtkPolyData> BuildAxis(const mitk::Point3D& center,
                                       const mitk::Vector3D& axis,
                                       double halflength,
                                       char vertexValueAxis,
                                       char vertexValueRing)
{
  const double shaftRadius = 1;
  const double arrowHeight = shaftRadius * 6;
  const int tubeSides = 15;

  // poly data appender to collect cones and tube that make up the axis
  vtkSmartPointer<vtkAppendPolyData> axisSource = vtkSmartPointer<vtkAppendPolyData>::New();

  // build two cones at the end of axis
  for (double sign = -1.0; sign < 3.0; sign += 2)
  {
    vtkSmartPointer<vtkConeSource> cone = vtkConeSource::New();
    cone->SetCenter(center[0] + sign * axis[0] * (halflength * 1.1 + arrowHeight * 0.5), // arrow tips at 110% of radius
                    center[1] + sign * axis[1] * (halflength * 1.1 + arrowHeight * 0.5),
                    center[2] + sign * axis[2] * (halflength * 1.1 + arrowHeight * 0.5));
    cone->SetDirection(sign * axis[0],
                       sign * axis[1],
                       sign * axis[2]);
    cone->SetRadius(shaftRadius * 3);
    cone->SetHeight(arrowHeight);
    cone->SetResolution(tubeSides);
    cone->CappingOn();
    cone->Update();
    axisSource->AddInputData(cone->GetOutput());
  }

  // build the axis itself (as a tube around the line defining the axis)
  vtkSmartPointer<vtkPolyData> shaftSkeleton = vtkSmartPointer<vtkPolyData>::New();
  vtkSmartPointer<vtkPoints> shaftPoints = vtkSmartPointer<vtkPoints>::New();
  shaftPoints->InsertPoint(0, (center - axis * halflength * 1.1).GetDataPointer());
  shaftPoints->InsertPoint(1, (center + axis * halflength * 1.1).GetDataPointer());
  shaftSkeleton->SetPoints(shaftPoints);

  vtkSmartPointer<vtkCellArray> shaftLines = vtkSmartPointer<vtkCellArray>::New();
  vtkIdType shaftLinePoints[] = {0,1};
  shaftLines->InsertNextCell(2, shaftLinePoints);
  shaftSkeleton->SetLines(shaftLines);

  vtkSmartPointer<vtkTubeFilter> shaftSource = vtkSmartPointer<vtkTubeFilter>::New();
  shaftSource->SetInputData(shaftSkeleton);
  shaftSource->SetNumberOfSides(tubeSides);
  shaftSource->SetVaryRadiusToVaryRadiusOff();
  shaftSource->SetRadius(shaftRadius);
  shaftSource->Update();

  axisSource->AddInputData(shaftSource->GetOutput());
  axisSource->Update();
  AssignScalarValueTo(axisSource->GetOutput(), vertexValueAxis);

  // build the ring orthogonal to the axis (as another tube)
  vtkSmartPointer<vtkPolyData> ringSkeleton = vtkSmartPointer<vtkPolyData>::New();
  vtkSmartPointer<vtkPoints> ringPoints = vtkSmartPointer<vtkPoints>::New();
  ringPoints->SetDataTypeToDouble(); // just some decision (see cast below)
  unsigned int numberOfRingPoints = 100;
  vtkSmartPointer<vtkCellArray> ringLines = vtkSmartPointer<vtkCellArray>::New();
  ringLines->InsertNextCell(numberOfRingPoints+1);
  mitk::Vector3D ringPointer;
  for (unsigned int segment = 0; segment < numberOfRingPoints; ++segment)
  {
    ringPointer[0] = 0;
    ringPointer[1] = std::cos( (double)(segment)/(double)numberOfRingPoints * 2.0 * vtkMath::Pi() );
    ringPointer[2] = std::sin( (double)(segment)/(double)numberOfRingPoints * 2.0 * vtkMath::Pi() );

    ringPoints->InsertPoint(segment, (ringPointer * halflength).GetDataPointer());

    ringLines->InsertCellPoint(segment);
  }
  ringLines->InsertCellPoint(0);

  // transform ring points (copied from vtkConeSource)
  vtkSmartPointer<vtkTransform> t = vtkSmartPointer<vtkTransform>::New();
  t->Translate(center.GetDataPointer());
  double vMag = vtkMath::Norm(axis.GetDataPointer());
  if ( axis[0] < 0.0 ) {
      // flip x -> -x to avoid instability
      t->RotateWXYZ(180.0, (axis[0] - vMag) / 2.0,
                    axis[1] / 2.0, axis[2] / 2.0);
      t->RotateWXYZ(180.0, 0, 1, 0);
  } else {
      t->RotateWXYZ(180.0, (axis[0] + vMag) / 2.0,
                    axis[1] / 2.0, axis[2] / 2.0);
  }

  double thisPoint[3];
  for ( int i = 0; i < numberOfRingPoints; ++i )
  {
      ringPoints->GetPoint(i, thisPoint);
      t->TransformPoint(thisPoint, thisPoint);
      ringPoints->SetPoint(i, thisPoint);
  }

  ringSkeleton->SetPoints(ringPoints);
  ringSkeleton->SetLines(ringLines);

  vtkSmartPointer<vtkTubeFilter> ringSource = vtkSmartPointer<vtkTubeFilter>::New();
  ringSource->SetInputData(ringSkeleton);
  ringSource->SetNumberOfSides(tubeSides);
  ringSource->SetVaryRadiusToVaryRadiusOff();
  ringSource->SetRadius(shaftRadius);
  ringSource->Update();
  AssignScalarValueTo( ringSource->GetOutput(), vertexValueRing );

  // assemble axis and ring
  vtkSmartPointer<vtkAppendPolyData> appenderGlobal = vtkSmartPointer<vtkAppendPolyData>::New();
  appenderGlobal->AddInputData( axisSource->GetOutput() );
  appenderGlobal->AddInputData( ringSource->GetOutput() );
  appenderGlobal->Update();

  // make everything shiny by adding normals
  vtkSmartPointer<vtkPolyDataNormals> normalsSource = vtkSmartPointer<vtkPolyDataNormals>::New();
  normalsSource->SetInputConnection( appenderGlobal->GetOutputPort() );
  normalsSource->ComputePointNormalsOn();
  normalsSource->ComputeCellNormalsOff();
  normalsSource->SplittingOn();
  normalsSource->Update();

  vtkSmartPointer<vtkPolyData> result = normalsSource->GetOutput();
  return result;
}

} // unnamed namespace

vtkSmartPointer<vtkPolyData> mitk::Gizmo::BuildGizmo()
{
  double longestAxis = std::max( m_Radius[0], m_Radius[1] );
  longestAxis = std::max( longestAxis, m_Radius[2] );

  vtkSmartPointer<vtkAppendPolyData> appender = vtkSmartPointer<vtkAppendPolyData>::New();
  appender->AddInputData( BuildAxis(m_Center, m_AxisX, longestAxis, MoveAlongAxisX, RotateAroundAxisX) );
  appender->AddInputData( BuildAxis(m_Center, m_AxisY, longestAxis, MoveAlongAxisY, RotateAroundAxisY) );
  appender->AddInputData( BuildAxis(m_Center, m_AxisZ, longestAxis, MoveAlongAxisZ, RotateAroundAxisZ) );

  auto sphereSource = vtkSmartPointer<vtkSphereSource>::New();
  sphereSource->SetCenter(m_Center[0], m_Center[1], m_Center[2]);
  sphereSource->SetRadius(longestAxis * 0.05);
  sphereSource->Update();
  AssignScalarValueTo( sphereSource->GetOutput(), MoveFreely );

  appender->AddInputData( sphereSource->GetOutput() );

  appender->Update();
  return appender->GetOutput();
}

void mitk::Gizmo::FollowGeometry(BaseGeometry* geom)
{
  auto observer = itk::SimpleMemberCommand<Gizmo>::New();
  observer->SetCallbackFunction(this, &Gizmo::OnFollowedGeometryModified);

  m_FollowedGeometry = geom;
  m_FollowedGeometry->AddObserver(itk::ModifiedEvent(), observer);

  // initial adjustment
  OnFollowedGeometryModified();
}

void mitk::Gizmo::OnFollowedGeometryModified()
{
    m_Center = m_FollowedGeometry->GetCenter();

    m_AxisX = m_FollowedGeometry->GetAxisVector(0);
    m_AxisY = m_FollowedGeometry->GetAxisVector(1);
    m_AxisZ = m_FollowedGeometry->GetAxisVector(2);

    m_AxisX.Normalize();
    m_AxisY.Normalize();
    m_AxisZ.Normalize();

    for ( int dim = 0; dim < 3; ++dim )
    {
      m_Radius[dim] = 0.5 * m_FollowedGeometry->GetExtentInMM(dim);
    }

    UpdateRepresentation();
}

mitk::Gizmo::HandleType mitk::Gizmo::GetHandleFromPointID(vtkIdType id)
{

#define CheckHandleType(type) \
  if (static_cast<int>(dataValue) == static_cast<int>(type)) \
    return type;

  assert(GetVtkPolyData());
  assert(GetVtkPolyData()->GetPointData());
  assert(GetVtkPolyData()->GetPointData()->GetScalars());
  // TODO check if id exists.. what otherwise?
  double dataValue = GetVtkPolyData()->GetPointData()->GetScalars()->GetTuple1(id);

  CheckHandleType(MoveFreely);
  CheckHandleType(MoveAlongAxisX);
  CheckHandleType(MoveAlongAxisY);
  CheckHandleType(MoveAlongAxisZ);
  CheckHandleType(RotateAroundAxisX);
  CheckHandleType(RotateAroundAxisY);
  CheckHandleType(RotateAroundAxisZ);
  return NoHandle;
}

std::string mitk::Gizmo::HandleTypeToString(HandleType type)
{
#define CheckHandleType(candidateType) \
  if (type == candidateType) \
    return std::string(#candidateType);

  CheckHandleType(MoveFreely);
  CheckHandleType(MoveAlongAxisX);
  CheckHandleType(MoveAlongAxisY);
  CheckHandleType(MoveAlongAxisZ);
  CheckHandleType(RotateAroundAxisX);
  CheckHandleType(RotateAroundAxisY);
  CheckHandleType(RotateAroundAxisZ);
  CheckHandleType(NoHandle);
  return "InvalidHandleType";
}
