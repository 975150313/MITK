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

#ifndef mitkDICOMImage_h
#define mitkDICOMImage_h

#include "mitkDICOMDataset.h"

#include "mitkImage.h"
#include "mitkPlaneGeometry.h"

namespace mitk
{

class DICOMImageImplementation;

/**
  \brief TODO TODO TODO Describe class.

  TODO TODO TODO Describe class in more detail.

*/
class mitkDICOM_EXPORT DICOMImage : public DICOMDataset
{
  public:

    mitkClassMacro( DICOMImage, DICOMDataset )
    itkNewMacro( DICOMImage );
    mitkCloneMacro( DICOMImage );

    Image::Pointer GetPixelDataContainer() const;
    void SetPixelDataContainer( Image::Pointer image );

    PlaneGeometry::Pointer GetImagePlane() const;
    void SetImagePlane( PlaneGeometry::Pointer plane );

  protected:

    DICOMImage();
    virtual ~DICOMImage();

    DICOMImage(const DICOMImage& other);
    DICOMImage& operator=(const DICOMImage& other);

  private:

    DICOMImageImplementation* p;
};

}

#endif
