set(APP_ICO_PATH "${CMAKE_SOURCE_DIR}/extra/xmoto.ico")
set(APP_LNK_NAME "${APP_NAME}.lnk")
set(APP_EXE_NAME "xmoto.exe")

if(WIN32)
  if("${CMAKE_VERSION}" VERSION_LESS "3.18.0")
    message(WARNING "Correctly generating NSIS installers requires CMake 3.18+")
  endif()

  set(CPACK_PACKAGE_INSTALL_DIRECTORY "${APP_NAME}")

  set(CPACK_NSIS_DISPLAY_NAME "${APP_NAME}")
  set(CPACK_NSIS_MANIFEST_DPI_AWARE TRUE)
  set(CPACK_NSIS_EXECUTABLES_DIRECTORY ".")
  set(CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL TRUE)
  set(CPACK_NSIS_UNINSTALL_NAME "Uninstall")
  set(CPACK_NSIS_MODIFY_PATH ON)

  set(CPACK_NSIS_CREATE_ICONS_EXTRA "
    CreateShortCut '$DESKTOP\\\\${APP_LNK_NAME}' '\\\"$INSTDIR\\\\${APP_EXE_NAME}\\\"'
    CreateShortCut '$SMPROGRAMS\\\\$STARTMENU_FOLDER\\\\${APP_LNK_NAME}' '\\\"$INSTDIR\\\\${APP_EXE_NAME}\\\"'

    ${CPACK_NSIS_CREATE_ICONS_EXTRA}
  ")

  set(CPACK_NSIS_DELETE_ICONS_EXTRA "
    Delete \\\"$DESKTOP\\\\${APP_LNK_NAME}\\\"
    Delete \\\"$SMPROGRAMS\\\\$MUI_TEMP\\\\${APP_LNK_NAME}\\\"

    ${CPACK_NSIS_DELETE_ICONS_EXTRA}
  ")

  # Removes the branding text.
  # Note: CMake has CPACK_NSIS_BRANDING_TEXT since version 3.20,
  # but MXE's Focal Fossa (Ubuntu 20.04) builds only have CMake 3.19
  set(CPACK_NSIS_DEFINES "BrandingText \\\" \\\"")

  set(CPACK_NSIS_MUI_ICON "${APP_ICO_PATH}")
  set(CPACK_NSIS_MUI_UNIICON "${APP_ICO_PATH}")
  set(CPACK_NSIS_MUI_HEADERIMAGE "${CMAKE_SOURCE_DIR}/extra/nsis/header.bmp")
  set(CPACK_NSIS_MUI_WELCOMEFINISHPAGE_BITMAP "${CMAKE_SOURCE_DIR}/extra/nsis/welcome.bmp")
  set(CPACK_NSIS_MUI_UNWELCOMEFINISHPAGE_BITMAP "${CMAKE_SOURCE_DIR}/extra/nsis/welcome.bmp")
  set(CPACK_NSIS_MUI_FINISHPAGE_RUN "${APP_EXE_NAME}")
  set(CPACK_NSIS_INSTALLED_ICON_NAME "${APP_EXE_NAME}")
endif()
