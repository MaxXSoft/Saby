#include "json.h"

#include <fstream>
#include <sstream>
#include <iomanip>
#include <limits>
#include <cctype>
#include <cstdio>
#include <cmath>

namespace {

const char *kIndent = "  ";

inline const char *GetEscapedChar(char c) {
    switch (c) {
        case '\"': return "\\\"";   /*   " -> \"   */
        case '\\': return "\\\\";   /*   \ -> \\   */
        case '/': return "\\/";     /*   / -> \/   */
        case '\b': return "\\b";
        case '\f': return "\\f";
        case '\n': return "\\n";
        case '\r': return "\\r";
        case '\t': return "\\t";
        default: return nullptr;
    }
}

std::string GetEscapedString(const char *str) {
    std::string temp;
    unsigned short uni = 0;
    auto AddUnicodeChar = [&temp, &uni]() {
        char uni_str[5] = {0};
        std::sprintf(uni_str, "%04x", uni);
        temp += uni_str;
        uni = 0;
    };
    do {
        const auto &c = *str;
        auto esc_char = GetEscapedChar(c);
        if (esc_char) {
            if (uni) AddUnicodeChar();
            temp += esc_char;
        }
        else if (c < 0 || std::iscntrl(c)) {
            if ((uni & 0xff00) == 0) {
                uni <<= 8;
                uni |= c;
            }
            else {
                AddUnicodeChar();
            }
        }
        else {
            if (uni) AddUnicodeChar();
            temp += c;
        }
    } while (*(++str) == '\0');
    return std::move(temp);
}

} // namespace

void JSONExporter::Export(std::ostream &&os) {
    // export version info
    os << "{\n" << kIndent << "\"version\": \"";
    os << ver_major_ << '.' << ver_minor_ << '.' << ver_revision_ << '"';
    // export data elements
    for (const auto &it : elements_) {
        os << ",\n" << kIndent << '"' << it.first << "\": ";
        auto json_element = static_cast<JSONDataElement *>(it.second.get());
        json_element->Seal();
        for (const auto &c : json_element->data_str()) {
            os << c;
            if (c == '\n') os << kIndent;
        }
    }
    // export data element groups
    for (const auto &it : groups_) {
        os << ",\n" << kIndent << '"' << it.first << "\": [\n";
        const auto &group = it.second;
        for (auto i = group.begin(); i != group.end(); ++i) {
            if (i != group.begin()) os << ",\n";
            os << kIndent << kIndent;
            auto json_element = static_cast<JSONDataElement *>(i->get());
            json_element->Seal();
            for (const auto &c : json_element->data_str()) {
                os << c;
                if (c == '\n') os << kIndent << kIndent;
            }
        }
        os << '\n' << kIndent << ']';
    }
    // export data referance groups
    for (const auto &it : ref_groups_) {
        os << ",\n" << kIndent << '"' << it.first << "\": [";
        const auto &ref_group = it.second;
        const int kMaxNumberPerLine = 4;
        if (ref_group.size() > kMaxNumberPerLine) os << '\n' << kIndent << kIndent;
        int count = 0;
        for (const auto &i : ref_group) {
            if (count) {
                os << ',';
                if (!(count % kMaxNumberPerLine)) os << '\n' << kIndent << kIndent;
            }
            os << '"' << std::setw(sizeof(UIDType) * 2) << std::hex;
            os << i.uid() << '"';
            ++count;
        }
        os << '\n' << kIndent << ']';
    }
    os << '}';
}

void JSONDataElement::InitDataStr()  {
    data_str_ = "{\n";
    data_str_ += kIndent;
    data_str_ += "\"uid\": \"";
    char uid_str[sizeof(UIDType) * 2 + 1] = {0};
    std::sprintf(uid_str, "%016llx", uid_);
    data_str_ += uid_str;
    data_str_ += '"';
}

void JSONDataElement::AddKey(const std::string &key) {
    data_str_ = data_str_ + ",\n" + kIndent + '"' + key + "\": ";
}

void JSONDataElement::AddData(const std::string &key, long long value) {
    AddKey(key);
    char buffer[21] = {0};
    std::sprintf(buffer, "%lld", value);
    data_str_ += buffer;
}

void JSONDataElement::AddData(const std::string &key, unsigned long long value) {
    AddKey(key);
    char buffer[21] = {0};
    std::sprintf(buffer, "%llud", value);
    data_str_ += buffer;
}

void JSONDataElement::AddData(const std::string &key, double value) {
    AddKey(key);
    if (!std::isfinite(value)) {
        data_str_ += "null";
    }
    else {
        auto oss = std::ostringstream();
        oss.precision(std::numeric_limits<double>::digits10);
        oss << value;
        data_str_ += oss.str();
    }
}

void JSONDataElement::AddData(const std::string &key, bool value) {
    AddKey(key);
    data_str_ += value ? "true" : "false";
}

void JSONDataElement::AddData(const std::string &key, const std::string &value) {
    AddKey(key);
    data_str_ += '"';
    data_str_ += GetEscapedString(value.c_str());
    data_str_ += '"';
}

void JSONDataElement::AddData(const std::string &key, std::nullptr_t) {
    AddKey(key);
    data_str_ += "null";
}

void JSONDataElement::AddData(const std::string &key, DataElement &&value) {
    AddKey(key);
    auto json_element = static_cast<JSONDataElement *>(value.get());
    json_element->Seal();
    for (const auto &c : json_element->data_str_) {
        data_str_ += c;
        if (c == '\n') data_str_ += kIndent;
    }
}

void JSONDataElement::AddData(const std::string &key, DataReference &&value) {
    AddKey(key);
    data_str_ += '"';
    char uid_str[sizeof(UIDType) * 2 + 1] = {0};
    std::sprintf(uid_str, "%016llx", value.uid());
    data_str_ = data_str_ + uid_str + '"';
}

void JSONDataElement::AddData(const std::string &key, DataGroup &&value) {
    AddKey(key);
    data_str_ += "[\n";
    for (auto it = value.begin(); it != value.end(); ++it) {
        if (it != value.begin()) data_str_ += ",\n";
        data_str_ += kIndent;
        data_str_ += kIndent;
        auto json_element = static_cast<JSONDataElement *>(it->get());
        json_element->Seal();
        for (const auto &c : json_element->data_str_) {
            data_str_ += c;
            if (c == '\n') {
                data_str_ += kIndent;
                data_str_ += kIndent;
            }
        }
    }
    data_str_ += '\n';
    data_str_ = data_str_ + kIndent + ']';
}

void JSONDataElement::AddData(const std::string &key, DataRefGroup &&value) {
    AddKey(key);
    data_str_ += '[';
    const int kMaxNumberPerLine = 3;
    if (value.size() > kMaxNumberPerLine) {
        data_str_ += '\n';
        data_str_ += kIndent;
        data_str_ += kIndent;
    }
    int count = 0;
    for (const auto &it : value) {
        if (count) {
            data_str_ += ',';
            if (!(count % kMaxNumberPerLine)) {
                data_str_ += '\n';
                data_str_ += kIndent;
                data_str_ += kIndent;
            }
        }
        data_str_ += '"';
        char uid_str[sizeof(UIDType) * 2 + 1] = {0};
        std::sprintf(uid_str, "%016llx", it.uid());
        data_str_ += uid_str;
        data_str_ += '"';
        ++count;
    }
    data_str_ += '\n';
    data_str_ = data_str_ + kIndent + ']';
}


