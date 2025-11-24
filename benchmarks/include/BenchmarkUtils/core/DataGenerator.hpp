#pragma once

#include "BenchmarkData.hpp"

#include <random>
#include <ctime>

class DataGenerator {
public:
    static BenchmarkData Generate(size_t num_values, size_t blob_size) {
        BenchmarkData data;
        std::mt19937 gen(std::random_device{}());
        std::uniform_real_distribution<> dist(0.0, 1.0);
        
        // Generate values
        data.values.reserve(num_values);
        for (size_t i = 0; i < num_values; ++i) {
            data.values.push_back(dist(gen));
        }

        // Generate metadata
        data.metadata["source"] = "benchmark";
        data.metadata["version"] = "1.0";
        data.metadata["items_count"] = std::to_string(num_values);
        
        // Generate other fields
        data.flag = (gen() % 2 == 0);
        data.timestamp = std::time(nullptr);
        data.blob = std::string(blob_size, static_cast<char>(gen() % 256));
        
        return data;
    }
};
