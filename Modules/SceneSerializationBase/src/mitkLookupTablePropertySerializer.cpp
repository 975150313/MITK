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

#include "mitkLookupTablePropertySerializer.h"
#include <mitkLookupTableProperty.h>

#include "mitkFloatToString.h"

TiXmlElement* mitk::LookupTablePropertySerializer::Serialize()
{
  if (const LookupTableProperty* prop = dynamic_cast<const LookupTableProperty*>(m_Property.GetPointer()))
  {
    LookupTable::Pointer mitkLut = const_cast<LookupTableProperty*>(prop)->GetLookupTable();
    if (mitkLut.IsNull()) return nullptr; // really?

    vtkLookupTable* lut = mitkLut->GetVtkLookupTable();
    if (!lut) return nullptr;

    auto  element = new TiXmlElement("LookupTable");

    double*  range;
    double*  rgba;

    element->SetAttribute("NumberOfColors", lut->GetNumberOfTableValues());
    element->SetAttribute("Scale", lut->GetScale());
    element->SetAttribute("Ramp", lut->GetRamp());

    range = lut->GetHueRange();
    auto  child = new TiXmlElement("HueRange");
    element->LinkEndChild( child );
    child->SetAttribute("min", DoubleToString(range[0]));
    child->SetAttribute("max", DoubleToString(range[1]));

    range = lut->GetValueRange();
    child = new TiXmlElement("ValueRange");
    element->LinkEndChild( child );
    child->SetAttribute("min", DoubleToString(range[0]));
    child->SetAttribute("max", DoubleToString(range[1]));

    range = lut->GetSaturationRange();
    child = new TiXmlElement("SaturationRange");
    element->LinkEndChild( child );
    child->SetAttribute("min", DoubleToString(range[0]));
    child->SetAttribute("max", DoubleToString(range[1]));

    range = lut->GetAlphaRange();
    child = new TiXmlElement("AlphaRange");
    element->LinkEndChild( child );
    child->SetAttribute("min", DoubleToString(range[0]));
    child->SetAttribute("max", DoubleToString(range[1]));

    range = lut->GetTableRange();
    child = new TiXmlElement("TableRange");
    element->LinkEndChild( child );
    child->SetAttribute("min", DoubleToString(range[0]));
    child->SetAttribute("max", DoubleToString(range[1]));

    child = new TiXmlElement("Table");
    element->LinkEndChild( child );
    for ( int index = 0; index < lut->GetNumberOfTableValues(); ++index)
    {
      auto  grandChild = new TiXmlElement("RgbaColor");
      rgba = lut->GetTableValue(index);
      grandChild->SetAttribute("R", DoubleToString(rgba[0]));
      grandChild->SetAttribute("G", DoubleToString(rgba[1]));
      grandChild->SetAttribute("B", DoubleToString(rgba[2]));
      grandChild->SetAttribute("A", DoubleToString(rgba[3]));
      child->LinkEndChild( grandChild );
    }
    return element;
  }
  else return nullptr;
}

mitk::BaseProperty::Pointer mitk::LookupTablePropertySerializer::Deserialize(TiXmlElement* element)
{
  if (!element) return nullptr;

  double range[2];
  double  rgba[4];

  std::string double_strings[4];

  vtkSmartPointer<vtkLookupTable> lut = vtkSmartPointer<vtkLookupTable>::New();

  int numberOfColors;
  int scale;
  int ramp; // hope the int values don't change betw. vtk versions...
  if ( element->QueryIntAttribute( "NumberOfColors", &numberOfColors ) == TIXML_SUCCESS )
  {
    lut->SetNumberOfTableValues( numberOfColors );
  }
  else
    return nullptr;
  if ( element->QueryIntAttribute( "Scale", &scale ) == TIXML_SUCCESS )
  {
    lut->SetScale( scale );
  }
  else
    return nullptr;
  if ( element->QueryIntAttribute( "Ramp", &ramp ) == TIXML_SUCCESS )
  {
    lut->SetRamp( ramp );
  }
  else
    return nullptr;

  TiXmlElement* child = element->FirstChildElement("HueRange");
  if (child)
  {
    if ( child->QueryStringAttribute( "min", &double_strings[0] ) != TIXML_SUCCESS ) return nullptr;
    if ( child->QueryStringAttribute( "max", &double_strings[1] ) != TIXML_SUCCESS ) return nullptr;
    StringsToDoubles(2, double_strings, range);
    lut->SetHueRange( range );
  }

  child = element->FirstChildElement("ValueRange");
  if (child)
  {
    if ( child->QueryStringAttribute( "min", &double_strings[0] ) != TIXML_SUCCESS ) return nullptr;
    if ( child->QueryStringAttribute( "max", &double_strings[1] ) != TIXML_SUCCESS ) return nullptr;
    StringsToDoubles(2, double_strings, range);
    lut->SetValueRange( range );
  }

  child = element->FirstChildElement("SaturationRange");
  if (child)
  {
    if ( child->QueryStringAttribute( "min", &double_strings[0] ) != TIXML_SUCCESS ) return nullptr;
    if ( child->QueryStringAttribute( "max", &double_strings[1] ) != TIXML_SUCCESS ) return nullptr;
    StringsToDoubles(2, double_strings, range);
    lut->SetSaturationRange( range );
  }

  child = element->FirstChildElement("AlphaRange");
  if (child)
  {
    if ( child->QueryStringAttribute( "min", &double_strings[0] ) != TIXML_SUCCESS ) return nullptr;
    if ( child->QueryStringAttribute( "max", &double_strings[1] ) != TIXML_SUCCESS ) return nullptr;
    StringsToDoubles(2, double_strings, range);
    lut->SetAlphaRange( range );
  }

  child = element->FirstChildElement("TableRange");
  if (child)
  {
    if ( child->QueryStringAttribute( "min", &double_strings[0] ) != TIXML_SUCCESS ) return nullptr;
    if ( child->QueryStringAttribute( "max", &double_strings[1] ) != TIXML_SUCCESS ) return nullptr;
    StringsToDoubles(2, double_strings, range);
    lut->SetTableRange( range );
  }

  child = element->FirstChildElement("Table");
  if (child)
  {
    unsigned int index(0);
    for( TiXmlElement* grandChild = child->FirstChildElement("RgbaColor"); grandChild; grandChild = grandChild->NextSiblingElement("RgbaColor"))
    {
      if ( grandChild->QueryStringAttribute("R", &double_strings[0]) != TIXML_SUCCESS ) return nullptr;
      if ( grandChild->QueryStringAttribute("G", &double_strings[1]) != TIXML_SUCCESS ) return nullptr;
      if ( grandChild->QueryStringAttribute("B", &double_strings[2]) != TIXML_SUCCESS ) return nullptr;
      if ( grandChild->QueryStringAttribute("A", &double_strings[3]) != TIXML_SUCCESS ) return nullptr;
      StringsToDoubles(4, double_strings, rgba);
      lut->SetTableValue( index, rgba );
      ++index;
    }
  }

  LookupTable::Pointer mitkLut = LookupTable::New();
  mitkLut->SetVtkLookupTable( lut );

  return LookupTableProperty::New(mitkLut).GetPointer();
}
