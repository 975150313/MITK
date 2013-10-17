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

#include "mitkImageVtkMapper2D.h"

#include <vtkSmartPointer.h>
#include <vtkPropAssembly.h>
#include <vtkPolyData.h>

class mitk::DICOMSeriesMapperVtk2DImplementation
{
  public:

  class LocalStorage : public mitk::Mapper::BaseLocalStorage
  {
    public:

      vtkSmartPointer<vtkPropAssembly> m_Actors;

      typedef std::map<Image*, DataNode::Pointer> ImageNodeContainer;
      ImageNodeContainer m_ImageNode;

      itk::TimeStamp m_LastUpdateTime;

      LocalStorage();
      ~LocalStorage();
  };

  DICOMSeriesMapperVtk2DImplementation(DICOMSeriesMapperVtk2D* object):m_Object(object){}
  DICOMSeriesMapperVtk2D* m_Object;

  /// Create/Update image mapper list
  void UpdateImageMapperList( BaseRenderer* renderer );

  /// Update all existing image mappers
  void UpdateImageMappers( BaseRenderer* renderer );

  LocalStorageHandler<LocalStorage> m_LSH;
};

mitk::DICOMSeriesMapperVtk2DImplementation::LocalStorage
::LocalStorage()
{
  m_Actors = vtkSmartPointer<vtkPropAssembly>::New();
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
    DICOMSeriesMapperVtk2DImplementation::LocalStorage* localStorage = p->m_LSH.GetLocalStorage(renderer);
/*
    for (DICOMSeriesMapperVtk2DImplementation::LocalStorage::ImageNodeContainer::iterator iter = localStorage->m_ImageNode.begin();
         iter != localStorage->m_ImageNode.end();
         ++iter)
    {
      // unless we know that image already, create/use an image mapper for it
      DataNode::Pointer node = iter->second;
      ImageVtkMapper2D* mapper = static_cast<ImageVtkMapper2D*>( node->GetMapper( BaseRenderer::Standard2D ) );

      vtkProp* prop = mapper->GetVtkProp( renderer );
      return prop;
  }
*/

  //return the actor corresponding to the renderer
  return localStorage->m_Actors;
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

  DICOMSeries::MutexLocker locker( data->GetHighPriorityLock() );

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
::KAPUTTUpdateVtkTransform(mitk::BaseRenderer* renderer)
{
  MITK_INFO << "::UpdateVtkTransform()";
  vtkLinearTransform * vtktransform = this->GetDataNode()->GetVtkTransform(this->GetTimestep());
  if (vtktransform)
  {
    vtktransform->Print(std::cout);
  }
  else
  {
    MITK_INFO << "NO transform";
  }
  vtkProp3D *prop = dynamic_cast<vtkProp3D*>( GetVtkProp(renderer) );
  if(prop)
  {
    MITK_INFO << "::  got prop 3D with transform ";
    prop->SetUserTransform(vtktransform);

    DICOMSeriesMapperVtk2DImplementation::LocalStorage* localStorage = p->m_LSH.GetLocalStorage(renderer);
    if ( localStorage->m_Actors->GetNumberOfPaths() > 1 )
    {
      vtkActor* secondaryActor = dynamic_cast<vtkActor*>( localStorage->m_Actors->GetParts()->GetItemAsObject(0) );
      secondaryActor->SetUserTransform(vtktransform);
    }
  }
  else
  {
    MITK_INFO << "::  got NO prop 3D";
  }
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
  if( !worldGeometry || !worldGeometry->IsValid() )
  {
    return;
  }

  // input must be updated (e.g. geometries exist etc.)
  input->Update();

  // assert one mapper per mitk::Image behind the DICOMImages
  if ( input->GetMTime() > localStorage->m_LastUpdateTime )
  {
    MITK_INFO << "Update of mappers neccessary!";
    p->UpdateImageMapperList( renderer );
  }

  // update the mitk::Image mappers
  p->UpdateImageMappers( renderer );

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

  MITK_INFO << "UpdateImageMapperList()";

  DICOMSeries::ConstDICOMImageList allDICOMImages = inputSeries->GetAllDICOMImages();

  //const Geometry2D* worldGeometry = renderer->GetCurrentWorldGeometry2D();
  // TODO remove useless, deactivate unused mappers!
//  localStorage->m_ImageNode.clear();


  /**

    TODO TODO TODO

    Try something like read-write locks or priorities! rendering must get quick access, image readers should wait if necessary!

    Read/Write-Lock could solve the problem:

    Rendering requests write lock
    Loaders request read locks

    Requesting the write lock must
      - wait for all reader locks to be returned
      - block all further release of reader locks (write has priority)


  */

  for (DICOMSeries::ConstDICOMImageList::iterator iter = allDICOMImages.begin();
       iter != allDICOMImages.end();
       ++iter)
  {
    // unless we know that image already, create/use an image mapper for it
    const DICOMImage* dicomImage = *iter;
    Image* mitkImage = dicomImage->GetPixelDataContainer();

    DataNode::Pointer& node = localStorage->m_ImageNode[ mitkImage ];
    if ( !node )
    {
      node = DataNode::New();
      node->SetData( mitkImage );
      /*
      MITK_INFO << "Created node[mitkImage] = " << (void*) localStorage->m_ImageNode[ mitkImage ].GetPointer();
      MITK_INFO << "  Has mapper = " << (void*) localStorage->m_ImageNode[ mitkImage ]->GetMapper( BaseRenderer::Standard2D );
      */

      VtkMapper* mapper = static_cast<VtkMapper*>( node->GetMapper( BaseRenderer::Standard2D ) );

      vtkProp* imageProp = mapper->GetVtkProp( renderer );
      //MITK_INFO << "  mapper has prop = " << (void*) imageProp;
      //imageProp->Print(std::cout);
      localStorage->m_Actors->AddPart( imageProp );
    }
  }
}

void
mitk::DICOMSeriesMapperVtk2DImplementation
::UpdateImageMappers( BaseRenderer* renderer )
{
  DICOMSeriesMapperVtk2DImplementation::LocalStorage* localStorage = this->m_LSH.GetLocalStorage(renderer);
  DICOMSeries* inputSeries = const_cast<DICOMSeries*>( m_Object->GetInput() );

  //MITK_INFO << "UpdateImageMappers():";

  for (LocalStorage::ImageNodeContainer::iterator iter = localStorage->m_ImageNode.begin();
       iter != localStorage->m_ImageNode.end();
       ++iter)
  {
    //std::cout << "." << std::flush;
    // unless we know that image already, create/use an image mapper for it
    DataNode::Pointer node = iter->second;
    ImageVtkMapper2D* mapper = static_cast<ImageVtkMapper2D*>( node->GetMapper( BaseRenderer::Standard2D ) );

    // ! this is what VtkPropRenderer does
    mapper->Update( renderer );
    mapper->UpdateVtkTransform( renderer );

    vtkProp* prop = mapper->GetVtkProp( renderer );
    assert(prop);
    vtkProp3D* imageProp = dynamic_cast<vtkProp3D*>(mapper->GetVtkProp( renderer ));
   // assert(imageProp);
    if (imageProp)
    {
      double* origin = imageProp->GetOrigin();
      MITK_INFO << "UPDATE: mapper has prop = " << (void*) imageProp << " with origin " << origin[0] << "," << origin[1] << "," << origin[2];
    }
  }

  //std::cout << std::endl;
}
