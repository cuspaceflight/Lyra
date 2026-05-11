{
  description = "Lyra source code";

  inputs.nixpkgs.url = "github:Nixos/nixpkgs/nixos-unstable";

  outputs = { self, nixpkgs, ... }: let
    system = "x86_64-linux";
    pkgs = nixpkgs.legacyPackages.${system};
    custom-pico-sdk = pkgs.pico-sdk.override {
      withSubmodules = true;
    };
    in {
    devShells.${system}.default = pkgs.mkShell {
      nativeBuildInputs = with pkgs; [
        custom-pico-sdk
        cmake
        gnumake
        python3
        ninja
        gcc-arm-embedded-13
      ];

      packages = with pkgs; [
        picotool
        tio
      ];

      PICO_SDK_PATH = "${custom-pico-sdk}/lib/pico-sdk";
    };

    packages.${system}.default = pkgs.stdenv.mkDerivation rec {
      pname = "Lyra";
      version = "0.1.0";
      name = "Lyra-${version}";

      src = ./.;

      nativeBuildInputs = with pkgs; [
        cmake
        custom-pico-sdk
        gcc-arm-embedded-13
        ninja
        python3
        picotool
      ];
      buildInputs = [
      ];

      dontUseCmakeConfigure = true;

      buildPhase = ''
        mkdir -p build
        cmake -B build -G Ninja -DPICO_SDK_PATH=${custom-pico-sdk}/lib/pico-sdk -DCMAKE_EXPORT_COMPILE_COMMANDS=1 .
        cmake --build build
      '';

      installPhase = ''
        mkdir -p $out
        cp build/src/Lyra.* $out/
        cp build/compile_commands.json $out/
      '';
    };
  };
}
