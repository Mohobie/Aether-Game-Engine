#pragma once
#include "math/vec3.h"
#include <vector>
#include <string>
#include <cstdint>
#include <cstring>

namespace vge {

// Client ID type
using ClientID = uint32_t;
constexpr ClientID INVALID_CLIENT_ID = 0;

// ============================================
// Network Serialization Buffer
// ============================================
class NetBuffer {
private:
    std::vector<uint8_t> data;
    size_t readPos;

public:
    NetBuffer() : readPos(0) {}
    explicit NetBuffer(const std::vector<uint8_t>& d) : data(d), readPos(0) {}

    // Write methods
    void WriteInt(int32_t value) {
        const uint8_t* bytes = reinterpret_cast<const uint8_t*>(&value);
        data.insert(data.end(), bytes, bytes + sizeof(int32_t));
    }

    void WriteFloat(float value) {
        const uint8_t* bytes = reinterpret_cast<const uint8_t*>(&value);
        data.insert(data.end(), bytes, bytes + sizeof(float));
    }

    void WriteBool(bool value) {
        data.push_back(value ? 1 : 0);
    }

    void WriteString(const std::string& value) {
        WriteInt(static_cast<int32_t>(value.size()));
        data.insert(data.end(), value.begin(), value.end());
    }

    void WriteVec3(const Vec3& value) {
        WriteFloat(value.x);
        WriteFloat(value.y);
        WriteFloat(value.z);
    }

    void WriteBytes(const std::vector<uint8_t>& bytes) {
        WriteInt(static_cast<int32_t>(bytes.size()));
        data.insert(data.end(), bytes.begin(), bytes.end());
    }

    // Read methods
    int32_t ReadInt() {
        int32_t value = 0;
        if (readPos + sizeof(int32_t) <= data.size()) {
            std::memcpy(&value, &data[readPos], sizeof(int32_t));
            readPos += sizeof(int32_t);
        }
        return value;
    }

    float ReadFloat() {
        float value = 0.0f;
        if (readPos + sizeof(float) <= data.size()) {
            std::memcpy(&value, &data[readPos], sizeof(float));
            readPos += sizeof(float);
        }
        return value;
    }

    bool ReadBool() {
        if (readPos < data.size()) {
            return data[readPos++] != 0;
        }
        return false;
    }

    std::string ReadString() {
        int32_t len = ReadInt();
        if (len <= 0 || readPos + len > static_cast<int32_t>(data.size())) {
            return "";
        }
        std::string value(reinterpret_cast<const char*>(&data[readPos]), len);
        readPos += len;
        return value;
    }

    Vec3 ReadVec3() {
        float x = ReadFloat();
        float y = ReadFloat();
        float z = ReadFloat();
        return Vec3(x, y, z);
    }

    std::vector<uint8_t> ReadBytes() {
        int32_t len = ReadInt();
        if (len <= 0 || readPos + len > static_cast<int32_t>(data.size())) {
            return {};
        }
        std::vector<uint8_t> result(data.begin() + readPos, data.begin() + readPos + len);
        readPos += len;
        return result;
    }

    // Utility
    const std::vector<uint8_t>& GetData() const { return data; }
    size_t GetSize() const { return data.size(); }
    void Clear() { data.clear(); readPos = 0; }
    bool HasMore() const { return readPos < data.size(); }
};

// ============================================
// RPC Arguments
// ============================================
class RPCArgs {
private:
    NetBuffer buffer;

public:
    RPCArgs() = default;
    explicit RPCArgs(const NetBuffer& b) : buffer(b) {}

    void WriteInt(int32_t value) { buffer.WriteInt(value); }
    void WriteFloat(float value) { buffer.WriteFloat(value); }
    void WriteBool(bool value) { buffer.WriteBool(value); }
    void WriteString(const std::string& value) { buffer.WriteString(value); }
    void WriteVec3(const Vec3& value) { buffer.WriteVec3(value); }

    int32_t ReadInt() { return buffer.ReadInt(); }
    float ReadFloat() { return buffer.ReadFloat(); }
    bool ReadBool() { return buffer.ReadBool(); }
    std::string ReadString() { return buffer.ReadString(); }
    Vec3 ReadVec3() { return buffer.ReadVec3(); }

    NetBuffer& GetBuffer() { return buffer; }
    const NetBuffer& GetBuffer() const { return buffer; }
};

} // namespace vge
