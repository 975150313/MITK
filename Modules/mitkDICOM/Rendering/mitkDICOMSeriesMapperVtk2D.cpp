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

#include "mitkDICOMSeriesMapperVtk2D.h"

#include "mitkLocalStorageHandler.h"

#include <vtkSmartPointer.h>
#include <vtkPropAssembly.h>
#include <vtkPolyData.h>

class mitk::DICOMSeriesMapperVtk2DImplementation
{
  public:

  class LocalStorage : public mitk::Mapper::BaseLocalStorage
  {
    public:

      vtkSmartPointer<vtkActor> m_EmptyActor;
      vtkSmartPointer<vtkPropAssembly> m_Actors;
      vtkSmartPointer<vtkPolyData> m_EmptyPolyData;
      vtkSmartPointer<vtkPolyDataMapper> m_Mapper;

      itk::TimeStamp m_LastUpdateTime;

      LocalStorage();
      ~LocalStorage();
  };

  DICOMSeriesMapperVtk2DImplementation(DICOMSeriesMapperVtk2D* object):m_Object(object){}
  DICOMSeriesMapperVtk2D* m_Object;

  void UpdateImageMapperList( BaseRenderer* renderer );

  LocalStorageHandler<LocalStorage> m_LSH;
};

mitk::DICOMSeriesMapperVtk2DImplementation::LocalStorage
::LocalStorage()
{
  m_EmptyPolyData = vtkSmartPointer<vtkPolyData>::New();

  m_Mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
  m_Mapper->SetInput( m_EmptyPolyData );

  m_EmptyActor = vtkSmartPointer<vtkActor>::New();
  m_EmptyActor->SetMapper( m_Mapper );

  m_Actors = vtkSmartPointer<vtkPropAssembly>::New();
  m_Actors->AddPart( m_EmptyActor );
}

mitk::DICOMSeriesMapperVtk2DImplementation::LocalStorage
::~LocalStorage()
{
}

mitk::DICOMSeriesMapperVtk2D
::DICOMSeriesMapperVtk2D()
:VtkMapper()
,p(new DICOMSeriesMapperVtk2DImplementation(this))
{
}

mitk::DICOMSeriesMapperVtk2D
::DICOMSeriesMapperVtk2D(const DICOMSeriesMapperVtk2D& other)
:VtkMapper()
,p(new DICOMSeriesMapperVtk2DImplementation(this))
{
  this->p = other.p;
}

mitk::DICOMSeriesMapperVtk2D
::~DICOMSeriesMapperVtk2D()
{
  delete p;
}

mitk::DICOMSeriesMapperVtk2D&
mitk::DICOMSeriesMapperVtk2D
::operator=(const mitk::DICOMSeriesMapperVtk2D& other)
{
  if (this != &other)
  {
    this->p = other.p;
  }

  return *this;
}

vtkProp*
mitk::DICOMSeriesMapperVtk2D
::GetVtkProp(mitk::BaseRenderer* renderer)
{
  //return the actor corresponding to the renderer
  return p->m_LSH.GetLocalStorage(renderer)->m_Actors;
}

const mitk::DICOMSeries*
mitk::DICOMSeriesMapperVtk2D
::GetInput()
{
  return static_cast<const mitk::DICOMSeries*>( GetDataNode()->GetData() );
}

void
mitk::DICOMSeriesMapperVtk2D
::Update(mitk::BaseRenderer* renderer)
{
  bool visible = true;
  GetDataNode()->GetVisibility(visible, renderer, "visible");

  if ( !visible )
  {
    return;
  }

  DICOMSeries* data  = const_cast<DICOMSeries *>( this->GetInput() );
  if ( !data ) return;

  // Calculate time step of the input data for the specified renderer (integer value)
  this->CalculateTimeStep( renderer );

  // Check if time step is valid
  const TimeSlicedGeometry *dataTimeGeometry = data->GetTimeSlicedGeometry();
  if ( ( dataTimeGeometry == NULL )
       || ( dataTimeGeometry->GetTimeSteps() == 0 )
       || ( !dataTimeGeometry->IsValidTime( this->GetTimestep() ) ) )
  {
    return;
  }

  const DataNode* node = this->GetDataNode();
  data->UpdateOutputInformation();
  DICOMSeriesMapperVtk2DImplementation::LocalStorage* localStorage = p->m_LSH.GetLocalStorage(renderer);

  //check if something important has changed and we need to rerender
  if ( (localStorage->m_LastUpdateTime < node->GetMTime()) //was the node modified?
       || (localStorage->m_LastUpdateTime < data->GetPipelineMTime()) //Was the data modified?
       || (localStorage->m_LastUpdateTime < renderer->GetCurrentWorldGeometry2DUpdateTime()) //was the geometry modified?
       || (localStorage->m_LastUpdateTime < renderer->GetCurrentWorldGeometry2D()->GetMTime())
       || (localStorage->m_LastUpdateTime < node->GetPropertyList()->GetMTime()) //was a property modified?
       || (localStorage->m_LastUpdateTime < node->GetPropertyList(renderer)->GetMTime()) )
  {
    this->GenerateDataForRenderer( renderer );
  }

  // since we have checked that nothing important has changed, we can set
  // m_LastUpdateTime to the current time
  localStorage->m_LastUpdateTime.Modified();
}


void
mitk::DICOMSeriesMapperVtk2D
::GenerateDataForRenderer( BaseRenderer* renderer )
{
  DICOMSeriesMapperVtk2DImplementation::LocalStorage* localStorage = p->m_LSH.GetLocalStorage(renderer);

  DICOMSeries* input = const_cast<DICOMSeries*>( this->GetInput() );

  if (!input) return;

  //check if there is a valid worldGeometry
  const Geometry2D* worldGeometry = renderer->GetCurrentWorldGeometry2D();
  if( !worldGeometry || !worldGeometry->IsValid() || !worldGeometry->HasReferenceGeometry() )
  {
    return;
  }

  input->Update();

  p->UpdateImageMapperList( renderer );
  /*
     GenerateDataForRenderer in ImageVtkMapper2D does:

     - check intersection of render geometry and data : early-out
       - call localStorage->m_Mapper->SetInput( emptyPolyData )

     - reslicer setinput setworld set timestep setreslicetransformbygeometry
     - reslicer MODIFIED, UPDATE
       - reslicedimage = reslicer output
     - levelwindowfilter setinput reslicedimage
     - texture setinput levelwindowfilter getoutput

     - m_Actor settexture texture
  */
}

void
mitk::DICOMSeriesMapperVtk2DImplementation
::UpdateImageMapperList( BaseRenderer* renderer )
{
  DICOMSeriesMapperVtk2DImplementation::LocalStorage* localStorage = this->m_LSH.GetLocalStorage(renderer);
  DICOMSeries* inputSeries = const_cast<DICOMSeries*>( m_Object->GetInput() );

  MITK_INFO << "UpdateImageMapperList(!)";

  //const Geometry2D* worldGeometry = renderer->GetCurrentWorldGeometry2D();

//  DICOMSeries::ConstDICOMImageList allDICOMImages = inputSeries->GetAllDICOMImages();

  //localStorage->m_Actors->AddPart( imageMapperActor );
}
