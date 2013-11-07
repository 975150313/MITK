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

#include "mitkDICOMDatasetGDCM.h"

#include <gdcmAttribute.h>
#include <gdcmDict.h>
#include <gdcmDicts.h>
#include <gdcmGlobal.h>

mitk::DICOMDatasetGDCM::DICOMDatasetGDCM()
:DICOMDataset()
,m_Initialized(false)
{
}

mitk::DICOMDatasetGDCM
::DICOMDatasetGDCM(const DICOMDatasetGDCM& other)
:DICOMDataset(other)
,m_Initialized(false)
,m_GDCMDataset(other.m_GDCMDataset)
{
  // all done in DICOMDataset
}

mitk::DICOMDatasetGDCM
::~DICOMDatasetGDCM()
{
}

mitk::DICOMDatasetGDCM&
mitk::DICOMDatasetGDCM
::operator=(const mitk::DICOMDatasetGDCM& other)
{
  if (this != &other)
  {
    this->m_Initialized = other.m_Initialized;
    this->m_GDCMDataset = other.m_GDCMDataset;
  }

  return *this;
}

void
mitk::DICOMDatasetGDCM
::Initialize( const gdcm::DataSet& dataset )
{
  if ( ! this->m_Initialized )
  {
    this->m_GDCMDataset = dataset; // overwrite all
    this->m_Initialized = true;
  }
  else
  {
    throw std::logic_error("Initialized DICOMDatasetGDCM twice! This is unsupported usage.");
  }
}

const mitk::DICOMDataset::TagList
mitk::DICOMDatasetGDCM
::GetTags() const
{
  TagList return_value;

  for ( gdcm::DataSet::ConstIterator iter = this->m_GDCMDataset.Begin();
        iter != this->m_GDCMDataset.End();
        ++iter )
  {
    int group = iter->GetTag().GetGroup();
    int element = iter->GetTag().GetElement();

    return_value.push_back( std::make_pair( group, element ) );
  }

  return return_value;
}

std::string
mitk::DICOMDatasetGDCM
::GetVR(int group, int element) const
{
  // this takes the tag that was read, does not look up in a dictionary
  const gdcm::Tag tag(group, element);
  if (m_GDCMDataset.FindDataElement(tag) )
  {
    const gdcm::DataElement& dataElement = m_GDCMDataset.GetDataElement(tag);
    const gdcm::VR& vr = dataElement.GetVR();

    static const gdcm::Dict& dict = gdcm::Global::GetInstance().GetDicts().GetPublicDict();
    std::stringstream ss;
    ss << vr;
    try
    {
      std::string name = dict.GetDictEntry(tag).GetName();
      ss << ":" << name;
    }
    catch(...) { }
    return ss.str();
  }
  else
  {
    return "??";
  }
}


// TODO move to private class
std::list<std::string> split(const std::string &s, char delim) {
  std::list<std::string> elems;
  std::stringstream ss(s);
  std::string item;
  while (std::getline(ss, item, delim)) {
    elems.push_back(item);
  }
  return elems;
}


bool
mitk::DICOMDatasetGDCM
::GetAttributeValueAsString  (int group, int element, std::string& value) const
{
  const gdcm::Tag tag(group, element);
  if (m_GDCMDataset.FindDataElement(tag) )
  {
    const gdcm::DataElement& dataElement = m_GDCMDataset.GetDataElement(tag);
    const gdcm::VR& vr = dataElement.GetVR();
    std::stringstream ss;
    switch (vr)
    {
      case gdcm::VR::AE:
      case gdcm::VR::AS:
      //case gdcm::VR::AT:
      case gdcm::VR::CS:
      case gdcm::VR::DA:
      case gdcm::VR::DS:
      case gdcm::VR::DT:
      //case gdcm::VR::FL:
      //case gdcm::VR::FD:
      case gdcm::VR::IS:
      case gdcm::VR::LO:
      case gdcm::VR::LT:
      //case gdcm::VR::OB: ?
      //case gdcm::VR::OF: ?
      //case gdcm::VR::OW: ?
      case gdcm::VR::PN:
      case gdcm::VR::SH:
      //case gdcm::VR::SL:
      //case gdcm::VR::SQ:
      //case gdcm::VR::SS:
      case gdcm::VR::ST:
      case gdcm::VR::TM:
      case gdcm::VR::UI:
      //case gdcm::VR::UL:
      //case gdcm::VR::US:
      case gdcm::VR::UL:
        {
          dataElement.GetValue().Print(ss);
          value = ss.str();
          break;
        }
      default:
        value = "";
        return false; // this is error handling
    }

    return true; // optimistic assumption
  }
  else
  {
    value = "";
    return false;
  }
}

bool
mitk::DICOMDatasetGDCM
::GetAttributeValueAsStrings (int group, int element, std::list<std::string>& result_values) const
{
  result_values.clear();
  const gdcm::Tag tag(group, element);
  if (m_GDCMDataset.FindDataElement(tag) )
  {
    const gdcm::DataElement& dataElement = m_GDCMDataset.GetDataElement(tag);
    const gdcm::VR& vr = dataElement.GetVR();
    std::stringstream ss;
    switch (vr)
    {
      case gdcm::VR::AE:
      case gdcm::VR::AS:
      //case gdcm::VR::AT:
      case gdcm::VR::CS:
      case gdcm::VR::DA:
      case gdcm::VR::DS:
      case gdcm::VR::DT:
      //case gdcm::VR::FL:
      //case gdcm::VR::FD:
      case gdcm::VR::IS:
      case gdcm::VR::LO:
      case gdcm::VR::LT:
      //case gdcm::VR::OB: ?
      //case gdcm::VR::OF: ?
      //case gdcm::VR::OW: ?
      case gdcm::VR::PN:
      case gdcm::VR::SH:
      //case gdcm::VR::SL:
      //case gdcm::VR::SQ:
      //case gdcm::VR::SS:
      case gdcm::VR::ST:
      case gdcm::VR::TM:
      case gdcm::VR::UI:
      //case gdcm::VR::UL:
      //case gdcm::VR::US:
      case gdcm::VR::UL:
        {
          dataElement.GetValue().Print(ss);
          result_values = split(ss.str(), '\\');
          break;
        }
      default:
        return false; // this is error handling
    }

    return true; // optimistic assumption
  }
  else
  {
    return false;
  }
}

bool
mitk::DICOMDatasetGDCM
::GetAttributeValueAsDouble (int group, int element, double& result_value) const
{
  std::list<double> result_values;
  bool returnValue = this->GetAttributeValueAsDoubles (group, element, result_values);
  if (returnValue && !result_values.empty())
  {
    result_value = result_values.front();
  }
  else
  {
    result_value = std::numeric_limits<double>::quiet_NaN();
  }
  return returnValue;
}

bool
mitk::DICOMDatasetGDCM
::GetAttributeValueAsDoubles (int group, int element, std::list<double>& result_values) const
{
  //if (m_Initialized)
  result_values.clear();

  const gdcm::Tag tag(group, element);
  if (m_GDCMDataset.FindDataElement(tag) )
  {
    const gdcm::DataElement& dataElement = m_GDCMDataset.GetDataElement(tag);
    const gdcm::VR& vr = dataElement.GetVR();
    std::stringstream ss;
    switch (vr)
    {
      case gdcm::VR::DS:
      case gdcm::VR::IS:
        {
          dataElement.GetValue().Print(ss);
          std::list<std::string> numberstrings = split(ss.str(), '\\');
          for (std::list<std::string>::iterator sIter = numberstrings.begin();
              sIter != numberstrings.end();
              ++sIter)
          {
            double d = ::atof( sIter->c_str() );
            // TODO error handling?
            result_values.push_back(d);
          }
          break;
        }
      default:
        return false; // this is error handling
    }

    return true; // optimistic assumption
  }

  return false;
}

void
mitk::DICOMDatasetGDCM
::PrintToStdOut() const
{
  this->m_GDCMDataset.Print(std::cout);
}
