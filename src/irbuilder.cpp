#include "irbuilder.h"

#include <memory>

SSAPtr IRBuilder::NewBlock(SSAPtrList body) {
    auto block = std::make_unique<BlockSSA>(current_block_++, std::move(body));
    var_id_.push_back(0);
    return std::move(block);
}

SSAPtr IRBuilder::NewVariable() {
    return std::make_unique<VariableSSA>(var_id_[current_block_]++);
}
