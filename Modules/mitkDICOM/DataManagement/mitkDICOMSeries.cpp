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

    DICOMSeriesImplementation(DICOMSeries* object):m_Object(object){}

    void UpdateGeometry();

    DICOMSeries* m_Object;
};

mitk::DICOMSeries
::DICOMSeries()
:BaseData()
,p(new DICOMSeriesImplementation(this))
{
}

mitk::DICOMSeries
::DICOMSeries(const DICOMSeries& other)
:BaseData()
,p(new DICOMSeriesImplementation(this))
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

  p->UpdateGeometry();
}

unsigned int
mitk::DICOMSeries
::GetNumberOfDICOMImages() const
{
  DICOMSeriesImplementation::MutexLocker locker(p->m_Lock);
  return p->m_DICOMImages.size();
}

void
mitk::DICOMSeries
::SetRequestedRegionToLargestPossibleRegion()
{
  // later, perhaps
}

bool
mitk::DICOMSeries
::RequestedRegionIsOutsideOfTheBufferedRegion()
{
  // later, perhaps
  return false;
}

bool
mitk::DICOMSeries
::VerifyRequestedRegion()
{
  // later, perhaps
  return true;
}

void
mitk::DICOMSeries
::SetRequestedRegion(const itk::DataObject* /*data*/)
{
  // later, perhaps
}

mitk::DICOMSeries::ConstDICOMImageList
mitk::DICOMSeries
::GetAllDICOMImages() const
{
  DICOMSeriesImplementation::MutexLocker locker(p->m_Lock);

  ConstDICOMImageList copy;
  std::copy( p->m_DICOMImages.begin(), p->m_DICOMImages.end(), copy.begin() );
  return copy;
}

void
mitk::DICOMSeriesImplementation
::UpdateGeometry()
{
  MITK_INFO << "DICOMSeries::UdpateGeometry()";

  for (DICOMSeries::DICOMImageList::iterator iter = m_DICOMImages.begin();
       iter != m_DICOMImages.end();
       ++iter)
  {
    PlaneGeometry::Pointer imagePlane = (*iter)->GetImagePlane();
    MITK_INFO <<" .. 1 more plane";
    m_Object->SetGeometry( imagePlane );
    break;
  }
}
