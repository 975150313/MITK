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
#include <gdcmSequenceOfItems.h>

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
  gdcm::VR vr = this->GetVRGDCM(group,element);
  if (vr != gdcm::VR::INVALID)
  {
    try
    {
      static const gdcm::Dict& dict = gdcm::Global::GetInstance().GetDicts().GetPublicDict();
      std::string name = dict.GetDictEntry( gdcm::Tag(group,element) ).GetName();

      std::stringstream ss;
      ss << name << " - " << vr;
      return ss.str();
    }
    catch(...) { }
  }

  return "??";
}

const gdcm::VR
mitk::DICOMDatasetGDCM
::GetVRGDCM(int group, int element) const
{
  const gdcm::Tag tag(group, element);
  try
  {
    gdcm::VR vr = gdcm::VR::INVALID;
    if (m_GDCMDataset.FindDataElement(tag) )
    {
      // first try to read explicitly from read dataset
      const gdcm::DataElement& dataElement = m_GDCMDataset.GetDataElement(tag);
      vr = dataElement.GetVR();
    }

    if (vr == gdcm::VR::INVALID)
    {
      // if dataset does not tell, then we check the dictionary
      static const gdcm::Dict& dict = gdcm::Global::GetInstance().GetDicts().GetPublicDict();
      vr = dict.GetDictEntry(tag).GetVR();
    }
    return vr;
  }
  catch(...)
  {
    return gdcm::VR::INVALID;
  }
}

// TODO move helper functions somewhere better

// from Stack Overflow: http://stackoverflow.com/questions/216823/whats-the-best-way-to-trim-stdstring
// trim from start
static inline std::string &ltrim(std::string &s) {
  s.erase(s.begin(), std::find_if(s.begin(), s.end(), std::not1(std::ptr_fun<int, int>(std::isspace))));
  return s;
}

// trim from end
static inline std::string &rtrim(std::string &s) {
  s.erase(std::find_if(s.rbegin(), s.rend(), std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
  return s;
}

// trim from both ends
static inline std::string &trim(std::string &s) {
  return ltrim(rtrim(s));
}

// from Stack Overflow: http://stackoverflow.com/questions/236129/how-to-split-a-string-in-c
std::list<std::string> split(const std::string &s, char delim,bool removeWhitespace) {
  std::list<std::string> elems;
  std::stringstream ss(s);
  std::string item;
  while (std::getline(ss, item, delim)) {
    if (removeWhitespace)
    {
      item = trim(item);
    }
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
    if (dataElement.IsEmpty())
    {
      value = "";
      return true; // there WAS something, but empty!
    }

    const gdcm::VR& vr = this->GetVRGDCM(group,element);

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
    if (dataElement.IsEmpty())
    {
      return true; // there WAS something, but empty!
    }

    const gdcm::VR& vr = this->GetVRGDCM(group,element);

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
          result_values = split(ss.str(), '\\', false);
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
    if (dataElement.IsEmpty())
    {
      return true; // there WAS something, but empty!
    }

    const gdcm::VR& vr = this->GetVRGDCM(group,element);

    std::stringstream ss;
    switch (vr)
    {
      case gdcm::VR::DS:
      case gdcm::VR::IS:
        {
          dataElement.GetValue().Print(ss);
          std::list<std::string> numberstrings = split(ss.str(), '\\', true);
          for (std::list<std::string>::iterator sIter = numberstrings.begin();
              sIter != numberstrings.end();
              ++sIter)
          {
            char* e((char*)1);
            double d = std::strtod( sIter->c_str(), &e );
            if (*e != 0)
            {
              // error
              return false;
            }
            else
            {
              // TODO error handling?
              result_values.push_back(d);
            }
          }
          return true; // could convert
        }
      default:
        return false; // this is error handling
    }
  }

  return false;
}

bool
mitk::DICOMDatasetGDCM
::GetAttributeValueAsSequence(int group, int element, Sequence& return_value) const
{
  const gdcm::Tag tag(group, element);
  if (m_GDCMDataset.FindDataElement(tag) )
  {
    const gdcm::VR& vr = this->GetVRGDCM(group,element);

    std::stringstream ss;
    switch (vr)
    {
      case gdcm::VR::SQ:
        {
          const gdcm::DataElement& dataElement = m_GDCMDataset.GetDataElement(tag);
          gdcm::SmartPointer<gdcm::SequenceOfItems> sequence = dataElement.GetValueAsSQ();
          if (sequence)
          {
            for (gdcm::SequenceOfItems::ItemVector::const_iterator seqIter = sequence->Items.begin();
                 seqIter != sequence->Items.end();
                 ++seqIter)
            {
              const gdcm::DataSet& currentDataSet = seqIter->GetNestedDataSet();
              DICOMDatasetGDCM::Pointer mitkWrapperSet = DICOMDatasetGDCM::New();
              mitkWrapperSet->Initialize( currentDataSet );
              return_value.push_back( mitkWrapperSet.GetPointer() );
            }
          }
          break;
        }
      default:
        return false;
    }
    return true;
  }
  return false;
}

// TODO to be deleted or re-written w/o direct use of GDCM
void
mitk::DICOMDatasetGDCM
::PrintToStdOut() const
{
  this->m_GDCMDataset.Print(std::cout);
}
