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

#include <gdcmScanner.h>

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



const char* DEFAULT_SETTINGS_FILTER_MODALITY = "CT|MR|PT|CR|DX|OT|NM";

const char* DEFAULT_SETTINGS_FILTER_IMAGE_TYPE_ACCEPT =          "DERIVED\\\\SECONDARY\\\\AXIAL\\\\CT_SOM. SPI DUAL";
const char* DEFAULT_SETTINGS_FILTER_IMAGE_TYPE_PIXEL_ACCEPT =    "";
const char* DEFAULT_SETTINGS_FILTER_IMAGE_TYPE_EXAM_ACCEPT =     "";
const char* DEFAULT_SETTINGS_FILTER_IMAGE_TYPE_MODALITY_ACCEPT = "";
const char* DEFAULT_SETTINGS_FILTER_IMAGE_TYPE_OTHER_ACCEPT =    "";

const char* DEFAULT_SETTINGS_FILTER_IMAGE_TYPE_REJECT =          "";
const char* DEFAULT_SETTINGS_FILTER_IMAGE_TYPE_PIXEL_REJECT =    "DERIVED";
const char* DEFAULT_SETTINGS_FILTER_IMAGE_TYPE_EXAM_REJECT =     "";
const char* DEFAULT_SETTINGS_FILTER_IMAGE_TYPE_MODALITY_REJECT = "LOCALIZER";
const char* DEFAULT_SETTINGS_FILTER_IMAGE_TYPE_OTHER_REJECT =    "MPR|MIP";



DICOMPredictorUI::DICOMPredictorUI( QWidget* parent, Qt::WindowFlags f )
: QWidget(parent, f)
, m_GUI( new Ui::DICOMPredictorUI )
, m_Config( CreateConfig() )
{
  m_GUI->setupUi(this);

  connect( m_GUI->btnScanDirectories, SIGNAL(clicked()), this, SLOT(AnalyzeSelectedDirectories()) );
  connect( m_GUI->btnResetFilters, SIGNAL(clicked()), this, SLOT(ResetFiltersToDefault()) );

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
         << "# files"
         << "Series Description"
         << "SOP Class"
         << "Directory"
         << "Series UID"
         ;

  m_GUI->resultsView->setColumnCount(labels.size());
  m_GUI->resultsView->setColumnWidth( 0, 90 );
  m_GUI->resultsView->setColumnWidth( 1, 90 );
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

  m_GUI->edtModalityAccept->setText(      m_Config->value( SETTINGS_FILTER_MODALITY, DEFAULT_SETTINGS_FILTER_MODALITY ).toString() );

  m_GUI->edtITAccept->setText(            m_Config->value( SETTINGS_FILTER_IMAGE_TYPE_ACCEPT,          DEFAULT_SETTINGS_FILTER_IMAGE_TYPE_ACCEPT ).toString() );
  m_GUI->edtITPixelDataAccept->setText(   m_Config->value( SETTINGS_FILTER_IMAGE_TYPE_PIXEL_ACCEPT,    DEFAULT_SETTINGS_FILTER_IMAGE_TYPE_PIXEL_ACCEPT ).toString() );
  m_GUI->edtITExaminationAccept->setText( m_Config->value( SETTINGS_FILTER_IMAGE_TYPE_EXAM_ACCEPT,     DEFAULT_SETTINGS_FILTER_IMAGE_TYPE_EXAM_ACCEPT ).toString() );
  m_GUI->edtITModalityAccept->setText(    m_Config->value( SETTINGS_FILTER_IMAGE_TYPE_MODALITY_ACCEPT, DEFAULT_SETTINGS_FILTER_IMAGE_TYPE_MODALITY_ACCEPT ).toString() );
  m_GUI->edtITOtherAccept->setText(       m_Config->value( SETTINGS_FILTER_IMAGE_TYPE_OTHER_ACCEPT,    DEFAULT_SETTINGS_FILTER_IMAGE_TYPE_OTHER_ACCEPT ).toString() );

  m_GUI->edtITReject->setText(            m_Config->value( SETTINGS_FILTER_IMAGE_TYPE_REJECT,          DEFAULT_SETTINGS_FILTER_IMAGE_TYPE_REJECT ).toString() );
  m_GUI->edtITPixelDataReject->setText(   m_Config->value( SETTINGS_FILTER_IMAGE_TYPE_PIXEL_REJECT,    DEFAULT_SETTINGS_FILTER_IMAGE_TYPE_PIXEL_REJECT ).toString() );
  m_GUI->edtITExaminationReject->setText( m_Config->value( SETTINGS_FILTER_IMAGE_TYPE_EXAM_REJECT,     DEFAULT_SETTINGS_FILTER_IMAGE_TYPE_EXAM_REJECT ).toString() );
  m_GUI->edtITModalityReject->setText(    m_Config->value( SETTINGS_FILTER_IMAGE_TYPE_MODALITY_REJECT, DEFAULT_SETTINGS_FILTER_IMAGE_TYPE_MODALITY_REJECT ).toString() );
  m_GUI->edtITOtherReject->setText(       m_Config->value( SETTINGS_FILTER_IMAGE_TYPE_OTHER_REJECT,    DEFAULT_SETTINGS_FILTER_IMAGE_TYPE_OTHER_REJECT ).toString() );

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

void DICOMPredictorUI::ResetFiltersToDefault()
{
  m_GUI->edtModalityAccept->setText(      DEFAULT_SETTINGS_FILTER_MODALITY );

  m_GUI->edtITAccept->setText(            DEFAULT_SETTINGS_FILTER_IMAGE_TYPE_ACCEPT );
  m_GUI->edtITPixelDataAccept->setText(   DEFAULT_SETTINGS_FILTER_IMAGE_TYPE_PIXEL_ACCEPT );
  m_GUI->edtITExaminationAccept->setText( DEFAULT_SETTINGS_FILTER_IMAGE_TYPE_EXAM_ACCEPT );
  m_GUI->edtITModalityAccept->setText(    DEFAULT_SETTINGS_FILTER_IMAGE_TYPE_MODALITY_ACCEPT );
  m_GUI->edtITOtherAccept->setText(       DEFAULT_SETTINGS_FILTER_IMAGE_TYPE_OTHER_ACCEPT );

  m_GUI->edtITReject->setText(            DEFAULT_SETTINGS_FILTER_IMAGE_TYPE_REJECT );
  m_GUI->edtITPixelDataReject->setText(   DEFAULT_SETTINGS_FILTER_IMAGE_TYPE_PIXEL_REJECT );
  m_GUI->edtITExaminationReject->setText( DEFAULT_SETTINGS_FILTER_IMAGE_TYPE_EXAM_REJECT );
  m_GUI->edtITModalityReject->setText(    DEFAULT_SETTINGS_FILTER_IMAGE_TYPE_MODALITY_REJECT );
  m_GUI->edtITOtherReject->setText(       DEFAULT_SETTINGS_FILTER_IMAGE_TYPE_OTHER_REJECT );

  this->UpdateFilterPatternsFromGUI();
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
    int numberOfFiles = 0;

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

      numberOfFiles += blockIter->GetFilenames().size();
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
      QTableWidgetItem *filesItem = new QTableWidgetItem( QString::number( numberOfFiles ) );

      int col = 0;
      m_GUI->resultsView->setItem( newRow, col++, blocksItem );
      m_GUI->resultsView->setItem( newRow, col++, filesItem );
      m_GUI->resultsView->setItem( newRow, col++, descriptionItem );
      m_GUI->resultsView->setItem( newRow, col++, sopItem );
      m_GUI->resultsView->setItem( newRow, col++, dirItem );
      m_GUI->resultsView->setItem( newRow, col++, uidItem );
    }
  }
}

void DICOMPredictorUI::AnalyzeSelectedDirectories()
{
  this->UpdateFilterPatternsFromGUI();

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

bool DICOMPredictorUI::MatchesRegularExpression( const QRegExp& expression, const QString& str)
{
  if (expression.isEmpty())
  {
    // we don't want to search for empty string, because we will ALWAYS find one
    return false;
  }

  return expression.indexIn( str ) > -1;
}

bool DICOMPredictorUI::AcceptableByByModalityAndImageType( const QString& modalityTagValue, const QString& imageTypeTagValue )
{
  if ( !MatchesRegularExpression( m_ValidModalities, modalityTagValue ) ) return false; // reject

  if ( MatchesRegularExpression( m_AcceptedImageTypes, imageTypeTagValue ) ) return true; // early accept

  static QRegExp imageTypeSplitter(  // tag structure: two mandatory fields, two more optional ones
      "^"
      "([^\\\\]*)\\\\([^\\\\]*)"
      ".*"
      "(?:"
        "\\\\([^\\\\]*)"
        "(?:"
          "\\\\(.*)"
        ")?"
      ")?"
      "$"
      );
  int matchPosition = imageTypeSplitter.indexIn( imageTypeTagValue );
  QStringList itc = imageTypeSplitter.capturedTexts();

  /*
  MITK_INFO << "Matched " << qPrintable(imageTypeTagValue )
            << " at " << matchPosition << " as "
            << qPrintable(itc.value(1)) << " : "
            << qPrintable(itc.value(2)) << " :  "
            << qPrintable(itc.value(3)) << " :  "
            << qPrintable(itc.value(4)) << ";";
  */

  if (matchPosition == 0)
  {
    QStringList imageTypeComponents = imageTypeSplitter.capturedTexts();

    if ( MatchesRegularExpression( m_AcceptedImageTypePixelData,   imageTypeComponents.value(1) ) ) return true;
    if ( MatchesRegularExpression( m_AcceptedImageTypeExamination, imageTypeComponents.value(2) ) ) return true;
    if ( MatchesRegularExpression( m_AcceptedImageTypeModality,    imageTypeComponents.value(3) ) ) return true;
    if ( MatchesRegularExpression( m_AcceptedImageTypeOther,       imageTypeComponents.value(4) ) ) return true;

    if ( MatchesRegularExpression( m_RejectedImageTypes, imageTypeTagValue ) ) return false; // reject
    if ( MatchesRegularExpression( m_RejectedImageTypePixelData,   imageTypeComponents.value(1) ) ) return false;
    if ( MatchesRegularExpression( m_RejectedImageTypeExamination, imageTypeComponents.value(2) ) ) return false;
    if ( MatchesRegularExpression( m_RejectedImageTypeModality,    imageTypeComponents.value(3) ) ) return false;
    if ( MatchesRegularExpression( m_RejectedImageTypeOther,       imageTypeComponents.value(4) ) ) return false;
  }
  else
  {
    MITK_WARN << matchPosition << "Found unparseable value for (0008,0008) Image Type: " << qPrintable( imageTypeTagValue );
  }

  return true; // fallback: accept
}

QStringList DICOMPredictorUI::FilterFilesByModalityAndImageType(const QStringList& filenames)
{
  QStringList result;

  gdcm::Scanner scanner;

  const gdcm::Tag tagModality(0x0008, 0x0060); // modality
    scanner.AddTag( tagModality );

  const gdcm::Tag tagImageType(0x0008, 0x0008); // image type
    scanner.AddTag( tagImageType );

  mitk::DicomSeriesReader::StringContainer gdcmInput = ConvertQStringListToDCMReaderInput( filenames );
  if ( scanner.Scan( gdcmInput ) )
  {
    for (gdcm::Scanner::ConstIterator fileIter = scanner.Begin();
         fileIter != scanner.End();
         ++fileIter)
    {
      if ( std::string(fileIter->first).empty() ) continue;
      if ( std::string(fileIter->first) == std::string("DICOMDIR") ) continue;

      gdcm::Scanner::TagToValue& tagValueMap = const_cast<gdcm::Scanner::TagToValue&>(fileIter->second);

      const char* modalityValue = tagValueMap[tagModality];
      const char* imageTypeValue = tagValueMap[tagImageType];

      if ( AcceptableByByModalityAndImageType( modalityValue, imageTypeValue ) )
      {
        result << fileIter->first;
        //MITK_INFO << "Accepted Modality, Image Type: " << modalityValue << " : " << imageTypeValue;
      }
      else
      {
        MITK_INFO << "REJECTED Modality, Image Type: " << modalityValue << " : " << imageTypeValue;
      }
    }
  }
  else
  {
    MITK_WARN << "Error during gdcm::Scanner::Scan(). Output might not be correct.";
    return filenames;
  }

  return result;
}

void DICOMPredictorUI::AnalyzeDirectory( const QStringList& paths )
{
  foreach (QString directory, paths)
  {
    qApp->processEvents( QEventLoop::ExcludeUserInputEvents ); // update list

    QStringList dirs;
    dirs << directory;
    QStringList allFilesToCheck = FindAllFilesIn( dirs, "", false ); // false = no recursion
    QStringList allFilteredFilesToCheck = FilterFilesByModalityAndImageType( allFilesToCheck );
    mitk::DicomSeriesReader::StringContainer allFilesToCheckDCMReader = ConvertQStringListToDCMReaderInput( allFilteredFilesToCheck );
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
        QTableWidgetItem *descriptionItem = new QTableWidgetItem( "No acceptable files in direcotry." );

        m_GUI->resultsView->setItem( newRow, 4, dirItem );
        m_GUI->resultsView->setItem( newRow, 2, descriptionItem );
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

      m_GUI->resultsView->setItem( newRow, 4, dirItem );
      m_GUI->resultsView->setItem( newRow, 2, descriptionItem );
    }

    m_Config->setValue( SETTINGS_LAST_FOLDER, directory );
  }
}

