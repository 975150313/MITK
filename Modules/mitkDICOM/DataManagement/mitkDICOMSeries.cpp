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

class mitk::DICOMSeriesImplementation
{
  public:

};


mitk::DICOMSeries::DICOMSeries()
:LightObject()
,p(new DICOMSeriesImplementation)
{
}

mitk::DICOMSeries::DICOMSeries(const DICOMSeries& other)
:LightObject()
,p(new DICOMSeriesImplementation)
{
  this->p = other.p;
}

mitk::DICOMSeries::~DICOMSeries()
{
  delete p;
}

mitk::DICOMSeries& mitk::DICOMSeries::operator=(const mitk::DICOMSeries& other)
{
  if (this != &other)
  {
    this->p = other.p;
  }

  return *this;
}
