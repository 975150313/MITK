MITK_CREATE_MODULE_TESTS()

set(DICOM_TEST_IMAGE_ROOT "/home/maleike/DICOM-Testbilder")

# simple plane image (charset test)
mitkAddCustomModuleTest(mitkDICOMImageReaderTest_SCSARAB mitkDICOMImageReaderTest 1 ${DICOM_TEST_IMAGE_ROOT}/charsets/SCSARAB)

#mitkAddCustomModuleTest(mitkDICOMImageReaderTest_MF mitkDICOMImageReaderTest 54 ${DICOM_TEST_IMAGE_ROOT}/multi-frame-beispiele/unpacked/CT0001)

# per-frame orientation (from NEMA collection)
mitkAddCustomModuleTest(mitkDICOMImageReaderTest_nema_0031 mitkDICOMImageReaderTest 15 ${DICOM_TEST_IMAGE_ROOT}/multiframe/CT/medical.nema.org/medical/Dicom/Multiframe/CT/DISCIMG/IMAGES/CT0031)
mitkAddCustomModuleTest(mitkDICOMImageReaderTest_nema_0033 mitkDICOMImageReaderTest 7 ${DICOM_TEST_IMAGE_ROOT}/multiframe/CT/medical.nema.org/medical/Dicom/Multiframe/CT/DISCIMG/IMAGES/CT0033)
