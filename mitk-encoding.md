# Observed problems

On a Windows machine with German_Germany.1252 locale (Latin1):
* (without any of this task's patches) editing DataNode names in Data Manager does not accept "ö" or other special characters. Input is possible, but as soon as the in-place editor is closed, the Data Manager does not display it as "ö" but as "?" (missing character symbol)
* when fixing Data Manager's display (by making it use QString<-->std::string conversions via default conversions), files named with special characters such as "ö.nrrd" will be loaded but their names will be displayed as "missing character"

# String representation summary

## What happens in Qt

* QString contains UTF-16 internally (but we should not care)
* QString offers convenient conversion to std::string via fromStdString() and toStdString(). Conversion logic is identical when constructing QString from a const char*
    * Behavior changed between Qt 4.8 and Qt 5.8 (most probably with the release of Qt 5):
        * **Qt 4.x was assuming Latin1** the default encoding for unspecified character strings: http://doc.qt.io/qt-4.8/qstring.html#fromStdString
        * **Qt 5.x is assuming UTF-8** the default encoding for unspecified character strings: http://doc.qt.io/qt-5/qstring.html#fromStdString 

## What happens in MITK's Qt UI

* Nearly all UI code converts using Qt's default conversion (toStdString/fromStdString). This seems reasonable. However, it changed its meaning with the migration to Qt 5. Before, MITK UI was assuming "std::string == Latin1", now MITK UI is assuming "std::string == UTF-8".
* Exceptions were found in:
    * Qt-based models: were using QFile::encodeName(). This method uses the the user's locale. Using encodeName() leads to broken string when typing "ö" into views based on those models (when QString's default conversion assumes UTF-8 and QFile::encodeName assumes something else, e.g. Latin1).
    * QmitkIOUtil: also uses QFile::encodeName(). This leads to **DataNode names encoded using the user's locale** (say Latin1) which is displayed in Data Manager assuming it was UTF-8 (i.e. missing character symbols)
    
## What happens in MITK's I/O

mitk::IOUtil and probably other ITK related file reader/writer use `itksys::SystemTools::FileExists()`. This method takes a `const char*` as input. Internally it uses a wrapped version of the locale-dependent `mbstowcs` (http://en.cppreference.com/w/cpp/string/multibyte) to create a wide-character representation of the input. On Windows this wide-character string is handed to `GetFileAttributesW` (https://msdn.microsoft.com/en-us/library/windows/desktop/aa364944(v=vs.85).aspx) to decide whether a file exists or not. GetFileAttributesW assumes unicode input.

# Summary / Conclusion

* Default conversions between QString and std::string seem the best choice for UI classes because it's Qt's default.
* However, MITK's I/O classes (notably QmitkIOUtil) are built around "Local8Bit()" (hidden behind QFile::encodeName).
* calls to SystemTools::FileExists() have been very lucky so far because: via QmitkIOUtil's QFile::encodeName() they got to see "locale" encoded C strings instead of UTF-8 - which fits the further processing. If we would feed UTF-8 into FileExists() it would not work (on Latin1 systems).
* **Core question**: What do we want to be in DataNode::m_Name? locale dependent strings or UTF-8? Currently we have a mix of assumptions in MITK that sometimes don't match.
