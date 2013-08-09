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

#ifndef QmitkRenderScene_h
#define QmitkRenderScene_h

#include <QtGui>

#include "mitkDisplayInteractor.h"
#include "mitkDataStorage.h"

namespace Ui
{
  class QmitkRenderScene;
}

namespace mitk
{
  class Geometry3D;
}

class QmitkRenderWindow;


/**
  \brief Rendering scene demonstrator.

  This widget is meant to test rendering performance of QmitkRenderWindow.
  The user is able to select a desired frame rate, load data for visualization,
  and get feedback on the achieved frame rate.

  \section s1 Loading data

  The "Load" buttons, including the presets below "Load data..." will
  all setup a rendering scene, i.e.:
   - create a new DataStorage, fill it with selected data
   - setup a QmitkRenderWindow to display this data
     - initializing a world geometry as RenderingManager would usually do (slices that match DataStorage contents)
     - using a new RenderingManager for just this render window

  Three preset buttons are meant to quickly load data from the MITK-Data repository,
  which is checked out in a super-build setup, thus found/described automatically by CMake.

  Preset 1: 2D DICOM image (512x512), some PlanarFigure objects representing measurements on top
  Preset 2: as preset 1, BUT the image is displayed as a plane in a 3D scene (which currently requires a hidded second render window to be setup)
  Preset 3: loads a colored 2D image (640x480) in a 2D window

  To get a different setup with a 2D plane in 3D space, it is easiest to exchange data in the MITK-Data repository checkout...

  \section s2 Rendering

  Rendering is triggered by a QTimer. Each timeout() signal leads to a forced rendering update.
  Elapsed time and rendered frames are both counted and reported to evaluate performance.

  A number of checkboxes are evaluated during loading and rendering to demonstrate several effects:
   - Scroll each frame: forwards the stepper of the associated SliceNavigationController, i.e. scrolls
     - this forces rendering to resample something new
   - Modify data each frame: marks all data modified (by calling Modified()) each frame
     - this forces rendering to do everything all over, simulating live data
   - Write text: enables display of PlanarFigure names, which have influence on the rendering time
   - Contours: decide between binary mask or contour rendering for binary images (contours being generally slower)

   \section s3 Class status

   This is just meant for demonstration of rendering, not for demonstration of nice code style,
   good structure or recommended use. Some parts are documented, some copied from other places,
   it is work in progress.

*/
class QmitkRenderScene : public QWidget
{
  Q_OBJECT

  public:

    QmitkRenderScene( QWidget* parent = 0, Qt::WindowFlags f = 0 );
    ~QmitkRenderScene();

    void setLabel( const QString& name );

  public slots:

    void LoadUserData();
    void LoadData(const QStringList& filenames);
    void LoadDataSet1();
    void LoadDataSet2();
    void LoadDataSet3();

    void ProgressAndRender();

    void SetUpdateRate(int hertz);

    void ResetFrameCounter();

    void ReDecorateNodes();

  protected:

    void InitRenderWindowToWorld2D( QmitkRenderWindow* renderWindow, mitk::Geometry3D* worldGeometry );
    void InitRenderWindowToWorld3D( QmitkRenderWindow* renderWindow, mitk::Geometry3D* worldGeometry );

    mitk::DataStorage::Pointer DecorateNodes( mitk::DataStorage* dataStorage );

    Ui::QmitkRenderScene* m_GUI;

    mitk::DisplayInteractor::Pointer m_Scroller;

    QTimer* m_RenderingUpdateTimer;
    QElapsedTimer* m_RenderingDurationTimer;
    unsigned int m_FrameCounter;

    QmitkRenderWindow* m_RenderWindow;
    QmitkRenderWindow* m_HiddenRenderWindow;
};

#endif
