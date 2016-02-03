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

#include "mitkFloatToString.h"

// number to string
#include <boost/lexical_cast.hpp>
#include <boost/format.hpp>

#include "mitkLogMacros.h"

std::string mitk::FloatToString(float f, unsigned int precision)
{
  return boost::lexical_cast<std::string>(f);
}

float mitk::StringToFloat(const std::string& s)
{
  return boost::lexical_cast<float>(s);
}

std::string mitk::DoubleToString(double d, unsigned int precision)
{
  return boost::lexical_cast<std::string>(d);
}

double mitk::StringToDouble(const std::string& s)
{
  return boost::lexical_cast<double>(s);
}
