#ifndef SABY_TYPE_H_
#define SABY_TYPE_H_

#include <vector>
#include <string>
#include <utility>

using TypeValue = long long;

using TypeList = std::vector<TypeValue>;
using VarType = std::pair<std::string, TypeValue>;
using VarTypeList = std::vector<VarType>;

using LibList = std::vector<std::string>;

constexpr TypeValue kTypeError = -1;

#endif // SABY_TYPE_H_
