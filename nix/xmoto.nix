{ lib, stdenv, buildPackages, fetchurl, makeWrapper, nix-gitignore
, curl, libjpeg, libpng, libxml2, sqlite, zlib
, enableCmakeNinja ? false
, enableDev ? false
, enableGettext ? false
, enableOpengl ? true, libGL ? null #, libGLU ? null
, enableSdl ? true, SDL ? null, SDL_mixer ? null, SDL_net ? null, SDL_ttf ? null
, enableSdl-gfx ? false, SDL_gfx ? null
, enableSystemBzip2 ? false, bzip2 ? null
, enableSystemLua ? false, lua ? null
, enableSystemOde ? false, ode ? null
, enableSystemXdg ? false, libxdg_basedir ? null
}:

assert enableCmakeNinja -> buildPackages.ninja != null;

# libintl is sometimes null.
assert enableGettext -> buildPackages.gettext != null;
# assert enableGettext -> buildPackages.libintl != null;

assert enableOpengl -> libGL != null;

# Can't disable SDL yet.
assert enableSdl;
assert enableSdl -> SDL != null;
assert enableSdl -> SDL_mixer != null;
assert enableSdl -> SDL_net != null;
assert enableSdl -> SDL_ttf != null;

assert enableSdl-gfx -> enableSdl;
assert enableSdl-gfx -> SDL_gfx != null;

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
  version = "0.6.1-head";

  src = nix-gitignore.gitignoreSource [ "/nix" ] ../.;

  nativeBuildInputs = [
    buildPackages.cmake
    makeWrapper
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
  ] ++  optionals enableOpengl [
    libGL
  ] ++ optionals enableSdl ([
    SDL
    SDL_mixer
    SDL_net
    SDL_ttf
  ] ++ optionals enableSdl-gfx [
    SDL_gfx
  ]) ++ optionals enableSystemBzip2 [
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
    (cmakeOption "USE_SDL" enableSdl)
    (cmakeOption "USE_SDL_GFX" enableSdl-gfx)
    (cmakeOption "PREFER_SYSTEM_BZip2" enableSystemBzip2)
    (cmakeOption "PREFER_SYSTEM_Lua" enableSystemLua)
    (cmakeOption "PREFER_SYSTEM_ODE" enableSystemOde)
    (cmakeOption "PREFER_SYSTEM_XDG" enableSystemXdg)
  ];

  preFixup = ''
    wrapProgram "$out/bin/xmoto" \
      --suffix XDG_DATA_DIRS ':' "$out/share" \
      #
  '';

  meta = with lib; {
    description =
      "A challenging 2D motocross platform game";
    longDescription = ''
      X-Moto is a challenging 2D motocross platform game, where physics play
      an all important role in the gameplay. You need to control your bike to
      its limit, if you want to have a chance finishing the more difficult of
      the challenges.

      First you'll try just to complete the levels, while later you'll compete
      with yourself and others, racing against the clock.
    '';
    homepage = "https://xmoto.tuxfamily.org/";
    downloadPage = "https://github.com/xmoto/xmoto/releases";
    changelog = "https://xmoto.tuxfamily.org/dev/ChangeLog";
    maintainers = with maintainers; [ bb010g raskin pSub ];
    platforms = platforms.all;
    license = licenses.gpl2Plus;
  };
}

