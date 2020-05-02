set(XMOTO_PACK "always" CACHE STRING "When to build xmoto.bin")
set_property(CACHE XMOTO_PACK PROPERTY STRINGS "manual" "always")

add_custom_command(
  OUTPUT xmoto.bin
  COMMAND xmoto --pack xmoto.bin "${CMAKE_CURRENT_SOURCE_DIR}"
  MAIN_DEPENDENCY package.lst
  COMMENT "Packing xmoto.bin"
)

if (XMOTO_PACK STREQUAL "always")
  add_custom_target(xmoto_pack ALL DEPENDS xmoto.bin)
else()
  add_custom_target(xmoto_pack DEPENDS xmoto.bin)
endif()

if(WIN32)
  install(FILES "${CMAKE_CURRENT_BINARY_DIR}/xmoto.bin" DESTINATION "./")
  install(DIRECTORY Textures/Musics DESTINATION ./Textures)
  install(DIRECTORY Textures/Fonts DESTINATION ./Textures)
else()
  install(FILES "${CMAKE_CURRENT_BINARY_DIR}/xmoto.bin" DESTINATION share/xmoto)
  install(DIRECTORY Textures/Musics DESTINATION share/xmoto/Textures)
  install(DIRECTORY Textures/Fonts DESTINATION share/xmoto/Textures)
endif()
