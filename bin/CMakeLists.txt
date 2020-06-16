set(XMOTO_PACK "always" CACHE STRING "When to build xmoto.bin")
set_property(CACHE XMOTO_PACK PROPERTY STRINGS "manual" "always")

if(WIN32 AND CMAKE_CROSSCOMPILING)
  add_custom_command(
    OUTPUT xmoto.bin
    # setting DISPLAY=:0.0 for travis
    COMMAND ${CMAKE_COMMAND} -E env DISPLAY=:0.0 wine
            "${PROJECT_BINARY_DIR}/src/xmoto.exe"
            --pack xmoto.bin "${CMAKE_CURRENT_SOURCE_DIR}"
    MAIN_DEPENDENCY package.lst
    COMMENT "Packing xmoto.bin"
  )
else()
  add_custom_command(
    OUTPUT xmoto.bin
    COMMAND xmoto --pack xmoto.bin "${CMAKE_CURRENT_SOURCE_DIR}"
    MAIN_DEPENDENCY package.lst
    COMMENT "Packing xmoto.bin"
  )
endif()

if(XMOTO_PACK STREQUAL "always")
  add_custom_target(xmoto_pack ALL DEPENDS xmoto.bin)
else()
  add_custom_target(xmoto_pack DEPENDS xmoto.bin)
endif()

if(WIN32)
  install(FILES "${CMAKE_CURRENT_BINARY_DIR}/xmoto.bin" DESTINATION "./")
  install(DIRECTORY Textures/Musics DESTINATION ./Textures)
  install(DIRECTORY Textures/Fonts DESTINATION ./Textures)
elseif(BUILD_MACOS_BUNDLE)
  install(CODE "
  set(RESOURCES_DIR_PATH
    \"\${CMAKE_INSTALL_PREFIX}/xmoto.app/Contents/Resources\")

  execute_process(COMMAND \"\${CMAKE_COMMAND}\"
    -E copy
    \"${CMAKE_CURRENT_BINARY_DIR}/xmoto.bin\"
    \"\${RESOURCES_DIR_PATH}/xmoto.bin\"
  )

  # Musics
  execute_process(COMMAND \"\${CMAKE_COMMAND}\"
    -E copy_directory
    \"${CMAKE_CURRENT_SOURCE_DIR}/Textures/Musics\"
    \"\${RESOURCES_DIR_PATH}/Textures/Musics\"
  )
  # Fonts
  execute_process(COMMAND \"\${CMAKE_COMMAND}\"
    -E copy_directory
    \"${CMAKE_CURRENT_SOURCE_DIR}/Textures/Fonts\"
    \"\${RESOURCES_DIR_PATH}/Textures/Fonts\"
  )
  ")
else()
  install(FILES "${CMAKE_CURRENT_BINARY_DIR}/xmoto.bin" DESTINATION share/xmoto)
  install(DIRECTORY Textures/Musics DESTINATION share/xmoto/Textures)
  install(DIRECTORY Textures/Fonts DESTINATION share/xmoto/Textures)
endif()
