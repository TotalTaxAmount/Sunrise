{ pkgs ? import <nixpkgs> {} }:

pkgs.mkShell {
  buildInputs = with pkgs; [
    cmake
    ninja
    llvmPackages_latest.clang
    llvmPackages_latest.lld
    llvmPackages_latest.llvm
    xwin
  ];

  CC="clang-cl";
  CXX="clang-cl";
  LD="lld-link";
  XWIN_SDK_VERSION="10.0.26100";
  XWIN_CRT_VERSION="17.10.35025";

  hardeningDisable = [ "pic" "fortify" ];

  shellHook = ''
    export XWIN_DIR="$PWD/.xwin-cache"

    if [ ! -d "$XWIN_DIR/sdk" ]; then
      xwin --accept-license splat --include-debug-libs --output "$XWIN_DIR"
    fi
  '';
}
