#ifndef SABY_SYMBOL_H_
#define SABY_SYMBOL_H_

#include <string>
#include <memory>
#include <map>

struct SymbolValue {
    long long num;
    double dec;
    std::string str;
};

class Symbol {
public:
    Symbol(int type, const SymbolValue &value)
            : type_(type), value_(value) {}
    ~Symbol() {}

    void set_value(const SymbolValue &value) { value_ = value; }

private:
    int type_;
    SymbolValue value_;
};

class Environment;
using EnvPtr = std::shared_ptr<Environment>;
using SymbolHash = std::map<std::string, Symbol>;

class Environment {
public:

    Environment(EnvPtr outer) : outer_(outer) {}
    ~Environment() {}

    void Insert(const std::string &id, const Symbol &&symbol) {
        table_.insert(SymbolHash::value_type(id, symbol));
    }

    const EnvPtr &outer() const { return outer_; }
    const SymbolHash &table() const { return table_; }

private:
    EnvPtr outer_;
    SymbolHash table_;
};

#endif // SABY_SYMBOL_H_
