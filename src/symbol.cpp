#include "symbol.h"

#include <fstream>

namespace {

const unsigned int header = 0x72297962;   // 'sabysymb' in T9 keyboard

} // namespace

TypeValue Environment::GetType(const std::string &id, bool recursive) {
    auto sym = table_.find(id);
    if (sym != table_.end()) {
        return sym->second;
    }
    else {
        if (!recursive) return kTypeError;
        if (outer_ != nullptr) {
            return outer_->GetType(id);
        }
        else {
            return kTypeError;
        }
    }
}

void Environment::SetType(const std::string &id, TypeValue type) {
    auto sym = table_.find(id);
    if (sym != table_.end()) {
        sym->second = type;
    }
    else {
        if (outer_ != nullptr) {
            outer_->SetType(id, type);
        }
    }
}

bool Environment::SaveEnv(const char *path) {
    std::ofstream out(path, std::ofstream::binary);
    if (!out.is_open()) return false;
    out.write((char *)&header, sizeof(header));
    for (const auto &i : table_) {
        out << i.first << '\0';
        out.write((char *)&i.second, sizeof(TypeValue));
    }
    out.close();
    return true;
}

bool Environment::LoadEnv(const char *path) {
    std::ifstream in(path, std::ifstream::binary);
    if (!in.is_open()) return false;
    in >> std::noskipws;

    unsigned int head;
    in.read((char *)&head, sizeof(head));
    if (head != header) return false;

    std::string id;
    char temp;
    TypeValue type;

    for (;;) {
        in >> temp;
        if (in.eof()) break;
        if (temp != '\0') {
            id.push_back(temp);
        }
        else {
            in.read((char *)&type, sizeof(TypeValue));
            table_.insert(SymbolHash::value_type(id, type));
            id.clear();
        }
    }

    in.close();
    return true;
}

