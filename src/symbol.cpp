#include "symbol.h"

#include <fstream>
#include <functional>   // std::hash
#include <cstdio>

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

bool Environment::SaveEnv(const char *path, const std::vector<std::string> &syms) {
    std::ofstream out(path, std::ofstream::binary);
    if (!out.is_open()) return false;
    out.write((char *)&header, sizeof(header));
    if (syms[0] == "*") {
        for (const auto &i : table_) {
            out << i.first << '\0';
            out.write((char *)&i.second, sizeof(TypeValue));
        }
    }
    else {   // TODO: reduce algorithm complexity
        for (const auto &i : syms) {
            auto it = table_.find(i);
            if (it == table_.end()) return false;
            out << it->first << '\0';
            out.write((char *)&it->second, sizeof(TypeValue));
        }
    }
    out.close();
    return true;
}

Environment::LoadEnvReturn Environment::LoadEnv(const char *path) {
    using LEReturn = Environment::LoadEnvReturn;

    auto str_hash = std::hash<std::string>()(path);
    if (!loaded_lib_.insert(str_hash).second) {
        // lib have already been added
        return LEReturn::LibConflicted;
    }

    std::ifstream in(path, std::ifstream::binary);
    if (!in.is_open()) return LEReturn::FileError;
    in >> std::noskipws;

    unsigned int head;
    in.read((char *)&head, sizeof(head));
    if (head != header) return LEReturn::FileError;

    std::string id;
    char temp;
    TypeValue type;

    LEReturn last_status = LEReturn::Success;
    for (;;) {
        in >> temp;
        if (in.eof()) break;
        if (temp != '\0') {
            id.push_back(temp);
        }
        else {
            in.read((char *)&type, sizeof(TypeValue));
            if (!table_.insert(SymbolHash::value_type(id, type)).second) {
                // there are two functions that have the same name
                last_status = LEReturn::FuncConflicted;
            }
            id.clear();
        }
    }

    in.close();
    return last_status;
}

