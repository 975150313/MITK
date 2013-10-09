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

#include "mitkDICOMImageReader.h"

class mitk::DICOMSeriesReaderImplementation
{
  public:

    DICOMSeriesReader::StringList m_Filenames;
    std::vector<DICOMSeries::Pointer> m_Outputs;

    void ProcessFile( const std::string& filename );
};


mitk::DICOMSeriesReader
::DICOMSeriesReader()
:itk::LightObject()
,p(new DICOMSeriesReaderImplementation)
{
}

mitk::DICOMSeriesReader
::DICOMSeriesReader(const DICOMSeriesReader& other)
:itk::LightObject()
,p(new DICOMSeriesReaderImplementation)
{
  this->p = other.p;
}

mitk::DICOMSeriesReader
::~DICOMSeriesReader()
{
  delete p;
}

mitk::DICOMSeriesReader&
mitk::DICOMSeriesReader
::operator=(const mitk::DICOMSeriesReader& other)
{
  if (this != &other)
  {
    this->p = other.p;
  }

  return *this;
}

void
mitk::DICOMSeriesReader
::SetFilename( const std::string& filename )
{
  StringList list;
  list.push_back( filename );
  this->SetFilenames(list);
}

void
mitk::DICOMSeriesReader
::SetFilenames( const StringList& filenames )
{
  p->m_Filenames = filenames;
}

void
mitk::DICOMSeriesReader
::Update()
{
  p->m_Outputs.clear();

  // TODO separate series UIDs into different outputs! Don't do this here!
  DICOMSeries::Pointer emptySeries = DICOMSeries::New();
  p->m_Outputs.push_back( emptySeries );

  for (StringList::const_iterator iter = p->m_Filenames.begin();
       iter != p->m_Filenames.end();
       ++iter)
  {
    p->ProcessFile( *iter );
  }
}

unsigned int
mitk::DICOMSeriesReader
::GetNumberOfOutputs() const
{
  return p->m_Outputs.size();
}

mitk::DICOMSeries::Pointer
mitk::DICOMSeriesReader
::GetOutput(unsigned int idx) const
{
  if ( idx > p->m_Outputs.size() )
  {
    std::stringstream ss;
    ss << "Index out of range in DICOMSeriesReader::GetOutput(" << idx << ")";
    throw std::invalid_argument( ss.str() );
  }

  return p->m_Outputs[idx];
}

void
mitk::DICOMSeriesReader
::MinimalContinuingUpdate()
{
  this->Update();
  // TODO
}

void
mitk::DICOMSeriesReaderImplementation
::ProcessFile(const std::string& filename)
{
  // TODO
  DICOMImageReader::Pointer imageReader = DICOMImageReader::New();
  imageReader->SetFilename( filename );
  try
  {
    imageReader->Update();
  }
  catch (...)
  {
    // TODO error handling
  }

  for (unsigned int imageIndex = 0; imageIndex < imageReader->GetNumberOfOutputs(); ++imageIndex)
  {
    DICOMImage::Pointer image = imageReader->GetOutput(imageIndex);
    this->m_Outputs.front()->AddDICOMImage( image );
  }
}
