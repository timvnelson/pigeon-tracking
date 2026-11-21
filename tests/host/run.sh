#!/bin/sh
# Builds and runs the computer-side tests of the schedule maths (no board
# needed). Needs a C++ compiler (g++ or clang++).
# Usage, from the repo root: sh tests/host/run.sh
set -e
here=$(cd "$(dirname "$0")" && pwd)
repo=$(cd "$here/../.." && pwd)
work=$(mktemp -d)
trap 'rm -rf "$work"' EXIT
# SolarSchedule.cpp includes "config.h" from its own folder, so copy it next
# to the test's stand-in config.h instead of the real one.
cp "$repo/loft/SolarSchedule.cpp" "$repo/loft/SolarSchedule.h" "$here/config.h" "$here/test_solar.cpp" "$work/"
${CXX:-g++} -std=c++17 -O2 -Wall -Wextra -Werror -o "$work/test_solar" "$work/test_solar.cpp" "$work/SolarSchedule.cpp"
"$work/test_solar"
