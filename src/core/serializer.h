#pragma once
#include <vector>
#include <stdint>
#include <string>
namespace vge {
class Serializer {
public:
    void writeInt32(int32_t value);
    void writeInt64(int64_t value);
    void writeFloat(float value);
    void writeDouble(double value);
    void writeString(const std::string& value);
    void writeBytes(const std::vector<uint8_t>& data);
    int32_t readInt32();
    int64_t readInt64();
    float readFloat();
    double readDouble();
    std::string readString();
    std::vector<uint8_t> readBytes(size_t length);
    const std::vector<uint8_t>& getData() const { return data; }
    void setData(const std::vector<uint8_t>& d) { data = d; position = 0; }
    size_t getPosition() const { return position; }
    size_t getSize() const { return data.size(); }
private:
    std::vector<uint8_t> data;
    size_t position = 0;
    void writeRaw(const void* ptr, size_t size);
    void readRaw(void* ptr, size_t size);
};
} // namespace vge
