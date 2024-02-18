{
  description = "Rugburn, a patcher for PangYa";

  inputs = {
    nixpkgs.url = "github:nixos/nixpkgs";
    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs = { self, nixpkgs, flake-utils }:
    flake-utils.lib.eachDefaultSystem (system:
      let
        pkgs = nixpkgs.legacyPackages.${system};
        common = {
          name = "rugburn";
          src = self;
          nativeBuildInputs = with pkgs; [
            # Build
            makeWrapper
            go_1_22
            open-watcom-v2

            # Test
            python3Packages.tappy
            wine
          ];
          checkPhase = ''
            export WINEPREFIX="$TMPDIR/.wine"
            make check
          '';
          buildPhase = ''
            make
          '';
          installPhase = ''
            install -D -t $out/lib out/rugburn.dll
            cp -r web/dist $out/dist
          '';
          vendorHash = (pkgs.lib.fileContents ./go.mod.sri);
          WATCOM = "${pkgs.open-watcom-v2.out}";
        };
        rugburn = pkgs.buildGo122Module common;
        devShell = pkgs.mkShell (common // {
          nativeBuildInputs = with pkgs; [
            # Build
            makeWrapper
            go_1_22
            open-watcom-v2

            # Test
            python3Packages.tappy

            # Development
            gopls
            clang-tools
          ];
        });
        setupClangd = pkgs.writeShellScriptBin "setup-clangd.sh" ''
          export WATCOM="${pkgs.open-watcom-v2.out}";
          exec ${./scripts/setup-clangd.sh} "$@"
        '';
        updateVendorHash = pkgs.writeShellScriptBin "update-vendor-hash.sh" ''
          export PATH="${pkgs.lib.makeBinPath [ pkgs.go_1_22 ]}:$PATH"
          exec ${./scripts/update-vendor-hash.sh} "$@"
        '';
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
      in {
        inherit devShell;
        packages = {
          inherit rugburn setupClangd updateVendorHash web dockerImage;
          default = rugburn;
        };
      }
    );
}
