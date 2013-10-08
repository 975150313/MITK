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

#ifndef mitkDICOMSeries_h
#define mitkDICOMSeries_h

#include "mitkDICOMImage.h"

namespace mitk
{

class DICOMSeriesImplementation;

/**
  \brief TODO TODO TODO Describe class.

  TODO TODO TODO Describe class in more detail.

*/
class mitkDICOM_EXPORT DICOMSeries : public itk::LightObject // TODO BaseData
{
  public:

    mitkClassMacro( DICOMSeries, itk::LightObject )
    itkNewMacro( DICOMSeries );
    mitkCloneMacro( DICOMSeries );

  protected:

    DICOMSeries();
    virtual ~DICOMSeries();

    DICOMSeries(const DICOMSeries& other);
    DICOMSeries& operator=(const DICOMSeries& other);

  private:

    DICOMSeriesImplementation* p;
};

}

#endif
