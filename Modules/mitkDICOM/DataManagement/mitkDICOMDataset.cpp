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

#include "mitkDICOMDataset.h"

class mitk::DICOMDatasetImplementation
{
  public:

};


mitk::DICOMDataset::DICOMDataset()
:LightObject()
,p(new DICOMDatasetImplementation)
{
}

mitk::DICOMDataset::DICOMDataset(const DICOMDataset& other)
:LightObject()
,p(new DICOMDatasetImplementation)
{
  this->p = other.p;
}

mitk::DICOMDataset::~DICOMDataset()
{
  delete p;
}

mitk::DICOMDataset& mitk::DICOMDataset::operator=(const mitk::DICOMDataset& other)
{
  if (this != &other)
  {
    this->p = other.p;
  }

  return *this;
}
