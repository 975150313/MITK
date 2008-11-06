SET(CPP_FILES
  manifest.cpp
  cherryLogView.cpp
  cherryQtControlWidget.cpp
  cherryQtEditorPart.cpp
  cherryQtLogView.cpp
  cherryQtUIPlugin.cpp
  cherryQtViewPart.cpp
  cherryQtWorkbenchPageTweaklet.cpp
  cherryQtWorkbenchTweaklet.cpp
  
  # application
  application/cherryQtWorkbenchAdvisor.cpp
)

SET(INTERNAL_CPP_FILES

  defaultpresentation/cherryEmptyTabFolder.cpp
  defaultpresentation/cherryEmptyTabItem.cpp
  defaultpresentation/cherryNativeTabFolder.cpp
  defaultpresentation/cherryNativeTabItem.cpp
  defaultpresentation/cherryQCTabBar.cpp
  defaultpresentation/cherryQtWorkbenchPresentationFactory.cpp
  
  util/cherryAbstractTabFolder.cpp
  util/cherryLeftToRightTabOrder.cpp
  util/cherryPartInfo.cpp
  util/cherryPresentablePartFolder.cpp
  util/cherryReplaceDragHandler.cpp
  util/cherryTabbedStackPresentation.cpp
  util/cherryTabFolderEvent.cpp

  cherryQtDnDTweaklet.cpp
  cherryQtErrorView.cpp
  cherryQtMainWindowShell.cpp
  cherryQtSash.cpp
  cherryQtShowViewAction.cpp
  cherryQtTracker.cpp
  cherryQtPlatformLogModel.cpp
  cherryQtShowViewDialog.cpp
  cherryQtWidgetsTweaklet.cpp
)

SET(MOC_H_FILES
  cherryQtControlWidget.h
  
  internal/defaultpresentation/cherryNativeTabFolder.h
  internal/defaultpresentation/cherryQCTabBar.h
  
  internal/cherryQtMainWindowShell.h
  internal/cherryQtSash.h
  internal/cherryQtShowViewAction.h
  internal/cherryQtTracker.h
  internal/cherryQtWidgetsTweaklet.h
)

SET(UI_FILES
  internal/cherryQtShowViewDialog.ui
  cherryQtLogView.ui
)

SET(RES_FILES
  resources.qrc
)

foreach(file ${INTERNAL_CPP_FILES})
  SET(CPP_FILES ${CPP_FILES} internal/${file})
endforeach(file ${INTERNAL_CPP_FILES})