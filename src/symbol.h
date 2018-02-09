#ifndef SABY_SYMBOL_H_
#define SABY_SYMBOL_H_

#include <string>
#include <memory>
#include <utility>
#include <map>
#include <vector>
#include <list>
#include <set>
#include <cstddef>

#include "type.h"

// TODO: reimplement this class in an elegant way

class Environment;
using EnvPtr = std::shared_ptr<Environment>;
// library info
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
    const LibListPtr &loaded_libs() const { return loaded_libs_; }
    const LibListPtr &exported_funcs() const { return exported_funcs_; }

    const EnvPtr &outer() const { return outer_; }
    Environment *outermost() {
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
    IDType &GetIDRecursive(const std::string &id);

    EnvPtr outer_;
    SymbolHash table_;
    SymbolID id_table_;
    // library info
    LibHashPtr lib_hash_;
    LibListPtr loaded_libs_, exported_funcs_;
};

#endif // SABY_SYMBOL_H_
