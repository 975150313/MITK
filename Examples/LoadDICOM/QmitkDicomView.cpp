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

  connect( m_GUI->btnNextSlice, SIGNAL(clicked()), this, SLOT(GoToPreviousSlice()) );
  connect( m_GUI->btnPreviousSlice, SIGNAL(clicked()), this, SLOT(GoToNextSlice()) );

  connect( this, SIGNAL(SignalProgressFromReaderThread()), this, SLOT(ReportProgressFromReader()), Qt::QueuedConnection );
  //connect( this, SIGNAL(SignalModifiedFromReaderThread()), this, SLOT(ProcessModifiedDataNode()), Qt::QueuedConnection );

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

template< typename T >
std::string int_to_hex( T i )
{
  std::stringstream stream;
  stream //<< "0x"
    << std::setfill ('0') << std::setw(4) << std::hex << i;
  return stream.str();
}

void QmitkDicomView::DumpImageTags( const mitk::DICOMImage* image )
{
  //image->PrintToStdOut(); // GDCM dump
  assert(image);

  mitk::DICOMDataset::TagList tags = image->GetTags();
  for (mitk::DICOMDataset::TagList::const_iterator tagIter = tags.begin();
       tagIter != tags.end();
       ++tagIter)
  {
    int gi = tagIter->first;
    int ei = tagIter->second;
    std::string gs = int_to_hex( gi );
    std::string es = int_to_hex( ei );
    std::string vrs = image->GetVR(gi,ei);
    std::string stringValue;
    double doubleValue;
    bool canBeString = image->GetAttributeValueAsString(gi,ei,stringValue);
    bool canBeDouble = image->GetAttributeValueAsDouble(gi,ei,doubleValue);
    std::cout << "(" << gs << "," << es << "): ";
    std::cout << "[" << vrs << "]: ";
    std::cout << " string:";
    if (canBeString)
      std::cout << "'" << stringValue << "'";
    else
      std::cout << "<no string>";

    std::cout << " double:";
    if (canBeDouble)
      std::cout << doubleValue;
    else
      std::cout << "<no double>";

    std::cout << std::endl;;
  }

  std::string imageType;
  if ( image->GetAttributeValueAsString(0x008, 0x008,imageType) )
  {
    std::cout << "Image Type: " << imageType << std::endl;
  }
  else
  {
    std::cout << "Image Type: <not readable>" << std::endl;
  }

  std::list<std::string> imageTypeComponents;
  if ( image->GetAttributeValueAsStrings(0x008, 0x008,imageTypeComponents) )
  {
    std::cout << "Image Type [array]: ";
    for ( std::list<std::string>::const_iterator i = imageTypeComponents.begin();
          i != imageTypeComponents.end();
          ++i )
    {
      std::cout << *i << " ";
    }
    std::cout << std::endl;
  }
  else
  {
    std::cout << "Image Type [array]: <not readable>" << std::endl;
  }

  double imageOrientationFirst;
  if ( image->GetAttributeValueAsDouble(0x020, 0x037,imageOrientationFirst) )
  {
    std::cout << "Image Orientation [first]: " << imageOrientationFirst << std::endl;
  }
  else
  {
    std::cout << "Image Type [first]: <not readable>" << std::endl;
  }


  std::list<double> imageOrientationPatient;
  if ( image->GetAttributeValueAsDoubles(0x020, 0x037,imageOrientationPatient) )
  {
    std::cout << "Image Orientation (Patient) [array]: ";
    for ( std::list<double>::const_iterator dimIter = imageOrientationPatient.begin();
          dimIter != imageOrientationPatient.end();
          ++dimIter )
    {
      std::cout << *dimIter << " ";
    }
    std::cout << std::endl;
  }
  else
  {
    std::cout << "Image Orientation (Patient): <not readable>" << std::endl;
  }
}


void QmitkDicomView::FirstSeriesLoadingResultAvailable()
{
  mitk::DICOMSeries::Pointer series = m_Reader->GetOutput(0);

  assert(series.IsNotNull());
  assert(series->GetGeometry() != 0);

  std::cout << "============== BEGIN First image attributes ================" << std::endl;
  mitk::DICOMImage::ConstPointer firstImage = series->GetAllDICOMImages().front();
  this->DumpImageTags(firstImage);
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
  this->ReinitViewToContainEverything(m_DataNode);
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
