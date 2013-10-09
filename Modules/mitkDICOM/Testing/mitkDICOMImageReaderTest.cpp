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

#include "mitkDICOMImageReader.h"

#include "mitkTestingMacros.h"

int mitkDICOMImageReaderTest(int argc, char* argv[])
{
  // always start with this!
  MITK_TEST_BEGIN("mitkDICOMImageReaderTest");

  MITK_TEST_CONDITION_REQUIRED( argc==3,"Test is called with exactly two arguments, the number of expected output images and the name of a DICOM image file");
  unsigned int expectedOutputs( atoi( argv[1] ) );
  std::string filename = argv[2];
  MITK_TEST_CONDITION_REQUIRED( !filename.empty(),"Test is called with a non-empty filename");

  MITK_TEST_OUTPUT( << "Reading DICOM image(s) in " << filename);

  mitk::DICOMImageReader::Pointer reader = mitk::DICOMImageReader::New();
  reader->SetFilename( filename );
  reader->Update();
  unsigned int outputs = reader->GetNumberOfOutputs();
  MITK_TEST_CONDITION_REQUIRED( outputs == expectedOutputs, "Reader creates " << expectedOutputs << " images. (Actual number " << outputs <<")" )
  mitk::DICOMImage::Pointer dicomImage = reader->GetOutput();

  MITK_TEST_CONDITION_REQUIRED( dicomImage.IsNotNull(),"DICOMImage read successfully");

  MITK_TEST_CONDITION(dicomImage->GetPixelDataContainer().IsNotNull(),"Read DICOMImage has pixel data");
  MITK_TEST_CONDITION(dicomImage->GetImagePlane().IsNotNull(),"Read DICOMImage has orientation");

  // TODO Add most testing parameters: expect certain features of images and planes and verify these.
  //      There is a nice Excel list for testing in the NEMA collection of multi-frame images.

  // always end with this!
  MITK_TEST_END();
}
