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

#ifndef QmitkDicomView_h
#define QmitkDicomView_h

#include "mitkDICOMSeriesReader.h"

#include "mitkRenderingManager.h"
#include "mitkStandaloneDataStorage.h"
#include "mitkDisplayInteractor.h"

#include "QmitkRenderWindow.h"

#include <QtGui>

namespace Ui
{
  class QmitkDicomView;
}

class QmitkRenderWindow;

class QmitkDicomViewUpdateEvent : public QEvent
{
public:
  enum Type
  {
    Modified = QEvent::MaxUser - 42
  };

  QmitkDicomViewUpdateEvent()
  : QEvent( (QEvent::Type) Modified ) {};
};



class QmitkDicomView : public QWidget
{
  Q_OBJECT

  public:

    QmitkDicomView( QWidget* parent = 0, Qt::WindowFlags f = 0 );
    ~QmitkDicomView();

  signals:

    void SignalProgressFromReaderThread();
    void SignalModifiedFromReaderThread();

  public slots:

    void ReceiveProgressFromReaderThread();
    void ReceiveModifiedFromReaderThread();

  protected slots:

    void LoadDICOMFiles();
    void LoadSomething();
    void FirstSeriesLoadingResultAvailable();
    void UpdateToModifiedSeries();
    void SeriesLoadingCompleted();

    void GoToNextSlice();
    void GoToPreviousSlice();

    // callback
    void ReportProgressFromReader();
    void ProcessModifiedDataNode();

    void ReinitViewToContainEverything(mitk::DataNode* node = NULL, bool really = false);

  protected:

    bool event( QEvent *event );

    void SetupRendering();

    Ui::QmitkDicomView* m_GUI;

    mitk::DICOMSeriesReader::Pointer m_Reader;

    mitk::DataStorage::Pointer m_DataStorage;
    mitk::DataNode::Pointer m_DataNode;
    mitk::RenderingManager::Pointer m_RenderingManager;
    QmitkRenderWindow* m_RenderWindow;
    mitk::DisplayInteractor::Pointer m_Scroller;

    unsigned long m_ProgressCallback;
    unsigned long m_ModifiedCallback;
};

#endif
