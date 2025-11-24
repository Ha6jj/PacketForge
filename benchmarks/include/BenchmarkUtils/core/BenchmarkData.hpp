#pragma once

#include <vector>
#include <string>
#include <map>

struct BenchmarkData {
    std::vector<double> values;
    std::map<std::string, std::string> metadata;
    bool flag;
    int64_t timestamp;
    std::string blob;
};
