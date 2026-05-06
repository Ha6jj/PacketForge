#pragma once

#include "BenchmarkUtils/ISerializer.hpp"

#include <google/protobuf/message.h>
#include <string>
#include <stdexcept>

template <typename Data, const char* Name>
class ProtobufSerializer : public ISerializer<Data> {
public:
    std::string GetName() const override {
        return Name;
    }

    std::string Serialize(const Data& data) override {
        std::string serialized;
        if (!data.SerializeToString(&serialized)) {
            throw std::runtime_error(std::string("Protobuf serialization failed: ") + Name);
        }
        return serialized;
    }

    Data Deserialize(const std::string& serialized) override {
        Data data;
        if (!data.ParseFromString(serialized)) {
            throw std::runtime_error(std::string("Protobuf deserialization failed: ") + Name);
        }
        return data;
    }

    size_t GetSerializedSize(const Data& data) override {
        return data.ByteSizeLong();
    }
};