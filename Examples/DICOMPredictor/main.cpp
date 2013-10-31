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

#include <QtCore>

#include "DICOMPredictorUI.h"

int main(int argc, char* argv[])
{
  QApplication qtapplication( argc, argv );
  DICOMPredictorUI ui;
  ui.show();
  return qtapplication.exec();
}
