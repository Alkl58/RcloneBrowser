cmake_minimum_required(VERSION 3.16)

if(WIN32)
  # Match Qt's prebuilt MSVC libraries (/MD) and avoid LNK4098 CRT conflicts.
  cmake_policy(SET CMP0091 NEW)
  set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreadedDLL")
  # link automatically to qtmain.lib on Windows
  cmake_policy(SET CMP0020 NEW)
endif()

project(rclone-browser)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Network)
# Multimedia is only used for the optional finished-transfer notification
# sound. Treat it as optional so the app still builds with Qt kits that do not
# ship the Multimedia module.
find_package(Qt6 QUIET OPTIONAL_COMPONENTS Multimedia)
if(APPLE)
  find_library(COCOA_LIB Cocoa REQUIRED)
  find_library(IOKKIT_LIB IOKit REQUIRED)
endif()

if(WIN32)
  set_property(GLOBAL PROPERTY USE_FOLDERS OFF)

  add_definitions("-D_UNICODE -DUNICODE -D_SCL_SECURE_NO_DEPRECATE -D_CRT_SECURE_NO_DEPRECATE")

  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
  set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /GF /Gy /GS- /GR /GL")

  set(CMAKE_EXE_LINKER_FLAGS "/INCREMENTAL:NO")
  set(CMAKE_EXE_LINKER_FLAGS_DEBUG "/DEBUG")
  set(CMAKE_EXE_LINKER_FLAGS_RELEASE "/LTCG /OPT:ICF /OPT:REF")

  macro(use_pch HEADER SOURCE FILES)
    foreach(FILE ${FILES})
      set_source_files_properties(${FILE} PROPERTIES COMPILE_FLAGS "/Yu${HEADER} /FI${HEADER}")
    endforeach()
    set_source_files_properties(${SOURCE} PROPERTIES COMPILE_FLAGS "/Yc${HEADER}")
  endmacro(use_pch)

else()

  macro(use_pch TARGET HEADER SOURCE)
    # TODO
  endmacro(use_pch)

endif()

file(READ "VERSION" RCLONE_BROWSER_VERSION)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${rclone-browser_BINARY_DIR}/build")

add_subdirectory(src)
