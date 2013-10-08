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

#include "gdcmImageReader.h"

#include "dcfilefo.h"
#include "dcdatset.h"
#include "dcmimage.h"
#include "dcdeftag.h"

class mitk::DICOMImageReaderImplementation
{
  public:

    std::string m_Filename;
    std::vector<DICOMImage::Pointer> m_Outputs;

    void ProcessFileGDCM(const std::string& filename);

    void ProcessFileDCMTK(const std::string& filename);
    DICOMImage::Pointer ReadFrameFromFileDCMTK(DcmDataset* dicomDataset, unsigned long frame);

    OFString findAndGetElementForFrame(DcmDataset* dataset, unsigned long frame, const DcmTagKey& tagKey, unsigned long pos = 0);
    float findAndGetFloatElementForFrame(DcmDataset* dataset, unsigned long frame, const DcmTagKey& tagKey, unsigned long pos = 0);
};


mitk::DICOMImageReader
::DICOMImageReader()
:itk::LightObject()
,p(new DICOMImageReaderImplementation)
{
}

mitk::DICOMImageReader
::DICOMImageReader(const DICOMImageReader& other)
:itk::LightObject()
,p(new DICOMImageReaderImplementation)
{
  this->p = other.p;
}

mitk::DICOMImageReader
::~DICOMImageReader()
{
  delete p;
}

mitk::DICOMImageReader&
mitk::DICOMImageReader
::operator=(const mitk::DICOMImageReader& other)
{
  if (this != &other)
  {
    this->p = other.p;
  }

  return *this;
}

void
mitk::DICOMImageReader
::SetFilename( const std::string& filename )
{
  p->m_Filename = filename;
}

void
mitk::DICOMImageReader
::Update()
{
  p->m_Outputs.clear();

  // TODO check validness!

  //p->ProcessFileGDCM( p->m_Filename );
  p->ProcessFileDCMTK( p->m_Filename );

  // read (multiple) mitk::Image(s) from file
  //   with plane geometry
  // fill p->m_Output

}

unsigned int
mitk::DICOMImageReader
::GetNumberOfOutputs() const
{
  return p->m_Outputs.size();
}

mitk::DICOMImage::Pointer
mitk::DICOMImageReader
::GetOutput(unsigned int idx) const
{
  if ( idx > p->m_Outputs.size() )
  {
    std::stringstream ss;
    ss << "Index out of range in DICOMImageReader::GetOutput(" << idx << ")";
    throw std::invalid_argument( ss.str() );
  }

  return p->m_Outputs[idx];
}

void
mitk::DICOMImageReaderImplementation
::ProcessFileGDCM(const std::string& filename)
{
  gdcm::ImageReader reader;
  reader.SetFileName( filename.c_str() );
  if ( !reader.Read() )
  {
    // error handling
    return;
  }

  gdcm::Image& gdcmImage = reader.GetImage();

  std::vector<DICOMImage::Pointer> slices;

  const unsigned int* dimension = gdcmImage.GetDimensions();

  //unsigned int dimX = dimension[0];
  //unsigned int dimY = dimension[1];
  unsigned int dimZ = dimension[2];

  for (unsigned int slice = 0; slice < dimZ; ++slice)
  {
    DICOMImage::Pointer dicomImage = DICOMImage::New();

    slices.push_back( dicomImage );
  }

  this->m_Outputs.insert( this->m_Outputs.end(), slices.begin(), slices.end());
}

void
mitk::DICOMImageReaderImplementation
::ProcessFileDCMTK(const std::string& filename)
{
  std::vector<DICOMImage::Pointer> slices;

  DcmFileFormat dicomFile;
  if ( !dicomFile.loadFile( filename.c_str() ).good() )
  {
    // error handling
  }

  DcmDataset* dicomDataset = dicomFile.getDataset();
  if ( !dicomDataset )
  {
    // error handling
  }

  long numberOfFrames = 0;

  if ( !dicomDataset->findAndGetLongInt( DCM_NumberOfFrames, numberOfFrames ).good() )
  {
    // error handling
  }

  for (long frame = 0; frame < numberOfFrames; ++frame)
  {
    DICOMImage::Pointer slice = this->ReadFrameFromFileDCMTK( dicomDataset, frame );
    this->m_Outputs.push_back( slice );
  }
}

mitk::DICOMImage::Pointer
mitk::DICOMImageReaderImplementation
::ReadFrameFromFileDCMTK(DcmDataset* dicomDataset, unsigned long frame)
{
  unsigned long flags = 0; // TODO: check all CIF_...
  unsigned long numberOfFrames = 1; // 0 = unlimited

  ::DicomImage dcmtkImage( dicomDataset, dicomDataset->getOriginalXfer(), flags, frame, numberOfFrames );

  unsigned long frameCount = dcmtkImage.getFrameCount();
  unsigned long width = dcmtkImage.getWidth();
  unsigned long height = dcmtkImage.getHeight();

  mitk::Point3D imageOrigin;

  try
  {
    imageOrigin[0] = this->findAndGetFloatElementForFrame( dicomDataset, frame, DCM_ImagePositionPatient, 0 );
    imageOrigin[1] = this->findAndGetFloatElementForFrame( dicomDataset, frame, DCM_ImagePositionPatient, 1 );
    imageOrigin[2] = this->findAndGetFloatElementForFrame( dicomDataset, frame, DCM_ImagePositionPatient, 2 );
  }
  catch (...)
  {
    MITK_WARN << "Could not read tag 'Image Position Patient', guessing (0,0,0)";

    imageOrigin.Fill(0.0);

    // TODO error handling
  }

  mitk::Vector3D imageOrientationX;
  mitk::Vector3D imageOrientationY;

  try
  {
    imageOrientationX[0] = this->findAndGetFloatElementForFrame( dicomDataset, frame, DCM_ImageOrientationPatient, 0 );
    imageOrientationX[1] = this->findAndGetFloatElementForFrame( dicomDataset, frame, DCM_ImageOrientationPatient, 1 );
    imageOrientationX[2] = this->findAndGetFloatElementForFrame( dicomDataset, frame, DCM_ImageOrientationPatient, 2 );
    imageOrientationY[0] = this->findAndGetFloatElementForFrame( dicomDataset, frame, DCM_ImageOrientationPatient, 3 );
    imageOrientationY[1] = this->findAndGetFloatElementForFrame( dicomDataset, frame, DCM_ImageOrientationPatient, 4 );
    imageOrientationY[2] = this->findAndGetFloatElementForFrame( dicomDataset, frame, DCM_ImageOrientationPatient, 5 );
  }
  catch (...)
  {
    MITK_WARN << "Could not read tag 'Image Orientation Patient', guessing (1,0,0), (0,1,0)";

    imageOrientationX[0] = 1.0;
    imageOrientationX[1] = 0.0;
    imageOrientationX[2] = 0.0;

    imageOrientationY[0] = 0.0;
    imageOrientationY[1] = 1.0;
    imageOrientationY[2] = 0.0;

    // TODO error handling
  }

  MITK_INFO << "Read frame " << frame << " as " << numberOfFrames << " image: "
            << width << "x" << height
            << " at (" << imageOrigin[0] << ", " << imageOrigin[1] << ", " << imageOrigin[2]
            << "), oriented with (" << imageOrientationX[0] << ", " << imageOrientationX[1] << ", " << imageOrientationX[2]
            << "),(" << imageOrientationY[0] << ", "<< imageOrientationY[1] << ", " << imageOrientationY[2] << ")";

  DICOMImage::Pointer dicomImage = DICOMImage::New();
  return dicomImage;
}

OFString
mitk::DICOMImageReaderImplementation
::findAndGetElementForFrame(DcmDataset* dataset, unsigned long frame, const DcmTagKey& tagKey, unsigned long pos)
{
  OFString result;
  if ( dataset->findAndGetOFString(tagKey, result, pos, true).bad() )
  {
    throw std::range_error("Could not find requested tag");
  };
  return result;
}

float
mitk::DICOMImageReaderImplementation
::findAndGetFloatElementForFrame(DcmDataset* dataset, unsigned long frame, const DcmTagKey& tagKey, unsigned long pos)
{
  OFString stringValue = this->findAndGetElementForFrame(dataset, frame, tagKey, pos);
  return static_cast<float>( atof( stringValue.c_str() ) ); // TODO: error handling
}
