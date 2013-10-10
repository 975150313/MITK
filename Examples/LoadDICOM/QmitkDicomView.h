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


class QmitkDicomView : public QWidget
{
  Q_OBJECT

  public:

    QmitkDicomView( QWidget* parent = 0, Qt::WindowFlags f = 0 );
    ~QmitkDicomView();

  signals:

    void SignalProgressFromReaderThread();

  public slots:

    void ReceiveProgressFromReaderThread();

  protected slots:

    void LoadDICOMFiles();
    void LoadSomething();
    void SeriesLoadingCompleted();

    // callback
    void ReportProgressFromReader();

    void ReinitViewToContainEverything();

  protected:

    void SetupRendering();

    Ui::QmitkDicomView* m_GUI;

    mitk::DICOMSeriesReader::Pointer m_Reader;

    mitk::DataStorage::Pointer m_DataStorage;
    mitk::RenderingManager::Pointer m_RenderingManager;
    QmitkRenderWindow* m_RenderWindow;
    mitk::DisplayInteractor::Pointer m_Scroller;

    unsigned long m_ProgressCallback;
};

#endif
