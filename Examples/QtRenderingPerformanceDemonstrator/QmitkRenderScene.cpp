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

#include "QmitkRenderScene.h"

#include "mitkIOUtil.h"

#include "QmitkRenderWindow.h"

#include <QElapsedTimer>

#include "ui_QmitkRenderScene.h"

#include "mitkModule.h"
#include "mitkModuleRegistry.h"
#include "mitkGetModuleContext.h"
#include <usModuleInitialization.h>

#include "mitkPlanarFigure.h"
#include "mitkNodePredicateDataType.h"
#include "mitkGeometry2DDataVtkMapper3D.h"
#include "mitkWeakPointerProperty.h"


#include "mitkTestDataLocation.h" // generated


//US_INITIALIZE_MODULE("QtRenderingPerformanceDemonstrator", "QtRenderingPerformanceDemonstrator", "", "0.1")

QmitkRenderScene::QmitkRenderScene(QWidget* parent, Qt::WindowFlags f)
: QWidget(parent, f)
, m_GUI( new Ui::QmitkRenderScene )
, m_Scroller( mitk::DisplayInteractor::New() )
, m_RenderingUpdateTimer( new QTimer(this) )
, m_RenderingDurationTimer( new QElapsedTimer() )
, m_RenderWindow(NULL)
, m_HiddenRenderWindow(NULL)
{
  m_GUI->setupUi(this);

  /*
  m_Scroller->LoadStateMachine("DisplayInteraction.xml");
  m_Scroller->SetEventConfig("DisplayConfigMITK.xml");

  mitk::ModuleContext* context = mitk::GetModuleContext();
  context->RegisterService<mitk::InteractionEventObserver>( m_Scroller.GetPointer() );
  */

  connect(m_GUI->btnLoad, SIGNAL(clicked()), this, SLOT(LoadUserData()));
  connect(m_GUI->btnData1, SIGNAL(clicked()), this, SLOT(LoadDataSet1()));
  connect(m_GUI->btnData2, SIGNAL(clicked()), this, SLOT(LoadDataSet2()));
  connect(m_GUI->btnData3, SIGNAL(clicked()), this, SLOT(LoadDataSet3()));

  connect(m_RenderingUpdateTimer, SIGNAL(timeout()), this, SLOT(ProgressAndRender()));
  connect(m_GUI->spnDesiredRenderRate, SIGNAL(valueChanged(int)), this, SLOT(SetUpdateRate(int)));
  connect(m_GUI->spnDesiredRenderRate, SIGNAL(valueChanged(int)), this, SLOT(ResetFrameCounter()));
  this->SetUpdateRate( m_GUI->spnDesiredRenderRate->value() );

  connect(m_GUI->btnStartRendering, SIGNAL(clicked()), m_RenderingUpdateTimer, SLOT(start()));
  connect(m_GUI->btnStartRendering, SIGNAL(clicked()), this, SLOT(ResetFrameCounter()));

  connect(m_GUI->btnStopRendering, SIGNAL(clicked()), m_RenderingUpdateTimer, SLOT(stop()));

  connect(m_GUI->chkWriteSimpleText, SIGNAL(clicked()), this, SLOT(ReDecorateNodes()));
  connect(m_GUI->chkBinaryContours, SIGNAL(clicked()), this, SLOT(ReDecorateNodes()));
}

QmitkRenderScene::~QmitkRenderScene()
{
  // delete only what is not deleted by Qt automatically via parent-child relations
  delete m_RenderingDurationTimer;
}

void QmitkRenderScene::ResetFrameCounter()
{
  m_FrameCounter = 0;
  m_RenderingDurationTimer->start();
  m_RenderingDurationTimer->restart();
}

void QmitkRenderScene::SetUpdateRate(int hertz)
{
  m_RenderingUpdateTimer->setInterval( static_cast<int>( 1000.0 / hertz ) );
}

void QmitkRenderScene::setLabel( const QString& name )
{
  m_GUI->label->setText(name);
}

void QmitkRenderScene::LoadUserData()
{
  MITK_INFO << "MITK test data expected in " << mitk::MITK_DATA_DIR;

  QStringList filenames = QFileDialog::getOpenFileNames(this, tr("Choose files...") );

  LoadData(filenames);
}

void QmitkRenderScene::LoadData(const QStringList& filenames)
{
  std::vector< std::string > stl_filenames;
  foreach (QString filename, filenames)
  {
    stl_filenames.push_back( qPrintable( filename) );
  }

  mitk::DataStorage::Pointer datastorage = DecorateNodes( mitk::IOUtil::LoadFiles( stl_filenames ) );

  mitk::RenderingManager::Pointer renderingManager = mitk::RenderingManager::New();
  renderingManager->SetDataStorage( datastorage );
  if (m_RenderWindow)
  {
    m_RenderWindow->deleteLater();
  }
  m_RenderWindow = new QmitkRenderWindow( m_GUI->renderWindowContainer, "some name", NULL, renderingManager );
  m_GUI->renderWindowContainer->layout()->addWidget( m_RenderWindow );
  m_RenderWindow->show();

  // Initialize the RenderWindow
  mitk::TimeSlicedGeometry::Pointer worldGeometry = datastorage->ComputeBoundingGeometry3D( datastorage->GetAll() );
  InitRenderWindowToWorld2D( m_RenderWindow, worldGeometry );

  // Make sure the slice keeps moving
  mitk::SliceNavigationController::Pointer snc = m_RenderWindow->GetSliceNavigationController();
  snc->GetSlice()->SetPos( 0 );
  snc->GetSlice()->SetAutoRepeat( true );
}

void QmitkRenderScene::ProgressAndRender()
{
  // forward scroll one slice if requested
  if (m_GUI->chkScroll->isChecked() )
  {
    m_RenderWindow->GetSliceNavigationController()->GetSlice()->Next();
    if (m_HiddenRenderWindow)
      m_HiddenRenderWindow->GetSliceNavigationController()->GetSlice()->Next();
  }

  // mark all data modified if requested
  if (m_GUI->chkModifyData->isChecked() )
  {
    mitk::DataStorage::SetOfObjects::ConstPointer objects = m_RenderWindow->GetRenderer()->GetDataStorage()->GetAll();
    for ( mitk::DataStorage::SetOfObjects::const_iterator iter = objects->begin();
          iter != objects->end();
          ++iter )
    {
      mitk::DataNode* node = *iter;
      if (node)
      {
        mitk::BaseData* data = node->GetData();
        if (data)
        {
          data->Modified();
        }
      }
    }
  }

  if (m_FrameCounter % 30 == 0) this->ResetFrameCounter(); // clear every now and then to see current rates

  // measure time
  QElapsedTimer frameTimer;
  frameTimer.start();

  // and render...
  if (m_HiddenRenderWindow)
  m_HiddenRenderWindow->GetRenderer()->ForceImmediateUpdate();
  m_RenderWindow->GetRenderer()->ForceImmediateUpdate();
  m_FrameCounter += 1;

  // report time
  m_GUI->achievedRenderingDuration->setText( QString::number( frameTimer.elapsed() ) );

  m_GUI->achievedRenderingRate->setText( QString::number( m_FrameCounter * 1000.0 / m_RenderingDurationTimer->elapsed(), 'f', 0 ) );
}

void QmitkRenderScene::InitRenderWindowToWorld2D( QmitkRenderWindow* renderWindow, mitk::Geometry3D* worldGeometry )
{
  // basically a copy of RenderingManager::InternalViewInitialization
  mitk::BaseRenderer* baseRenderer = renderWindow->GetRenderer();
  mitk::SliceNavigationController *nc = baseRenderer->GetSliceNavigationController();

  // Re-initialize view direction
  nc->SetViewDirectionToDefault();

  // Set geometry for NC
  nc->SetInputWorldGeometry( worldGeometry );
  nc->Update();

  nc->GetSlice()->SetPos( nc->GetSlice()->GetSteps() / 2 );

  // Fit the render window DisplayGeometry
  baseRenderer->GetDisplayGeometry()->Fit();
  //baseRenderer->GetCameraController()->SetViewToAnterior(); // ??? irritating term, should be axial or at least head, no?
}

void QmitkRenderScene::InitRenderWindowToWorld3D( QmitkRenderWindow* renderWindow, mitk::Geometry3D* worldGeometry )
{
  // basically a copy of RenderingManager::InternalViewInitialization
  mitk::BaseRenderer* baseRenderer = renderWindow->GetRenderer();
  mitk::SliceNavigationController *nc = baseRenderer->GetSliceNavigationController();

  // Re-initialize view direction
  nc->SetViewDirectionToDefault();

  // Set geometry for NC
  nc->SetInputWorldGeometry( worldGeometry );
  nc->Update();

  // Fit the render window DisplayGeometry
  baseRenderer->GetDisplayGeometry()->Fit();
  baseRenderer->GetCameraController()->SetViewToCranial();
}


mitk::DataStorage::Pointer QmitkRenderScene::DecorateNodes(mitk::DataStorage* storage)
{
  mitk::DataStorage::SetOfObjects::ConstPointer objects = storage->GetAll();
  for ( mitk::DataStorage::SetOfObjects::const_iterator iter = objects->begin();
      iter != objects->end();
      ++iter )
  {
    mitk::DataNode* node = *iter;
    if (node)
    {
      mitk::BaseData* data = node->GetData();
      if (data)
      {
        if (dynamic_cast<mitk::Image*>(data))
        {
          node->SetProperty("layer", mitk::IntProperty::New(1));
          node->SetProperty("outline binary", mitk::BoolProperty::New( m_GUI->chkBinaryContours->isChecked() ));
          //node->SetProperty("volumerendering", mitk::BoolProperty::New( true ));
        }
        else
        if (dynamic_cast<mitk::Surface*>(data))
        {
          node->SetProperty("layer", mitk::IntProperty::New(10));
          node->SetProperty("color", mitk::ColorProperty::New(0,1,0));
        }
        else
        if (dynamic_cast<mitk::PlanarFigure*>(data))
        {
          node->SetProperty("layer", mitk::IntProperty::New(20));
          node->SetProperty("color", mitk::ColorProperty::New(1,0,0));
          node->SetProperty("drawquantities", mitk::BoolProperty::New( m_GUI->chkWriteSimpleText->isChecked() ));
          if (m_GUI->chkWriteSimpleText->isChecked())
          {
            node->SetName("measured");
          }
          else
          {
            node->SetName("");
          }
        }
        else
        {
        }
      }
    }
  }

  return storage;
}

void QmitkRenderScene::LoadDataSet1()
{
  MITK_INFO << "MITK test data expected in " << mitk::MITK_DATA_DIR;
  QString root = QString::fromStdString(mitk::MITK_DATA_DIR);

  QStringList filenames;
  filenames << root + "/RenderingTestData/viewport/input_Square/SCSFREN.dcm"
            << root + "/RenderingTestData/PlanarFigures/FourPointAngle1.pf"
            << root + "/RenderingTestData/PlanarFigures/Line1.pf"
            << root + "/RenderingTestData/PlanarFigures/Path1.pf"
            << root + "/RenderingTestData/PlanarFigures/Polygon1.pf"
            << root + "/RenderingTestData/PlanarFigures/Rectangle1.pf";

  LoadData(filenames);
}
/*
  3D: 2D plane

  checkbox: planarfigures: quantities/names on/off (textactor, writesimpletext)
  checkbox: binary image: contours vs mask, opacity
  data: oblique slices
*/

void QmitkRenderScene::LoadDataSet2()
{
  LoadDataSet1(); // 2D image with some measurements
  ProgressAndRender();

  // now hide this render window, create a second 3D render window, add the image geometry as plane
  m_HiddenRenderWindow = m_RenderWindow;
  m_HiddenRenderWindow->hide();

  /* add GeometryData object */
  mitk::DataStorage::Pointer datastorage = m_HiddenRenderWindow->GetRenderer()->GetDataStorage();
  mitk::TNodePredicateDataType<mitk::Image>::Pointer isImage = mitk::TNodePredicateDataType<mitk::Image>::New();
  mitk::DataNode::Pointer imageNode = datastorage->GetSubset( isImage )->front();
  mitk::Image::Pointer image = dynamic_cast<mitk::Image*>(imageNode->GetData());
  if (image.IsNull())
  {
    MITK_ERROR << "wrong assumptions, didn't find image";
  }

  mitk::Geometry2DData::Pointer planeObject = mitk::Geometry2DData::New();
  mitk::TimeSlicedGeometry::Pointer tsg = dynamic_cast<mitk::TimeSlicedGeometry*>(image->GetGeometry() );
  mitk::SlicedGeometry3D::Pointer sg3 = dynamic_cast<mitk::SlicedGeometry3D*>(image->GetGeometry());
  if (sg3.IsNotNull())
  {
    mitk::Geometry2D::Pointer g2d = sg3->GetGeometry2D(0);
    planeObject->SetGeometry( g2d );
  }


  /* add a new renderwindow */
  m_RenderWindow = new QmitkRenderWindow( m_GUI->renderWindowContainer, "2D window", NULL, m_HiddenRenderWindow->GetRenderer()->GetRenderingManager() );
  m_GUI->renderWindowContainer->layout()->addWidget( m_RenderWindow );
  m_RenderWindow->show();
  m_RenderWindow->GetRenderer()->SetMapperID( mitk::BaseRenderer::Standard3D );

  mitk::DataNode::Pointer planeNode = mitk::DataNode::New();
  planeNode->SetData( planeObject );
  datastorage->Add( planeNode );
  mitk::Mapper* mapper3D = planeNode->GetMapper( mitk::BaseRenderer::Standard3D );
  if (mapper3D)
  {
    if (mitk::Geometry2DDataVtkMapper3D* gmapper3D = dynamic_cast<mitk::Geometry2DDataVtkMapper3D*>(mapper3D))
    {
      MITK_INFO << "Fixing datastorage for plane renderer...";
      gmapper3D->SetDataStorageForTexture( datastorage );
    }
  }

  // this must be done in some hidden place if we use default setups ??
  planeNode->SetProperty("renderer", mitk::WeakPointerProperty::New( m_HiddenRenderWindow->GetRenderer() ));

  // Initialize the RenderWindow
  mitk::TimeSlicedGeometry::Pointer worldGeometry = datastorage->ComputeBoundingGeometry3D( datastorage->GetAll() );
  InitRenderWindowToWorld3D( m_RenderWindow, worldGeometry );
}

void QmitkRenderScene::LoadDataSet3()
{
  MITK_INFO << "MITK test data expected in " << mitk::MITK_DATA_DIR;
  QString root = QString::fromStdString(mitk::MITK_DATA_DIR);

  QStringList filenames;
  filenames << root + "/NrrdWritingTestImage.jpg";

  LoadData(filenames);
}

void QmitkRenderScene::ReDecorateNodes()
{
  DecorateNodes( m_RenderWindow->GetRenderer()->GetDataStorage() );
}
