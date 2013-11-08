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

    DICOMDataset::Pointer m_InternalDataset;
};


mitk::DICOMDataset::DICOMDataset()
:LightObject()
,p(new DICOMDatasetImplementation)
{
}

mitk::DICOMDataset
::DICOMDataset(const DICOMDataset& other)
:LightObject()
,p(new DICOMDatasetImplementation)
{
  this->p = other.p;
}

mitk::DICOMDataset
::~DICOMDataset()
{
  delete p;
}

mitk::DICOMDataset&
mitk::DICOMDataset
::operator=(const mitk::DICOMDataset& other)
{
  if (this != &other)
  {
    this->p = other.p;
    if ( other.p->m_InternalDataset.IsNotNull() ) // do not duplicate pointers, create copies of the objects
    {
      this->p->m_InternalDataset = other.p->m_InternalDataset->Clone();
    }
  }

  return *this;
}

void
mitk::DICOMDataset
::PrintToStdOut() const
{
  std::cout << "DICOMDataset dump:" << std::endl;
  if (p->m_InternalDataset.IsNotNull())
  {
    p->m_InternalDataset->PrintToStdOut();
  }
}

void
mitk::DICOMDataset
::SetAttributes( const DICOMDataset& initializer )
{
  p->m_InternalDataset = initializer.Clone();
}

const mitk::DICOMDataset::TagList
mitk::DICOMDataset
::GetTags() const
{
  return p->m_InternalDataset.IsNotNull() ? p->m_InternalDataset->GetTags() : TagList();
}

std::string
mitk::DICOMDataset
::GetVR(int group, int element) const
{
  return p->m_InternalDataset.IsNotNull() ? p->m_InternalDataset->GetVR(group,element) : "??";
}

bool
mitk::DICOMDataset
::GetAttributeValueAsString  (int group, int element, std::string& value) const
{
  return p->m_InternalDataset.IsNotNull() ? p->m_InternalDataset->GetAttributeValueAsString(group, element, value) : false;
}

bool
mitk::DICOMDataset
::GetAttributeValueAsStrings (int group, int element, std::list<std::string>& values) const
{
  return p->m_InternalDataset.IsNotNull() ? p->m_InternalDataset->GetAttributeValueAsStrings(group, element, values) : false;
}

bool
mitk::DICOMDataset
::GetAttributeValueAsDouble (int group, int element, double& value) const
{
  return p->m_InternalDataset.IsNotNull() ? p->m_InternalDataset->GetAttributeValueAsDouble(group, element, value) : false;
}

bool
mitk::DICOMDataset
::GetAttributeValueAsDoubles (int group, int element, std::list<double>& values) const
{
  return p->m_InternalDataset.IsNotNull() ? p->m_InternalDataset->GetAttributeValueAsDoubles(group, element, values) : false;
}

bool
mitk::DICOMDataset
::GetAttributeValueAsSequence(int group, int element, Sequence& value) const
{
  return p->m_InternalDataset.IsNotNull() ? p->m_InternalDataset->GetAttributeValueAsSequence(group, element, value) : false;
}
