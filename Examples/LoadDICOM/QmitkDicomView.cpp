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

#include "mitkDataNode.h"
#include "mitkIOUtil.h"
#include "mitkDisplayInteractor.h"
#include "mitkDICOMSeriesMapperVtk2D.h"

#include "usGetModuleContext.h"

#include <itkCommand.h>

QmitkDicomView::QmitkDicomView(QWidget* parent, Qt::WindowFlags f)
: QWidget(parent, f)
, m_GUI( new Ui::QmitkDicomView )
, m_Scroller( mitk::DisplayInteractor::New() )
{
  m_GUI->setupUi(this);

  connect( m_GUI->btnLoadDICOM, SIGNAL(clicked()), this, SLOT(LoadDICOMFiles()) );
  connect( m_GUI->btnLoadSth, SIGNAL(clicked()), this, SLOT(LoadSomething()) );

  connect( this, SIGNAL(SignalProgressFromReaderThread()), this, SLOT(ReportProgressFromReader()), Qt::QueuedConnection );

  m_GUI->progressBar->setFormat("%v / %m files");

  this->SetupRendering();
}

void QmitkDicomView::SetupRendering()
{
  static bool twice = false;
  if (twice) exit(0);
  m_DataStorage = mitk::StandaloneDataStorage::New().GetPointer();

  m_RenderingManager = mitk::RenderingManager::New();
  m_RenderingManager->SetDataStorage( m_DataStorage );
  m_RenderWindow = new QmitkRenderWindow( m_GUI->renderWindowContainer, "", NULL, m_RenderingManager );
  m_GUI->renderWindowContainer->layout()->addWidget( m_RenderWindow );
  m_RenderWindow->show();

  m_Scroller->LoadStateMachine("DisplayInteraction.xml");
  m_Scroller->SetEventConfig("DisplayConfigMITK.xml");

  us::GetModuleContext()->RegisterService<mitk::InteractionEventObserver>( m_Scroller.GetPointer() );

  twice = true;
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

  this->FirstSeriesLoadingResultAvailable();
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

void QmitkDicomView::FirstSeriesLoadingResultAvailable()
{
  mitk::DICOMSeries::Pointer series = m_Reader->GetOutput(0);

  mitk::DataNode::Pointer node = mitk::DataNode::New();
  node->SetData( series );
  node->SetName( "DICOM series" );

  mitk::DICOMSeriesMapperVtk2D::Pointer mapper = mitk::DICOMSeriesMapperVtk2D::New();
  node->SetMapper(mitk::BaseRenderer::Standard2D, mapper);

  m_DataStorage->Add( node );
}

void QmitkDicomView::SeriesLoadingCompleted()
{
}

void QmitkDicomView::LoadSomething()
{
  QStringList filenames;
  filenames << "/RAID/home/maleike/Pic3D.nrrd";

  std::vector< std::string > stl_filenames;
  foreach (QString filename, filenames)
  {
    stl_filenames.push_back( qPrintable( filename) );
  }

  mitk::DataStorage::Pointer storage = mitk::IOUtil::LoadFiles( stl_filenames );
  mitk::DataStorage::SetOfObjects::ConstPointer objects = storage->GetAll();
  for ( mitk::DataStorage::SetOfObjects::const_iterator iter = objects->begin();
      iter != objects->end();
      ++iter )
  {
    m_DataStorage->Add( *iter );
  }

  this->ReinitViewToContainEverything();
}

void QmitkDicomView::ReinitViewToContainEverything()
{
  mitk::TimeSlicedGeometry::Pointer worldGeometry = m_DataStorage->ComputeBoundingGeometry3D( m_DataStorage->GetAll() );

  // basically a copy of RenderingManager::InternalViewInitialization
  mitk::BaseRenderer* baseRenderer = m_RenderWindow->GetRenderer();
  mitk::SliceNavigationController *nc = baseRenderer->GetSliceNavigationController();

  // Re-initialize view direction
  nc->SetViewDirectionToDefault();

  // Set geometry for NC
  nc->SetInputWorldGeometry( worldGeometry );
  nc->Update();

  nc->GetSlice()->SetPos( nc->GetSlice()->GetSteps() / 2 );

  // Fit the render window DisplayGeometry
  baseRenderer->GetDisplayGeometry()->Fit();

  // Make sure the slice keeps moving
  nc->GetSlice()->SetAutoRepeat( true );
}
