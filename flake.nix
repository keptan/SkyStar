{
  description = "c++23 buildenv";
  inputs.nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";

  outputs = { self, nixpkgs }:
    let
      supportedSystems = [ "x86_64-linux"];
      forEachSupportedSystem = f: nixpkgs.lib.genAttrs supportedSystems (system: f {
        pkgs = import nixpkgs { inherit system;
		config.allowUnfree = true;};
      });
    in
    {
      devShells = forEachSupportedSystem ({ pkgs }: {
        default = pkgs.mkShell.override
          {
            # Override stdenv in order to change compiler:
             stdenv = pkgs.gcc15Stdenv;
          }
          {
            packages = with pkgs; [
					cmake
					pkg-config
					gdb
					SDL2
					SDL2_ttf
					SDL2_image
					SDL2.dev
					SDL2_mixer
					xorg.libX11
					perf
					valgrind
					jetbrains.clion
            ];

	    shellHook = '' echo Welcome back commander :3 '';
          };
      });
    };
}
