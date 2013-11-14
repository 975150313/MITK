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

#ifndef mitkDICOMSeries_h
#define mitkDICOMSeries_h

#include "mitkDICOMImage.h"
#include "mitkDICOMSeriesSortCriterion.h" // implmement less(dataset1,dataset2) or (image1,image2)??

#include "mitkBaseData.h"

#include <itkMutexLockHolder.h>

namespace mitk
{

class Lockable
{
  public:
    virtual void Lock() = 0;
    virtual void Unlock() = 0;
};

class DICOMSeriesImplementation;

/**
  \brief TODO TODO TODO Describe class.

  TODO TODO TODO Describe class in more detail.

*/
class mitkDICOM_EXPORT DICOMSeries : public BaseData
{
  public:

    typedef std::vector<DICOMImage::Pointer> DICOMImageList;
    typedef std::vector<DICOMImage::ConstPointer> ConstDICOMImageList;

    typedef Lockable MutexType;
    typedef itk::MutexLockHolder<MutexType> MutexLocker;

    mitkClassMacro( DICOMSeries, BaseData )
    itkNewMacro( DICOMSeries );
    mitkCloneMacro( DICOMSeries );

    MutexType& GetHighPriorityLock() const;
    MutexType& GetLock() const;

    void AddDICOMImage( DICOMImage::Pointer image );
    unsigned int GetNumberOfDICOMImages() const;
    ConstDICOMImageList GetAllDICOMImages() const;

    // slice sorting
    void SetSortCriterion(const DICOMSeriesSortCriterion::ConstPointer sorting);

    // from BaseData
    virtual void SetRequestedRegionToLargestPossibleRegion();
    virtual bool RequestedRegionIsOutsideOfTheBufferedRegion();
    virtual bool VerifyRequestedRegion();
    virtual void SetRequestedRegion(const itk::DataObject *data);

    virtual void Print(std::ostream& os) const;

  protected:

    DICOMSeries();
    virtual ~DICOMSeries();

    DICOMSeries(const DICOMSeries& other);
    DICOMSeries& operator=(const DICOMSeries& other);

  private:

    DICOMSeriesImplementation* p;
};

}

#endif
