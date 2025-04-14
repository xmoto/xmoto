set(XMOTO_PACK "native" CACHE STRING "Whether to pack xmoto.bin natively or through wine")
set_property(CACHE XMOTO_PACK PROPERTY STRINGS "native" "wine")

file(GLOB_RECURSE BIN_FILES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/**/*")

if(WIN32 AND CMAKE_CROSSCOMPILING AND XMOTO_PACK STREQUAL "wine")
  add_custom_command(
    OUTPUT xmoto.bin
    COMMAND
      "${PROJECT_SOURCE_DIR}/tools/mxe_pack.sh"
      "${CMAKE_BINARY_DIR}/wine-prefix" # Wine prefix directory
      "${CMAKE_CURRENT_SOURCE_DIR}" # Input directory
      "${CMAKE_BINARY_DIR}" # Build directory
      "${MXE_TOOLCHAIN_PATH}" # Toolchain directory
    DEPENDS ${BIN_FILES}
    COMMENT "Packing xmoto.bin"
  )
else()
  add_custom_command(
    OUTPUT xmoto.bin
    COMMAND xmoto --pack xmoto.bin "${CMAKE_CURRENT_SOURCE_DIR}"
    DEPENDS ${BIN_FILES}
    COMMENT "Packing xmoto.bin"
  )
endif()

add_custom_target(xmoto_pack ALL DEPENDS xmoto.bin)
add_dependencies(xmoto_pack xmoto)

if(WIN32)
  install(FILES "${CMAKE_CURRENT_BINARY_DIR}/xmoto.bin" DESTINATION "./")
  install(DIRECTORY Textures/Musics DESTINATION ./Textures)
  install(DIRECTORY Textures/Fonts DESTINATION ./Textures)
elseif(BUILD_MACOS_BUNDLE)
  install(CODE "
  set(RESOURCES_DIR_PATH
    # CPACK_BUNDLE_NAME is set in cpack/macos.cmake
    \"\${CMAKE_INSTALL_PREFIX}/${CPACK_BUNDLE_NAME}.app/Contents/Resources\")

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
