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

int main(int argc, char* argv[])
{
  QmitkRegisterClasses(); // I cannot believe this is still necessary :-(

  if (argc != 2)
  {
    MITK_ERROR << "Usage: " << argv[0] << " <number of windows>\n"
                  "\n"
                  "Displays <number of windows> QmitkRenderWindows\n"
                  "with options to load and render data.\n";
                  "\n"
                  "See source of QmitkRenderScene.h for details.";
  }

  QApplication qtapplication( argc, argv );

  for (unsigned int number = 1; number <= atoi(argv[1]); ++number)
  {
    QmitkRenderScene* scene = new QmitkRenderScene;
    scene->setLabel( QString("Renderer ") + QString::number(number) );
    scene->show();
  }

  return qtapplication.exec();
}
