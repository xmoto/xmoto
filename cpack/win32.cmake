set(APP_EXE_NAME "xmoto.exe")
set(APP_ICO_PATH "${CMAKE_SOURCE_DIR}/extra/xmoto.ico")
set(NSIS_PATH "${CMAKE_SOURCE_DIR}/extra/nsis")

if(WIN32)
  if("${CMAKE_VERSION}" VERSION_LESS "3.18.0")
    message(WARNING "Correctly generating NSIS installers requires CMake 3.18+")
  endif()

  set(CPACK_PACKAGE_INSTALL_DIRECTORY "${APP_NAME}")
  set(CPACK_PACKAGE_EXECUTABLES "xmoto;${APP_NAME}")
  set(CPACK_CREATE_DESKTOP_LINKS "xmoto")

  set(CPACK_NSIS_DISPLAY_NAME "${APP_NAME}")
  set(CPACK_NSIS_MANIFEST_DPI_AWARE ON)
  set(CPACK_NSIS_EXECUTABLES_DIRECTORY ".")
  set(CPACK_NSIS_UNINSTALL_NAME "Uninstall")
  set(CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL ON)
  set(CPACK_NSIS_MODIFY_PATH ON)

  # Removes the branding text.
  # Note: CMake has CPACK_NSIS_BRANDING_TEXT since version 3.20,
  # but MXE's Focal Fossa (Ubuntu 20.04) builds only have CMake 3.19
  set(CPACK_NSIS_DEFINES "BrandingText \\\" \\\"")

  set(CPACK_NSIS_MUI_ICON "${APP_ICO_PATH}")
  set(CPACK_NSIS_MUI_UNIICON "${APP_ICO_PATH}")
  set(CPACK_NSIS_MUI_HEADERIMAGE "${NSIS_PATH}/header.bmp")
  set(CPACK_NSIS_MUI_WELCOMEFINISHPAGE_BITMAP "${NSIS_PATH}/welcome.bmp")
  set(CPACK_NSIS_MUI_UNWELCOMEFINISHPAGE_BITMAP "${NSIS_PATH}/welcome.bmp")
  set(CPACK_NSIS_MUI_FINISHPAGE_RUN "${APP_EXE_NAME}")
  set(CPACK_NSIS_INSTALLED_ICON_NAME "${APP_EXE_NAME}")

  set(CPACK_NSIS_EXTRA_PREINSTALL_COMMANDS "
    !include \\\"${NSIS_PATH}\\\\fileassoc.nsh\\\"
  ")

  set(CPACK_NSIS_EXTRA_INSTALL_COMMANDS "\
    !insertmacro APP_ASSOCIATE \
      'lvl' \
      '${APP_NAME}.levelfile' \
      '${APP_NAME} Level File' \
      '$INSTDIR\\\\${APP_EXE_NAME},0' \
      'Open in ${APP_NAME}' \
      '$INSTDIR\\\\${APP_EXE_NAME} \\\"%1\\\"'

    !insertmacro APP_ASSOCIATE \
      'rpl' \
      '${APP_NAME}.replayfile' \
      '${APP_NAME} Replay File' \
      '$INSTDIR\\\\${APP_EXE_NAME},0' \
      'Open in ${APP_NAME}' \
      '$INSTDIR\\\\${APP_EXE_NAME} \\\"%1\\\"'

    !insertmacro APP_ASSOCIATE \
      'dmo' \
      '${APP_NAME}.demofile' \
      '${APP_NAME} Demo File' \
      '$INSTDIR\\\\${APP_EXE_NAME},0' \
      'Open in ${APP_NAME}' \
      '$INSTDIR\\\\${APP_EXE_NAME} \\\"%1\\\"'

    !insertmacro UPDATEFILEASSOC
  ")

  set(CPACK_NSIS_EXTRA_UNINSTALL_COMMANDS "
    !insertmacro APP_UNASSOCIATE 'lvl' '${APP_NAME}.levelfile'
    !insertmacro APP_UNASSOCIATE 'rpl' '${APP_NAME}.replayfile'
    !insertmacro APP_UNASSOCIATE 'dmo' '${APP_NAME}.demofile'

    !insertmacro UPDATEFILEASSOC
  ")
endif()
