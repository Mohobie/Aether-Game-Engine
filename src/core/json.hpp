#pragma once
#include <string>
#include <vector>
#include <map>
#include <stdexcept>
#include <sstream>

namespace nlohmann {

class json {
public:
    enum class value_t {
        null,
        object,
        array,
        string,
        boolean,
        number_integer,
        number_float
    };

private:
    value_t type;
    std::string stringValue;
    long long intValue;
    double floatValue;
    bool boolValue;
    std::map<std::string, json> objectValue;
    std::vector<json> arrayValue;

public:
    json() : type(value_t::null) {}
    json(const std::string& s) : type(value_t::string), stringValue(s) {}
    json(const char* s) : type(value_t::string), stringValue(s) {}
    json(long long i) : type(value_t::number_integer), intValue(i) {}
    json(int i) : type(value_t::number_integer), intValue(i) {}
    json(double d) : type(value_t::number_float), floatValue(d) {}
    json(float f) : type(value_t::number_float), floatValue(f) {}
    json(bool b) : type(value_t::boolean), boolValue(b) {}

    static json object() { json j; j.type = value_t::object; return j; }
    static json array() { json j; j.type = value_t::array; return j; }

    bool contains(const std::string& key) const {
        if (type != value_t::object) return false;
        return objectValue.find(key) != objectValue.end();
    }

    json& operator[](const std::string& key) {
        if (type == value_t::null) type = value_t::object;
        return objectValue[key];
    }

    const json& operator[](const std::string& key) const {
        auto it = objectValue.find(key);
        if (it == objectValue.end()) {
            static json null_json;
            return null_json;
        }
        return it->second;
    }

    json& operator[](size_t index) {
        if (type == value_t::null) { type = value_t::array; }
        if (index >= arrayValue.size()) arrayValue.resize(index + 1);
        return arrayValue[index];
    }

    void push_back(const json& value) {
        if (type == value_t::null) type = value_t::array;
        arrayValue.push_back(value);
    }

    template<typename T>
    T value(const std::string& key, T defaultValue) const {
        if (!contains(key)) return defaultValue;
        const json& j = (*this)[key];
        return j.get<T>();
    }

    template<typename T>
    T get() const {
        if constexpr (std::is_same_v<T, std::string>) {
            if (type == value_t::string) return stringValue;
            return "";
        } else if constexpr (std::is_same_v<T, int>) {
            if (type == value_t::number_integer) return static_cast<int>(intValue);
            if (type == value_t::number_float) return static_cast<int>(floatValue);
            return 0;
        } else if constexpr (std::is_same_v<T, long long>) {
            if (type == value_t::number_integer) return intValue;
            return 0;
        } else if constexpr (std::is_same_v<T, double>) {
            if (type == value_t::number_float) return floatValue;
            if (type == value_t::number_integer) return static_cast<double>(intValue);
            return 0.0;
        } else if constexpr (std::is_same_v<T, float>) {
            if (type == value_t::number_float) return static_cast<float>(floatValue);
            if (type == value_t::number_integer) return static_cast<float>(intValue);
            return 0.0f;
        } else if constexpr (std::is_same_v<T, bool>) {
            if (type == value_t::boolean) return boolValue;
            return false;
        }
        return T{};
    }

    auto begin() { return objectValue.begin(); }
    auto end() { return objectValue.end(); }
    auto begin() const { return objectValue.begin(); }
    auto end() const { return objectValue.end(); }

    bool is_object() const { return type == value_t::object; }
    bool is_array() const { return type == value_t::array; }
    bool is_null() const { return type == value_t::null; }

    std::string dump(int indent = -1) const {
        std::ostringstream oss;
        dump_internal(oss, indent, 0);
        return oss.str();
    }

private:
    void dump_internal(std::ostringstream& oss, int indent, int current_indent) const {
        switch (type) {
            case value_t::null:
                oss << "null";
                break;
            case value_t::boolean:
                oss << (boolValue ? "true" : "false");
                break;
            case value_t::number_integer:
                oss << intValue;
                break;
            case value_t::number_float:
                oss << floatValue;
                break;
            case value_t::string:
                oss << "\"" << escape_string(stringValue) << "\"";
                break;
            case value_t::array: {
                oss << "[";
                if (indent >= 0 && !arrayValue.empty()) {
                    oss << "\n";
                    for (size_t i = 0; i < arrayValue.size(); ++i) {
                        oss << std::string(current_indent + indent, ' ');
                        arrayValue[i].dump_internal(oss, indent, current_indent + indent);
                        if (i < arrayValue.size() - 1) oss << ",";
                        oss << "\n";
                    }
                    oss << std::string(current_indent, ' ');
                } else {
                    for (size_t i = 0; i < arrayValue.size(); ++i) {
                        arrayValue[i].dump_internal(oss, indent, current_indent);
                        if (i < arrayValue.size() - 1) oss << ", ";
                    }
                }
                oss << "]";
                break;
            }
            case value_t::object: {
                oss << "{";
                if (indent >= 0 && !objectValue.empty()) {
                    oss << "\n";
                    size_t i = 0;
                    for (const auto& [key, value] : objectValue) {
                        oss << std::string(current_indent + indent, ' ');
                        oss << "\"" << escape_string(key) << "\": ";
                        value.dump_internal(oss, indent, current_indent + indent);
                        if (i < objectValue.size() - 1) oss << ",";
                        oss << "\n";
                        ++i;
                    }
                    oss << std::string(current_indent, ' ');
                } else {
                    size_t i = 0;
                    for (const auto& [key, value] : objectValue) {
                        oss << "\"" << escape_string(key) << "\": ";
                        value.dump_internal(oss, indent, current_indent);
                        if (i < objectValue.size() - 1) oss << ", ";
                        ++i;
                    }
                }
                oss << "}";
                break;
            }
        }
    }

    static std::string escape_string(const std::string& s) {
        std::string result;
        for (char c : s) {
            switch (c) {
                case '"': result += "\\\""; break;
                case '\\': result += "\\\\"; break;
                case '\b': result += "\\b"; break;
                case '\f': result += "\\f"; break;
                case '\n': result += "\\n"; break;
                case '\r': result += "\\r"; break;
                case '\t': result += "\\t"; break;
                default: result += c; break;
            }
        }
        return result;
    }

    friend std::ostream& operator<<(std::ostream& os, const json& j) {
        os << j.dump();
        return os;
    }

    friend std::istream& operator>>(std::istream& is, json& j) {
        // Simple parser - just enough for our config needs
        std::string str((std::istreambuf_iterator<char>(is)),
                        std::istreambuf_iterator<char>());
        j = parse_value(str, 0).first;
        return is;
    }

    static std::pair<json, size_t> parse_value(const std::string& str, size_t pos) {
        // Skip whitespace
        while (pos < str.size() && (str[pos] == ' ' || str[pos] == '\t' || str[pos] == '\n' || str[pos] == '\r')) {
            ++pos;
        }

        if (pos >= str.size()) return {json(), pos};

        char c = str[pos];

        // String
        if (c == '"') {
            std::string value;
            ++pos; // skip opening quote
            while (pos < str.size() && str[pos] != '"') {
                if (str[pos] == '\\' && pos + 1 < str.size()) {
                    switch (str[pos + 1]) {
                        case '"': value += '"'; pos += 2; break;
                        case '\\': value += '\\'; pos += 2; break;
                        case 'n': value += '\n'; pos += 2; break;
                        case 't': value += '\t'; pos += 2; break;
                        default: value += str[pos + 1]; pos += 2; break;
                    }
                } else {
                    value += str[pos++];
                }
            }
            if (pos < str.size()) ++pos; // skip closing quote
            return {json(value), pos};
        }

        // Number
        if (c == '-' || (c >= '0' && c <= '9')) {
            size_t end = pos;
            bool isFloat = false;
            while (end < str.size() && ((str[end] >= '0' && str[end] <= '9') || str[end] == '.' || str[end] == '-' || str[end] == 'e' || str[end] == 'E')) {
                if (str[end] == '.' || str[end] == 'e' || str[end] == 'E') isFloat = true;
                ++end;
            }
            std::string numStr = str.substr(pos, end - pos);
            if (isFloat) {
                return {json(std::stod(numStr)), end};
            } else {
                return {json(std::stoll(numStr)), end};
            }
        }

        // Boolean or null
        if (str.substr(pos, 4) == "true") {
            return {json(true), pos + 4};
        }
        if (str.substr(pos, 5) == "false") {
            return {json(false), pos + 5};
        }
        if (str.substr(pos, 4) == "null") {
            return {json(), pos + 4};
        }

        // Array
        if (c == '[') {
            json arr = json::array();
            ++pos; // skip [
            while (pos < str.size() && str[pos] != ']') {
                auto [value, newPos] = parse_value(str, pos);
                arr.push_back(value);
                pos = newPos;
                while (pos < str.size() && (str[pos] == ' ' || str[pos] == '\t' || str[pos] == '\n' || str[pos] == '\r' || str[pos] == ',')) {
                    ++pos;
                }
            }
            if (pos < str.size()) ++pos; // skip ]
            return {arr, pos};
        }

        // Object
        if (c == '{') {
            json obj = json::object();
            ++pos; // skip {
            while (pos < str.size() && str[pos] != '}') {
                auto [key, keyPos] = parse_value(str, pos);
                pos = keyPos;
                while (pos < str.size() && (str[pos] == ' ' || str[pos] == '\t' || str[pos] == '\n' || str[pos] == '\r' || str[pos] == ':')) {
                    ++pos;
                }
                auto [value, valPos] = parse_value(str, pos);
                pos = valPos;
                obj[key.get<std::string>()] = value;
                while (pos < str.size() && (str[pos] == ' ' || str[pos] == '\t' || str[pos] == '\n' || str[pos] == '\r' || str[pos] == ',')) {
                    ++pos;
                }
            }
            if (pos < str.size()) ++pos; // skip }
            return {obj, pos};
        }

        return {json(), pos};
    }
};

} // namespace nlohmann
