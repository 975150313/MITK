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

#ifndef mitkDICOMSortByTag_h
#define mitkDICOMSortByTag_h

#include "mitkDICOMSeriesSortCriterion.h"

namespace mitk
{

class DICOMSortByTagImplementation;

/**
  \brief TODO TODO TODO Describe class.

  TODO TODO TODO Describe class in more detail.

*/
class mitkDICOM_EXPORT DICOMSortByTag : public DICOMSeriesSortCriterion
{
  public:

    mitkClassMacro( DICOMSortByTag, DICOMSeriesSortCriterion )
    itkNewMacro( DICOMSortByTag )
    mitkCloneMacro( DICOMSortByTag )

    void SetTag(int group, int element, bool ascendingOrder = true);

    virtual bool LeftImageIsLessThanRightImage( const DICOMImage* left, const DICOMImage* right ) const;

  protected:

    DICOMSortByTag();
    virtual ~DICOMSortByTag();

    DICOMSortByTag(const DICOMSortByTag& other);
    DICOMSortByTag& operator=(const DICOMSortByTag& other);

  private:

    DICOMSortByTagImplementation* p;
};

}

#endif
