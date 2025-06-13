{
  description = "Morphine environment";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-25.05";
    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs =
    {
      self,
      nixpkgs,
      flake-utils,
    }:
    flake-utils.lib.eachDefaultSystem (
      system:
      let
        pkgs = import nixpkgs {
          inherit system;
          config = {
            allowUnfree = true;
            permittedInsecurePackages = [
              "segger-jlink-qt4-810"
            ];
            segger-jlink.acceptLicense = true;
          };
        };
      in
      {
        devShells.default = pkgs.mkShell.override { stdenv = pkgs.gccStdenv; } {
          nativeBuildInputs = with pkgs; [
            meson
            ninja
            gcc-arm-embedded
            openocd
            segger-jlink
          ];

          shellHook = ''
            echo "
CompileFlags:
  Add:
    - -I${pkgs.gcc-arm-embedded}/arm-none-eabi/include
            " > .clangd
          '';
        };
      }
    );
}
