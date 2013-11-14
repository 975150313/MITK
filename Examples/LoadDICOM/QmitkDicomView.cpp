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
#include "mitkDICOMSortByTag.h"

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

  connect( m_GUI->btnNextSlice, SIGNAL(clicked()), this, SLOT(GoToPreviousSlice()) );
  connect( m_GUI->btnPreviousSlice, SIGNAL(clicked()), this, SLOT(GoToNextSlice()) );

  connect( this, SIGNAL(SignalProgressFromReaderThread()), this, SLOT(ReportProgressFromReader()), Qt::QueuedConnection );
  //connect( this, SIGNAL(SignalModifiedFromReaderThread()), this, SLOT(ProcessModifiedDataNode()), Qt::QueuedConnection );

  m_GUI->progressBar->setFormat("%v / %m files");

  mitk::DICOMSortByTag::Pointer instanceNumberSort = mitk::DICOMSortByTag::New();
  instanceNumberSort->SetTag(0x0020, 0x0013);
  m_SortCriteria[ m_GUI->radioInstanceNumber ] = instanceNumberSort.GetPointer();

  mitk::DICOMSortByTag::Pointer sliceLocationSort = mitk::DICOMSortByTag::New();
  sliceLocationSort->SetTag(0x0020, 0x1041);
  m_SortCriteria[ m_GUI->radioSliceLocation ] = sliceLocationSort.GetPointer();

  /*
  // we need real spatial sorting here.. and we need to describe how different orientations within a group are sorted
  mitk::DICOMSortByTag::Pointer ippSort = mitk::DICOMSortByTag::New();
  ippSort->SetTag(0x0020, 0x0032);
  m_SortCriteria[ m_GUI->radioImagePositionPatient ] = ippSort.GetPointer();
  */

  connect( m_GUI->radioInstanceNumber, SIGNAL(toggled(bool)), this, SLOT(SelectSortCriterion(bool)) );
  connect( m_GUI->radioSliceLocation, SIGNAL(toggled(bool)), this, SLOT(SelectSortCriterion(bool)) );
  connect( m_GUI->radioImagePositionPatient, SIGNAL(toggled(bool)), this, SLOT(SelectSortCriterion(bool)) );

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
  m_Scroller->SetEventConfig("DisplayConfigPACS.xml");

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
  //m_Reader->Update();

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

void QmitkDicomView::ReceiveModifiedFromReaderThread()
{
  // once through the event loop to end up in GUI thread
  //emit SignalModifiedFromReaderThread();

  QApplication::postEvent( this, new QmitkDicomViewUpdateEvent );
}

void QmitkDicomView::ProcessModifiedDataNode()
{
  this->UpdateToModifiedSeries();
}

void QmitkDicomView::DumpDataset( const mitk::DICOMDataset* dataset, const std::string& indent )
{
  dataset->PrintToStdOut(); // GDCM dump
}


void QmitkDicomView::FirstSeriesLoadingResultAvailable()
{
  mitk::DICOMSeries::Pointer series = m_Reader->GetOutput(0);

  assert(series.IsNotNull());
  assert(series->GetGeometry() != 0);

  std::cout << "============== BEGIN First image attributes ================" << std::endl;
  mitk::DICOMImage::ConstPointer firstImage = series->GetAllDICOMImages().front();
  this->DumpDataset(firstImage);
  std::cout << "============== END First image attributes ================" << std::endl;

  m_DataNode = mitk::DataNode::New();
  m_DataNode->SetData( series );
  m_DataNode->SetName( "DICOM series" );

  mitk::DICOMSeriesMapperVtk2D::Pointer mapper = mitk::DICOMSeriesMapperVtk2D::New();
  m_DataNode->SetMapper(mitk::BaseRenderer::Standard2D, mapper);

  itk::SimpleMemberCommand<QmitkDicomView>::Pointer modifiedCommand = itk::SimpleMemberCommand<QmitkDicomView>::New();
  modifiedCommand->SetCallbackFunction( this, &QmitkDicomView::ReceiveModifiedFromReaderThread );
  m_ModifiedCallback = series->AddObserver( itk::ModifiedEvent(), modifiedCommand );


  /*
  std::cout << "--------------- BEGIN DataNode for series ----------------" << std::endl;
  m_DataNode->Print( std::cout );
  std::cout << "--------------- Series ----------------" << std::endl;
  series->Print( std::cout );
  std::cout << "--------------- END Series ----------------" << std::endl;
  std::cout << "--------------- END DataNode for series ----------------" << std::endl;
  */

  m_DataStorage->Add( m_DataNode );

  this->UpdateToModifiedSeries();
}

bool QmitkDicomView::event( QEvent *event )
{
  if ( event->type() == (QEvent::Type) QmitkDicomViewUpdateEvent::Modified )
  {
    // Directly process all pending rendering requests
    this->UpdateToModifiedSeries();
    return true;
  }

  return QWidget::event(event);
}

void QmitkDicomView::UpdateToModifiedSeries()
{
  mitk::DICOMSeries::Pointer series = dynamic_cast<mitk::DICOMSeries*>( m_DataNode->GetData() );
  //MITK_INFO << "-------------- Series geometry -----------";
  //mitk::Geometry3D::Pointer geo = series->GetGeometry();
  //geo->Print(std::cout);
  this->ReinitViewToContainEverything(m_DataNode);
}

void QmitkDicomView::SeriesLoadingCompleted()
{
  //this->UpdateToModifiedSeries();
  this->ReinitViewToContainEverything(m_DataNode, true);
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

#include <time.h>

int diff_ms(timeval t1, timeval t2)
{
  return (((t1.tv_sec - t2.tv_sec) * 1000000) +
      (t1.tv_usec - t2.tv_usec))/1000;
}

void QmitkDicomView::ReinitViewToContainEverything(mitk::DataNode* node, bool really)
{

  if (!really)
  {
    static clock_t lasttime = 0;
    static clock_t minWaitTime = CLOCKS_PER_SEC * 1.0; // minimum a second before updating geometry again!
    if (clock() - lasttime < minWaitTime) return;
    lasttime = clock();
  }

  // TODO before updating, remember old position and reinit to view this position!
  bool oldPositionDefined = false;
  mitk::Point3D oldPosition;

  // Set geometry for NC
  if (node)
  {
    mitk::DICOMSeries* series = dynamic_cast<mitk::DICOMSeries*>( node->GetData() );

    if (series)
    {
      // basically a copy of RenderingManager::InternalViewInitialization
      mitk::BaseRenderer* baseRenderer = m_RenderWindow->GetRenderer();

      mitk::SliceNavigationController *nc = baseRenderer->GetSliceNavigationController();
      if (nc->GetCurrentPlaneGeometry())
      {
        oldPosition = nc->GetCurrentPlaneGeometry()->GetCenter();
        oldPositionDefined = true;
      }
      nc->SetViewDirection( mitk::SliceNavigationController::Original );

      mitk::DICOMSeries::MutexLocker locker( series->GetHighPriorityLock() );
      MITK_INFO << "Init to object";
      nc->SetInputWorldGeometry( node->GetData()->GetGeometry()->Clone() );
    }
  }
  else
  {
    mitk::BaseRenderer* baseRenderer = m_RenderWindow->GetRenderer();
    mitk::SliceNavigationController *nc = baseRenderer->GetSliceNavigationController();
    nc->SetViewDirectionToDefault();
    MITK_INFO << "Init to world";
    mitk::TimeSlicedGeometry::Pointer worldGeometry = m_DataStorage->ComputeBoundingGeometry3D( m_DataStorage->GetAll() );
    nc->SetInputWorldGeometry( worldGeometry );
  }
  mitk::BaseRenderer* baseRenderer = m_RenderWindow->GetRenderer();
  mitk::SliceNavigationController *nc = baseRenderer->GetSliceNavigationController();
  nc->Update();

  if (oldPositionDefined)
  {
    nc->SelectSliceByPoint( oldPosition );
  }
  else
  {
    nc->GetSlice()->SetPos( nc->GetSlice()->GetSteps() / 2 );
  }

  // Fit the render window DisplayGeometry
  baseRenderer->GetDisplayGeometry()->Fit();

  // Make sure the slice keeps moving
  nc->GetSlice()->SetAutoRepeat( true );

  mitk::RenderingManager::GetInstance()->RequestUpdateAll();

/*
  MITK_INFO << "-------------- World geometry -----------";
//  worldGeometry->Print(std::cout);
  MITK_INFO << "-------------- Current render geometry 2D -----------";
  baseRenderer->GetCurrentWorldGeometry2D()->Print(std::cout);
  MITK_INFO << "-------------- Current display geometry -----------";
  baseRenderer->GetDisplayGeometry()->Print(std::cout);
  MITK_INFO << "-------------- end -----------";
*/
}

void QmitkDicomView::GoToNextSlice()
{
  mitk::BaseRenderer* baseRenderer = m_RenderWindow->GetRenderer();
  mitk::SliceNavigationController *nc = baseRenderer->GetSliceNavigationController();
  nc->GetSlice()->Next();

  MITK_INFO << "NOW AT SLICE " << nc->GetSlice()->GetPos()+1 << " of " << nc->GetSlice()->GetSteps();
  MITK_INFO << "-------------- Current render geometry 2D -----------";
  baseRenderer->GetCurrentWorldGeometry2D()->Print(std::cout);
  MITK_INFO << "-------------- end -----------";
}

void QmitkDicomView::GoToPreviousSlice()
{
  mitk::BaseRenderer* baseRenderer = m_RenderWindow->GetRenderer();
  mitk::SliceNavigationController *nc = baseRenderer->GetSliceNavigationController();
  nc->GetSlice()->Previous();

  MITK_INFO << "NOW AT SLICE " << nc->GetSlice()->GetPos()+1 << " of " << nc->GetSlice()->GetSteps();
  MITK_INFO << "-------------- Current render geometry 2D -----------";
  baseRenderer->GetCurrentWorldGeometry2D()->Print(std::cout);
  MITK_INFO << "-------------- end -----------";
}

void QmitkDicomView::SelectSortCriterion(bool checked)
{
  if (checked)
  {
    mitk::DICOMSeriesSortCriterion::ConstPointer criterion = m_SortCriteria[sender()];

    mitk::DICOMSeries::Pointer series = dynamic_cast<mitk::DICOMSeries*>( m_DataNode->GetData() );

    if (criterion.IsNotNull() && series.IsNotNull())
    {
      MITK_INFO << "Change sorting to mode defined by " << qPrintable( sender()->objectName() );
      series->SetSortCriterion( criterion );
      series->Print( std::cout );
      this->ReinitViewToContainEverything( m_DataNode, true );
    }
  }
}
