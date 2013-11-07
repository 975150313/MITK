
MITK_CREATE_MODULE( mitkDICOM
  INTERNAL_INCLUDE_DIRS DataManagement IO Rendering Testing
  DEPENDS Mitk
  PACKAGE_DEPENDS DCMTK
)

add_subdirectory(Testing)
