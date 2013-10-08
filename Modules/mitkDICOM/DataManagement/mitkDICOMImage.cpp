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

#include "mitkDICOMImage.h"

class mitk::DICOMImageImplementation
{
  public:

    mitk::Image::Pointer m_MitkImage;
    mitk::PlaneGeometry::Pointer m_ImagePlane;
};


mitk::DICOMImage::DICOMImage()
:DICOMDataset()
,p(new DICOMImageImplementation)
{
}

mitk::DICOMImage::DICOMImage(const DICOMImage& other)
:DICOMDataset()
,p(new DICOMImageImplementation)
{
  this->p = other.p;
}

mitk::DICOMImage::~DICOMImage()
{
  delete p;
}

mitk::DICOMImage& mitk::DICOMImage::operator=(const mitk::DICOMImage& other)
{
  if (this != &other)
  {
    DICOMDataset::operator=(other);

    this->p = other.p;
  }

  return *this;
}

mitk::Image::Pointer mitk::DICOMImage::GetPixelDataContainer() const
{
  return p->m_MitkImage;
}

void mitk::DICOMImage::SetPixelDataContainer( Image::Pointer image )
{
  p->m_MitkImage = image;
}

mitk::PlaneGeometry::Pointer mitk::DICOMImage::GetImagePlane() const
{
  return p->m_ImagePlane;
}

void mitk::DICOMImage::SetImagePlane( PlaneGeometry::Pointer plane )
{
  p->m_ImagePlane = plane;
}
