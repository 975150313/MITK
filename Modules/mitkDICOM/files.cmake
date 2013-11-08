set(H_FILES
  mitkDICOMDataset.h
  mitkDICOMImage.h
  mitkDICOMSeries.h

  mitkDICOMSeriesReader.h
  mitkDICOMImageReader.h

  mitkDICOMSeriesMapperVtk2D.h
)

set(CPP_FILES
  DataManagement/mitkDICOMDataset.cpp
  DataManagement/mitkDICOMImage.cpp
  DataManagement/mitkDICOMSeries.cpp
  DataManagement/mitkDICOMSeriesSortCriterion.cpp
  DataManagement/mitkDICOMSortByTag.cpp

  IO/mitkDICOMSeriesReader.cpp
  IO/mitkDICOMImageReader.cpp

  Rendering/mitkDICOMSeriesMapperVtk2D.cpp

# module internal classes
  DataManagement/mitkDICOMDatasetGDCM.cpp
)
