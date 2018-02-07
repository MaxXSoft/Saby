#include "symbol.h"

#include <fstream>
#include <functional>   // std::hash
#include <cassert>

namespace {

const unsigned int header = 0x72297962;   // 'sabysymb' in T9 keyboard

} // namespace

const EnvPtr &Environment::GetEnvOutermost(const EnvPtr &current) const {
    return !current->outer_ ? current : GetEnvOutermost(current->outer_);
}

EnvPtr Environment::MakeLibEnv() {
    auto new_env = MakeEnvironment(nullptr);
    new_env->lib_hash_ = std::make_unique<LibHashSet>();
    new_env->loaded_libs_ = std::make_unique<LibList>();
    new_env->exported_funcs_ = std::make_unique<LibList>();
    return std::move(new_env);
}

EnvPtr Environment::GetLibEnv() {
    // make sure there is an environment saves imported symbols
    // and get it or create it
    EnvPtr lib_env;
    if (!outer_) {   // has no outer environment
        // because lib_env cannot be visited currently
        // and current environment has no outer environment
        // so create a new environment as lib_env
        lib_env = MakeLibEnv();
        outer_ = lib_env;
    }
    else {   // has outer environment
        const auto &outermost = GetEnvOutermost(outer_);   // get outermost
        if (!outermost->lib_hash_) {   // outermost is not a lib_env
            // create a lib_env and set as outermost's outer environment
            lib_env = MakeLibEnv();
            outermost->outer_ = lib_env;
        }
        else {
            lib_env = outermost;
        }
    }
    return std::move(lib_env);
}

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

bool Environment::SaveEnv(const char *path, const LibList &syms) {
    auto lib_env = GetLibEnv();
    // check if the outer environment of current environment is lib_env
    if (outer_ != lib_env) return false;
    auto &lib_list = *lib_env->exported_funcs_;

    std::ofstream out(path, std::ofstream::binary);
    if (!out.is_open()) return false;
    out.write((char *)&header, sizeof(header));
    if (syms.front() == "*") {
        for (const auto &i : table_) {
            if (i.second >= kFuncTypeBase) {
                out << i.first << '\0';
                out.write((char *)&i.second, sizeof(TypeValue));
                // save library info
                lib_list.push_back(i.first);
            }
        }
    }
    else {   // TODO: reduce algorithm complexity
        for (const auto &i : syms) {
            auto it = table_.find(i);
            if (it == table_.end()) return false;
            if (it->second < kFuncTypeBase) return false;
            out << it->first << '\0';
            out.write((char *)&it->second, sizeof(TypeValue));
            // save library info
            lib_list.push_back(it->first);
        }
    }
    out.close();
    return true;
}

Environment::LoadEnvReturn Environment::LoadEnv(const char *path, const std::string &lib_name) {
    using LEReturn = Environment::LoadEnvReturn;

    auto lib_env = GetLibEnv();
    auto &table = lib_env->table_;
    auto &hash_set = *lib_env->lib_hash_;
    auto &lib_list = *lib_env->loaded_libs_;

    auto str_hash = std::hash<std::string>()(path);
    if (!hash_set.insert(str_hash).second) {   // TODO
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
            if (!table.insert(SymbolHash::value_type(id, type)).second) {
                // there are two functions that have the same name
                last_status = LEReturn::FuncConflicted;
            }
            // save library info
            lib_list.push_back(lib_name + "." + id);
            id.clear();
        }
    }

    in.close();
    return last_status;
}

IDType &Environment::GetIDRef(const std::string &id) {
    /*
        NOTICE: consider the following situation:
            var a = 1
            if ... {
                a += 1   # mark 1
            }
            a += 2   # mark 2
        mark 1: generate new 'a' in if-body (block)
        mark 2: get id of 'a' in outer block

        QUESTION: variable use in mark 2 may fail?
        ANSWER: this situation will not happen (WHY?)
    */
    auto it = id_table_.find(id);
    if (it != id_table_.end()) {
        return it->second;
    }
    else {
        assert(outer_ != nullptr);
        return outer_->GetIDRef(id);
    }
}
