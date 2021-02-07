{ ... } @ commonArgs: let
  pkgs = commonArgs.pkgs or import <nixpkgs> { };
  commonArgsPruned = builtins.removeAttrs commonArgs [
    "pkgs"
  ];
common = {
  xmoto = pkgs.callPackage ./xmoto.nix commonArgsPruned;
  xmotoShell = pkgs.mkShell {
    name = "xmotoShell";
    inputsFrom = [ (common.xmoto.override (o: {
      enableCmakeNinja = true;
      enableDev = true;
      enableGettext = true;
      enableOpengl = true;
      enableSdl = true;
      enableSdl-gfx = true;
      enableSystem = true;
    })) ];
    nativeBuildInputs = [
      pkgs.clang-tools
      pkgs.include-what-you-use
    ];
  };
}; in common
