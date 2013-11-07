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
#include "mitkDICOMDatasetGDCM.h"

#include "mitkImageGenerator.h"

#include <itkGDCMImageIO.h>
#include <itkImageSeriesReader.h>
#include <itkMutexLockHolder.h>

#include <gdcmImageReader.h>

#include <dcfilefo.h>
#include <dcdatset.h>
#include <dcmimage.h>
#include <dcdeftag.h>

/*

  TODO: make sure we don't trigger any levelwindow.SetAuto() or detect-binary in mitk::Image.
        These functions cost a LOT of time and don't add value. We can
         - exclude binary, doesn't have meaning for our images
         - read level / window from dicom tags


*/

class mitk::DICOMImageReaderImplementation
{
  public:

    typedef itk::SimpleMutexLock MutexType;
    typedef itk::MutexLockHolder<MutexType> MutexLocker;

    static MutexType m_Lock;

    std::string m_Filename;
    DICOMImageReader::DICOMImageList m_Outputs;
    PlaneGeometry::Pointer m_ImagePlane;

    void ProcessFileGDCM(const std::string& filename);

    template <typename PixelType>
    void ProcessTypedFileGDCM(const std::string& filename);

    void CreatePlaneGeometryAndFillTagsGDCM( const std::string& filename, DICOMImage* dicomImage );

    void ProcessFileDCMTK(const std::string& filename);
    DICOMImage::Pointer ReadFrameFromFileDCMTK(DcmDataset* dicomDataset, unsigned long frame);

    OFString findAndGetElementForFrame(DcmDataset* dataset, unsigned long frame, const DcmTagKey& tagKey, unsigned long pos = 0);
    float findAndGetFloatElementForFrame(DcmDataset* dataset, unsigned long frame, const DcmTagKey& tagKey, unsigned long pos = 0);

    Image::Pointer PrepareMITKImage( DcmDataset* dataset, DicomImage* dcmtkImage,
                                     unsigned long width, unsigned long height,
                                     double sliceThickness, double spacing[2],
                                     Point3D imageOrigin,
                                     Vector3D imageOrientationX,
                                     Vector3D imageOrientationY );

    PlaneGeometry::Pointer GetImagePlaneGeometry( Image* mitkImage );
};

mitk::DICOMImageReaderImplementation::MutexType mitk::DICOMImageReaderImplementation::m_Lock; // need to define



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

  p->ProcessFileGDCM( p->m_Filename );
  //p->ProcessFileDCMTK( p->m_Filename );

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
  MutexLocker lock(this->m_Lock); // TODO hmm, this is NOT ideal...

  typedef itk::GDCMImageIO IOType;
  typename IOType::Pointer io = IOType::New();

  if ( io->CanReadFile( filename.c_str() ) )
  {
    io->SetFileName( filename.c_str());
    io->ReadImageInformation();
    if (io->GetPixelType() == itk::ImageIOBase::SCALAR)
    {
      switch (io->GetComponentType())
      {
        case IOType::UCHAR:  ProcessTypedFileGDCM<unsigned char>(filename); break;
        case IOType::CHAR:   ProcessTypedFileGDCM<char>(filename); break;
        case IOType::USHORT: ProcessTypedFileGDCM<unsigned short>(filename); break;
        case IOType::SHORT:  ProcessTypedFileGDCM<short>(filename); break;
        case IOType::UINT:   ProcessTypedFileGDCM<unsigned int>(filename); break;
        case IOType::INT:    ProcessTypedFileGDCM<int>(filename); break;
        case IOType::ULONG:  ProcessTypedFileGDCM<long unsigned int>(filename); break;
        case IOType::LONG:   ProcessTypedFileGDCM<long int>(filename); break;
        case IOType::FLOAT:  ProcessTypedFileGDCM<float>(filename); break;
        case IOType::DOUBLE: ProcessTypedFileGDCM<double>(filename); break;
        default:
          MITK_ERROR << "Found unsupported DICOM scalar pixel type: (enum value) " << io->GetComponentType();
      }
    }
    else if (io->GetPixelType() == itk::ImageIOBase::RGB)
    {
      switch (io->GetComponentType())
      {
        case IOType::UCHAR:  ProcessTypedFileGDCM<itk::RGBPixel<unsigned char> >( filename ); break;
        case IOType::CHAR:   ProcessTypedFileGDCM<itk::RGBPixel<char> >( filename ); break;
        case IOType::USHORT: ProcessTypedFileGDCM<itk::RGBPixel<unsigned short> >( filename ); break;
        case IOType::SHORT:  ProcessTypedFileGDCM<itk::RGBPixel<short> >( filename ); break;
        case IOType::UINT:   ProcessTypedFileGDCM<itk::RGBPixel<unsigned int> >( filename ); break;
        case IOType::INT:    ProcessTypedFileGDCM<itk::RGBPixel<int> >( filename ); break;
        case IOType::ULONG:  ProcessTypedFileGDCM<itk::RGBPixel<long unsigned int> >( filename ); break;
        case IOType::LONG:   ProcessTypedFileGDCM<itk::RGBPixel<long int> >( filename ); break;
        case IOType::FLOAT:  ProcessTypedFileGDCM<itk::RGBPixel<float> >( filename ); break;
        case IOType::DOUBLE: ProcessTypedFileGDCM<itk::RGBPixel<double> >( filename ); break;
        default:
          MITK_ERROR << "Found unsupported DICOM scalar pixel type: (enum value) " << io->GetComponentType();
      }
    }
    else
    {
      MITK_ERROR << "Found unsupported DICOM pixel type: (enum value) " << io->GetPixelType();
    }
  }
  else
  {
    MITK_ERROR << "GDCM cannot read file " << filename;
  }
}

template <typename PixelType>
void
mitk::DICOMImageReaderImplementation
::ProcessTypedFileGDCM(const std::string& filename)
{
  typedef itk::Image<PixelType, 3> ImageType; // expect 3D
  typedef itk::ImageSeriesReader<ImageType> ReaderType;
  typedef itk::GDCMImageIO IOType;

  typename IOType::Pointer io = IOType::New();
  typename ReaderType::Pointer reader = ReaderType::New();
  reader->SetImageIO(io);
  reader->ReverseOrderOff();

  reader->SetFileName(filename);

  try
  {
    reader->Update();
  }
  catch( std::exception& e )
  {
    // TODO some error handling?
    MITK_ERROR << "ITK exception while reading file " << filename << ": " << e.what();
  }

  typename ImageType::Pointer readVolume = reader->GetOutput();
  readVolume->DisconnectPipeline();

  /*
  // if we detected that the images are from a tilted gantry acquisition, we need to push some pixels into the right position
  if (correctTilt)
  {
    readVolume = InPlaceFixUpTiltedGeometry( reader->GetOutput(), tiltInfo ); // TODO only possible with multi-frame and that is probably NOT supported by gdcm at the moment
  }
  */

  Image::Pointer mitkImage = Image::New();
  mitkImage->InitializeByItk( readVolume.GetPointer() );
  mitkImage->SetImportVolume( readVolume->GetBufferPointer() );

  DICOMImage::Pointer dicomImage = DICOMImage::New();
  dicomImage->SetPixelDataContainer( mitkImage );

  /*
  PlaneGeometry::Pointer planeGeometry = dynamic_cast<PlaneGeometry*>( mitkImage->GetGeometry() );
  MITK_INFO << "Plane geometry " << (void*) planeGeometry.GetPointer();
  dicomImage->SetImagePlane( planeGeometry );
  */
  this->CreatePlaneGeometryAndFillTagsGDCM( filename, dicomImage ); // MIGHT not create anything (reader errors)

  this->m_Outputs.push_back( dicomImage );
}

void
mitk::DICOMImageReaderImplementation
::CreatePlaneGeometryAndFillTagsGDCM( const std::string& filename, DICOMImage* dicomImage )
{
  assert(dicomImage);

  gdcm::ImageReader reader;
  reader.SetFileName( filename.c_str() );
  if ( !reader.Read() )
  {
    // error handling
    MITK_ERROR << "GDCM unable to read file " << filename << ". Perhaps we can fallback to DCMTK?";
    return;
  }

  gdcm::Image& gdcmImage = reader.GetImage();

  // "plane production"
  unsigned long width = gdcmImage.GetColumns();
  unsigned long height = gdcmImage.GetRows();

  mitk::Point3D imageOrigin;
  for (unsigned int d = 0; d < 3; ++d)
    imageOrigin[d] = gdcmImage.GetOrigin(d);

  mitk::Vector3D imageOrientationX;
  mitk::Vector3D imageOrientationY;

  for (unsigned int d = 0; d < 3; ++d)
    imageOrientationX[d] = gdcmImage.GetDirectionCosines(d);
  for (unsigned int d = 0; d < 3; ++d)
    imageOrientationY[d] = gdcmImage.GetDirectionCosines(d+3); // +3 ! direction cosine is a 6-tuple in GDCM (as in DICOM)

  mitk::Vector3D mitkSpacing;
  for (unsigned int d = 0; d < 3; ++d)
    mitkSpacing[d] = gdcmImage.GetSpacing(d); // TODO: check what is done here for d==2 exactly (1 for undefined, but what happens with simple multi-frames?)

  //InitializeStandardPlane(rightVector, downVector, spacing)
  mitk::PlaneGeometry::Pointer planeGeometry = mitk::PlaneGeometry::New();
  planeGeometry->InitializeStandardPlane( width, height, imageOrientationX, imageOrientationY, &mitkSpacing);
  planeGeometry->ChangeImageGeometryConsideringOriginOffset(true);
  planeGeometry->SetOrigin(imageOrigin);

  dicomImage->SetImagePlane( planeGeometry );
  DICOMDatasetGDCM::Pointer datasetGDCM = DICOMDatasetGDCM::New();
  datasetGDCM->Initialize( reader.GetFile().GetDataSet() );
  dicomImage->SetAttributes( *datasetGDCM );
}

/*
void
mitk::DICOMImageReaderImplementation
::ProcessFileGDCM(const std::string& filename)
{
  gdcm::ImageReader reader;
  reader.SetFileName( filename.c_str() );
  if ( !reader.Read() )
  {
    // error handling
    MITK_ERROR << "GDCM unable to read file " << filename << ". Perhaps we can fallback to DCMTK?";
    return;
  }

  gdcm::Image& gdcmImage = reader.GetImage();

  std::vector<DICOMImage::Pointer> slices;

  // "plane production"
  unsigned long width = dcmtkImage.getColumns();
  unsigned long height = dcmtkImage.getRows();

  mitk::Point3D imageOrigin;
  for (unsigned int d = 0; d < 3; ++d)
    imageOrigin[d] = gdcmImage.getOrigin(d);

  mitk::Vector3D imageOrientationX;
  mitk::Vector3D imageOrientationY;

  for (unsigned int d = 0; d < 3; ++d)
    imageOrientationX[d] = gdcmImage.getDirectionCosines(d);
  for (unsigned int d = 0; d < 3; ++d)
    imageOrientationY[d] = gdcmImage.getDirectionCosines(d+3); // +3 ! direction cosine is a 6-tuple in GDCM (as in DICOM)

  mitk::Vector3D mitkSpacing;
  for (unsigned int d = 0; d < 3; ++d)
    mitkSpacing[d] = gdcmImage.GetSpacing(d); // TODO: check what is done here for d==2 exactly (1 for undefined, but what happens with simple multi-frames?)

  //InitializeStandardPlane(rightVector, downVector, spacing)
  mitk::PlaneGeometry::Pointer planeGeometry = mitk::PlaneGeometry::New();
  planeGeometry->InitializeStandardPlane( width, height, imageOrientationX, imageOrientationY, &mitkSpacing);
  planeGeometry->ChangeImageGeometryConsideringOriginOffset(true);
  planeGeometry->SetOrigin(imageOrigin);

  // TODO special multi-frame case required!




  for (unsigned int slice = 0; slice < dimZ; ++slice)
  {
    DICOMImage::Pointer dicomImage = DICOMImage::New();

    // TODO this is the goal
    dicomImage->SetImagePlane( planeGeometry );

    //dicomImage->SetPixelDataContainer( mitkSliceImage );

    slices.push_back( dicomImage );
  }

  this->m_Outputs.insert( this->m_Outputs.end(), slices.begin(), slices.end());
}
*/

void
mitk::DICOMImageReaderImplementation
::ProcessFileDCMTK(const std::string& filename)
{
  std::vector<DICOMImage::Pointer> slices;

  DcmFileFormat dicomFile;
  if ( dicomFile.loadFile( filename.c_str() ).bad() )
  {
    // error handling
  }

  DcmDataset* dicomDataset = dicomFile.getDataset();
  if ( !dicomDataset )
  {
    // error handling
  }

  long numberOfFrames = 1;

  if ( dicomDataset->findAndGetLongInt( DCM_NumberOfFrames, numberOfFrames ).bad() )
  {
    // if we don't have DCM_NumberOfFrames, this file is single-frame so our default init is ok
    numberOfFrames = 1;
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
  MITK_INFO << "Read " << frameCount << " frames out of this file";
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

  double spacing[2] = {1.0, 1.0};
  try
  {
    spacing[0] = this->findAndGetFloatElementForFrame( dicomDataset, frame, DCM_PixelSpacing, 0 );
    spacing[1] = this->findAndGetFloatElementForFrame( dicomDataset, frame, DCM_PixelSpacing, 1 );
  }
  catch (...)
  {
    try
    {
      spacing[0] = this->findAndGetFloatElementForFrame( dicomDataset, frame, DCM_ImagerPixelSpacing, 0 );
      spacing[1] = this->findAndGetFloatElementForFrame( dicomDataset, frame, DCM_ImagerPixelSpacing, 1 );
    }
    catch (...)
    {
      // TODO error handling
      spacing[0] = 1.0;
      spacing[1] = 1.0;
    }
  }

  double sliceThickness = 1.0;
  try
  {
    sliceThickness = this->findAndGetFloatElementForFrame( dicomDataset, frame, DCM_SliceThickness, 0 );
  }
  catch (...)
  {
    // TODO error handling
    sliceThickness = 1.0;
  }

  //assert(frameCount == 1); // later! DicomImage seems to be picky
/*
  MITK_INFO << "Frame " << frame << ": " << sliceThickness << " mm image of "
            << width << "x" << height
            << " px of size "
            << spacing[0] << "x" << spacing[1]
            << " mm at position (" << imageOrigin[0] << ", " << imageOrigin[1] << ", " << imageOrigin[2]
            << "), oriented with r(" << imageOrientationX[0] << ", " << imageOrientationX[1] << ", " << imageOrientationX[2]
            << "), u(" << imageOrientationY[0] << ", "<< imageOrientationY[1] << ", " << imageOrientationY[2] << ")";
*/
  DICOMImage::Pointer dicomImage = DICOMImage::New();
  Image::Pointer mitkSliceImage = this->PrepareMITKImage( dicomDataset, &dcmtkImage, width, height, sliceThickness, spacing, imageOrigin, imageOrientationX, imageOrientationY );
  dicomImage->SetPixelDataContainer( mitkSliceImage );
  dicomImage->SetImagePlane( this->GetImagePlaneGeometry( mitkSliceImage ) );
  return dicomImage;
}

OFString
mitk::DICOMImageReaderImplementation
::findAndGetElementForFrame(DcmDataset* dataset, unsigned long frame, const DcmTagKey& tagKey, unsigned long pos)
{
  /*
    try to find
     1. per-frame functional group sequence element
     2. shared functional groups sequence element
     3. out of these sequences
  */
  OFString result;

  // 1. per-frame functional group sequence element
  DcmItem* perFrameFunctionalGroupSequence(NULL);
  OFCondition perFrameFunctionalGroupSequencePresence =
    dataset->findAndGetSequenceItem( DCM_PerFrameFunctionalGroupsSequence,
                                     perFrameFunctionalGroupSequence,
                                     frame );

  if (perFrameFunctionalGroupSequencePresence.good() && perFrameFunctionalGroupSequence)
  {
    // try to find frame-specific value
    if ( perFrameFunctionalGroupSequence->findAndGetOFString(tagKey, result, pos, true).good() )
    {
      return result;
    }
  }

  // 2. shared functional groups sequence element
  DcmItem* sharedFunctionalGroupsSequence(NULL);
  OFCondition sharedFunctionalGroupsSequencePresence =
    dataset->findAndGetSequenceItem( DCM_SharedFunctionalGroupsSequence,
                                     sharedFunctionalGroupsSequence,
                                     frame );

  if (sharedFunctionalGroupsSequencePresence.good() && sharedFunctionalGroupsSequence)
  {
    // try to find frame-specific value
    if ( sharedFunctionalGroupsSequence->findAndGetOFString(tagKey, result, pos, true).good() )
    {
      return result;
    }
  }

  // 3. out of these sequences
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

mitk::Image::Pointer
mitk::DICOMImageReaderImplementation
::PrepareMITKImage( DcmDataset* dataset, DicomImage* dcmtkImage,
                    unsigned long width, unsigned long height,
                    double sliceThickness, double spacing[2],
                    Point3D imageOrigin,
                    Vector3D imageOrientationX,
                    Vector3D imageOrientationY)
{
  unsigned int numberOfSlices = 1;

  mitk::Vector3D mitkSpacing;
  mitkSpacing[0] = spacing[0];
  mitkSpacing[1] = spacing[1];
  mitkSpacing[2] = sliceThickness; // NOT equal to z-spacing, may overlap. but for a single slice we can say thickness == z-spacing

  //InitializeStandardPlane(rightVector, downVector, spacing)
  mitk::PlaneGeometry::Pointer planeGeometry = mitk::PlaneGeometry::New();
  planeGeometry->InitializeStandardPlane( width, height, imageOrientationX, imageOrientationY, &mitkSpacing);
  planeGeometry->ChangeImageGeometryConsideringOriginOffset(true);
  planeGeometry->SetOrigin(imageOrigin);

  // Testing Initialize(const mitk::PixelType& type, const mitk::Geometry3D& geometry, unsigned int slices) with PlaneGeometry and GetData(): ";
  //Image::Pointer mitkImage = Image::New();
  //mitkImage->Initialize( mitk::MakePixelType<int, int, 1>(), *planeGeometry, numberOfSlices );
  Image::Pointer mitkImage = ImageGenerator::GenerateRandomImage<int>( width, height, 1, 1, spacing[0], spacing[1], sliceThickness, 2000, -1000 );
  mitkImage->SetGeometry( planeGeometry );

  // TODO pixel data!

  this->m_ImagePlane = planeGeometry;

  return mitkImage;
}

mitk::PlaneGeometry::Pointer
mitk::DICOMImageReaderImplementation
::GetImagePlaneGeometry( Image* mitkImage )
{
  return this->m_ImagePlane;
}
