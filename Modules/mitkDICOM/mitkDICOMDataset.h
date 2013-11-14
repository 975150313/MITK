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

    typedef std::vector<std::pair<int,int> > TagList;
    typedef std::vector<DICOMDataset::Pointer> Sequence;

    virtual const TagList GetTags() const;
    virtual std::string GetVR(int group, int element) const;

    virtual bool GetAttributeValueAsString  (int group, int element, std::string& value) const;
    virtual bool GetAttributeValueAsStrings (int group, int element, std::list<std::string>& values) const;
    virtual bool GetAttributeValueAsDouble (int group, int element, double& value) const;
    virtual bool GetAttributeValueAsDoubles (int group, int element, std::list<double>& values) const;
    /*
    virtual bool GetAttributeValueAsInteger(int group, int element, int& value) const;
    virtual bool GetAttributeValueAsIntegers(int group, int element, std::list<int>& values) const;
    */
    virtual bool GetAttributeValueAsSequence(int group, int element, Sequence& value) const;

    void SetAttributes( const DICOMDataset& initializer );

    virtual void PrintToStdOut() const;

  protected:

    DICOMDataset();
    virtual ~DICOMDataset();

    DICOMDataset(const DICOMDataset& other);
    DICOMDataset& operator=(const DICOMDataset& other);

    static void DumpDataset( const DICOMDataset* dataset, const std::string& indent = "" );

  private:

    DICOMDatasetImplementation* p;
};

}

#endif
