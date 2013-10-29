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

#include <QtCore>

#include <mitkLogMacros.h>

#include <mitkDicomSeriesReader.h>

typedef std::map<std::string, std::list<mitk::DicomSeriesReader::ImageBlockDescriptor> > DicomSeriesDescriptor;

QStringList ConvertArgvArgcToStringList(int argc, char* argv[])
{
  QStringList list;

  for (int a = 1; a < argc; ++a)
    list << QString::fromLocal8Bit(argv[a]);

  return list;
}

mitk::DicomSeriesReader::StringContainer ConvertQStringListToDCMReaderInput(const QStringList& filenameList)
{
  mitk::DicomSeriesReader::StringContainer result;
  foreach (QString s, filenameList)
  {
    result.push_back( s.toLocal8Bit().data() );
  }
  return result;
}

QStringList FindAllFilesIn(const QStringList& listOfFilesAndDirectories, const QString& directoryname = "" )
{
  QStringList result;

  foreach(QString candidate, listOfFilesAndDirectories)
  {
    QString fullCandidateName = directoryname + QDir::separator() + candidate;
    QDir directory( fullCandidateName );
    if ( directory.exists() )
    {
      // check this directory
      foreach(QString filename, directory.entryList( QDir::Files ))
      {
        result.append( fullCandidateName + QDir::separator() + filename ); // use all files
      }

      result.append( FindAllFilesIn( directory.entryList( QDir::Dirs | QDir::NoDotAndDotDot ), fullCandidateName ) ); // process all sub-directories
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

DicomSeriesDescriptor GroupSortingResultsBySeries( mitk::DicomSeriesReader::FileNamesGrouping imageBlockGrouping )
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

std::string ConcatStringList( std::set<std::string> stringList )
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

void OutputSeriesGroupingResults( DicomSeriesDescriptor seriesGrouping, bool& multiBlocksPresent )
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

      sopclassuids.insert( blockIter->GetSOPClassUID() + " (=" + blockIter->GetSOPClassUIDAsString() + ")" );
      descriptions.insert( blockIter->GetSeriesDescription() );
      modalities.insert( blockIter->GetModality() );
      psinterprets.insert( psInterpret );
      gantrytilts.insert( blockIter->HasGantryTiltCorrected() ? "yes" : "no" );
      multiframes.insert( blockIter->IsMultiFrameImage() ? "yes" : "no" );
    }

    bool problem = seriesIter->second.size() > 1;
    multiBlocksPresent |= problem;

    if (problem)
    {
      MITK_INFO << "------------------------------------------------------------------------";
      MITK_INFO << "Series Instance UID: " << seriesIter->first;
      MITK_INFO << "Series Description:  " << ConcatStringList( descriptions );
      MITK_INFO << "SOP Class UIDs:      " << ConcatStringList( sopclassuids );
      MITK_INFO << "Modalities:          " << ConcatStringList( modalities );
      MITK_INFO << "Number of blocks:    " << seriesIter->second.size();
      //MITK_INFO << "Pixel spacing type:  " << ConcatStringList( psinterprets );
      //MITK_INFO << "Gantry tilt:         " << ConcatStringList( gantrytilts );
      //MITK_INFO << "Multi-frame images:  " << ConcatStringList( multiframes );

      // TODO more complex:
      //   - output different orientations
      //   - output series description
    }
  }

  MITK_INFO << "------------------------------------------------------------------------";
  MITK_INFO << "End of problem report";
}


int main(int argc, char* argv[])
{
  int exitCode = EXIT_SUCCESS;

  QStringList commandlineParameters = ConvertArgvArgcToStringList(argc,argv);

  /*
  MITK_INFO << "Checking DICOM series in ";
  foreach(QString p, commandlineParameters)
    MITK_INFO << "  " << qPrintable(p);
  /**/

  QStringList allFilesToCheck = FindAllFilesIn( commandlineParameters );

  /*
  MITK_INFO << "-------------------------------------------";
  MITK_INFO << "Qt file list: ";
  int count = 0;
  foreach(QString p, allFilesToCheck)
  {
    MITK_INFO << "  " << ++count << ": "<< qPrintable(p);
  }
  /**/

  mitk::DicomSeriesReader::StringContainer allFilesToCheckDCMReader = ConvertQStringListToDCMReaderInput( allFilesToCheck );

  /*
  MITK_INFO << "-------------------------------------------";
  MITK_INFO << "STL file list: ";
  count = 0;
  for (mitk::DicomSeriesReader::StringContainer::const_iterator iter = allFilesToCheckDCMReader.begin();
       iter != allFilesToCheckDCMReader.end();
       ++iter)
  {
    MITK_INFO << "  " << ++count << ": "<< *iter;
  }
  /**/

  MITK_INFO << "Checking for problems in " << allFilesToCheckDCMReader.size() << " files. Please wait...";

  mitk::DicomSeriesReader::FileNamesGrouping imageBlockGrouping = mitk::DicomSeriesReader::GetSeries( allFilesToCheckDCMReader, false, true );

  /*
  for( mitk::DicomSeriesReader::FileNamesGrouping::const_iterator iter = imageBlockGrouping.begin();
       iter != imageBlockGrouping.end();
       ++iter )
  {
    std::string psInterpret;
    if ( iter->second.PixelSpacingRelatesToPatient() )
      psInterpret = "patient";
    else if ( iter->second.PixelSpacingRelatesToDetector() )
      psInterpret = "detector";
    else if ( iter->second.PixelSpacingIsUnknown() )
      psInterpret = "unknown";

    MITK_INFO << "------------------------------------------------------------------------";
    MITK_INFO << "Series Instance UID: " << iter->second.GetSeriesInstanceUID();
//    MITK_INFO << "Enhanced Series UID: " << iter->second.GetImageBlockUID();
    MITK_INFO << "SOP Class UID:       " << iter->second.GetSOPClassUID() << " (=" << iter->second.GetSOPClassUIDAsString() << ")";
    MITK_INFO << "Modality:            " << iter->second.GetModality();
    MITK_INFO << "Pixel spacing type:  " << psInterpret;
    MITK_INFO << "Gantry tilt:         " << (iter->second.HasGantryTiltCorrected() ? "yes" : "no");
    MITK_INFO << "Multi-frame image:   " << (iter->second.IsMultiFrameImage() ? "yes" : "no");
  }
  */

  DicomSeriesDescriptor seriesGrouping = GroupSortingResultsBySeries( imageBlockGrouping );

  bool problem (false);
  OutputSeriesGroupingResults( seriesGrouping, problem );

  return problem ? 1 : 0;
}
