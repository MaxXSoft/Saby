#ifndef SABY_IRBUILDER_H_
#define SABY_IRBUILDER_H_

#include <map>
#include <cstddef>
#include <string>

#include "ssa.h"

class IRBuilder {
public:
    IRBuilder() {}
    ~IRBuilder() {}

    SSAPtr NewBlock(SSAPtrList body);
    unsigned int GetVariableID(SSAPtr &block, const std::string &var_name);

private:
    std::map<size_t, std::map<std::string, unsigned int>> block_var_def_;
    std::map<size_t, unsigned int> block_var_id_;
};

#endif // SABY_IRBUIDER_H_
