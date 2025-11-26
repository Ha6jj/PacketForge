#pragma once

#include "core/BenchmarkData.hpp"

#include <string>

class ISerializer {
public:
    virtual ~ISerializer() = default;
    virtual std::string GetName() const = 0;
    virtual std::string Serialize(const BenchmarkData& data) = 0;
    virtual BenchmarkData Deserialize(const std::string& serialized) = 0;
    
    virtual size_t GetSerializedSize(const BenchmarkData& data) {
        return Serialize(data).size();
    }
};