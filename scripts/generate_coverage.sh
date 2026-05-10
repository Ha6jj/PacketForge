#!/bin/bash
set -euo pipefail

echo "Dependencies check..."
if ! command -v gcovr &> /dev/null; then
    echo "Installing gcovr..."
    pip3 install --user gcovr
    export PATH="$HOME/.local/bin:$PATH"
fi

PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_DIR="${PROJECT_ROOT}/build"
COVERAGE_DIR="${BUILD_DIR}/coverage"

echo "Cleaning & Configuring..."
rm -rf "${BUILD_DIR}"
cmake -B "${BUILD_DIR}" -S "${PROJECT_ROOT}" -DBUILD_TESTS=ON -DENABLE_COVERAGE=ON

echo "Building..."
cmake --build "${BUILD_DIR}" --parallel "$(nproc)"

echo "Running Tests..."
ctest --test-dir "${BUILD_DIR}" --output-on-failure

mkdir "${COVERAGE_DIR}"

echo "Report generation..."
gcovr \
    --root "${PROJECT_ROOT}"    \
    --filter "include/PacketForge"         \
    --html "${COVERAGE_DIR}/coverage_report.html"    \
    --gcov-ignore-parse-errors=negative_hits.warn_once_per_file \
    --html-details              \
    --print-summary

echo "Report: file://$(pwd)/build/coverage_report.html"