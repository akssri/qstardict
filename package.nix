{
  lib,
  stdenv,
  fetchgit,
  qt6,
  pkg-config,
  glib,
  zlib,
  xorg,
  libxkbcommon,
  espeak-ng,
}:

stdenv.mkDerivation (finalAttrs: {
  pname = "qstardict";
  version = "3.0.0-unstable-2026-02-26";

  src = fetchgit {
    url = "https://hub.mos.ru/a-rodin/qstardict.git";
    rev = "7d73b341717e3a42ab4f49bfd79a4636ef861e18";
    hash = "sha256-pRLcWtZ2kpV4Gje8M5bGKbSB5PEM9icAr9+EKH2SdIE=";
  };

  patches = [
    ./patches/0001-per-dict-transliteration-scheme.patch
  ];

  nativeBuildInputs = [
    qt6.qmake
    qt6.wrapQtAppsHook
    pkg-config
  ];

  buildInputs = [
    qt6.qtbase
    qt6.qtmultimedia
    glib
    zlib
    xorg.libX11
    xorg.libxcb
    libxkbcommon
  ];

  qmakeFlags = [
    "INSTALL_PREFIX=${placeholder "out"}"
  ];

  qtWrapperArgs = [
    "--prefix PATH : ${lib.makeBinPath [ espeak-ng ]}"
  ];

  meta = {
    description = "Dictionary application for learning languages with StarDict 2.x support";
    homepage = "https://hub.mos.ru/a-rodin/qstardict";
    license = lib.licenses.gpl2Plus;
    platforms = lib.platforms.linux;
    mainProgram = "qstardict";
  };
})
