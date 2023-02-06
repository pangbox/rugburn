with import <nixpkgs> {};

stdenv.mkDerivation rec {
  name = "env";
  nativeBuildInputs = [ makeWrapper open-watcom-v2 ];
  buildInputs = [ ];
}
