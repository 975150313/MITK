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
    void SeriesLoadingCompleted();

    // callback
    void ReportProgressFromReader();

  protected:

    Ui::QmitkDicomView* m_GUI;

    mitk::DICOMSeriesReader::Pointer m_Reader;

    unsigned long m_ProgressCallback;
};

#endif
