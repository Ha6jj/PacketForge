#pragma once

#include "core/BenchmarkData.hpp"

// !!! RENAME CLASS !!!
class ISerializer {
public:
    virtual ~ISerializer() = default;
    
    // Unique identifier for this serializer (e.g., "protobuf", "flatbuffers")
    virtual std::string GetName() const = 0;
    
    // Serialize BenchmarkData to bytes
    virtual std::string Serialize(const BenchmarkData& data) = 0;
    
    // Deserialize bytes back to BenchmarkData
    virtual BenchmarkData Deserialize(const std::string& serialized) = 0;
    
    // Optional: Get serialized size without full serialization
    virtual size_t GetSerializedSize(const BenchmarkData& data) {
        return Serialize(data).size();
    }
};
