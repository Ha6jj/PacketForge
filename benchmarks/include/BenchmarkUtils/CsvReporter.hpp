#pragma once

#include <benchmark/benchmark.h>
#include <fstream>
#include <stdexcept>
#include <vector>
#include <string>
#include <algorithm>

class CsvReporter : public benchmark::BenchmarkReporter {
public:
    explicit CsvReporter(const std::string& filename) : file_(filename) {
        if (!file_.is_open()) {
            throw std::runtime_error("Failed to open CSV file: " + filename);
        }
        file_.precision(6);
        file_ << "serializer,operation,"
              << "iterations,avg_real_time_us,avg_cpu_time_us,"
              << "total_real_time_s,payload_size_bytes\n";
    }

    bool ReportContext(const Context& context) override { return true; }
    
    void ReportRuns(const std::vector<Run>& reports) override {
        for (const auto& run : reports) {
            ProcessRun(run);
        }
        file_.flush();
    }

    void Finalize() override {
        file_.close();
    }

private:
    void ProcessRun(const Run& run) {
        std::string serializer, operation;
        ParseBenchmarkName(run.benchmark_name(), serializer, operation);

        double payload_size = 0.0;
        auto counter_it = run.counters.find("payload_size");
        if (counter_it != run.counters.end()) {
            payload_size = counter_it->second.value;
        }

        double avg_real_time_us = run.GetAdjustedRealTime() * 1e6;
        double avg_cpu_time_us  = run.GetAdjustedCPUTime() * 1e6;

        file_ << serializer << "," 
              << operation << ","
              << run.iterations << ","
              << avg_real_time_us << ","
              << avg_cpu_time_us << ","
              << run.real_accumulated_time << ","
              << payload_size << "\n";
    }

    void ParseBenchmarkName(const std::string& name, 
                           std::string& serializer, 
                           std::string& operation) {
        std::string clean_name = name;
        size_t time_pos = clean_name.find("/real_time");
        if (time_pos != std::string::npos) {
            clean_name.erase(time_pos);
        }
        
        time_pos = clean_name.find("/cpu_time");
        if (time_pos != std::string::npos) {
            clean_name.erase(time_pos);
        }

        size_t args_pos = clean_name.find("/");
        if (args_pos != std::string::npos) {
            clean_name.erase(args_pos);
        }

        size_t underscore_pos = clean_name.find_last_of('_');
        if (underscore_pos == std::string::npos) {
            serializer = clean_name;
            operation = "unknown";
        } else {
            serializer = clean_name.substr(0, underscore_pos);
            operation = clean_name.substr(underscore_pos + 1);
        }
    }

    std::ofstream file_;
};
