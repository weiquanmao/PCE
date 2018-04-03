#----------------------------------------------------------------
# Generated CMake target import file for configuration "Release".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "Cpd::Library-C++" for configuration "Release"
set_property(TARGET Cpd::Library-C++ APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(Cpd::Library-C++ PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_RELEASE "CXX"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/lib/cpd.lib"
  )

list(APPEND _IMPORT_CHECK_TARGETS Cpd::Library-C++ )
list(APPEND _IMPORT_CHECK_FILES_FOR_Cpd::Library-C++ "${_IMPORT_PREFIX}/lib/cpd.lib" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
