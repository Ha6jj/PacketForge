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
        file_ << "serializer,label,operation,trial,"
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
        std::string serializer, operation, trial, label;
        ParseBenchmarkName(run.benchmark_name(), serializer, label, operation, trial);

        double payload_size = 0.0;
        auto counter_it = run.counters.find("payload_size");
        if (counter_it != run.counters.end()) {
            payload_size = counter_it->second.value;
        }

        double avg_real_time_us = run.GetAdjustedRealTime() * 1e6;
        double avg_cpu_time_us  = run.GetAdjustedCPUTime() * 1e6;

        file_ << serializer << "," 
              << label << ","
              << operation << ","
              << trial << ","
              << run.iterations << ","
              << avg_real_time_us << ","
              << avg_cpu_time_us << ","
              << run.real_accumulated_time << ","
              << payload_size << "\n";
    }

   void ParseBenchmarkName(const std::string& name,
                           std::string& serializer,
                           std::string& label,
                           std::string& operation,
                           std::string& trial) {
        std::string clean = name;

        for (const char* suffix : {"/real_time", "/cpu_time"}) {
            auto pos = clean.find(suffix);
            if (pos != std::string::npos) clean.erase(pos);
        }

        size_t args_pos = clean.find('/');
        if (args_pos != std::string::npos) {
            trial = clean.substr(args_pos + 1);
            clean.erase(args_pos);
        } else {
            trial = "no_args";
        }

        size_t last_ = clean.find_last_of('_');
        if (last_ == std::string::npos) {
            serializer = clean; label = "unknown"; operation = "unknown"; return;
        }
        operation = clean.substr(last_ + 1);
        clean.erase(last_);

        size_t prev_ = clean.find_last_of('_');
        if (prev_ == std::string::npos) {
            serializer = clean; label = "unknown"; return;
        }
        label = clean.substr(prev_ + 1);
        serializer = clean.substr(0, prev_);
    }

    std::ofstream file_;
};
