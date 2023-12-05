# Install script for directory: /Users/kevin.staunton-lambert/workspace/github_kevleyski/shaka-packager/packager/third_party/libxml2/source

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/usr/local")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "Debug")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

# Set default install directory permissions.
if(NOT DEFINED CMAKE_OBJDUMP)
  set(CMAKE_OBJDUMP "/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/objdump")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "development" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/libxml2/libxml" TYPE FILE FILES
    "/Users/kevin.staunton-lambert/workspace/github_kevleyski/shaka-packager/packager/third_party/libxml2/source/include/libxml/c14n.h"
    "/Users/kevin.staunton-lambert/workspace/github_kevleyski/shaka-packager/packager/third_party/libxml2/source/include/libxml/catalog.h"
    "/Users/kevin.staunton-lambert/workspace/github_kevleyski/shaka-packager/packager/third_party/libxml2/source/include/libxml/chvalid.h"
    "/Users/kevin.staunton-lambert/workspace/github_kevleyski/shaka-packager/packager/third_party/libxml2/source/include/libxml/debugXML.h"
    "/Users/kevin.staunton-lambert/workspace/github_kevleyski/shaka-packager/packager/third_party/libxml2/source/include/libxml/dict.h"
    "/Users/kevin.staunton-lambert/workspace/github_kevleyski/shaka-packager/packager/third_party/libxml2/source/include/libxml/DOCBparser.h"
    "/Users/kevin.staunton-lambert/workspace/github_kevleyski/shaka-packager/packager/third_party/libxml2/source/include/libxml/encoding.h"
    "/Users/kevin.staunton-lambert/workspace/github_kevleyski/shaka-packager/packager/third_party/libxml2/source/include/libxml/entities.h"
    "/Users/kevin.staunton-lambert/workspace/github_kevleyski/shaka-packager/packager/third_party/libxml2/source/include/libxml/globals.h"
    "/Users/kevin.staunton-lambert/workspace/github_kevleyski/shaka-packager/packager/third_party/libxml2/source/include/libxml/hash.h"
    "/Users/kevin.staunton-lambert/workspace/github_kevleyski/shaka-packager/packager/third_party/libxml2/source/include/libxml/HTMLparser.h"
    "/Users/kevin.staunton-lambert/workspace/github_kevleyski/shaka-packager/packager/third_party/libxml2/source/include/libxml/HTMLtree.h"
    "/Users/kevin.staunton-lambert/workspace/github_kevleyski/shaka-packager/packager/third_party/libxml2/source/include/libxml/list.h"
    "/Users/kevin.staunton-lambert/workspace/github_kevleyski/shaka-packager/packager/third_party/libxml2/source/include/libxml/nanoftp.h"
    "/Users/kevin.staunton-lambert/workspace/github_kevleyski/shaka-packager/packager/third_party/libxml2/source/include/libxml/nanohttp.h"
    "/Users/kevin.staunton-lambert/workspace/github_kevleyski/shaka-packager/packager/third_party/libxml2/source/include/libxml/parser.h"
    "/Users/kevin.staunton-lambert/workspace/github_kevleyski/shaka-packager/packager/third_party/libxml2/source/include/libxml/parserInternals.h"
    "/Users/kevin.staunton-lambert/workspace/github_kevleyski/shaka-packager/packager/third_party/libxml2/source/include/libxml/pattern.h"
    "/Users/kevin.staunton-lambert/workspace/github_kevleyski/shaka-packager/packager/third_party/libxml2/source/include/libxml/relaxng.h"
    "/Users/kevin.staunton-lambert/workspace/github_kevleyski/shaka-packager/packager/third_party/libxml2/source/include/libxml/SAX.h"
    "/Users/kevin.staunton-lambert/workspace/github_kevleyski/shaka-packager/packager/third_party/libxml2/source/include/libxml/SAX2.h"
    "/Users/kevin.staunton-lambert/workspace/github_kevleyski/shaka-packager/packager/third_party/libxml2/source/include/libxml/schemasInternals.h"
    "/Users/kevin.staunton-lambert/workspace/github_kevleyski/shaka-packager/packager/third_party/libxml2/source/include/libxml/schematron.h"
    "/Users/kevin.staunton-lambert/workspace/github_kevleyski/shaka-packager/packager/third_party/libxml2/source/include/libxml/threads.h"
    "/Users/kevin.staunton-lambert/workspace/github_kevleyski/shaka-packager/packager/third_party/libxml2/source/include/libxml/tree.h"
    "/Users/kevin.staunton-lambert/workspace/github_kevleyski/shaka-packager/packager/third_party/libxml2/source/include/libxml/uri.h"
    "/Users/kevin.staunton-lambert/workspace/github_kevleyski/shaka-packager/packager/third_party/libxml2/source/include/libxml/valid.h"
    "/Users/kevin.staunton-lambert/workspace/github_kevleyski/shaka-packager/packager/third_party/libxml2/source/include/libxml/xinclude.h"
    "/Users/kevin.staunton-lambert/workspace/github_kevleyski/shaka-packager/packager/third_party/libxml2/source/include/libxml/xlink.h"
    "/Users/kevin.staunton-lambert/workspace/github_kevleyski/shaka-packager/packager/third_party/libxml2/source/include/libxml/xmlIO.h"
    "/Users/kevin.staunton-lambert/workspace/github_kevleyski/shaka-packager/packager/third_party/libxml2/source/include/libxml/xmlautomata.h"
    "/Users/kevin.staunton-lambert/workspace/github_kevleyski/shaka-packager/packager/third_party/libxml2/source/include/libxml/xmlerror.h"
    "/Users/kevin.staunton-lambert/workspace/github_kevleyski/shaka-packager/packager/third_party/libxml2/source/include/libxml/xmlexports.h"
    "/Users/kevin.staunton-lambert/workspace/github_kevleyski/shaka-packager/packager/third_party/libxml2/source/include/libxml/xmlmemory.h"
    "/Users/kevin.staunton-lambert/workspace/github_kevleyski/shaka-packager/packager/third_party/libxml2/source/include/libxml/xmlmodule.h"
    "/Users/kevin.staunton-lambert/workspace/github_kevleyski/shaka-packager/packager/third_party/libxml2/source/include/libxml/xmlreader.h"
    "/Users/kevin.staunton-lambert/workspace/github_kevleyski/shaka-packager/packager/third_party/libxml2/source/include/libxml/xmlregexp.h"
    "/Users/kevin.staunton-lambert/workspace/github_kevleyski/shaka-packager/packager/third_party/libxml2/source/include/libxml/xmlsave.h"
    "/Users/kevin.staunton-lambert/workspace/github_kevleyski/shaka-packager/packager/third_party/libxml2/source/include/libxml/xmlschemas.h"
    "/Users/kevin.staunton-lambert/workspace/github_kevleyski/shaka-packager/packager/third_party/libxml2/source/include/libxml/xmlschemastypes.h"
    "/Users/kevin.staunton-lambert/workspace/github_kevleyski/shaka-packager/packager/third_party/libxml2/source/include/libxml/xmlstring.h"
    "/Users/kevin.staunton-lambert/workspace/github_kevleyski/shaka-packager/packager/third_party/libxml2/source/include/libxml/xmlunicode.h"
    "/Users/kevin.staunton-lambert/workspace/github_kevleyski/shaka-packager/packager/third_party/libxml2/source/include/libxml/xmlwriter.h"
    "/Users/kevin.staunton-lambert/workspace/github_kevleyski/shaka-packager/packager/third_party/libxml2/source/include/libxml/xpath.h"
    "/Users/kevin.staunton-lambert/workspace/github_kevleyski/shaka-packager/packager/third_party/libxml2/source/include/libxml/xpathInternals.h"
    "/Users/kevin.staunton-lambert/workspace/github_kevleyski/shaka-packager/packager/third_party/libxml2/source/include/libxml/xpointer.h"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "development" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "/Users/kevin.staunton-lambert/workspace/github_kevleyski/shaka-packager/cmake-build-debug/packager/third_party/libxml2/source/libxml2.a")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libxml2.a" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libxml2.a")
    execute_process(COMMAND "/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/ranlib" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libxml2.a")
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "documentation" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/man/man3" TYPE FILE FILES "/Users/kevin.staunton-lambert/workspace/github_kevleyski/shaka-packager/packager/third_party/libxml2/source/libxml.3")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "documentation" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/man/man1" TYPE FILE FILES "/Users/kevin.staunton-lambert/workspace/github_kevleyski/shaka-packager/packager/third_party/libxml2/source/doc/xmlcatalog.1")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "documentation" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/man/man1" TYPE FILE FILES "/Users/kevin.staunton-lambert/workspace/github_kevleyski/shaka-packager/packager/third_party/libxml2/source/doc/xmllint.1")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "documentation" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/doc/libxml2" TYPE DIRECTORY FILES "/Users/kevin.staunton-lambert/workspace/github_kevleyski/shaka-packager/packager/third_party/libxml2/source/doc/" REGEX "/makefile\\.[^/]*$" EXCLUDE)
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "development" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/libxml2-2.9.14" TYPE FILE FILES "/Users/kevin.staunton-lambert/workspace/github_kevleyski/shaka-packager/cmake-build-debug/packager/third_party/libxml2/source/libxml2-config.cmake")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "development" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/libxml2-2.9.14" TYPE FILE FILES "/Users/kevin.staunton-lambert/workspace/github_kevleyski/shaka-packager/cmake-build-debug/packager/third_party/libxml2/source/libxml2-config-version.cmake")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "development" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/libxml2-2.9.14/libxml2-export.cmake")
    file(DIFFERENT _cmake_export_file_changed FILES
         "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/libxml2-2.9.14/libxml2-export.cmake"
         "/Users/kevin.staunton-lambert/workspace/github_kevleyski/shaka-packager/cmake-build-debug/packager/third_party/libxml2/source/CMakeFiles/Export/8133f73f37ae8548341794eaa86f4e26/libxml2-export.cmake")
    if(_cmake_export_file_changed)
      file(GLOB _cmake_old_config_files "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/libxml2-2.9.14/libxml2-export-*.cmake")
      if(_cmake_old_config_files)
        string(REPLACE ";" ", " _cmake_old_config_files_text "${_cmake_old_config_files}")
        message(STATUS "Old export file \"$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/libxml2-2.9.14/libxml2-export.cmake\" will be replaced.  Removing files [${_cmake_old_config_files_text}].")
        unset(_cmake_old_config_files_text)
        file(REMOVE ${_cmake_old_config_files})
      endif()
      unset(_cmake_old_config_files)
    endif()
    unset(_cmake_export_file_changed)
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/libxml2-2.9.14" TYPE FILE FILES "/Users/kevin.staunton-lambert/workspace/github_kevleyski/shaka-packager/cmake-build-debug/packager/third_party/libxml2/source/CMakeFiles/Export/8133f73f37ae8548341794eaa86f4e26/libxml2-export.cmake")
  if(CMAKE_INSTALL_CONFIG_NAME MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/libxml2-2.9.14" TYPE FILE FILES "/Users/kevin.staunton-lambert/workspace/github_kevleyski/shaka-packager/cmake-build-debug/packager/third_party/libxml2/source/CMakeFiles/Export/8133f73f37ae8548341794eaa86f4e26/libxml2-export-debug.cmake")
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "development" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/libxml2/libxml" TYPE FILE FILES "/Users/kevin.staunton-lambert/workspace/github_kevleyski/shaka-packager/cmake-build-debug/packager/third_party/libxml2/source/libxml/xmlversion.h")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "development" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/pkgconfig" TYPE FILE FILES "/Users/kevin.staunton-lambert/workspace/github_kevleyski/shaka-packager/cmake-build-debug/packager/third_party/libxml2/source/libxml-2.0.pc")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "development" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/bin" TYPE PROGRAM FILES "/Users/kevin.staunton-lambert/workspace/github_kevleyski/shaka-packager/cmake-build-debug/packager/third_party/libxml2/source/xml2-config")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "development" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE FILE FILES "/Users/kevin.staunton-lambert/workspace/github_kevleyski/shaka-packager/cmake-build-debug/packager/third_party/libxml2/source/xml2Conf.sh")
endif()

