with import <nixpkgs> {};

stdenv.mkDerivation rec {
  name = "env";
  nativeBuildInputs = [ makeWrapper open-watcom-v2 go ];
  buildInputs = [ ];
  WATCOM = "${pkgs.open-watcom-v2.out}";
}
