#include "network/packet.h"
#include <cstring>
#include <iostream>

namespace vge {

// Packet implementation
Packet::Packet() : readPos(0) {}

void Packet::WriteByte(uint8_t value) {
    data.push_back(value);
}

void Packet::WriteInt(int32_t value) {
    data.push_back((value >> 24) & 0xFF);
    data.push_back((value >> 16) & 0xFF);
    data.push_back((value >> 8) & 0xFF);
    data.push_back(value & 0xFF);
}

void Packet::WriteFloat(float value) {
    uint32_t bits;
    std::memcpy(&bits, &value, sizeof(float));
    WriteInt((int32_t)bits);
}

void Packet::WriteString(const std::string& value) {
    WriteInt((int32_t)value.length());
    for (char c : value) {
        WriteByte((uint8_t)c);
    }
}

uint8_t Packet::ReadByte() {
    if (readPos >= data.size()) return 0;
    return data[readPos++];
}

int32_t Packet::ReadInt() {
    if (readPos + 4 > data.size()) return 0;
    int32_t value = (data[readPos] << 24) | (data[readPos + 1] << 16) | 
                    (data[readPos + 2] << 8) | data[readPos + 3];
    readPos += 4;
    return value;
}

float Packet::ReadFloat() {
    int32_t bits = ReadInt();
    float value;
    std::memcpy(&value, &bits, sizeof(float));
    return value;
}

std::string Packet::ReadString() {
    int32_t length = ReadInt();
    if (length <= 0 || readPos + length > (int32_t)data.size()) return "";
    std::string value((char*)&data[readPos], length);
    readPos += length;
    return value;
}

void Packet::ResetRead() {
    readPos = 0;
}

void Packet::Clear() {
    data.clear();
    readPos = 0;
}

} // namespace vge