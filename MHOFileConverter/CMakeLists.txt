# CMakeList.txt : CMake project for MHOFileConverter, include source and define
# project specific logic here.
#

# Add source to this project's executable.
add_executable (MHOFileConverter   "CryXmlbReader.cpp" "tinyxml2.cpp" "CryXmlbReader.h" "MhoEncryption.h" "MhoEncryption.cpp" "CryXmlBWriter.h" "CryXmlBWriter.cpp" "main.cpp"  "Utils.h" "Utils.cpp")

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET MHOFileConverter PROPERTY CXX_STANDARD 20)
endif()

# TODO: Add tests and install targets if needed.
