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
#include "QmitkRenderWindow.h"

#include <mitkDataNodeFactory.h>
#include <mitkStandaloneDataStorage.h>

#include <itksys/SystemTools.hxx>

#include <QApplication>
#include "QmitkRenderScene.h"

// Load image (nrrd format) and display it in a 2D view
int main(int argc, char* argv[])
{
  QmitkRegisterClasses(); // I cannot believe this is still necessary :-(

  QApplication qtapplication( argc, argv );

  for (unsigned int number = 1; number <= atoi(argv[1]); ++number)
  {
    QmitkRenderScene* scene = new QmitkRenderScene;
    scene->setLabel( QString("Renderer ") + QString::number(number) );
    scene->show();
  }

  return qtapplication.exec();
}
