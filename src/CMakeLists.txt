include(CMakeDependentOption)

option(STATIC_BUILD "Build a static executable" OFF)
option(USE_OPENGL "Build with OpenGL for graphics" ON)
option(USE_SDL "Build with SDL" ON)
CMAKE_DEPENDENT_OPTION(USE_SDL_GFX "Build with SDL for graphics" OFF
  "USE_SDL" OFF)
option(USE_GETTEXT "Build with Gettext for internationalization" ON)
option(PREFER_SYSTEM_BZip2 "Prefer to build with system BZip2" ON)
option(PREFER_SYSTEM_Lua "Prefer to build with system Lua" ON)
option(PREFER_SYSTEM_ODE "Prefer to build with system ODE" ON)
option(PREFER_SYSTEM_XDG "Prefer to build with system XDG" ON)
option(ALLOW_DEV "Enable some development/debug features" OFF)
option(BUILD_MACOS_BUNDLE "Build xmoto as a macOS Bundle" OFF)

if(CMAKE_GENERATOR MATCHES "Ninja")
  message("-- Using Ninja, try to colorize")
  check_cxx_compiler_flag(-fdiagnostics-color FLAG_CXX_DIAGNOSTICS_COLOR)
  if(FLAG_CXX_DIAGNOSTICS_COLOR)
    add_compile_options("-fdiagnostics-color")
  endif()
else()
  message("-- Using " ${CMAKE_GENERATOR})
endif()

add_executable(xmoto)

# find dependencies
find_package(BZip2)
set(USE_SYSTEM_BZip2 $<IF:$<AND:$<BOOL:${PREFER_SYSTEM_BZip2}>,$<BOOL:${BZIP2_FOUND}>,$<NOT:$<BOOL:${WIN32}>>>,ON,OFF>)

find_package(CURL REQUIRED)
if(USE_GETTEXT)
  find_package(Gettext REQUIRED)
  find_package(Intl REQUIRED)
endif()
find_package(JPEG REQUIRED)

find_package(LibLZMA)
find_package(LibXml2 REQUIRED)

find_package(ODE)
set(USE_SYSTEM_ODE $<IF:$<AND:$<BOOL:${PREFER_SYSTEM_ODE}>,$<BOOL:${ODE_FOUND}>>,ON,OFF>)
if(USE_OPENGL)
  find_package(OpenGL REQUIRED)
endif()
find_package(PNG REQUIRED)

find_package(Lua)
set(USE_SYSTEM_Lua $<IF:$<AND:$<BOOL:${PREFER_SYSTEM_Lua}>,$<BOOL:${LUA_FOUND}>,$<NOT:$<BOOL:${WIN32}>>>,ON,OFF>)
if(NOT LUA_VERSION_STRING VERSION_LESS 5.2 AND LUA_VERSION_STRING VERSION_LESS 5.3)
  add_definitions("-DLUA_COMPAT_ALL")
elseif(LUA_VERSION_STRING VERSION_GREATER_EQUAL "5.3")
  add_definitions("-DLUA_COMPAT_5_1")
endif()

# Can't disable yet
if(USE_SDL)
  find_package(SDL REQUIRED)
  find_package(SDL_gfx) # REQUIRED?
  find_package(SDL_mixer REQUIRED)
  find_package(SDL_net REQUIRED)
  find_package(SDL_ttf REQUIRED)
  if(USE_SDL_GFX)
    # find_package(SDL_gfx REQUIRED)
  endif()
else()
  message(FATAL_ERROR "SDL must be used.")
  if(USE_SDL_GFX)
    message(FATAL_ERROR "SDL_gfx must be used with SDL.")
  endif()
endif()

find_package(SQLite3 REQUIRED)

find_package(XDG)
set(USE_SYSTEM_XDG $<IF:$<AND:$<BOOL:${PREFER_SYSTEM_XDG}>,$<BOOL:${XDG_FOUND}>>,ON,OFF>)

find_package(ZLIB REQUIRED)

# sources

if(NOT ${USE_SYSTEM_BZip2})
  add_subdirectory("${PROJECT_SOURCE_DIR}/vendor/bzip2" bzip2)
endif()
add_subdirectory("${PROJECT_SOURCE_DIR}/vendor/chipmunk" chipmunk)
if(NOT ${USE_SYSTEM_Lua})
  add_subdirectory("${PROJECT_SOURCE_DIR}/vendor/lua" lua)
endif()
add_subdirectory("${PROJECT_SOURCE_DIR}/vendor/md5sum" md5sum)
if(NOT ${USE_SYSTEM_ODE})
  add_subdirectory("${PROJECT_SOURCE_DIR}/vendor/libccd" libccd)
  add_subdirectory("${PROJECT_SOURCE_DIR}/vendor/ode" ode)
endif()
if(NOT ${USE_SYSTEM_XDG})
  add_subdirectory("${PROJECT_SOURCE_DIR}/vendor/xdgbasedir" xdgbasedir)
endif()
add_subdirectory("${PROJECT_SOURCE_DIR}/vendor/glad" glad)

include("${CMAKE_CURRENT_SOURCE_DIR}/common/CMakeSources.cmake")
include("${CMAKE_CURRENT_SOURCE_DIR}/db/CMakeSources.cmake")
include("${CMAKE_CURRENT_SOURCE_DIR}/drawlib/CMakeSources.cmake")
include("${CMAKE_CURRENT_SOURCE_DIR}/gui/CMakeSources.cmake")
include("${CMAKE_CURRENT_SOURCE_DIR}/helpers/CMakeSources.cmake")
include("${CMAKE_CURRENT_SOURCE_DIR}/image/CMakeSources.cmake")
include("${CMAKE_CURRENT_SOURCE_DIR}/net/CMakeSources.cmake")
include("${CMAKE_CURRENT_SOURCE_DIR}/states/CMakeSources.cmake")
include("${CMAKE_CURRENT_SOURCE_DIR}/thread/CMakeSources.cmake")
include("${CMAKE_CURRENT_SOURCE_DIR}/include/CMakeSources.cmake")
include("${CMAKE_CURRENT_SOURCE_DIR}/xmoto/CMakeSources.cmake")
include("${CMAKE_CURRENT_SOURCE_DIR}/xmscene/CMakeSources.cmake")

set(XMOTO_MACOS_ICON "${PROJECT_SOURCE_DIR}/extra/X-Moto.icns")

set_source_files_properties(
  "${XMOTO_MACOS_ICON}" PROPERTIES
  MACOSX_PACKAGE_LOCATION Resources
)

#if(STATIC_BUILD)
  #set(CMAKE_FIND_LIBRARY_SUFFIXES ".a")
  #set(BUILD_SHARED_LIBRARIES OFF)
  #set(CMAKE_EXE_LINKER_FLAGS "-static")
#endif()

target_sources(xmoto PRIVATE
  "$<$<PLATFORM_ID:Windows>:${PROJECT_SOURCE_DIR}/extra/xmoto.rc>"
  "$<$<PLATFORM_ID:Darwin>:${XMOTO_MACOS_ICON}>"
)

if(NOT APPLE AND BUILD_MACOS_BUNDLE)
  message(FATAL_ERROR "Can't build a macOS bundle for non-macOS platforms")
elseif(APPLE AND BUILD_MACOS_BUNDLE)
  set_property(TARGET xmoto PROPERTY MACOSX_BUNDLE TRUE)
endif()

target_link_libraries(xmoto PUBLIC CURL::libcurl)
target_compile_definitions(xmoto PUBLIC $<$<BOOL:${STATIC_BUILD}>:CURL_STATICLIB>)
target_link_libraries(xmoto PUBLIC JPEG::JPEG)
target_link_libraries(xmoto PUBLIC LibXml2::LibXml2)
target_compile_definitions(xmoto PUBLIC $<$<BOOL:${STATIC_BUILD}>:LIBXML_STATIC>)
target_link_libraries(xmoto PUBLIC PNG::PNG)
target_link_libraries(xmoto PUBLIC SQLite::SQLite3)
target_link_libraries(xmoto PUBLIC ZLIB::ZLIB)

target_include_directories(xmoto PRIVATE
  "${PROJECT_SOURCE_DIR}/src"
  "$<$<BOOL:${USE_SDL}>:${SDL_INCLUDE_DIR}>"
  "$<$<BOOL:${USE_SDL_GFX}>:${SDL_GFX_INCLUDE_DIRS}>"
  "$<$<BOOL:${USE_SDL}>:${SDL_MIXER_INCLUDE_DIRS}>"
  "$<$<BOOL:${USE_SDL}>:${SDL_NET_INCLUDE_DIRS}>"
  "$<$<BOOL:${USE_SDL}>:${SDL_TTF_INCLUDE_DIRS}>"
  "$<IF:$<BOOL:${USE_SYSTEM_Lua}>,${LUA_INCLUDE_DIR},${PROJECT_SOURCE_DIR}/vendor/lua/lua>"
)

target_link_libraries(xmoto PUBLIC
  $<$<PLATFORM_ID:Windows>:userenv>

  "$<IF:$<BOOL:${USE_SYSTEM_BZip2}>,${BZIP2_LIBRARIES},bzip2>"
  chipmunk
    $<$<BOOL:${STATIC_BUILD}>:gnutls>
      #$<$<BOOL:${STATIC_BUILD}>:idn>
      #$<$<BOOL:${STATIC_BUILD}>:unistring>
    $<$<BOOL:${STATIC_BUILD}>:idn2>
      $<$<BOOL:${STATIC_BUILD}>:unistring>
    $<$<BOOL:${STATIC_BUILD}>:ssh2>
      $<$<BOOL:${STATIC_BUILD}>:gcrypt>
      $<$<BOOL:${STATIC_BUILD}>:gpg-error>
    $<$<BOOL:${STATIC_BUILD}>:wldap32>
  "${Intl_LIBRARIES}"
    "$<$<BOOL:${STATIC_BUILD}>:${LIBLZMA_LIBRARIES}>"
  "$<IF:$<BOOL:${USE_SYSTEM_Lua}>,${LUA_LIBRARIES},lua>"
  md5sum
  "$<IF:$<BOOL:${USE_SYSTEM_ODE}>,${ODE_LIBRARY},ode>"
  glad
  "${CMAKE_DL_LIBS}"
  "${OPENGL_LIBRARIES}"
  "$<$<BOOL:${USE_SDL}>:${SDL_LIBRARY}>"
  "$<$<BOOL:${USE_SDL_GFX}>:${SDL_GFX_LIBRARIES}>"
  "$<$<BOOL:${USE_SDL}>:${SDL_MIXER_LIBRARIES}>"
  "$<$<BOOL:${USE_SDL}>:${SDL_NET_LIBRARIES}>"
  "$<$<BOOL:${USE_SDL}>:${SDL_TTF_LIBRARIES}>"
  "$<IF:$<BOOL:${USE_SYSTEM_XDG}>,${XDG_LIBRARY},xdgbasedir>"
)

set_property(TARGET xmoto PROPERTY CXX_STANDARD 11)
# strip release builds
set_target_properties(xmoto PROPERTIES LINK_FLAGS_RELEASE -s)

check_symbol_exists(putenv stdlib.h HAVE_PUTENV)
check_symbol_exists(setenv stdlib.h HAVE_SETENV)
target_compile_definitions(xmoto PUBLIC HAVE_PUTENV=$<BOOL:${HAVE_PUTENV}>)
target_compile_definitions(xmoto PUBLIC HAVE_SETENV=$<BOOL:${HAVE_SETENV}>)
check_prototype_definition(mkdir
  "int mkdir(const char *_Path)"
  "0"
  "sys/types.h;sys/stat.h"
  MS_MKDIR
)
target_compile_definitions(xmoto PUBLIC MS_MKDIR=$<BOOL:${MS_MKDIR}>)

target_compile_definitions(xmoto PUBLIC USE_OPENGL=$<BOOL:${USE_OPENGL}>)
target_compile_definitions(xmoto PUBLIC USE_SDL=$<BOOL:${USE_SDL}>)
target_compile_definitions(xmoto PUBLIC USE_SDL_GFX=$<BOOL:${USE_SDL_GFX}>)
target_compile_definitions(xmoto PUBLIC USE_GETTEXT=$<BOOL:${USE_GETTEXT}>)
target_compile_definitions(xmoto PUBLIC ALLOW_DEV=$<BOOL:${ALLOW_DEV}>)
target_compile_definitions(xmoto PUBLIC BUILD_MACOS_BUNDLE=$<BOOL:${BUILD_MACOS_BUNDLE}>)


target_compile_definitions(xmoto PUBLIC SVN_REV=\"git\")

# options
target_compile_definitions(xmoto PRIVATE CONFIGURE_DEFAULT_THEME=\"Modern\")

if(CMAKE_SYSTEM_NAME MATCHES Windows)
  target_compile_definitions(xmoto PRIVATE
    DEFAULT_ASIAN_TTF_FILE=\"Textures/Fonts/bkai00mp.ttf\")
else()
  target_compile_definitions(xmoto PRIVATE
    DEFAULT_ASIAN_TTF_FILE=\"/usr/share/fonts/truetype/arphic/bkai00mp.ttf\")
endif()

set(CPACK_RPM_EXCLUDE_FROM_AUTO_FILELIST_ADDITION
  "/usr/share/applications"
  "/usr/share/pixmaps"
  "/usr/share/locale"
)

# Handle localization
# Based on Dolphin Emulator's Source/Core/DolphinWX/CMakeLists.txt (831ae6a)
if(USE_GETTEXT AND GETTEXT_MSGMERGE_EXECUTABLE AND GETTEXT_MSGFMT_EXECUTABLE)
  set(pot_file "${CMAKE_SOURCE_DIR}/i18n/po/xmoto.pot")
  file(GLOB LINGUAS "${CMAKE_SOURCE_DIR}/i18n/po/*.po")

  target_sources(xmoto PRIVATE "${pot_file}" ${LINGUAS})
  source_group("i18n" FILES ${LINGUAS})
  source_group("i18n\\\\generated" FILES "${pot_file}")

  foreach(po ${LINGUAS})
    get_filename_component(lang ${po} NAME_WE)
    set(mo_dir "${CMAKE_CURRENT_BINARY_DIR}/${lang}")
    set(mo "${mo_dir}/xmoto.mo")

    target_sources(xmoto PRIVATE "${mo}")
    source_group("i18n\\\\generated" FILES "${mo}")

    if(BUILD_MACOS_BUNDLE AND CMAKE_SYSTEM_NAME STREQUAL "Darwin")
      set_source_files_properties("${mo}" PROPERTIES
        MACOSX_PACKAGE_LOCATION "Resources/locale/${lang}/LC_MESSAGES"
      )
    else()
      install(FILES "${mo}" DESTINATION share/locale/${lang}/LC_MESSAGES)

      # work around a problem where cpack thinks it's a good
      # idea to throw in the parent directories of files in
      # the rpm that will be installed in the filesystem...
      set(CPACK_RPM_EXCLUDE_FROM_AUTO_FILELIST_ADDITION
        ${CPACK_RPM_EXCLUDE_FROM_AUTO_FILELIST_ADDITION}
        "/usr/share/locale/${lang}"
        "/usr/share/locale/${lang}/LC_MESSAGES"
      )
    endif()

    add_custom_command(OUTPUT "${mo}"
      COMMAND mkdir -p "${mo_dir}"
      COMMAND "${GETTEXT_MSGMERGE_EXECUTABLE}" --quiet --update --backup=none -s ${po} "${pot_file}"
      COMMAND "${GETTEXT_MSGFMT_EXECUTABLE}" -o "${mo}" ${po}
      DEPENDS ${po}
    )
  endforeach()
endif()

# make this visible to its parent (and therefore to cpack/)
set(CPACK_RPM_EXCLUDE_FROM_AUTO_FILELIST_ADDITION
  ${CPACK_RPM_EXCLUDE_FROM_AUTO_FILELIST_ADDITION}
  PARENT_SCOPE
)

#target_compile_options(xmoto PRIVATE -Wall)
#target_compile_options(xmoto PRIVATE -Werror)
# chipmunk on Windows
target_compile_options(xmoto PRIVATE $<$<COMPILE_LANGUAGE:CXX>:-fpermissive>)

function(set_win32_compat_path COMPAT REAL)
  # stupid win32 paths..
  string(REGEX REPLACE "^C:(/|\\\\)?" "/c/" _COMPAT "${REAL}")
  set(${COMPAT} "${_COMPAT}" PARENT_SCOPE)
endfunction()

function(win32_run_script _CMD _ARGS _IS_MSYS2)
  if(IS_MSYS2)
    # msys2's cmake is a native win32 program, and it can't run bash scripts directly
    install(CODE "execute_process(COMMAND bash -l -c '${_CMD} ${_ARGS}' \n\
      RESULT_VARIABLE ret ERROR_VARIABLE err)")
  else()
    install(CODE "execute_process(COMMAND ${_CMD} ${_ARGS} \n\
      RESULT_VARIABLE ret ERROR_VARIABLE err)")
  endif()

  get_filename_component(_SCRIPT_NAME "${_CMD}" NAME)
  install(CODE "\
    if(NOT \"\${ret}\" STREQUAL \"0\") \n\
      message(FATAL_ERROR \"${_SCRIPT_NAME} failed:\n\${err}\") \n\
    endif() \n\
  ")
endfunction()

if(WIN32)
  # MXE exports MSYS=1 so we can't use that to
  # differentiate between it and MSYS2
  if(DEFINED ENV{MSYSTEM_PREFIX})
    set(IS_MSYS2 TRUE)
  else()
    set(IS_MSYS2 FALSE)
  endif()
  message("IS_MSYS2: ${IS_MSYS2}")

  if(IS_MSYS2)
    # use msys prefix path as the target
    set_win32_compat_path(DLLCOPY_TARGET $ENV{MSYSTEM_PREFIX})
  else()
    set(DLLCOPY_TARGET "i686-w64-mingw32.shared")
  endif()
  message("DLLCOPY_TARGET: ${DLLCOPY_TARGET}")

  if(NOT IS_MSYS2)
    # this is required to get rid of the console window
    set(CMAKE_EXE_LINKER_FLAGS "-mwindows")
  endif()

  if(IS_MSYS2)
    set_win32_compat_path(WIN32_COMPAT_SOURCE_DIR "${CMAKE_SOURCE_DIR}")
    set_win32_compat_path(WIN32_COMPAT_CURRENT_BINARY_DIR "${CMAKE_CURRENT_BINARY_DIR}")
    set_win32_compat_path(WIN32_COMPAT_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")
  else()
    set(WIN32_COMPAT_SOURCE_DIR "${CMAKE_SOURCE_DIR}")
    set(WIN32_COMPAT_CURRENT_BINARY_DIR "${CMAKE_CURRENT_BINARY_DIR}")
    set(WIN32_COMPAT_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")
  endif()

  install(
    FILES
      "${PROJECT_SOURCE_DIR}/README.md"
      "${PROJECT_SOURCE_DIR}/COPYING"
      "${PROJECT_SOURCE_DIR}/ChangeLog"
    DESTINATION "./"
  )
  set(WIN32_DLL_DEST_DIR "${CMAKE_CURRENT_BINARY_DIR}/dll")
  set_win32_compat_path(WIN32_COMPAT_DLL_DEST_DIR "${WIN32_DLL_DEST_DIR}")
  set(COPY_DLLS_CMD "${WIN32_COMPAT_SOURCE_DIR}/tools/win32_copy_dlls.sh")
  string(CONCAT COPY_DLLS_ARGS
    " -t \"${DLLCOPY_TARGET}\""
    " -o \"${WIN32_COMPAT_DLL_DEST_DIR}\""
    " -e \"${WIN32_COMPAT_CURRENT_BINARY_DIR}/xmoto.exe\""
    " -r"
  )
  win32_run_script("${COPY_DLLS_CMD}" "${COPY_DLLS_ARGS}" IS_MSYS2)

  install(TARGETS xmoto RUNTIME DESTINATION "./")

  install(
    # NOTE: don't use WIN32_COMPAT_DLL_DEST_DIR here!
    DIRECTORY "${WIN32_DLL_DEST_DIR}/"
    DESTINATION "./"
    FILES_MATCHING PATTERN "*.dll"
  )

  if(CMAKE_BUILD_TYPE STREQUAL "Release")
    # strip the dlls
    set(STRIP_DLLS_CMD "${WIN32_COMPAT_SOURCE_DIR}/tools/win32_strip_dlls.sh")
    string(CONCAT
      STRIP_DLLS_ARGS
      " \"${CMAKE_STRIP}\"" # (path to) strip program
      " \"${WIN32_COMPAT_INSTALL_PREFIX}\"" # DLL folder
    )
    win32_run_script("${STRIP_DLLS_CMD}" "${STRIP_DLLS_ARGS}" IS_MSYS2)
  endif()
elseif(NOT BUILD_MACOS_BUNDLE)
  # for macOS bundles, this is set in cpack/macos.cmake
  # because otherwise fixup_bundle() would complain
  install(TARGETS xmoto RUNTIME DESTINATION bin)
endif()

message("BZip2     libraries: ${BZIP2_LIBRARIES}")
message("libcurl   libraries: ${CURL_LIBRARIES}")
message("Intl      libraries: ${Intl_LIBRARIES}")
message("JPEG      libraries: ${JPEG_LIBRARIES}")
message("LibLZMA   libraries: ${LIBLZMA_LIBRARIES}")
message("LibXml2   libraries: ${LIBXML2_LIBRARIES}")
message("Lua       libraries: ${LUA_LIBRARIES}")
message("ODE       libraries: ${ODE_LIBRARY}")
message("OpenGL    libraries: ${OPENGL_LIBRARIES}")
message("PNG       libraries: ${PNG_LIBRARIES}")
message("SDL       libraries: ${SDL_LIBRARY}")
message("SDL_gfx   libraries: ${SDL_GFX_LIBRARIES}")
message("SDL_mixer libraries: ${SDL_MIXER_LIBRARIES}")
message("SDL_net   libraries: ${SDL_NET_LIBRARIES}")
message("SDL_ttf   libraries: ${SDL_TTF_LIBRARIES}")
message("SQLite3   libraries: ${SQLite3_LIBRARIES}")
message("XDG       libraries: ${XDG_LIBRARY}")
message("ZLIB      libraries: ${ZLIB_LIBRARIES}")
message("")

message("Installation path: ${CMAKE_INSTALL_PREFIX}")
message("Flags:")
if(${ALLOW_DEV})
  message("-- Dev enabled")
endif()
