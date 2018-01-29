#ifndef SABY_TYPE_H_
#define SABY_TYPE_H_

#include <vector>
#include <string>
#include <utility>

// store the type info during analyzing process
using TypeValue = long long;

using TypeList = std::vector<TypeValue>;
using VarType = std::pair<std::string, TypeValue>;
using VarTypeList = std::vector<VarType>;

// store the libs which are imported/exported
using LibList = std::vector<std::string>;

constexpr TypeValue kTypeError = -1;

// just a limit which can simplify code generating
constexpr int kFuncMaxArgNum = 6;

// store the id of SSA
using IDType = std::size_t;

#endif // SABY_TYPE_H_
