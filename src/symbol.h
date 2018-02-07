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
using LibListPtr = std::unique_ptr<LibList>;

inline EnvPtr MakeEnvironment(EnvPtr outer) {
    return std::make_shared<Environment>(outer);
}

class Environment {
public:
    // variable name -> SSA id
    using SymbolID = std::map<std::string, IDType>;

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
    bool SaveEnv(const char *path, const LibList &syms);
    LoadEnvReturn LoadEnv(const char *path, const std::string &lib_name);

    // used in IR generating process
    SymbolID &id_table() { return id_table_; }
    IDType &GetIDRef(const std::string &id);

    const EnvPtr &outer() const { return outer_; }
    const Environment *outermost() const {
        return !outer_ ? this : GetEnvOutermost(outer_).get();
    }

private:
    // variable name -> type info
    using SymbolHash = std::map<std::string, TypeValue>;
    // store the hash of lib name
    using LibHashSet = std::set<std::size_t>;
    using LibHashPtr = std::unique_ptr<LibHashSet>;

    const EnvPtr &GetEnvOutermost(const EnvPtr &current) const;
    EnvPtr MakeLibEnv();
    EnvPtr GetLibEnv();

    EnvPtr outer_;
    SymbolHash table_;
    SymbolID id_table_;
    // library info
    LibHashPtr lib_hash_;
    LibListPtr loaded_lib_, exported_lib_;
};

#endif // SABY_SYMBOL_H_
