{
  description = "";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-26.05";
    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs =
    {
      self,
      nixpkgs,
      flake-utils,
      ...
    }:
    flake-utils.lib.eachDefaultSystem (
      system:
      let
        pkgs = nixpkgs.legacyPackages.${system};
      in
      {
        packages.wallpicker = pkgs.stdenv.mkDerivation {
          pname = "hypr-wallpicker";
          version = "0.4.0";
          src = ./.;

          nativeBuildInputs = with pkgs; [
            gcc
            gnumake
          ];

          buildInputs = with pkgs; [
            raylib
          ];

          buildPhase = "make";
          installPhase = ''
            make install PREFIX=$out
          '';

          meta = {
            description = "";
            license = pkgs.lib.licenses.mit;
            maintainers = [
              ""
            ];

            platforms = [
              "x86_64-linux"
              "aarch64-linux"
            ];
            mainProgram = "wallpicker";
          };
        };

        packages.default = self.packages.${system}.wallpicker;

        devShells.default = pkgs.mkShell {
          buildInputs = with pkgs; [
            gcc
            clang-tools
            gnumake
            raylib
          ];
        };
      }
    );
}
