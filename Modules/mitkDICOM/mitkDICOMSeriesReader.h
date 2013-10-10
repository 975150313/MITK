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

#ifndef mitkDICOMSeriesReader_h
#define mitkDICOMSeriesReader_h

#include "mitkDICOMSeries.h"

namespace mitk
{

class DICOMSeriesReaderImplementation;

/**
  \brief TODO TODO TODO Describe class.

  TODO TODO TODO Describe class in more detail.

*/
class mitkDICOM_EXPORT DICOMSeriesReader : public itk::Object
{
  public:

    struct Progress
    {
      bool complete;
      unsigned int filesLoaded;
      unsigned int filesTotal;
    };

    typedef std::vector<std::string> StringList;

    mitkClassMacro( DICOMSeriesReader, itk::LightObject )
    itkNewMacro( DICOMSeriesReader );
    mitkCloneMacro( DICOMSeriesReader );

    Progress GetProgress() const;

    void SetFilename( const std::string& filename );
    void SetFilenames( const StringList& filenames );
    void MinimalContinuingUpdate();
    void Update(bool minimalAndContinuing=false);

    unsigned int GetNumberOfOutputs() const;
    DICOMSeries::Pointer GetOutput(unsigned int idx = 0) const;

  protected:

    DICOMSeriesReader();
    virtual ~DICOMSeriesReader();

    DICOMSeriesReader(const DICOMSeriesReader& other);
    DICOMSeriesReader& operator=(const DICOMSeriesReader& other);

  private:

    DICOMSeriesReaderImplementation* p;
};

}

#endif
