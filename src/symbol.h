#ifndef SABY_SYMBOL_H_
#define SABY_SYMBOL_H_

#include <string>
#include <memory>
#include <map>
#include <vector>
#include <set>
#include <cstddef>

#include "type.h"

class Environment;
using EnvPtr = std::shared_ptr<Environment>;
// variable name -> type info
using SymbolHash = std::map<std::string, TypeValue>;
// variable name -> SSA id
using SymbolID = std::map<std::string, IDType>;
// store the hash of lib name
using LibHashSet = std::set<std::size_t>;

inline EnvPtr MakeEnvironment(EnvPtr outer) {
    return std::make_shared<Environment>(outer);
}

class Environment {
public:
    enum class LoadEnvReturn : char {
        Success, FileError, LibConflicted, FuncConflicted
    };

    Environment(EnvPtr outer) : outer_(outer) {}
    ~Environment() {}

    void Insert(const std::string &id, TypeValue type) {
        table_.insert(SymbolHash::value_type(id, type));
    }

    TypeValue GetType(const std::string &id, bool recursive = true);
    void SetType(const std::string &id, TypeValue type);
    bool SaveEnv(const char *path, const std::vector<std::string> &syms);
    LoadEnvReturn LoadEnv(const char *path);

    // used in IR generating process
    SymbolID &id_table() { return id_table_; }

    const EnvPtr &outer() const { return outer_; }

private:
    EnvPtr outer_;
    SymbolHash table_;
    SymbolID id_table_;
    LibHashSet loaded_lib_;
};

#endif // SABY_SYMBOL_H_
