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

#include "DICOMPredictorUI.h"

#include "ui_DICOMPredictorUI.h"

#include <mitkLogMacros.h>

#include "DICOMPredictorUI.h"

const char* SETTINGS_LAST_FOLDER = "folder/location";

const char* SETTINGS_FILTER_MODALITY = "filter.modality/accept";

const char* SETTINGS_FILTER_IMAGE_TYPE_ACCEPT =          "filter.imagetype/accept";
const char* SETTINGS_FILTER_IMAGE_TYPE_PIXEL_ACCEPT =    "filter.imagetype/pixel.accept";
const char* SETTINGS_FILTER_IMAGE_TYPE_EXAM_ACCEPT =     "filter.imagetype/examination.accept";
const char* SETTINGS_FILTER_IMAGE_TYPE_MODALITY_ACCEPT = "filter.imagetype/modality.accept";
const char* SETTINGS_FILTER_IMAGE_TYPE_OTHER_ACCEPT =    "filter.imagetype/other.accept";

const char* SETTINGS_FILTER_IMAGE_TYPE_REJECT =          "filter.imagetype/reject";
const char* SETTINGS_FILTER_IMAGE_TYPE_PIXEL_REJECT =    "filter.imagetype/pixel.reject";
const char* SETTINGS_FILTER_IMAGE_TYPE_EXAM_REJECT =     "filter.imagetype/examination.reject";
const char* SETTINGS_FILTER_IMAGE_TYPE_MODALITY_REJECT = "filter.imagetype/modality.reject";
const char* SETTINGS_FILTER_IMAGE_TYPE_OTHER_REJECT =    "filter.imagetype/other.reject";


DICOMPredictorUI::DICOMPredictorUI( QWidget* parent, Qt::WindowFlags f )
: QWidget(parent, f)
, m_GUI( new Ui::DICOMPredictorUI )
, m_Config( CreateConfig() )
{
  m_GUI->setupUi(this);

  connect( m_GUI->button, SIGNAL(clicked()), this, SLOT(AnalyzeSelectedDirectories()) );

  m_DirModel.setFilter( QDir::Dirs | QDir::NoDotAndDotDot );
  QString topLevelDir = QDesktopServices::storageLocation(QDesktopServices::DesktopLocation);
  m_DirModel.setRootPath( topLevelDir );

  m_ProxyModel.setSourceModel( &m_DirModel );

  m_GUI->dirView->setModel( &m_ProxyModel );

  m_GUI->dirView->setSortingEnabled( true );
  m_GUI->dirView->sortByColumn( 0, Qt::AscendingOrder );
  m_GUI->dirView->setColumnHidden( 1, true );
  m_GUI->dirView->setColumnHidden( 2, true );
  m_GUI->dirView->setColumnHidden( 3, true );

  QStringList labels;
  labels
         << "# blocks"
         << "Series Description"
         << "SOP Class UID"
         << "Directory"
         << "UID"
         ;

  m_GUI->resultsView->setColumnCount(labels.size());
  m_GUI->resultsView->setColumnWidth( 0, 90 );
  m_GUI->resultsView->setHorizontalHeaderLabels( labels );

  this->RestoreSavedPresets();

  connect( qApp, SIGNAL(aboutToQuit()), this, SLOT(SavePresets()) );

  // TODO call SavePresets on application exit!
}


DICOMPredictorUI::~DICOMPredictorUI()
{
}

void DICOMPredictorUI::RestoreSavedPresets()
{
  QString lastPath = m_Config->value( SETTINGS_LAST_FOLDER, QDir::currentPath() ).toString();
  QModelIndex selectedIndex = m_ProxyModel.mapFromSource( m_DirModel.index( lastPath ) );
  m_GUI->dirView->setCurrentIndex( selectedIndex );
  m_GUI->dirView->scrollTo( selectedIndex, QAbstractItemView::PositionAtBottom ); // does not seem to work?

  m_GUI->edtModalityAccept->setText(      m_Config->value( SETTINGS_FILTER_MODALITY, "CT|MR|PT|CR|DX|OT|NM" ).toString() );

  m_GUI->edtITAccept->setText(            m_Config->value( SETTINGS_FILTER_IMAGE_TYPE_ACCEPT, "" ).toString() );
  m_GUI->edtITPixelDataAccept->setText(   m_Config->value( SETTINGS_FILTER_IMAGE_TYPE_PIXEL_ACCEPT, "" ).toString() );
  m_GUI->edtITExaminationAccept->setText( m_Config->value( SETTINGS_FILTER_IMAGE_TYPE_EXAM_ACCEPT, "" ).toString() );
  m_GUI->edtITModalityAccept->setText(    m_Config->value( SETTINGS_FILTER_IMAGE_TYPE_MODALITY_ACCEPT, "" ).toString() );
  m_GUI->edtITOtherAccept->setText(       m_Config->value( SETTINGS_FILTER_IMAGE_TYPE_OTHER_ACCEPT, "" ).toString() );

  m_GUI->edtITReject->setText(            m_Config->value( SETTINGS_FILTER_IMAGE_TYPE_REJECT, "" ).toString() );
  m_GUI->edtITPixelDataReject->setText(   m_Config->value( SETTINGS_FILTER_IMAGE_TYPE_PIXEL_REJECT, "" ).toString() );
  m_GUI->edtITExaminationReject->setText( m_Config->value( SETTINGS_FILTER_IMAGE_TYPE_EXAM_REJECT, "" ).toString() );
  m_GUI->edtITModalityReject->setText(    m_Config->value( SETTINGS_FILTER_IMAGE_TYPE_MODALITY_REJECT, "" ).toString() );
  m_GUI->edtITOtherReject->setText(       m_Config->value( SETTINGS_FILTER_IMAGE_TYPE_OTHER_REJECT, "" ).toString() );

  this->UpdateFilterPatternsFromGUI();
}

void DICOMPredictorUI::SavePresets()
{
  /* would be nice to do this here, but we already store each iterated directory
  QString lastPath = m_Config->value( SETTINGS_LAST_FOLDER, QDir::currentPath() ).toString();
  QModelIndex selectedIndex = m_ProxyModel.mapFromSource( m_DirModel.index( lastPath ) );
  m_GUI->dirView->setCurrentIndex( selectedIndex );
  m_GUI->dirView->scrollTo( selectedIndex, QAbstractItemView::PositionAtBottom ); // does not seem to work?
  */

  this->UpdateFilterPatternsFromGUI();

  m_Config->setValue( SETTINGS_FILTER_MODALITY,                   m_ValidModalities.pattern() );

  m_Config->setValue( SETTINGS_FILTER_IMAGE_TYPE_ACCEPT,          m_AcceptedImageTypes.pattern() );
  m_Config->setValue( SETTINGS_FILTER_IMAGE_TYPE_PIXEL_ACCEPT,    m_AcceptedImageTypePixelData.pattern() );
  m_Config->setValue( SETTINGS_FILTER_IMAGE_TYPE_EXAM_ACCEPT,     m_AcceptedImageTypeExamination.pattern() );
  m_Config->setValue( SETTINGS_FILTER_IMAGE_TYPE_MODALITY_ACCEPT, m_AcceptedImageTypeModality.pattern() );
  m_Config->setValue( SETTINGS_FILTER_IMAGE_TYPE_OTHER_ACCEPT,    m_AcceptedImageTypeOther.pattern() );

  m_Config->setValue( SETTINGS_FILTER_IMAGE_TYPE_REJECT,          m_RejectedImageTypes.pattern() );
  m_Config->setValue( SETTINGS_FILTER_IMAGE_TYPE_PIXEL_REJECT,    m_RejectedImageTypePixelData.pattern() );
  m_Config->setValue( SETTINGS_FILTER_IMAGE_TYPE_EXAM_REJECT,     m_RejectedImageTypeExamination.pattern() );
  m_Config->setValue( SETTINGS_FILTER_IMAGE_TYPE_MODALITY_REJECT, m_RejectedImageTypeModality.pattern() );
  m_Config->setValue( SETTINGS_FILTER_IMAGE_TYPE_OTHER_REJECT,    m_RejectedImageTypeOther.pattern() );

  m_Config->sync();
}

void DICOMPredictorUI::UpdateFilterPatternsFromGUI()
{
  m_ValidModalities.setPattern(               m_GUI->edtModalityAccept->text() );

  m_AcceptedImageTypes.setPattern(            m_GUI->edtITAccept->text() );
  m_AcceptedImageTypePixelData.setPattern(    m_GUI->edtITPixelDataAccept->text() );
  m_AcceptedImageTypeExamination.setPattern(  m_GUI->edtITExaminationAccept->text() );
  m_AcceptedImageTypeModality.setPattern(     m_GUI->edtITModalityAccept->text() );
  m_AcceptedImageTypeOther.setPattern(        m_GUI->edtITOtherAccept->text() );

  m_RejectedImageTypes.setPattern(            m_GUI->edtITReject->text() );
  m_RejectedImageTypePixelData.setPattern(    m_GUI->edtITPixelDataReject->text() );
  m_RejectedImageTypeExamination.setPattern(  m_GUI->edtITExaminationReject->text() );
  m_RejectedImageTypeModality.setPattern(     m_GUI->edtITModalityReject->text() );
  m_RejectedImageTypeOther.setPattern(        m_GUI->edtITOtherReject->text() );
}


QSettings* DICOMPredictorUI::CreateConfig()
{
  QSettings* settings = new QSettings( QApplication::applicationDirPath() + QDir::separator() + "predictor.ini", QSettings::IniFormat, this );
  return settings;
}


mitk::DicomSeriesReader::StringContainer DICOMPredictorUI::ConvertQStringListToDCMReaderInput(const QStringList& filenameList)
{
  mitk::DicomSeriesReader::StringContainer result;
  foreach (QString s, filenameList)
  {
    result.push_back( s.toLocal8Bit().data() );
  }
  return result;
}

QStringList DICOMPredictorUI::FindAllFilesIn(const QStringList& listOfFilesAndDirectories, const QString& directoryname, bool recurse)
{
  QStringList result;

  foreach(QString candidate, listOfFilesAndDirectories)
  {
    QString fullCandidateName = candidate;
    if (!directoryname.isEmpty())
    {
      fullCandidateName = directoryname + QDir::separator() + candidate;
    }
    QDir directory( fullCandidateName );
    if ( directory.exists() )
    {
      // check this directory
      foreach(QString filename, directory.entryList( QDir::Files ))
      {
        result.append( fullCandidateName + QDir::separator() + filename ); // use all files
      }

      if (recurse)
      {
        result.append( FindAllFilesIn( directory.entryList( QDir::Dirs | QDir::NoDotAndDotDot ), fullCandidateName ) ); // process all sub-directories
      }
    }
    else if ( QFile::exists( candidate ) )
    {
      // add this file
      result << fullCandidateName;
    }
    else
    {
      MITK_WARN << "Ignoring item '" << qPrintable(candidate) << "', which is neither file nor directory";
    }
  }

  return result;
}

DicomSeriesDescriptor DICOMPredictorUI::GroupSortingResultsBySeries( mitk::DicomSeriesReader::FileNamesGrouping imageBlockGrouping )
{
  DicomSeriesDescriptor result;

  for( mitk::DicomSeriesReader::FileNamesGrouping::const_iterator iter = imageBlockGrouping.begin();
       iter != imageBlockGrouping.end();
       ++iter )
  {
    mitk::DicomSeriesReader::ImageBlockDescriptor blockDescriptor = iter->second;
    result[ blockDescriptor.GetSeriesInstanceUID() ].push_back( blockDescriptor );
  }

  return result;
}

std::string DICOMPredictorUI::ConcatStringList( std::set<std::string> stringList )
{
  std::stringstream ss;
  if (stringList.size() > 1)
  {
    ss << "[" << stringList.size() << "] ";
  }

  ss << *(stringList.begin());

  if (stringList.size() > 1)
  {
    for (std::set<std::string>::const_iterator iter = ++(stringList.begin());
        iter != stringList.end();
        ++iter)
    {
      ss << ", " << *iter;
    }
  }

  return ss.str();
}

void DICOMPredictorUI::OutputSeriesGroupingResultsToList( const QString& path,  DicomSeriesDescriptor seriesGrouping, bool& multiBlocksPresent )
{
  multiBlocksPresent = false;

  for( DicomSeriesDescriptor::const_iterator seriesIter = seriesGrouping.begin();
       seriesIter != seriesGrouping.end();
       ++seriesIter )
  {
    std::set<std::string> sopclassuids;
    std::set<std::string> modalities;
    std::set<std::string> descriptions;
    std::set<std::string> psinterprets;
    std::set<std::string> gantrytilts;
    std::set<std::string> multiframes;
    for( std::list< mitk::DicomSeriesReader::ImageBlockDescriptor >::const_iterator blockIter = seriesIter->second.begin();
       blockIter != seriesIter->second.end();
       ++blockIter )
    {
      std::string psInterpret;
      if ( blockIter->PixelSpacingRelatesToPatient() )
        psInterpret = "patient";
      else if ( blockIter->PixelSpacingRelatesToDetector() )
        psInterpret = "detector";
      else if ( blockIter->PixelSpacingIsUnknown() )
        psInterpret = "unknown";

      sopclassuids.insert( blockIter->GetSOPClassUIDAsString() );
      descriptions.insert( blockIter->GetSeriesDescription() );
      modalities.insert( blockIter->GetModality() );
      psinterprets.insert( psInterpret );
      gantrytilts.insert( blockIter->HasGantryTiltCorrected() ? "yes" : "no" );
      multiframes.insert( blockIter->IsMultiFrameImage() ? "yes" : "no" );
    }

    bool problem = seriesIter->second.size() > 1;
    multiBlocksPresent |= problem;

    if (problem || !problem) // always :-)
    {
      int newRow = m_GUI->resultsView->rowCount();
      m_GUI->resultsView->insertRow( newRow );

      QTableWidgetItem *dirItem = new QTableWidgetItem( path );
      QTableWidgetItem *descriptionItem = new QTableWidgetItem( QString::fromStdString(ConcatStringList( descriptions )) );
      QTableWidgetItem *uidItem = new QTableWidgetItem( QString::fromStdString( seriesIter->first ) );
      QTableWidgetItem *sopItem = new QTableWidgetItem( QString::fromStdString(ConcatStringList( sopclassuids )) );
      QTableWidgetItem *blocksItem = new QTableWidgetItem( QString::number( seriesIter->second.size() ) );
      int col = 0;
      m_GUI->resultsView->setItem( newRow, col++, blocksItem );
      m_GUI->resultsView->setItem( newRow, col++, descriptionItem );
      m_GUI->resultsView->setItem( newRow, col++, sopItem );
      m_GUI->resultsView->setItem( newRow, col++, dirItem );
      m_GUI->resultsView->setItem( newRow, col++, uidItem );
    }
  }
}

void DICOMPredictorUI::AnalyzeSelectedDirectories()
{
  QModelIndexList selectedIndexes = m_GUI->dirView->selectionModel()->selectedRows();

  m_GUI->progressBar->setMaximum( selectedIndexes.size() );
  m_GUI->progressBar->setValue( 0 );

  foreach(QModelIndex index, selectedIndexes)
  {
    QModelIndex selectedDirectoryIndex = m_ProxyModel.mapToSource( index );
    QFileInfo fi = m_DirModel.fileInfo( selectedDirectoryIndex );
    this->AnalyzeDirectory( fi.absoluteFilePath() );

    m_GUI->progressBar->setValue( m_GUI->progressBar->value() + 1 );
    qApp->processEvents( QEventLoop::ExcludeUserInputEvents ); // update list
  }
}

void DICOMPredictorUI::AnalyzeDirectory( const QString& path )
{
  QStringList dirs;
  dirs << path;
  this->AnalyzeDirectory( dirs );
}

void DICOMPredictorUI::AnalyzeDirectory( const QStringList& paths )
{
  foreach (QString directory, paths)
  {
    qApp->processEvents( QEventLoop::ExcludeUserInputEvents ); // update list

    QStringList dirs;
    dirs << directory;
    QStringList allFilesToCheck = FindAllFilesIn( dirs, "", false ); // false = no recursion
    mitk::DicomSeriesReader::StringContainer allFilesToCheckDCMReader = ConvertQStringListToDCMReaderInput( allFilesToCheck );
    try
    {
      mitk::DicomSeriesReader::FileNamesGrouping imageBlockGrouping = mitk::DicomSeriesReader::GetSeries( allFilesToCheckDCMReader, false, true );
      DicomSeriesDescriptor seriesGrouping = GroupSortingResultsBySeries( imageBlockGrouping );
      bool problem (false);
      if (seriesGrouping.empty())
      {
        int newRow = m_GUI->resultsView->rowCount();
        m_GUI->resultsView->insertRow( newRow );

        QTableWidgetItem *dirItem = new QTableWidgetItem( directory );
        QTableWidgetItem *descriptionItem = new QTableWidgetItem( "No DICOM files in direcotry." );

        m_GUI->resultsView->setItem( newRow, 3, dirItem );
        m_GUI->resultsView->setItem( newRow, 1, descriptionItem );
      }
      else
      {
        OutputSeriesGroupingResultsToList( directory, seriesGrouping, problem );
      }
    }
    catch( const std::exception& e )
    {
      int newRow = m_GUI->resultsView->rowCount();
      m_GUI->resultsView->insertRow( newRow );

      QTableWidgetItem *dirItem = new QTableWidgetItem( directory );
      QTableWidgetItem *descriptionItem = new QTableWidgetItem( QString("Scan error:") + e.what() );

      m_GUI->resultsView->setItem( newRow, 3, dirItem );
      m_GUI->resultsView->setItem( newRow, 1, descriptionItem );
    }

    m_Config->setValue( SETTINGS_LAST_FOLDER, directory );
  }
}

