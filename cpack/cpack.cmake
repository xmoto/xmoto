set(APP_NAME "X-Moto")

set(CPACK_PACKAGE_NAME "${APP_NAME}")
set(CPACK_PACKAGE_VERSION_MAJOR 0)
set(CPACK_PACKAGE_VERSION_MINOR 6)
set(CPACK_PACKAGE_VERSION_PATCH 1)

set(APP_VERSION "\
${CPACK_PACKAGE_VERSION_MAJOR}.\
${CPACK_PACKAGE_VERSION_MINOR}.\
${CPACK_PACKAGE_VERSION_PATCH}")

set(CPACK_PACKAGE_FILE_NAME "xmoto-${APP_VERSION}")
set(CPACK_PACKAGE_HOMEPAGE_URL "https://xmoto.tuxfamily.org")
set(CPACK_PACKAGE_CONTACT "https://github.com/xmoto/xmoto")
set(CPACK_PACKAGE_VENDOR "X-Moto Developers")

set(CPACK_RESOURCE_FILE_LICENSE "${PROJECT_SOURCE_DIR}/COPYING")
set(CPACK_RESOURCE_FILE_README "${PROJECT_SOURCE_DIR}/README.md")

set(CPACK_SOURCE_IGNORE_FILES "/\\\\.git/")
set(CPACK_SOURCE_GENERATOR STGZ)

set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "A challenging 2D motocross platform game")
set(CPACK_PACKAGE_DESCRIPTION "\
X-Moto is a challenging 2D motocross platform game, where physics play \
an all important role in the gameplay. You need to control your bike to \
its limit, if you want to have a chance finishing the more difficult of \
the challenges.\
")

include(cpack/macos.cmake)
include(cpack/win32.cmake)
