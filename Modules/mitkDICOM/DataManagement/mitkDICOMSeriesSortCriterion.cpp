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

#include "mitkDICOMSeriesSortCriterion.h"

mitk::DICOMSeriesSortCriterion
::DICOMSeriesSortCriterion()
:itk::LightObject()
{
}

mitk::DICOMSeriesSortCriterion
::~DICOMSeriesSortCriterion()
{
}

mitk::DICOMSeriesSortCriterion
::DICOMSeriesSortCriterion(const DICOMSeriesSortCriterion&)
:itk::LightObject()
{
}

mitk::DICOMSeriesSortCriterion&
mitk::DICOMSeriesSortCriterion
::operator=(const DICOMSeriesSortCriterion&)
{
  return *this;
}
