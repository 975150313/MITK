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

#ifndef mitkDICOMDataset_h
#define mitkDICOMDataset_h

#include "itkObjectFactory.h"
#include "mitkCommon.h"

#include "mitkDICOMExports.h"

namespace mitk
{

class DICOMDatasetImplementation;

/**
  \brief TODO TODO TODO Describe class.

  TODO TODO TODO Describe class in more detail.

*/
class mitkDICOM_EXPORT DICOMDataset : public itk::LightObject
{
  public:

    mitkClassMacro( DICOMDataset, itk::LightObject )
    itkNewMacro( DICOMDataset );
    mitkCloneMacro( DICOMDataset );

    template <typename T>
    T
    GetAttributeValue(int /*group*/, int /*element*/) const // TODO replace by DICOMTag class // TODO hide implementation as much as possible
    {
      T empty;
      throw empty; // not yet implemented
      return empty;
    }

  protected:

    DICOMDataset();
    virtual ~DICOMDataset();

    DICOMDataset(const DICOMDataset& other);
    DICOMDataset& operator=(const DICOMDataset& other);

  private:

    DICOMDatasetImplementation* p;
};

}

#endif
