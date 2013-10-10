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

#ifndef mitkDICOMSeriesMapperVtk2D_h
#define mitkDICOMSeriesMapperVtk2D_h

#include "mitkDICOMSeries.h"

#include "mitkVtkMapper.h"

namespace mitk
{

class DICOMSeriesMapperVtk2DImplementation;

/**
  \brief TODO TODO TODO Describe class.

  TODO TODO TODO Describe class in more detail.

*/
class mitkDICOM_EXPORT DICOMSeriesMapperVtk2D : public VtkMapper
{
  public:

    mitkClassMacro( DICOMSeriesMapperVtk2D, VtkMapper )
    itkNewMacro( DICOMSeriesMapperVtk2D );
    mitkCloneMacro( DICOMSeriesMapperVtk2D );

    const mitk::DICOMSeries* GetInput();
    virtual vtkProp* GetVtkProp(mitk::BaseRenderer*);

    virtual void Update(mitk::BaseRenderer* renderer);

  protected:

    DICOMSeriesMapperVtk2D();
    virtual ~DICOMSeriesMapperVtk2D();

    DICOMSeriesMapperVtk2D(const DICOMSeriesMapperVtk2D& other);
    DICOMSeriesMapperVtk2D& operator=(const DICOMSeriesMapperVtk2D& other);

    virtual void GenerateDataForRenderer(BaseRenderer* renderer);

  private:

    DICOMSeriesMapperVtk2DImplementation* p;
};

}

#endif
