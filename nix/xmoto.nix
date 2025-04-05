{ lib, stdenv, buildPackages, fetchurl, nix-gitignore
, curl, libjpeg, libpng, libxml2, sqlite, zlib
, enableCmakeNinja ? false
, enableDev ? false
, enableGettext ? false
, enableOpengl ? true, libGL ? null, libGLU ? null
, enableSdl ? false, SDL2 ? null, SDL2_gfx ? null, SDL2_mixer ? null, SDL2_net ? null, SDL2_ttf ? null
, enableSystemBzip2 ? false, bzip2 ? null
, enableSystemLua ? false, lua ? null
, enableSystemOde ? false, ode ? null
, enableSystemXdg ? false, libxdg_basedir ? null
}:

assert enableCmakeNinja -> buildPackages.ninja != null;

assert enableGettext -> buildPackages.gettext != null;
assert enableGettext -> buildPackages.libintl != null;

# Can't disable OpenGL yet.
assert libGL != null;

assert enableOpengl -> libGL != null;

# Can't disable SDL yet.
assert SDL2 != null;
assert SDL2_mixer != null;
assert SDL2_net != null;
assert SDL2_ttf != null;

assert enableSdl -> SDL2 != null;
assert enableSdl -> SDL2_gfx != null;
assert enableSdl -> SDL2_mixer != null;
assert enableSdl -> SDL2_net != null;
assert enableSdl -> SDL2_ttf != null;

assert enableSystemBzip2 -> bzip2 != null;
assert enableSystemLua -> lua != null;
assert enableSystemOde -> ode != null;
assert enableSystemXdg -> libxdg_basedir != null;

let
  inherit (lib) optionals;
  cmakeCacheEntry = var: value: "-D${var}=${value}";
  cmakeCacheTypedEntry = var: type: value: "-D${var}:${type}=${value}";
  cmakeOption = var: value:
    cmakeCacheTypedEntry var "BOOL" (if value then "ON" else "OFF");
in
stdenv.mkDerivation rec {
  pname = "xmoto";
  version = "0.6.2-head";

  src = nix-gitignore.gitignoreSource [ "/nix" ] ../.;

  nativeBuildInputs = [
    buildPackages.cmake
  ] ++ optionals enableCmakeNinja [
    buildPackages.ninja
  ] ++ optionals enableGettext [
    buildPackages.gettext
    buildPackages.libintl
  ];

  buildInputs = [
    curl
    libjpeg
    libpng
    libxml2
    sqlite
    zlib
  ] ++ [
    libGL
    libGLU
  ] ++ optionals enableOpengl [
    # libGL
  ] ++ [
    SDL2
    SDL2_mixer
    SDL2_net
    SDL2_ttf
  ] ++ optionals enableSdl [
    # SDL
    SDL2_gfx
    # SDL_mixer
    # SDL_net
    # SDL_ttf
  ] ++ optionals enableSystemBzip2 [
    bzip2
  ] ++ optionals enableSystemLua [
    lua
  ] ++ optionals enableSystemOde [
    ode
  ] ++ optionals enableSystemXdg [
    libxdg_basedir
  ];

  cmakeFlags = [
    (cmakeOption "ALLOW_DEV" enableDev)
    (cmakeOption "USE_GETTEXT" enableGettext)
    (cmakeOption "USE_OPENGL" enableOpengl)
    (cmakeOption "USE_SDLGFX" enableSdl)
    (cmakeOption "PREFER_SYSTEM_BZip2" enableSystemBzip2)
    (cmakeOption "PREFER_SYSTEM_Lua" enableSystemLua)
    (cmakeOption "PREFER_SYSTEM_ODE" enableSystemOde)
    (cmakeOption "PREFER_SYSTEM_XDG" enableSystemXdg)
  ];

  meta = with lib; {
    description =
      "A challenging 2D motocross platform game, where physics play an important role";
    longDescription = ''
      X-Moto is a challenging 2D motocross platform game, where physics plays an all important role in the gameplay.
      You need to control your bike to its limits, if you want to have a chance to finish the most difficult challenges.
    '';
    homepage = "https://xmoto.org";
    maintainers = with maintainers; [ bb010g raskin pSub ];
    platforms = platforms.all;
    license = licenses.gpl2Plus;
  };
}

