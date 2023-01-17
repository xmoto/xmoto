set(CPACK_BUNDLE_NAME "XMoto")

set(MACOS_PLIST_PATH "${CMAKE_BINARY_DIR}/macos/Info.plist")
set(MACOS_PLIST_IN_PATH "${PROJECT_SOURCE_DIR}/macos/Info.plist.in")

set(CPACK_BUNDLE_ICON "${PROJECT_SOURCE_DIR}/macos/xmoto.icns")
set(CPACK_DMG_VOLUME_NAME "${APP_NAME} ${APP_VERSION}")

include(InstallRequiredSystemLibraries)

if(BUILD_MACOS_BUNDLE)
  configure_file("${MACOS_PLIST_IN_PATH}" "${MACOS_PLIST_PATH}")

  set_target_properties(xmoto PROPERTIES
    MACOSX_BUNDLE_INFO_PLIST
    "${MACOS_PLIST_PATH}"
  )
  install(TARGETS xmoto
    RUNTIME DESTINATION . COMPONENT Runtime
    BUNDLE DESTINATION . COMPONENT Runtime
  )

  install(CODE "
    include(BundleUtilities)
    fixup_bundle(\"\${CMAKE_INSTALL_PREFIX}/xmoto.app\"  \"\"  \"\")
    " COMPONENT Runtime)
endif()
