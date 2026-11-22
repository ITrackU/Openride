#!/usr/bin/env bash
# Builds and runs every core/<module> unit-test project. Used locally and by CI.
# Each module is its own CMake project, so we loop instead of keeping a root CMakeLists.
set -u
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
fail=0
for dir in "$ROOT"/core/*/; do
  [ -f "$dir/CMakeLists.txt" ] || continue
  name="$(basename "$dir")"
  build="$(mktemp -d)"
  echo "=== core/$name ==="
  if cmake -S "$dir" -B "$build" >/dev/null && cmake --build "$build" >/dev/null && (cd "$build" && ctest --output-on-failure); then
    :
  else
    echo "FAILED: core/$name"; fail=1
  fi
  rm -rf "$build"
done
exit $fail
