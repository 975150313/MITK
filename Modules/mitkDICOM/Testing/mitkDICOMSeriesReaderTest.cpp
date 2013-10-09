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

#include "mitkDICOMSeriesReader.h"

#include "mitkTestingMacros.h"

int mitkDICOMSeriesReaderTest(int argc, char* argv[])
{
  // always start with this!
  MITK_TEST_BEGIN("mitkDICOMSeriesReaderTest");

  MITK_TEST_CONDITION_REQUIRED( argc>=3,"Test is called with at least two arguments, the number of expected output images (first series!) and the names of a DICOM files to load");
  unsigned int expectedNumberOfImages( atoi( argv[1] ) );

  mitk::DICOMSeriesReader::StringList filenames;
  std::stringstream filenamesString; filenamesString << "'";

  for (int a = 2; a < argc; ++a)
  {
    std::string filename = argv[a];
    filenames.push_back( filename );
    filenamesString << ", '" << filename;
  }

  filenamesString << "'";

  MITK_TEST_OUTPUT(<< "Reading DICOM image(s) in " << filenamesString.str() );

  mitk::DICOMSeriesReader::Pointer reader = mitk::DICOMSeriesReader::New();
  reader->SetFilenames( filenames );
  reader->Update();

  unsigned int outputs = reader->GetNumberOfOutputs();
  MITK_TEST_CONDITION_REQUIRED( outputs == 1, "Reader creates 1 series. (Actual number " << outputs <<")" )
  mitk::DICOMSeries::Pointer dicomSeries = reader->GetOutput(0);
  MITK_TEST_CONDITION_REQUIRED( dicomSeries.IsNotNull(),"DICOMSeries read successfully");

  unsigned int numberOfImages = dicomSeries->GetNumberOfDICOMImages();
  MITK_TEST_CONDITION_REQUIRED( numberOfImages == expectedNumberOfImages, "Reader creates " << expectedNumberOfImages << " series. (Actual number " << numberOfImages <<")" )

  // TODO Add most testing parameters: expect certain features of images and planes and verify these.
  //      There is a nice Excel list for testing in the NEMA collection of multi-frame images.

  // always end with this!
  MITK_TEST_END();
}
