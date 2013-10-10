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

#include "QmitkDicomView.h"

#include "ui_QmitkDicomView.h"

#include <itkCommand.h>

QmitkDicomView::QmitkDicomView(QWidget* parent, Qt::WindowFlags f)
: QWidget(parent, f)
, m_GUI( new Ui::QmitkDicomView )
{
  m_GUI->setupUi(this);

  connect( m_GUI->btnLoad, SIGNAL(clicked()), this, SLOT(LoadDICOMFiles()) );

  connect( this, SIGNAL(SignalProgressFromReaderThread()), this, SLOT(ReportProgressFromReader()), Qt::QueuedConnection );

  m_GUI->progressBar->setFormat("%v / %m files");
}

QmitkDicomView::~QmitkDicomView()
{
}

void QmitkDicomView::LoadDICOMFiles()
{
  // request files
  QFileDialog dialog(this, "Select one or more DICOM files to open", "",  "DICOM images (*)");
  dialog.setOption( QFileDialog::DontUseNativeDialog, false );
  dialog.setFileMode( QFileDialog::ExistingFiles );
  dialog.exec();
  dialog.hide();
  QStringList filesToOpen = dialog.selectedFiles();

  if ( filesToOpen.isEmpty() ) return;

  // convert to input filelist type
  mitk::DICOMSeriesReader::StringList filenames;
  foreach(QString filename, filesToOpen)
  {
    filenames.push_back( filename.toStdString() );
  }

  // init GUI
  m_GUI->progressBar->setMinimum(0);
  m_GUI->progressBar->setMaximum( filenames.size() );
  m_GUI->progressBar->setValue(0);
  m_GUI->progressBar->show();

  // start DICOMSeriesReader
  m_Reader = mitk::DICOMSeriesReader::New();

  itk::SimpleMemberCommand<QmitkDicomView>::Pointer progressCommand = itk::SimpleMemberCommand<QmitkDicomView>::New();
  progressCommand->SetCallbackFunction( this, &QmitkDicomView::ReceiveProgressFromReaderThread );
  m_ProgressCallback = m_Reader->AddObserver( itk::ProgressEvent(), progressCommand );

  m_Reader->SetFilenames( filenames );

  m_Reader->MinimalContinuingUpdate();
}

void QmitkDicomView::ReceiveProgressFromReaderThread()
{
  // once through the event loop to end up in GUI thread
  emit SignalProgressFromReaderThread();
}

void QmitkDicomView::ReportProgressFromReader()
{
  mitk::DICOMSeriesReader::Progress progress = m_Reader->GetProgress();

  m_GUI->progressBar->setValue( progress.filesLoaded );

  if (progress.filesLoaded == progress.filesTotal)
  {
    this->SeriesLoadingCompleted();
  }
}

void QmitkDicomView::SeriesLoadingCompleted()
{
}
