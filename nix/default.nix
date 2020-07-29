{ ... } @ commonArgs: let
  pkgs = commonArgs.pkgs or import <nixpkgs> { };
  commonArgsPruned = builtins.removeAttrs commonArgs [
    "pkgs"
  ];
common = {
  xmoto = pkgs.callPackage ./xmoto.nix commonArgsPruned;
}; in common
