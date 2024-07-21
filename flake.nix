{
  description = "Rugburn, a patcher for PangYa";

  inputs = {
    nixpkgs.url = "github:nixos/nixpkgs";
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
        pkgs = nixpkgs.legacyPackages.${system};

        # Common dependencies
        nativeBuildInputs = [
          pkgs.makeWrapper
          pkgs.bear
          pkgs.pkgsCross.mingw32.stdenv.cc
        ];

        nativeCheckInputs = [ pkgs.python3Packages.tappy ];

        # Rugburn derivation
        rugburn = pkgs.stdenv.mkDerivation {
          name = "rugburn";
          src = self;

          inherit nativeBuildInputs;

          nativeCheckInputs = nativeCheckInputs ++ [ pkgs.wine ];

          checkPhase = ''
            runHook preCheck

            export WINEPREFIX="$TMPDIR/.wine"
            make check

            runHook postCheck
          '';

          buildPhase = ''
            runHook preBuild

            bear -- make -j

            runHook postBuild
          '';

          installPhase = ''
            runHook preInstall

            install -D -t $out/lib out/ijl15.dll
            install -D --mode=0644 -t $out compile_commands.json

            runHook postInstall
          '';
        };

        # Dev shell
        devShell = pkgs.mkShell { nativeBuildInputs = nativeBuildInputs ++ nativeCheckInputs; };

        # Clangd setup
        setupClangd = pkgs.writeShellScriptBin "setup-clangd.sh" ''
          install -D --mode=0644 -t . ${rugburn}/compile_commands.json
        '';

        # Website setup
        caddyfile = pkgs.writeText "caddyfile" ''
          :8080
          root * ${rugburn}/dist
          file_server
        '';

        web = pkgs.writeShellScriptBin "web.sh" ''
          exec ${pkgs.caddy}/bin/caddy run --adapter caddyfile --config ${caddyfile}
        '';

        dockerImage = pkgs.dockerTools.buildImage {
          name = "rugburn-docker";
          config.Cmd = [ "${web}/bin/web.sh" ];
        };
      in
      {
        inherit devShell;
        packages = {
          inherit
            rugburn
            setupClangd
            web
            dockerImage
            ;
          default = rugburn;
        };
      }
    );
}
