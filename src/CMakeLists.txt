if (NOT ${CMAKE_VERSION} VERSION_LESS "3.0.0")
	cmake_policy(SET CMP0028 NEW)
endif()

if(WIN32)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4 /WX /wd4100 /wd4189")
else()
  add_definitions("-pedantic -Wall -Wextra -Werror")
endif()

if (APPLE)
  set(CMAKE_OSX_DEPLOYMENT_TARGET 12.0)
endif()

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Network)
find_package(Qt6 QUIET OPTIONAL_COMPONENTS Multimedia)

# Qt 6 makes the QFileInfo(const QString &) constructor explicit by default.
# Keep the Qt 5 implicit behaviour to avoid churn across the codebase.
add_definitions(-DQT_IMPLICIT_QFILEINFO_CONSTRUCTION)

# Enable the notification sound only when the Multimedia module is available.
set(OPTIONAL_QT_LIBS "")
if(TARGET Qt6::Multimedia)
  add_definitions(-DRB_HAVE_MULTIMEDIA)
  list(APPEND OPTIONAL_QT_LIBS Qt6::Multimedia)
endif()

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(UI
  main_window.ui
  remote_widget.ui
  transfer_dialog.ui
  mount_dialog.ui
  export_dialog.ui
  check_dialog.ui
  dedupe_dialog.ui
  progress_dialog.ui
  job_widget.ui
  mount_widget.ui
  stream_widget.ui
  preferences_dialog.ui
  scheduler_widget.ui
  delete_progress_dialog.ui
  remote_folder_dialog.ui
)

set(MOC
  main_window.h
  remote_widget.h
  transfer_dialog.h
  mount_dialog.h
  export_dialog.h
  check_dialog.h
  dedupe_dialog.h
  progress_dialog.h
  job_widget.h
  mount_widget.h
  stream_widget.h
  preferences_dialog.h
  icon_cache.h
  list_of_job_options.h
  item_model.h
  qcron.h
  scheduler_widget.h
  hours_spinbox.h
  minutes_spinbox.h
  delete_progress_dialog.h
  file_dialog.h
  remote_folder_dialog.h
)

set(OTHER
  pch.h
  utils.h
  job_options.h
  global.h
  qcronfield.h
  qcronnode.h
)

set(SOURCE
  pch.cpp
  main.cpp
  main_window.cpp
  remote_widget.cpp
  transfer_dialog.cpp
  mount_dialog.cpp
  export_dialog.cpp
  check_dialog.cpp
  dedupe_dialog.cpp
  progress_dialog.cpp
  job_widget.cpp
  mount_widget.cpp
  stream_widget.cpp
  preferences_dialog.cpp
  icon_cache.cpp
  item_model.cpp
  utils.cpp
  job_options.cpp
  list_of_job_options.cpp
  qcron.cpp
  qcronfield.cpp
  qcronnode.cpp
  scheduler_widget.cpp
  hours_spinbox.cpp
  minutes_spinbox.cpp
  delete_progress_dialog.cpp
  file_dialog.cpp
  remote_folder_dialog.cpp
)

if(WIN32)
  set(OTHER ${OTHER} resources.rc)
elseif(APPLE)
  set(OTHER ${OTHER} osx_helper.h)
  set(SOURCE ${SOURCE} osx_helper.mm)

  set(OTHER ${OTHER} mac_os_notifications.h)
  set(SOURCE ${SOURCE} mac_os_notifications.mm)

  set(OTHER ${OTHER} mac_os_power_saving.h)
  set(SOURCE ${SOURCE} mac_os_power_saving.mm)
endif()

set(QRC resources.qrc)

add_definitions(-DRCLONE_BROWSER_VERSION="${RCLONE_BROWSER_VERSION}")

qt6_wrap_ui(UI_OUT ${UI})
qt6_wrap_cpp(MOC_OUT ${MOC})
qt6_add_resources(QRC_OUT ${QRC} OPTIONS "-no-compress")

source_group("" FILES ${SOURCE} ${MOC} ${UI} ${QRC} ${OTHER})
source_group("Generated" FILES ${MOC_OUT} ${UI_OUT} ${MOC_OUT} ${QRC_OUT})

MACRO(ADD_MSVC_PRECOMPILED_HEADER PrecompiledHeader PrecompiledSource SourcesVar)
  IF(MSVC)
    GET_FILENAME_COMPONENT(PrecompiledBasename ${PrecompiledHeader} NAME_WE)
    SET(PrecompiledBinary "${CMAKE_CURRENT_BINARY_DIR}/${PrecompiledBasename}.pch")
    SET(Sources ${${SourcesVar}})

    SET_SOURCE_FILES_PROPERTIES(${PrecompiledSource}
                                PROPERTIES COMPILE_FLAGS "/Yc\"${PrecompiledHeader}\" /Fp\"${PrecompiledBinary}\""
                                           OBJECT_OUTPUTS "${PrecompiledBinary}")
    SET_SOURCE_FILES_PROPERTIES(${Sources}
                                PROPERTIES COMPILE_FLAGS "/Yu\"${PrecompiledHeader}\" /FI\"${PrecompiledHeader}\" /Fp\"${PrecompiledBinary}\""
                                           OBJECT_DEPENDS "${PrecompiledBinary}")
    # Add precompiled header to SourcesVar
    LIST(APPEND ${SourcesVar} ${PrecompiledSource})
  ENDIF(MSVC)
ENDMACRO(ADD_MSVC_PRECOMPILED_HEADER)

ADD_MSVC_PRECOMPILED_HEADER(pch.h pch.cpp SOURCE)
ADD_MSVC_PRECOMPILED_HEADER(pch.h pch.cpp MOC_OUT)

if(WIN32)
  add_executable(RcloneBrowser WIN32 ${SOURCE} ${BACKEND} ${OTHER} ${MOC} ${MOC_OUT} ${UI_OUT} ${MOC_OUT} ${QRC_OUT})
  target_link_libraries(RcloneBrowser Qt6::Widgets Qt6::Network ${OPTIONAL_QT_LIBS})
elseif(APPLE)
  set_source_files_properties(icon.icns PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")
  add_executable(rclone-browser MACOSX_BUNDLE ${SOURCE} ${BACKEND} ${OTHER} ${MOC} ${MOC_OUT} ${UI_OUT} ${MOC_OUT} ${QRC_OUT} icon.icns)
  target_link_libraries(rclone-browser
    Qt6::Widgets
    Qt6::Network
    ${OPTIONAL_QT_LIBS}
    ${COCOA_LIB}
    ${IOKKIT_LIB}
    ${USER_NOTIFICATIONS_LIB}
    ${UNIFORM_TYPE_IDENTIFIERS_LIB}
  )
  set_target_properties(rclone-browser PROPERTIES MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/Info.plist")

else()
  add_executable(rclone-browser ${SOURCE} ${BACKEND} ${OTHER} ${MOC} ${MOC_OUT} ${UI_OUT} ${MOC_OUT} ${QRC_OUT})
  target_link_libraries(rclone-browser Qt6::Widgets Qt6::Network ${OPTIONAL_QT_LIBS})

  install(TARGETS rclone-browser RUNTIME DESTINATION bin)
  install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/../assets/rclone-browser.svg" DESTINATION "share/icons/hicolor/scalable/apps" RENAME "rclone-browser.svg")
  install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/../assets/rclone-browser-32x32.png" DESTINATION "share/icons/hicolor/32x32/apps" RENAME "rclone-browser.png")
  install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/../assets/rclone-browser-64x64.png" DESTINATION "share/icons/hicolor/64x64/apps" RENAME "rclone-browser.png")
  install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/../assets/rclone-browser-128x128.png" DESTINATION "share/icons/hicolor/128x128/apps" RENAME "rclone-browser.png")
  install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/../assets/rclone-browser-256x256.png" DESTINATION "share/icons/hicolor/256x256/apps" RENAME "rclone-browser.png")
  install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/../assets/rclone-browser-512x512.png" DESTINATION "share/icons/hicolor/512x512/apps" RENAME "rclone-browser.png")
  install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/../assets/rclone-browser.desktop" DESTINATION "share/applications")
endif()
