#ifndef SABY_SYMBOL_H_
#define SABY_SYMBOL_H_

#include "ast.h"

#include <string>
#include <memory>
#include <map>

class Symbol {
public:
    Symbol(int type, const ASTPtr &value) // TODO: unique_ptr& ??
            : type_(type), value_(value) {}
    ~Symbol() {}

private:
    int type_;
    ASTPtr &value_;
};

class Environment {
public:
    using EnvPtr = std::shared_ptr<Environment>;
    using SymbolHash = std::map<std::string, Symbol>;

    Environment(EnvPtr outer) : outer_(outer) {}
    ~Environment() {}

    void Insert(const std::string &id, const Symbol &&symbol) {
        table_.insert(SymbolHash::value_type(id, symbol));
    }

    const SymbolHash &table() const { return table_; }
    const EnvPtr &outer() const { return outer_; }

private:
    EnvPtr outer_;
    SymbolHash table_;
};

#endif // SABY_SYMBOL_H_
