set(CPACK_BUNDLE_NAME "XMoto")
set(CPACK_BUNDLE_PLIST ${PROJECT_SOURCE_DIR}/macos/Info.plist)
set(CPACK_BUNDLE_ICON ${PROJECT_SOURCE_DIR}/macos/xmoto.icns)

set(CPACK_DMG_VOLUME_NAME "X-Moto-${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}")

include(InstallRequiredSystemLibraries)

if(BUILD_MACOS_BUNDLE)
  set_target_properties(xmoto PROPERTIES
    MACOSX_BUNDLE_INFO_PLIST
    "${PROJECT_SOURCE_DIR}/macos/Info.plist"
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
