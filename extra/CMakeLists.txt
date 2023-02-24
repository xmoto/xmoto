if(NOT WIN32 AND NOT BUILD_MACOS_BUNDLE)
  install(
    FILES xmoto.desktop
    DESTINATION share/applications
  )

  install(
    FILES xmoto.png
    DESTINATION share/pixmaps
  )

  install(
    FILES
      xmoto.bmp
      xmoto.ico
    DESTINATION share/xmoto
  )
endif()
