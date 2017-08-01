#ifndef SABY_SYMBOL_H_
#define SABY_SYMBOL_H_

#include <string>
#include <memory>
#include <map>
#include <vector>

using TypeValue = long long;
constexpr TypeValue kTypeError = -1;

class Environment;
using EnvPtr = std::shared_ptr<Environment>;
using SymbolHash = std::map<std::string, TypeValue>;

inline EnvPtr MakeEnvironment(EnvPtr outer) {
    return std::make_shared<Environment>(outer);
}

class Environment {
public:
    Environment(EnvPtr outer) : outer_(outer) {}
    ~Environment() {}

    void Insert(const std::string &id, TypeValue type) {
        table_.insert(SymbolHash::value_type(id, type));
    }

    TypeValue GetType(const std::string &id, bool recursive = true);
    void SetType(const std::string &id, TypeValue type);
    bool SaveEnv(const char *path);
    bool LoadEnv(const char *path);

    const EnvPtr &outer() const { return outer_; }

private:
    EnvPtr outer_;
    SymbolHash table_;
};

#endif // SABY_SYMBOL_H_
