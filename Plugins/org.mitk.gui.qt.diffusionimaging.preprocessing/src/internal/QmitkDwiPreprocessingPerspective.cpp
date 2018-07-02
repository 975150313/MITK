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

#include "QmitkDwiPreprocessingPerspective.h"
#include "berryIViewLayout.h"

void  QmitkDwiPreprocessingPerspective::CreateInitialLayout(berry::IPageLayout::Pointer layout)
{
    /////////////////////////////////////////////////////
    // all di-app perspectives should have the following:
    /////////////////////////////////////////////////////

    QString editorArea = layout->GetEditorArea();

    layout->AddStandaloneViewPlaceholder("org.mitk.views.viewnavigatorview", berry::IPageLayout::LEFT, 0.3f, editorArea, false);

    layout->AddStandaloneView("org.mitk.views.datamanager",
                              false, berry::IPageLayout::LEFT, 0.3f, editorArea);

    layout->AddStandaloneView("org.mitk.views.controlvisualizationpropertiesview",
                              false, berry::IPageLayout::BOTTOM, .15f, "org.mitk.views.datamanager");

    berry::IFolderLayout::Pointer left =
            layout->CreateFolder("org.mbi.diffusionimaginginternal.leftcontrols",
                                 berry::IPageLayout::BOTTOM, 0.15f, "org.mitk.views.controlvisualizationpropertiesview");

    layout->AddStandaloneViewPlaceholder("org.mitk.views.imagenavigator",
                                         berry::IPageLayout::BOTTOM, .7f, "org.mbi.diffusionimaginginternal.leftcontrols", false);

    /////////////////////////////////////////////
    // here goes the perspective specific stuff
    /////////////////////////////////////////////

    left->AddView("org.mitk.views.diffusionpreprocessing");
    left->AddView("org.mitk.views.denoisingview");
    left->AddView("org.mitk.views.simpleregistrationview");
    left->AddView("org.mitk.views.headmotioncorrectionview");
//    left->AddView("org.mitk.views.brainextraction");
}