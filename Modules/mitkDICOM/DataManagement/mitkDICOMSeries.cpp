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

#include "mitkSlicedGeometry3D.h"

#include <itkMutexLockHolder.h>

namespace mitk
{

template< typename TMutex >
class PriorityMutexLock : public Lockable
{
  public:

  PriorityMutexLock( bool highPriority, TMutex& baseLock, unsigned int& importantRequests, TMutex& importantRequestsLock )
  :m_IAmImportant(highPriority)
  ,m_BaseLock( baseLock )
  ,m_ImportantRequests( importantRequests )
  ,m_ImportantRequestsLock( importantRequestsLock )
  {
  }

  void Lock()
  {
    if ( m_IAmImportant )
    {
      m_ImportantRequestsLock.Lock();
      ++m_ImportantRequests;
      m_ImportantRequestsLock.Unlock();

      m_BaseLock.Lock();
    }
    else
    {
      m_ImportantRequestsLock.Lock();
      unsigned int importantWaiters = m_ImportantRequests;
      m_ImportantRequestsLock.Unlock();
      while ( importantWaiters > 0 )
      {
#ifdef WIN32
        Sleep(10); // ms
#else
        usleep(10 * 1000); // ms * 1000 = micro seconds
#endif

        // update number of waiters
        m_ImportantRequestsLock.Lock();
        importantWaiters = m_ImportantRequests;
        m_ImportantRequestsLock.Unlock();
      }

      m_BaseLock.Lock();
    }
  }

  void Unlock()
  {
    if (m_IAmImportant)
    {
      m_ImportantRequestsLock.Lock();
      --m_ImportantRequests;
      m_ImportantRequestsLock.Unlock();

      m_BaseLock.Unlock();
    }
    else
    {
      m_BaseLock.Unlock();
    }
  }

  protected:

    bool m_IAmImportant;
    TMutex& m_BaseLock;
    unsigned int& m_ImportantRequests;
    TMutex& m_ImportantRequestsLock;
};

class HighLowPriorityMutexLock
{
  public:

    typedef itk::SimpleMutexLock MutexType;

    HighLowPriorityMutexLock()
    : m_ImportantRequests(0)
    , m_HighPrioLock( true, m_BaseLock, m_ImportantRequests, m_ImportantRequestsLock )
    , m_LowPrioLock( false, m_BaseLock, m_ImportantRequests, m_ImportantRequestsLock )
    {
    }

    Lockable& GetHighPriorityMutexLock()
    {
      return m_HighPrioLock;
    }

    Lockable& GetLowPriorityMutexLock()
    {
      return m_LowPrioLock;
    }

  protected:

    MutexType m_BaseLock;
    MutexType m_ImportantRequestsLock;

    unsigned int m_ImportantRequests;

    PriorityMutexLock<MutexType> m_HighPrioLock;
    PriorityMutexLock<MutexType> m_LowPrioLock;
};

} // namespace mitk



class mitk::DICOMSeriesImplementation
{
  public:

    typedef Lockable MutexType;
    typedef itk::MutexLockHolder<Lockable> MutexLocker;

    HighLowPriorityMutexLock m_Lock;
    DICOMSeries::DICOMImageList m_DICOMImages;
    DICOMSeriesSortCriterion::ConstPointer m_SortCriterion;

    DICOMSeriesImplementation(DICOMSeries* object):m_Object(object){}

    void UpdateGeometry();
    void UpdateImageSorting();

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
  DICOMSeriesImplementation::MutexLocker lockerThis(p->m_Lock.GetLowPriorityMutexLock());
  DICOMSeriesImplementation::MutexLocker lockerOther(other.p->m_Lock.GetLowPriorityMutexLock());

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
    DICOMSeriesImplementation::MutexLocker lockerThis(p->m_Lock.GetLowPriorityMutexLock());
    DICOMSeriesImplementation::MutexLocker lockerOther(other.p->m_Lock.GetLowPriorityMutexLock());
    this->p = other.p;
  }

  return *this;
}

mitk::DICOMSeries::MutexType&
mitk::DICOMSeries
::GetHighPriorityLock() const
{
  return p->m_Lock.GetHighPriorityMutexLock();
}

mitk::DICOMSeries::MutexType&
mitk::DICOMSeries
::GetLock() const
{
  return p->m_Lock.GetLowPriorityMutexLock();
}

void
mitk::DICOMSeries
::AddDICOMImage(DICOMImage::Pointer image)
{
  {
    DICOMSeriesImplementation::MutexLocker locker(p->m_Lock.GetLowPriorityMutexLock());
    p->m_DICOMImages.push_back( image );

    p->UpdateGeometry();
  }
  this->Modified();
}

unsigned int
mitk::DICOMSeries
::GetNumberOfDICOMImages() const
{
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
  ConstDICOMImageList copy;
  copy.insert( copy.end(), p->m_DICOMImages.begin(), p->m_DICOMImages.end() );
  return copy;
}

void
mitk::DICOMSeriesImplementation
::UpdateGeometry()
{
  SlicedGeometry3D::Pointer slicedGeometry = SlicedGeometry3D::New(); // collects all slices
  slicedGeometry->SetEvenlySpaced( false );
  slicedGeometry->InitializeSlicedGeometry( m_DICOMImages.size() );
  slicedGeometry->SetImageGeometry( true ); // TODO probably true.. check alignment with regular images

  BoundingBox::BoundsArrayType bounds;
  Point3D origin; origin.Fill(0.0);
  Vector3D spacing; spacing.Fill(1.0);

  // once for the spacing which is a bit weird
  if ( !m_DICOMImages.empty() )
  {
    PlaneGeometry::Pointer imagePlane = m_DICOMImages.front()->GetImagePlane();
    spacing = imagePlane->GetSpacing();
  }

  // TODO This is extremely bad:
  //   if we don't decide on a single spacing for the whole SlicedGeometry3D,
  //   then we'll get unit spacing for the display geometries in rendering (instead of getting the spacing of the "current slice")
  slicedGeometry->SetSpacing( spacing ); // this eradicates all Geometry2Ds! clearly in the code but why does it make sense?

  // then for all other properties
  unsigned int sliceNumber = 0;
  for (DICOMSeries::DICOMImageList::iterator iter = m_DICOMImages.begin();
       iter != m_DICOMImages.end();
       ++iter, ++sliceNumber)
  {
    PlaneGeometry::Pointer imagePlane = (*iter)->GetImagePlane();
    slicedGeometry->SetGeometry2D( imagePlane, sliceNumber );

    if (sliceNumber == 0)
    {
      origin = imagePlane->GetOrigin();
      bounds = imagePlane->GetBounds();
      spacing = imagePlane->GetSpacing();
    }
    else
    {
      // bounds = ( 0 , spacingX * pixels, 0, spacingY * pixels, 0, slices )
      // TODO should we enlarge bounds? is that meaningful?

      // calculate minimum origin
      Point3D planeOrigin = imagePlane->GetOrigin();
      for (unsigned int dim = 0; dim < 3; ++dim)
        if ( planeOrigin[dim] < origin[dim] )
          origin[dim] = planeOrigin[dim];
    }
  }

  slicedGeometry->SetOrigin( origin );
  slicedGeometry->SetBounds( bounds );
  //slicedGeometry->SetSpacing( spacing ); // this eradicates all Geometry2Ds! clearly in the code but why does it make sense?

  /*
  slicedGeometry->SetDirectionVector( does not make sense );
  slicedGeometry->SetBounds( must be the hull of all 2D slices );
  slicedGeometry->SetSpacing( does not make sense );
  slicedGeometry->SetTimeBounds( must be the hull of all 2D slices ); // should default to 0 .. 1
  */

  m_Object->SetGeometry( slicedGeometry );
}

struct myLess
{
  myLess(mitk::DICOMSeriesSortCriterion::ConstPointer compare) : m_Compare(compare) {}
  mitk::DICOMSeriesSortCriterion::ConstPointer m_Compare;

  bool operator() (const mitk::DICOMImage::Pointer& left, const mitk::DICOMImage::Pointer& right)
  {
    return m_Compare->LeftImageIsLessThanRightImage(left.GetPointer(),right.GetPointer());
  }
};

void
mitk::DICOMSeriesImplementation
::UpdateImageSorting()
{
  DICOMSeriesImplementation::MutexLocker locker(this->m_Lock.GetLowPriorityMutexLock());
  if (this->m_SortCriterion.IsNotNull())
  {
    std::sort( this->m_DICOMImages.begin(), this->m_DICOMImages.end(), myLess(this->m_SortCriterion) );
  }
}

void
mitk::DICOMSeries
::SetSortCriterion(const DICOMSeriesSortCriterion::ConstPointer sorting)
{
  {
    DICOMSeriesImplementation::MutexLocker locker(p->m_Lock.GetLowPriorityMutexLock());
    p->m_SortCriterion = sorting;
  }
  p->UpdateImageSorting();
}
