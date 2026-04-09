#!/usr/bin/env bash
set -euo pipefail

repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
duration_seconds="${1:-8}"
demo_binary="$repo_root/demo"

if [[ ! -x "$demo_binary" ]]; then
  echo "The demo binary was not found. Run scripts/setup-dev.sh or make first." >&2
  exit 1
fi

if ! command -v xvfb-run >/dev/null 2>&1; then
  echo "xvfb-run is required for headless execution." >&2
  exit 1
fi

if ! command -v timeout >/dev/null 2>&1; then
  exec xvfb-run -a "$demo_binary"
fi

set +e
xvfb-run -a timeout "${duration_seconds}s" "$demo_binary"
status=$?
set -e

if [[ "$status" -eq 124 ]]; then
  echo "The demo ran headlessly for ${duration_seconds}s and was stopped intentionally."
  exit 0
fi

if [[ "$status" -ne 0 ]]; then
  exit "$status"
fi

echo "The demo exited successfully."
