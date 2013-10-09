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

#include "mitkDICOMSeries.h"

#include <itkMutexLockHolder.h>

class mitk::DICOMSeriesImplementation
{
  public:

    typedef itk::SimpleMutexLock MutexType;
    typedef itk::MutexLockHolder<MutexType> MutexLocker;

    MutexType m_Lock;
    DICOMSeries::DICOMImageList m_DICOMImages;
};

mitk::DICOMSeries
::DICOMSeries()
:LightObject()
,p(new DICOMSeriesImplementation)
{
}

mitk::DICOMSeries
::DICOMSeries(const DICOMSeries& other)
:LightObject()
,p(new DICOMSeriesImplementation)
{
  DICOMSeriesImplementation::MutexLocker lockerThis(p->m_Lock);
  DICOMSeriesImplementation::MutexLocker lockerOther(other.p->m_Lock);

  this->p = other.p;
}

mitk::DICOMSeries
::~DICOMSeries()
{
  delete p;
}

mitk::DICOMSeries&
mitk::DICOMSeries
::operator=(const mitk::DICOMSeries& other)
{
  if (this != &other)
  {
    DICOMSeriesImplementation::MutexLocker lockerThis(p->m_Lock);
    DICOMSeriesImplementation::MutexLocker lockerOther(other.p->m_Lock);
    this->p = other.p;
  }

  return *this;
}

void
mitk::DICOMSeries
::AddDICOMImage(DICOMImage::Pointer image)
{
  DICOMSeriesImplementation::MutexLocker locker(p->m_Lock);
  p->m_DICOMImages.push_back( image );
}

unsigned int
mitk::DICOMSeries
::GetNumberOfDICOMImages() const
{
  DICOMSeriesImplementation::MutexLocker locker(p->m_Lock);
  return p->m_DICOMImages.size();
}
