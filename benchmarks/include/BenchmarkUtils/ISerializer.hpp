#pragma once

#include <string>

template <typename Data>
class ISerializer {
public:
    virtual ~ISerializer() = default;
    virtual std::string GetName() const = 0;
    virtual std::string Serialize(const Data& data) = 0;
    virtual Data Deserialize(const std::string& serialized) = 0;
    
    virtual size_t GetSerializedSize(const Data& data) {
        return Serialize(data).size();
    }
};