if(${CMAKE_SYSTEM_NAME} MATCHES "Linux|FreeBSD|Darwin" AND NOT BUILD_MACOS_BUNDLE)
  install(FILES xmoto.6 DESTINATION share/man/man6 COMPONENT doc)
endif()

