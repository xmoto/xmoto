if(WIN32)
  set(CPACK_NSIS_MUI_ICON "${CMAKE_SOURCE_DIR}/extra/xmoto_icone.ico")
  set(CPACK_NSIS_MUI_UNIICON "${CPACK_NSIS_MUI_ICON}")
  set(CPACK_NSIS_EXECUTABLES_DIRECTORY "")
  set(CPACK_INCLUDE_TOPLEVEL_DIRECTORY FALSE)

  set(CPACK_NSIS_CREATE_ICONS_EXTRA
    "CreateShortCut \\\"$DESKTOP\\\\X-Moto.lnk\\\" \\\"$INSTDIR\\\\xmoto.exe\\\""
    "CreateShortCut \\\"$STARTMENU_FOLDER\\\\X-Moto.lnk\\\" \\\"$INSTDIR\\\\xmoto.exe\\\""
  )

  # cpack will complain if this is set for the NSIS generator,
  # but there doesn't seem to be any way to conditionally set it...
  set(CPACK_INCLUDE_TOPLEVEL_DIRECTORY TRUE)
endif()

