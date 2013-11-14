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

#ifndef mitkDICOMDatasetGDCM_h
#define mitkDICOMDatasetGDCM_h

#include "mitkDICOMDataset.h"

#include "gdcmDataSet.h"

namespace mitk
{

/**
  \brief TODO TODO TODO Describe class.

  TODO TODO TODO Describe class in more detail.

*/
class DICOMDatasetGDCM : public DICOMDataset
{
  public:

    mitkClassMacro( DICOMDatasetGDCM, DICOMDataset )
    itkNewMacro( DICOMDatasetGDCM );
    mitkCloneMacro( DICOMDatasetGDCM );

    virtual const TagList GetTags() const;
    virtual std::string GetVR(int group, int element) const;

    virtual bool GetAttributeValueAsString  (int group, int element, std::string& value) const;
    virtual bool GetAttributeValueAsStrings (int group, int element, std::list<std::string>& values) const;
    virtual bool GetAttributeValueAsDouble (int group, int element, double& value) const;
    virtual bool GetAttributeValueAsDoubles (int group, int element, std::list<double>& values) const;
    /*
    virtual bool GetAttributeValueAsInteger(int group, int element, int& value) const;
    virtual bool GetAttributeValueAsIntegers(int group, int element, std::list<int>& value) const;
    */
    virtual bool GetAttributeValueAsSequence(int group, int element, Sequence& value) const; // SQ

    void Initialize( const gdcm::DataSet& dataset );

    virtual void PrintToStdOut() const;

  protected:

    DICOMDatasetGDCM();
    virtual ~DICOMDatasetGDCM();

    DICOMDatasetGDCM(const DICOMDatasetGDCM& other);
    DICOMDatasetGDCM& operator=(const DICOMDatasetGDCM& other);


    const gdcm::VR GetVRGDCM(int group, int element) const;

  private:

    bool m_Initialized;
    gdcm::DataSet m_GDCMDataset;
};

}

#endif
