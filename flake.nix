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
          pkgs.pkgsCross.mingw32.stdenv.cc
        ];

        nativeCheckInputs = [ pkgs.python3Packages.tappy ];

        # Rugburn derivation
        rugburn = pkgs.stdenv.mkDerivation {
          name = "rugburn";
          src = self;

          inherit nativeBuildInputs;

          nativeCheckInputs = nativeCheckInputs ++ [ pkgs.winePackages.minimal ];

          FONTCONFIG_FILE = pkgs.makeFontsConf { fontDirectories = [ ]; };

          checkPhase = ''
            runHook preCheck

            export WINEPREFIX="$PWD/.wine"
            export WINEARCH="win32"
            HOME="$PWD" WINEDEBUG="-all" wineboot
            HOME="$PWD" WINEDEBUG="fixme-all" make check

            runHook postCheck
          '';

          doCheck = true;

          buildPhase = ''
            runHook preBuild

            make -j

            (printf -- "-nostdlib\n-nostdinc\n-nostdinc++\n--target=i686-pc-windows-gnu\n-D_WIN32\n-D__MINGW32__\n-DSTRSAFE_NO_DEPRECATE" \
              && </dev/null i686-w64-mingw32-gcc -E -v - 2>&1 \
                | grep ^COLLECT_GCC_OPTIONS= \
                | tail -1 \
                | cut -d= -f2- \
                | xargs -n1 printf "%s\n" \
            ) > compile_flags.txt

            runHook postBuild
          '';

          installPhase = ''
            runHook preInstall

            install -D -t $out/lib out/ijl15.dll
            install -D --mode=0644 -t $out compile_flags.txt

            runHook postInstall
          '';
        };

        # Dev shell
        devShell = pkgs.mkShell {
          nativeBuildInputs = nativeBuildInputs ++ nativeCheckInputs ++ [ pkgs.clang.cc ];
        };

        # Clangd setup
        setupClangd = pkgs.writeShellScriptBin "setup-clangd.sh" ''
          install -D --mode=0644 -t . ${rugburn}/compile_flags.txt
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
