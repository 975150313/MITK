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

#include <itkMultiThreader.h>
#include <itkMutexLockHolder.h>
#include <itkConditionVariable.h>

class mitk::DICOMSeriesReaderImplementation
{
  public:

    DICOMSeriesReaderImplementation(DICOMSeriesReader* object):m_Object(object){}

    DICOMSeriesReader::StringList m_Filenames;
    std::vector<DICOMSeries::Pointer> m_Outputs;

    itk::MultiThreader::Pointer m_Threader;
    DICOMSeriesReader::StringList m_InputsToProcess;
    typedef itk::SimpleMutexLock MutexType;
    typedef itk::MutexLockHolder<MutexType> MutexLocker;

    std::list<int> m_ThreadIDs;

    void ProcessFile( const std::string& filename );
    std::string GetNextFileToProcess();

    MutexType m_Lock;
    MutexType m_WaitingLock;
    itk::ConditionVariable::Pointer m_FirstResultsAvailable;

    DICOMSeriesReader::Progress m_Progress;
    DICOMSeriesReader* m_Object;

    static ITK_THREAD_RETURN_TYPE FileLoader(void*);
};


mitk::DICOMSeriesReader
::DICOMSeriesReader()
:itk::Object()
,p(new DICOMSeriesReaderImplementation(this))
{
}

mitk::DICOMSeriesReader
::DICOMSeriesReader(const DICOMSeriesReader& other)
:itk::Object()
,p(new DICOMSeriesReaderImplementation(this))
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
::MinimalContinuingUpdate()
{
  this->Update(true);
}

std::string
mitk::DICOMSeriesReaderImplementation
::GetNextFileToProcess()
{
  MutexLocker lock(this->m_Lock);

  if (this->m_InputsToProcess.empty())
  {
    return std::string("");
  }

  std::string nextFileToProcess = this->m_InputsToProcess.back();
  this->m_InputsToProcess.pop_back();
  return nextFileToProcess;
}

void
mitk::DICOMSeriesReaderImplementation
::ProcessFile(const std::string& filename)
{
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
    {
      MutexLocker lock(this->m_Lock);
      this->m_Outputs.front()->AddDICOMImage( image );
      this->m_Progress.filesLoaded += 1;
      if (this->m_Progress.filesLoaded == this->m_Progress.filesTotal)
      {
        this->m_Progress.complete = true;
      }
      //MITK_INFO << "Read file " << filename;

      this->m_FirstResultsAvailable->Broadcast();
      //MITK_INFO << "InvokeEvent (itk::ProgressEvent() )";
      this->m_Object->InvokeEvent( itk::ProgressEvent() );
    }
  }
}

ITK_THREAD_RETURN_TYPE
mitk::DICOMSeriesReaderImplementation
::FileLoader(void* infoIn)
{
  // run in a thread, meant to load files from an input list
  // as long as there are more inputs. terminates after the last input.

  itk::MultiThreader::ThreadInfoStruct* info = static_cast<itk::MultiThreader::ThreadInfoStruct*>(infoIn);
  //itk::ThreadIdType tnum = info->ThreadID;
  DICOMSeriesReaderImplementation* reader = static_cast<DICOMSeriesReaderImplementation*>(info->UserData);

  std::string filename;
  while ( !(filename = reader->GetNextFileToProcess() ).empty() )
  {
    reader->ProcessFile( filename );
  }
#ifndef WIN32
  return 0; // removes warning about returning nothing from here
#endif
}


void
mitk::DICOMSeriesReader
::Update(bool minimalAndContinuing)
{
  p->m_Outputs.clear();
  p->m_ThreadIDs.clear();

  p->m_InputsToProcess = p->m_Filenames;
  p->m_FirstResultsAvailable = itk::ConditionVariable::New();

  // TODO separate series UIDs into different outputs! Don't do this here!
  DICOMSeries::Pointer emptySeries = DICOMSeries::New();
  p->m_Outputs.push_back( emptySeries );

  p->m_Progress.complete = false;
  p->m_Progress.filesLoaded = 0;
  p->m_Progress.filesTotal = p->m_InputsToProcess.size();

  // TODO: check that no old loading thread is still running (or that it can continue safely!)
  p->m_Threader = itk::MultiThreader::New();
  unsigned int numberOfThreads(5); // TODO check how much is good for I/O
  for (unsigned int threadIdx = 0; threadIdx < numberOfThreads; ++threadIdx)
  {
    int threadID = p->m_Threader->SpawnThread( mitk::DICOMSeriesReaderImplementation::FileLoader, p );
    p->m_ThreadIDs.push_back(threadID);
  }

  if (minimalAndContinuing)
  {
    // wait just for some first result
    p->m_FirstResultsAvailable->Wait( &p->m_WaitingLock );
  }
  else
  {
    // cleaning, this would also wait but perhaps a signal becomes handy
    for (std::list<int>::const_iterator threadIter = p->m_ThreadIDs.begin();
        threadIter != p->m_ThreadIDs.end();
        ++threadIter)
    {
      // wait actually
      p->m_Threader->TerminateThread( *threadIter );
    }
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

mitk::DICOMSeriesReader::Progress
mitk::DICOMSeriesReader
::GetProgress() const
{
  DICOMSeriesReaderImplementation::MutexLocker lock(p->m_Lock);
  return p->m_Progress;
}
