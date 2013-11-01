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

  This class implements a simple GUI where the user can
   - select one or more directories from his local file system
   - analyse selected files regarding how many blocks (mitk::Images) would be created by DicomSeriesReader

  Analysis is fixed to the assumption that DicomSeriesReader is asked to scan with the following options:
   - create separate blocks for slices at the same position (3D+t = false)
   - also form blocks of images from gantry tilt acquisitions (gantryTilt = true)

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

  protected:

      QSettings* CreateConfig();

      mitk::DicomSeriesReader::StringContainer ConvertQStringListToDCMReaderInput(const QStringList& filenameList);
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
