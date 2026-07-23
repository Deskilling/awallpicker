{
  description = "wallpaper picker";

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
        packages.awallpicker = pkgs.stdenv.mkDerivation {
          pname = "awallpicker";
          version = "0.4.1";
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
            description = "wallpaper picker";
            license = pkgs.lib.licenses.mit;
            maintainers = [
              "Deskilling"
            ];

            platforms = [
              "x86_64-linux"
              "aarch64-linux"
            ];
            mainProgram = "awallpicker";
          };
        };

        packages.default = self.packages.${system}.awallpicker;

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
