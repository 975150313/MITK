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

#include "mitkDICOMSortByTag.h"

#include "mitkDICOMImage.h"

class mitk::DICOMSortByTagImplementation
{
  public:

    bool m_TagDefined;
    int m_Group;
    int m_Element;
    bool m_AscendingOrder;

    DICOMSortByTagImplementation()
    :m_TagDefined(false)
    ,m_Group(0)
    ,m_Element(0)
    ,m_AscendingOrder(true)
    {
    }
};


mitk::DICOMSortByTag::DICOMSortByTag()
:DICOMSeriesSortCriterion()
,p(new DICOMSortByTagImplementation)
{
}

mitk::DICOMSortByTag
::DICOMSortByTag(const DICOMSortByTag& other)
:DICOMSeriesSortCriterion(other)
,p(new DICOMSortByTagImplementation)
{
  this->p = other.p;
}

mitk::DICOMSortByTag
::~DICOMSortByTag()
{
  delete p;
}

mitk::DICOMSortByTag&
mitk::DICOMSortByTag
::operator=(const mitk::DICOMSortByTag& other)
{
  if (this != &other)
  {
    this->p = other.p;
  }

  return *this;
}

void
mitk::DICOMSortByTag
::SetTag(int group, int element, bool ascendingOrder)
{
  p->m_Group = group;
  p->m_Element = element;
  p->m_AscendingOrder = ascendingOrder;
  p->m_TagDefined = true;
}

bool
mitk::DICOMSortByTag
::LeftImageIsLessThanRightImage( const DICOMImage* left, const DICOMImage* right ) const
{
  if (!p->m_TagDefined)
  {
    throw std::logic_error("Uninitialized DICOMSortByTag class used for sorting.");
  }
  else
  {
    assert(left);
    assert(right);

    // if possible, compare numerical values by value

    static double doubleLeft(0.0);
    static double doubleRight(0.0);

    static bool leftIsDouble(false);
    static bool rightIsDouble(false);

    leftIsDouble =   left->GetAttributeValueAsDouble( p->m_Group, p->m_Element, doubleLeft );
    rightIsDouble = right->GetAttributeValueAsDouble( p->m_Group, p->m_Element, doubleRight );

    if (leftIsDouble && rightIsDouble)
    {
      MITK_INFO << "Compare double " << doubleLeft << " against " << doubleRight;
      return p->m_AscendingOrder ? doubleLeft < doubleRight : doubleRight < doubleLeft;
    }
    else
    {
      // else, fall back to comparison by strings
      static std::string stringLeft;
      static std::string stringRight;

      static bool leftIsString(false);
      static bool rightIsString(false);

      leftIsString =   left->GetAttributeValueAsString( p->m_Group, p->m_Element, stringLeft );
      rightIsString = right->GetAttributeValueAsString( p->m_Group, p->m_Element, stringRight );

      if (leftIsString && rightIsString)
      {
        MITK_INFO << "Compare string " << stringLeft << " against " << stringRight;
        // TODO! return p->m_AscendingOrder ? stringLeft < doubleRight : doubleRight < doubleLeft;
        return p->m_AscendingOrder ? left < right : right < left;
      }
      else
      {
        // fall back to comparing memory addresses (not meaningful but consistent)
        MITK_INFO << "Compare memory addresses " << left << " against " << right;
        return p->m_AscendingOrder ? left < right : right < left;
      }
    }
  }
}
