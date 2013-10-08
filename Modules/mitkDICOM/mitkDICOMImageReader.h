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

#ifndef mitkDICOMImageReader_h
#define mitkDICOMImageReader_h

#include "mitkDICOMImage.h"

namespace mitk
{

class DICOMImageReaderImplementation;

/**
  \brief TODO TODO TODO Describe class.

  TODO TODO TODO Describe class in more detail.

*/
class mitkDICOM_EXPORT DICOMImageReader : public itk::LightObject
{
  public:

    mitkClassMacro( DICOMImageReader, itk::LightObject )
    itkNewMacro( DICOMImageReader );
    mitkCloneMacro( DICOMImageReader );

    void SetFilename( const std::string& filename );
    void Update();
    unsigned int GetNumberOfOutputs() const;
    DICOMImage::Pointer GetOutput(unsigned int idx = 0) const;

  protected:

    DICOMImageReader();
    virtual ~DICOMImageReader();

    DICOMImageReader(const DICOMImageReader& other);
    DICOMImageReader& operator=(const DICOMImageReader& other);

  private:

    DICOMImageReaderImplementation* p;
};

}

#endif
