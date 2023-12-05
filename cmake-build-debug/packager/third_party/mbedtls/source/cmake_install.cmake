# Install script for directory: /Users/kevin.staunton-lambert/workspace/github_kevleyski/shaka-packager/packager/third_party/mbedtls/source

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

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/cmake" TYPE FILE FILES
    "/Users/kevin.staunton-lambert/workspace/github_kevleyski/shaka-packager/cmake-build-debug/packager/third_party/mbedtls/source/cmake/MbedTLSConfig.cmake"
    "/Users/kevin.staunton-lambert/workspace/github_kevleyski/shaka-packager/cmake-build-debug/packager/third_party/mbedtls/source/cmake/MbedTLSConfigVersion.cmake"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/cmake/MbedTLSTargets.cmake")
    file(DIFFERENT _cmake_export_file_changed FILES
         "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/cmake/MbedTLSTargets.cmake"
         "/Users/kevin.staunton-lambert/workspace/github_kevleyski/shaka-packager/cmake-build-debug/packager/third_party/mbedtls/source/CMakeFiles/Export/272ceadb8458515b2ae4b5630a6029cc/MbedTLSTargets.cmake")
    if(_cmake_export_file_changed)
      file(GLOB _cmake_old_config_files "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/cmake/MbedTLSTargets-*.cmake")
      if(_cmake_old_config_files)
        string(REPLACE ";" ", " _cmake_old_config_files_text "${_cmake_old_config_files}")
        message(STATUS "Old export file \"$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/cmake/MbedTLSTargets.cmake\" will be replaced.  Removing files [${_cmake_old_config_files_text}].")
        unset(_cmake_old_config_files_text)
        file(REMOVE ${_cmake_old_config_files})
      endif()
      unset(_cmake_old_config_files)
    endif()
    unset(_cmake_export_file_changed)
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/cmake" TYPE FILE FILES "/Users/kevin.staunton-lambert/workspace/github_kevleyski/shaka-packager/cmake-build-debug/packager/third_party/mbedtls/source/CMakeFiles/Export/272ceadb8458515b2ae4b5630a6029cc/MbedTLSTargets.cmake")
  if(CMAKE_INSTALL_CONFIG_NAME MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/cmake" TYPE FILE FILES "/Users/kevin.staunton-lambert/workspace/github_kevleyski/shaka-packager/cmake-build-debug/packager/third_party/mbedtls/source/CMakeFiles/Export/272ceadb8458515b2ae4b5630a6029cc/MbedTLSTargets-debug.cmake")
  endif()
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for each subdirectory.
  include("/Users/kevin.staunton-lambert/workspace/github_kevleyski/shaka-packager/cmake-build-debug/packager/third_party/mbedtls/source/include/cmake_install.cmake")
  include("/Users/kevin.staunton-lambert/workspace/github_kevleyski/shaka-packager/cmake-build-debug/packager/third_party/mbedtls/source/3rdparty/cmake_install.cmake")
  include("/Users/kevin.staunton-lambert/workspace/github_kevleyski/shaka-packager/cmake-build-debug/packager/third_party/mbedtls/source/library/cmake_install.cmake")

endif()

