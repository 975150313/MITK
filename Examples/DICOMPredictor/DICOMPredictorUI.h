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

#ifndef DICOMPredictorUI_h
#define DICOMPredictorUI_h

#include <mitkDicomSeriesReader.h>
#include <QtGui>

typedef std::map<std::string, std::list<mitk::DicomSeriesReader::ImageBlockDescriptor> > DicomSeriesDescriptor;

namespace Ui
{
  class DICOMPredictorUI;
}

/**
  \brief Reports the scanning results of DicomSeriesReader.

  This GUI offers the user to
   1. Select one or more directories from his local file system
   2. Filter files in these directories based on rules for DICOM tags (0008,0008) Image Type and (0008,0060) Modality
   3. Analyze filtered files regarding how many blocks (mitk::Images) would be created by DicomSeriesReader

  The result list reports of each analyzed SERIES within a directory:
   - Number of blocks that would be created/loaded
   - Number of files in those blocks
   - Series Description
   - SOP class (e.g. CT Image Storage or PET Image Storage)
   - Directory of the files analyzed
   - Series Instance UID

  Analysis is fixed to the assumption that DicomSeriesReader is asked to scan with the following options:
   - Create separate blocks for slices at the same position (3D+t = false)
   - Also form blocks of images from gantry tilt acquisitions (gantryTilt = true)

  Filtering of files works in three steps:
   - check modality against a positive regular expression
   - check image type against accept-reject rules, which are applied in order:
     1. if an accept rule matches, the file is immediately accepted, no further rules are evaluated
     2. if a reject rule matches, the file is immediately rejected
   - if none of the above brought a descision, accept

*/
class DICOMPredictorUI : public QWidget
{
  Q_OBJECT

  public:

    DICOMPredictorUI( QWidget* parent = 0, Qt::WindowFlags f = 0 );
    ~DICOMPredictorUI();

  public slots:

    void AnalyzeSelectedDirectories();
    void AnalyzeDirectory( const QString& path );
    void AnalyzeDirectory( const QStringList& paths );

    void OutputSeriesGroupingResultsToList( const QString& path, DicomSeriesDescriptor seriesGrouping, bool& multiBlocksPresent );

    void RestoreSavedPresets();
    void SavePresets();
    void UpdateFilterPatternsFromGUI();

    void ResetFiltersToDefault();

  protected:

    QSettings* CreateConfig();

    mitk::DicomSeriesReader::StringContainer ConvertQStringListToDCMReaderInput(const QStringList& filenameList);
    bool AcceptableByByModalityAndImageType( const QString& modalityTagValue, const QString& imageTypeTagValue );
    bool MatchesRegularExpression( const QRegExp& expression, const QString& str);
    QStringList FilterFilesByModalityAndImageType(const QStringList& filenames);
    QStringList FindAllFilesIn(const QStringList& listOfFilesAndDirectories, const QString& directoryname = "", bool recurse = true );
    DicomSeriesDescriptor GroupSortingResultsBySeries( mitk::DicomSeriesReader::FileNamesGrouping imageBlockGrouping );
    std::string ConcatStringList( std::set<std::string> stringList );

    Ui::DICOMPredictorUI* m_GUI;

    QFileSystemModel m_DirModel;
    QSortFilterProxyModel m_ProxyModel;

    QSettings* m_Config;

    QRegExp m_ValidModalities;

    QRegExp m_AcceptedImageTypes;
    QRegExp m_AcceptedImageTypePixelData;
    QRegExp m_AcceptedImageTypeExamination;
    QRegExp m_AcceptedImageTypeModality;
    QRegExp m_AcceptedImageTypeOther;

    QRegExp m_RejectedImageTypes;
    QRegExp m_RejectedImageTypePixelData;
    QRegExp m_RejectedImageTypeExamination;
    QRegExp m_RejectedImageTypeModality;
    QRegExp m_RejectedImageTypeOther;
};

#endif
