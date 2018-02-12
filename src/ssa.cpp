#include "ssa.h"

#include <iostream>
#include <iomanip>
#include <utility>
#include <cstdio>

namespace {

void PrintVarName(const std::string &id, VariableSSA *ptr) {
    std::cout << '$' << id << '_';
    auto ptr_id = reinterpret_cast<BlockIDType>(ptr);
    std::cout << std::hex << std::setw(3) << std::setfill('0');
    std::cout << ((ptr_id >> 4) & 0xFFF);
}

void PrintValue(const SSAPtr &value) {
    const auto &name = value->name();
    switch (name[0]) {
        case 'b': {
            auto block_ptr = static_cast<BlockSSA *>(value.get());
            std::cout << "{block: " << block_ptr->id() << '}';
            break;
        }
        case '$': {
            if (name[1] == 'v') {
                auto var_ptr = static_cast<VariableSSA *>(value.get());
                PrintVarName(var_ptr->id(), var_ptr);
                break;
            }
        }
        default: {
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
    for (auto it = begin(); it != end(); ++it) {
        PrintValue(it->value());
        if (it != end() - 1) std::cout << ", ";
    }
    std::cout << ')';
}

void BlockSSA::Print() {
    std::cout << name() << ' ' << id_ << std::endl << "preds: ";
    if (!size()) {
        std::cout << "null";
    }
    else {
        for (auto it = begin(); it != end(); ++it) {
            auto block_ptr = SSACast<BlockSSA>(it->value());
            std::cout << "{block: " << block_ptr->id() << '}';
            if (it != end() - 1) std::cout << ", ";
        }
    }
    std::cout << std::endl;
    for (const auto &it : insts_) {
        std::cout << '\t';
        it->Print();
        std::cout << std::endl;
    }
}

void JumpSSA::Print() {
    std::cout << name() << "{block: ";
    std::cout << SSACast<BlockSSA>((*this)[0].value())->id();
    std::cout << '}';
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
    PrintVarName(id_, this);
    if ((*this).size()) {
        std::cout << " = ";
        PrintValue((*this)[0].value());
    }
}
