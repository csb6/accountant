#!/usr/bin/env sh

# Run from root of project tree

set -e

ASAN_OPTIONS=detect_leaks=1 LSAN_OPTIONS=suppressions=scripts/leak-suppressions.txt build/src/accountant $@
