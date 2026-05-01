#!/usr/bin/bash
set -euo pipefail

PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="${PROJECT_ROOT}/build"
RESULTS_BASE="${PROJECT_ROOT}/benchmark_results"
TIMESTAMP=$(date +"%Y-%m-%d_%H-%M-%S")
RUN_DIR="${RESULTS_BASE}/${TIMESTAMP}"

GREEN='\033[0;32m'
BLUE='\033[0;34m'
YELLOW='\033[1;33m'
NC='\033[0m'

echo -e "${BLUE}Benchmark Pipeline Started${NC}"

echo -e "${BLUE}Configuring & Building ...${NC}"
cmake -B "${BUILD_DIR}" -S "${PROJECT_ROOT}" -DBUILD_BENCHMARKS=ON
cmake --build "${BUILD_DIR}" --parallel "$(nproc)"

echo -e "${BLUE} Discovering & Running Benchmarks...${NC}"
BENCHMARKS=()
while IFS= read -r -d '' bench; do
    BENCHMARKS+=("$bench")
done < <(find "${BUILD_DIR}" -type f -executable -name "*_benchmark" -print0 2>/dev/null)

if [ ${#BENCHMARKS[@]} -eq 0 ]; then
    echo -e "${YELLOW}No benchmark executables found!${NC}"
    exit 1
fi

mkdir -p "${RUN_DIR}"

for bench in "${BENCHMARKS[@]}"; do
    name=$(basename "$bench" | sed 's/_benchmark$//')
    mkdir -p "${RUN_DIR}"

    echo -e "${GREEN}Running: ${name}${NC}"
    (cd "${RUN_DIR}" && "$bench")
done

echo -e "\n${BLUE}All benchmarks completed!${NC}"
echo -e "Results saved to: ${RUN_DIR}"
echo "Generated CSVs:"
find "${RUN_DIR}" -name "*.csv" -exec ls -lh {} \; 2>/dev/null || echo "No CSV files found."