# Build only if DCMQI is available
if (NOT MITK_USE_DCMQI)
  return()
endif()

MITK_CREATE_MODULE( DICOMQIIO
  DEPENDS MitkMultilabel
  PACKAGE_DEPENDS
    PRIVATE DCMQI DCMTK
  AUTOLOAD_WITH MitkCore
)
