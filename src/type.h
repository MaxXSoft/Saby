#ifndef SABY_TYPE_H_
#define SABY_TYPE_H_

#include <vector>
#include <list>
#include <string>
#include <utility>
#include <cstdint>

// store the type info during analyzing process
using TypeValue = long long;

using TypeList = std::vector<TypeValue>;
using VarType = std::pair<std::string, TypeValue>;
using VarTypeList = std::vector<VarType>;

// store the libs which are imported/exported
// store the library info during semantic analysis
using LibList = std::list<std::string>;

constexpr TypeValue kTypeError = -1;

// just a limit which can simplify code generating
constexpr int kFuncMaxArgNum = 6;
constexpr TypeValue kFuncTypeBase = 131;

inline TypeValue GetFuncRetType(TypeValue func_type) {
    return (func_type - kFuncTypeBase) % kFuncTypeBase;
}

// store the id of SSA
using IDType = std::string;
using BlockIDType = std::size_t;

#endif // SABY_TYPE_H_
