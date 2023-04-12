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
          nativeBuildInputs = with pkgs; [ makeWrapper go open-watcom-v2 ];
          src = self;
          buildPhase = ''
            make
          '';
          checkPhase = "";
          WATCOM = "${pkgs.open-watcom-v2.out}";
          vendorHash = "sha256-zIJxqzc9C9tFWMm8R4dvkRmSh7mtfdI95WuzU+My10w=";
        };
      in rec {
        packages = rec {
          default = pkgs.buildGoModule (common // {
            name = "rugburn";
            installPhase = "install -D -t $out/lib out/rugburn.dll";
          });
          web = pkgs.buildGoModule (common // {
            name = "rugburn-web";
            installPhase = "mkdir -p $out && cp -r web/dist $out";
          });
          dockerImage = pkgs.dockerTools.buildImage {
            name = "rugburn-docker";
            config = {
              Cmd = [
                "${pkgs.caddy}/bin/caddy"
                "run"
                "--adapter"
                "caddyfile"
                "--config"
                (pkgs.writeText "caddyfile"
                ''
                :8080
                root * ${web.out}/dist
                file_server
                '')
              ];
            };
          };
        };
        devShell = pkgs.mkShell common;
      }
    );
}
