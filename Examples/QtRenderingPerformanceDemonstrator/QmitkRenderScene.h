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
