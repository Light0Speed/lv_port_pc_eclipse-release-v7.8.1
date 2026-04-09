#!/usr/bin/env bash
set -euo pipefail

repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
jobs="$(getconf _NPROCESSORS_ONLN 2>/dev/null || nproc 2>/dev/null || echo 4)"

if command -v apt-get >/dev/null 2>&1; then
  packages=(build-essential gcc libsdl2-dev xvfb)
  missing_packages=()

  for package in "${packages[@]}"; do
    if ! dpkg -s "$package" >/dev/null 2>&1; then
      missing_packages+=("$package")
    fi
  done

  if ((${#missing_packages[@]} > 0)); then
    sudo apt-get update
    sudo apt-get install -y "${missing_packages[@]}"
  fi
else
  echo "apt-get was not found; install SDL2 development headers and Xvfb manually." >&2
fi

make -C "$repo_root" clean
make -C "$repo_root" -j"$jobs"

echo "Build complete: $repo_root/demo"
