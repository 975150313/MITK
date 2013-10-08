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

#include "mitkTestingMacros.h"

int mitkDICOMImageTest(int /* argc */, char* /*argv*/[])
{
  // always start with this!
  MITK_TEST_BEGIN("mitkDICOMImageTest");

  mitk::DICOMImage::Pointer dicomImage = mitk::DICOMImage::New();

  MITK_TEST_CONDITION_REQUIRED(dicomImage.IsNotNull(),"DICOMImage creation");

  MITK_TEST_CONDITION(dicomImage->GetPixelDataContainer().IsNull(),"Uninitialized DICOMImage has no pixel data");
  MITK_TEST_CONDITION(dicomImage->GetImagePlane().IsNull(),"Uninitialized DICOMImage has no orientation");

  try {
    dicomImage->GetAttributeValue<double>(0x0028, 0x0030); // pixel spacing
    MITK_TEST_CONDITION( false ,"GetAttributeValue not yet implemented");
  }
  catch (...)
  {
    MITK_TEST_CONDITION( true ,"GetAttributeValue not yet implemented");
  }

  mitk::Image::Pointer mitkImage = mitk::Image::New();
  dicomImage->SetPixelDataContainer( mitkImage );
  MITK_TEST_CONDITION(dicomImage->GetPixelDataContainer() == mitkImage, "DICOMImage has mitk::Image container function");

  mitk::PlaneGeometry::Pointer plane = mitk::PlaneGeometry::New();
  dicomImage->SetImagePlane( plane );
  MITK_TEST_CONDITION(dicomImage->GetImagePlane() == plane, "DICOMImage has mitk::PlaneGeometry container function");

  // always end with this!
  MITK_TEST_END();
}
