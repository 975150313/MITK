#-----------------------------------------------------------------------------
# CTK
#-----------------------------------------------------------------------------

if(MITK_USE_CTK)

  # Sanity checks
  if(DEFINED CTK_DIR AND NOT EXISTS ${CTK_DIR})
    message(FATAL_ERROR "CTK_DIR variable is defined but corresponds to non-existing directory")
  endif()

  set(proj CTK)
  if (MITK_USE_DCMTK)
    set(proj_DEPENDENCIES DCMTK)
  else()
    set(proj_DEPENDENCIES)
  endif()

  set(CTK_DEPENDS ${proj})

  if(NOT DEFINED CTK_DIR)

    set(revision_tag fc06bedc) # Revision currently points to commit in https://github.com/kislinsk/CTK.git

    set(ctk_optional_cache_args )
    if(MITK_USE_Python)
      list(APPEND ctk_optional_cache_args
           -DCTK_LIB_Scripting/Python/Widgets:BOOL=ON
           -DCTK_ENABLE_Python_Wrapping:BOOL=ON
           -DCTK_APP_ctkSimplePythonShell:BOOL=ON
           -DPYTHON_EXECUTABLE:FILEPATH=${PYTHON_EXECUTABLE}
           -DPYTHON_INCLUDE_DIR:PATH=${PYTHON_INCLUDE_DIR}
           -DPYTHON_INCLUDE_DIR2:PATH=${PYTHON_INCLUDE_DIR2}
           -DPYTHON_LIBRARY:FILEPATH=${PYTHON_LIBRARY}
      )
    else()
      list(APPEND ctk_optional_cache_args
           -DCTK_LIB_Scripting/Python/Widgets:BOOL=OFF
           -DCTK_ENABLE_Python_Wrapping:BOOL=OFF
           -DCTK_APP_ctkSimplePythonShell:BOOL=OFF
      )
    endif()

    if(MITK_USE_DCMTK)
      list(APPEND ctk_optional_cache_args
        -DDCMTK_DIR:PATH=${DCMTK_DIR}
        -DCTK_LIB_DICOM/Widgets:BOOL=ON
      )
    else()
      list(APPEND ctk_optional_cache_args
        -DCTK_LIB_DICOM/Widgets:BOOL=OFF
      )
    endif()


    if(NOT MITK_USE_Python)
      list(APPEND ctk_optional_cache_args
        -DDCMTK_CMAKE_DEBUG_POSTFIX:STRING=d
      )
    endif()

    if(CTEST_USE_LAUNCHERS)
      list(APPEND ctk_optional_cache_args
        "-DCMAKE_PROJECT_${proj}_INCLUDE:FILEPATH=${CMAKE_ROOT}/Modules/CTestUseLaunchers.cmake"
      )
    endif()

    FOREACH(type RUNTIME ARCHIVE LIBRARY)
      IF(DEFINED CTK_PLUGIN_${type}_OUTPUT_DIRECTORY)
        LIST(APPEND mitk_optional_cache_args -DCTK_PLUGIN_${type}_OUTPUT_DIRECTORY:PATH=${CTK_PLUGIN_${type}_OUTPUT_DIRECTORY})
      ENDIF()
    ENDFOREACH()

    ExternalProject_Add(${proj}
      LIST_SEPARATOR ${sep}
      URL ${MITK_THIRDPARTY_DOWNLOAD_PREFIX_URL}/CTK_${revision_tag}.tar.gz
      URL_MD5 c500f54dd6d2ce2803d705cbb663d2f2
      # PATCH_COMMAND ${PATCH_COMMAND} -N -p1 -i ${CMAKE_CURRENT_LIST_DIR}/CTK.patch
      UPDATE_COMMAND ""
      INSTALL_COMMAND ""
      CMAKE_GENERATOR ${gen}
      CMAKE_ARGS
        ${ep_common_args}
        ${ctk_optional_cache_args}
        # The CTK PluginFramework cannot cope with
        # a non-empty CMAKE_DEBUG_POSTFIX for the plugin
        # libraries yet.
        -DCMAKE_DEBUG_POSTFIX:STRING=
        -DCTK_QT_VERSION:STRING=5
        -DQt5_DIR=${Qt5_DIR}
        -DGit_EXECUTABLE:FILEPATH=${GIT_EXECUTABLE}
        -DGIT_EXECUTABLE:FILEPATH=${GIT_EXECUTABLE}
        -DCTK_LIB_CommandLineModules/Backend/LocalProcess:BOOL=ON
        -DCTK_LIB_CommandLineModules/Frontend/QtGui:BOOL=ON
        -DCTK_LIB_PluginFramework:BOOL=ON
        -DCTK_LIB_Widgets:BOOL=ON
        -DCTK_LIB_XNAT/Core:BOOL=ON
        -DCTK_PLUGIN_org.commontk.eventadmin:BOOL=ON
        -DCTK_PLUGIN_org.commontk.configadmin:BOOL=ON
        -DCTK_USE_GIT_PROTOCOL:BOOL=OFF
        -DqRestAPI_URL:STRING=${MITK_THIRDPARTY_DOWNLOAD_PREFIX_URL}/qRestAPI_c5e4c2a7_patched.tar.gz
        -DPythonQt_URL:STRING=${MITK_THIRDPARTY_DOWNLOAD_PREFIX_URL}/PythonQt_e39be131.tar.gz # From https://github.com/kislinsk/PythonQt.git
      CMAKE_CACHE_ARGS
        ${ep_common_cache_args}
      CMAKE_CACHE_DEFAULT_ARGS
        ${ep_common_cache_default_args}
      DEPENDS ${proj_DEPENDENCIES}
     )

    ExternalProject_Get_Property(${proj} binary_dir)
    set(CTK_DIR ${binary_dir})

  else()

    mitkMacroEmptyExternalProject(${proj} "${proj_DEPENDENCIES}")

  endif()

endif()
