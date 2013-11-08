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

#ifndef mitkDICOMSeriesSortCriterion_h
#define mitkDICOMSeriesSortCriterion_h

#include "mitkDICOMImage.h"

namespace mitk
{

/**
  \brief TODO TODO TODO Describe class.

  TODO TODO TODO Describe class in more detail.

*/
class mitkDICOM_EXPORT DICOMSeriesSortCriterion : public itk::LightObject
{
  public:

    mitkClassMacro( DICOMSeriesSortCriterion, itk::LightObject )

    virtual bool LeftImageIsLessThanRightImage( const DICOMImage* left, const DICOMImage* right ) const = 0; // for specific criteria to implement

  protected:

    DICOMSeriesSortCriterion();
    virtual ~DICOMSeriesSortCriterion();

    DICOMSeriesSortCriterion(const DICOMSeriesSortCriterion& other);
    DICOMSeriesSortCriterion& operator=(const DICOMSeriesSortCriterion& other);
};

}

#endif
