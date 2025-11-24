#pragma once

#include <benchmark/benchmark.h>

#include <fstream>
#include <stdexcept>

class CsvReporter : public benchmark::BenchmarkReporter {
public:
    explicit CsvReporter(const std::string& filename) : file_(filename) {
        if (!file_.is_open()) {
            throw std::runtime_error("Failed to open CSV file: " + filename);
        }
        file_.precision(6);
        file_ << "serializer,operation,payload_values,blob_size,"
              << "avg_time_us,iterations,cpu_time_us,real_time_us,"
              << "payload_size_bytes\n";
    }

    bool ReportContext(const Context& context) override { return true; }
    
    void ReportRuns(const std::vector<Run>& reports) override {
        for (const auto& run : reports) {
            // Extract parameters from benchmark name
            auto name_parts = SplitName(run.benchmark_name());
            std::string serializer = name_parts[0];
            std::string operation = name_parts[1];
            auto args = ExtractArgs(run.benchmark_name());
            
            file_ << serializer << ","
                  << operation << ","
                  << args[0] << ","  // num_values
                  << args[1] << ","  // blob_size
                  << (run.real_accumulated_time / run.iterations) << ","
                  << run.iterations << ","
                  << (run.cpu_accumulated_time / run.iterations) << ","
                  << (run.real_accumulated_time / run.iterations) << ","
                  << run.counters.at("payload_size").value << "\n";
        }
        file_.flush();
    }

    void Finalize() override {
        file_.close();
    }

private:
    std::vector<std::string> SplitName(const std::string& name) {
        size_t pos = name.find('_');
        if (pos == std::string::npos) return {name, ""};
        return {name.substr(0, pos), name.substr(pos + 1)};
    }

    std::vector<size_t> ExtractArgs(const std::string& name) {
        std::vector<size_t> args;
        size_t start = name.find("/");
        while (start != std::string::npos) {
            size_t end = name.find("/", start + 1);
            std::string arg_str = name.substr(start + 1, 
                end == std::string::npos ? std::string::npos : end - start - 1);
            args.push_back(std::stoul(arg_str));
            start = end;
        }
        return args;
    }

    std::ofstream file_;
};
