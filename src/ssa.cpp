#include "ssa.h"

#include <iostream>
#include <utility>
#include <cstdio>

namespace {

// TODO: reimplement this function
void PrintValue(const SSAPtr &value) {
    auto block_ptr = SSACast<BlockSSA>(value);
    if (block_ptr) {
        std::cout << "(block: " << block_ptr->id() << ')';
    }
    else {
        auto var_ptr = SSACast<VariableSSA>(value);
        if (var_ptr) {
            std::cout << "$var_" << var_ptr->id();
        }
        else {
            value->Print();
        }
    }
}

}

void ValueSSA::Print() {
    std::cout << name() << '(';
    switch (type_) {
        case ValueType::Number: {
            std::cout << num_val_;
            break;
        }
        case ValueType::Decimal: {
            std::cout << dec_val_;
            break;
        }
        case ValueType::String: {
            std::cout << '"' << str_val_ << '"';
            break;
        }
    }
    std::cout << ')';
}

void ArgGetterSSA::Print() {
    std::cout << name() << '(' << arg_id_ << ')';
}

void ExternFuncSSA::Print() {
    std::cout << name() << '(' << func_name_ << ')';
}

void AsmSSA::Print() {
    std::cout << name() << std::endl << '\t';
    for (const auto &i : text_) {
        if (i == '\n') {
            puts("\n\t");
        }
        else {
            putchar(i);
        }
    }
    fflush(stdout);
}

void UndefSSA::Print() {
    std::cout << name();
}

void PhiSSA::Print() {
    std::cout << name() << '(';
    for (const auto &it : *this) {
        it.value()->Print();
        std::cout << ", ";
    }
    std::cout << "\b\b)";
}

void BlockSSA::Print() {
    std::cout << name() << ' ' << id_ << std::endl;
    for (const auto &it : insts_) {
        std::cout << '\t';
        it->Print();
        std::cout << std::endl;
    }
    std::cout << '\b';
}

void JumpSSA::Print() {
    std::cout << name() << "(block: ";
    std::cout << SSACast<BlockSSA>((*this)[0].value())->id();
    std::cout << ')';
    if (size() == 2) {
        std::cout << " if ";
        PrintValue((*this)[1].value());
    }
}

void ArgSetterSSA::Print() {
    std::cout << name() << '_' << arg_pos_ << " = ";
    PrintValue((*this)[0].value());
}

void CallSSA::Print() {
    std::cout << name() << "(callee: ";
    PrintValue((*this)[0].value());
    std::cout << ", arg_num: " << size() - 1 << ')';
}

void RtnGetterSSA::Print() {
    std::cout << name() << "(call, arg_num: ";
    auto user = static_cast<User *>((*this)[0].value().get());
    std::cout << (user->size() - 1) << ')';
}

void QuadSSA::Print() {
    const char *op_str[] = {
        "(num)", "(dec)", "(str)",
        "and", "xor", "or", "not", "shl", "shr",
        "add", "sub", "mul", "div", "mod", "pow",
        "lt", "le", "gt", "ge", "eq", "neq",
        "ret"
    };
    std::cout << '[' << op_str[static_cast<int>(op_)] << ", ";
    PrintValue((*this)[0].value());
    if ((*this).size() == 2) {
        std::cout << ", ";
        PrintValue((*this)[1].value());
    }
    std::cout << ']';
}

void VariableSSA::Print() {
    std::cout << name() << '_' << id_;
    if ((*this).size()) {
        std::cout << " = ";
        PrintValue((*this)[0].value());
    }
}
