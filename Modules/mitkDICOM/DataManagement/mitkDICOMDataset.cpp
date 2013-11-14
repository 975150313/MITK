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

#include <iostream>
#include <iomanip>

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
    DumpDataset( p->m_InternalDataset );
  }
}

template< typename T >
std::string int_to_hex( T i )
{
  std::stringstream stream;
  stream //<< "0x"
    << std::setfill ('0') << std::setw(4) << std::hex << i;
  return stream.str();
}

void
mitk::DICOMDataset
::DumpDataset( const mitk::DICOMDataset* dataset, const std::string& indent )
{
  //dataset->PrintToStdOut(); // GDCM dump
  assert(dataset);

  mitk::DICOMDataset::TagList tags = dataset->GetTags();
  for (mitk::DICOMDataset::TagList::const_iterator tagIter = tags.begin();
       tagIter != tags.end();
       ++tagIter)
  {
    int gi = tagIter->first;
    int ei = tagIter->second;
    std::string gs = int_to_hex( gi );
    std::string es = int_to_hex( ei );
    std::string vrs = dataset->GetVR(gi,ei);
    std::string stringValue;
    double doubleValue;
    bool canBeString = dataset->GetAttributeValueAsString(gi,ei,stringValue);
    bool canBeDouble = dataset->GetAttributeValueAsDouble(gi,ei,doubleValue);
    mitk::DICOMDataset::Sequence sequenceValue;
    bool isSequence = dataset->GetAttributeValueAsSequence(gi,ei,sequenceValue);
    std::cout << indent;
    std::cout << "(" << gs << "," << es << "): ";
    std::cout << "[" << vrs << "]: ";
    if (isSequence)
    {
      std::cout << " sequence:" << std::endl;
      unsigned int itemCount = 1;
      for (mitk::DICOMDataset::Sequence::const_iterator seqIter = sequenceValue.begin();
           seqIter != sequenceValue.end();
           ++seqIter, ++itemCount)
      {
        std::cout << indent << "  " << "Item " << itemCount << ":" << std::endl;
        DumpDataset( *seqIter, indent + "  " ); // recurse
      }
    }
    else
    {
      if (canBeDouble)
      {
        std::cout << doubleValue << std::endl;
      }
      else if (canBeString)
      {
        std::cout << "'" << stringValue << "'" << std::endl;
      }
      else
      {
        std::cout << "<Unknown attribute type>" << std::endl;
      }
    }
  }

  if (indent.empty()) // this is for debugging only!
  {
    std::string imageType;
    if ( dataset->GetAttributeValueAsString(0x008, 0x008,imageType) )
    {
      std::cout << "Image Type: " << imageType << std::endl;
    }
    else
    {
      std::cout << "Image Type: <not readable>" << std::endl;
    }

    std::list<std::string> imageTypeComponents;
    if ( dataset->GetAttributeValueAsStrings(0x008, 0x008,imageTypeComponents) )
    {
      std::cout << "Image Type [array]: ";
      for ( std::list<std::string>::const_iterator i = imageTypeComponents.begin();
          i != imageTypeComponents.end();
          ++i )
      {
        std::cout << *i << " ";
      }
      std::cout << std::endl;
    }
    else
    {
      std::cout << "Image Type [array]: <not readable>" << std::endl;
    }

    double imageOrientationFirst;
    if ( dataset->GetAttributeValueAsDouble(0x020, 0x037,imageOrientationFirst) )
    {
      std::cout << "Image Orientation [first]: " << imageOrientationFirst << std::endl;
    }
    else
    {
      std::cout << "Image Type [first]: <not readable>" << std::endl;
    }


    std::list<double> imageOrientationPatient;
    if ( dataset->GetAttributeValueAsDoubles(0x020, 0x037,imageOrientationPatient) )
    {
      std::cout << "Image Orientation (Patient) [array]: ";
      for ( std::list<double>::const_iterator dimIter = imageOrientationPatient.begin();
          dimIter != imageOrientationPatient.end();
          ++dimIter )
      {
        std::cout << *dimIter << " ";
      }
      std::cout << std::endl;
    }
    else
    {
      std::cout << "Image Orientation (Patient): <not readable>" << std::endl;
    }
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
