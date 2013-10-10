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

#include "QmitkRegisterClasses.h"

#include <QApplication>
#include "QmitkDicomView.h"

int main(int argc, char* argv[])
{
  QmitkRegisterClasses(); // I cannot believe this is still necessary :-(

  QApplication qtapplication( argc, argv );
  QmitkDicomView scene;
  scene.show();

  return qtapplication.exec();
}
