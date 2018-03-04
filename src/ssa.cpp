#include "ssa.h"

#include <iostream>
#include <iomanip>
#include <utility>
#include <cstdio>
#include <cctype>

namespace {

void PrintVarName(const std::string &id, VariableSSA *ptr) {
    std::cout << '$' << id << '_';
    auto ptr_id = reinterpret_cast<BlockIDType>(ptr);
    std::cout << std::hex << std::setw(3) << std::setfill('0');
    std::cout << ((ptr_id >> 4) & 0xFFF);
}

void PrintValue(const SSAPtr &value) {
    if (IsSSAType<BlockSSA>(value)) {
        auto block_ptr = SSACast<BlockSSA>(value);
        std::cout << "{block: " << block_ptr->id() << '}';
    }
    else if (IsSSAType<VariableSSA>(value)) {
        auto var_ptr = SSACast<VariableSSA>(value);
        PrintVarName(var_ptr->id(), var_ptr);
    }
    else {
        value->Print();
    }
}

std::string GetEscapedString(const char *str) {
    std::string temp;
    do {
        if (std::iscntrl(*str)) {
            char hex[3];
            std::sprintf(hex, "%02x", *str);
            temp.push_back('\\');
            temp += hex;
        }
        else if (*str == '\\') {
            temp += "\\\\";
        }
        else {
            temp.push_back(*str);
        }
    } while (*(++str));
    return std::move(temp);
}

}

void ValueSSA::Print() {
    std::cout << name() << '(';
    switch (type_) {
        case ValueType::Number: {
            std::cout << std::dec << num_val_;
            break;
        }
        case ValueType::Decimal: {
            std::cout << dec_val_;
            break;
        }
        case ValueType::String: {
            std::cout << '"' << GetEscapedString(str_val_.c_str()) << '"';
            break;
        }
    }
    std::cout << ')';
}

void ArgGetterSSA::Print() {
    std::cout << name() << '(' << arg_id_ << ')';
}

void EnvGetterSSA::Print() {
    std::cout << name() << '(' << position_ << ')';
}

void ExternFuncSSA::Print() {
    std::cout << name() << '(' << func_name_ << ')';
}

void AsmSSA::Print() {
    std::cout << name() << std::endl << "\t\t";
    for (const auto &i : text_) {
        if (i == '\n') {
            fputs("\n\t\t", stdout);
        }
        else {
            putchar(i);
        }
    }
    fflush(stdout);
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
    std::cout << name() << ' ' << id_;
    if (is_func_) std::cout << " (function)";
    std::cout << std::endl << "preds: ";
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

void FuncRefSSA::Print() {
    std::cout << name() << '(';
    PrintValue((*this)[0].value());
    std::cout << ", ";
    auto env = (*this)[1].value();
    if (env) {
        PrintValue(env);
    }
    else {
        std::cout << "null";
    }
    std::cout << ')';
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

void EnvSSA::Print() {
    std::cout << name() << '<';
    for (auto it = begin(); it != end(); ++it) {
        PrintValue(it->value());
        if (it != end() - 1) std::cout << ", ";
    }
    std::cout << '>';
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

void ReturnSSA::Print() {
    std::cout << name() << '(';
    if (size()) {
        PrintValue((*this)[0].value());
    }
    else {
        std::cout << "void";
    }
    std::cout << ')';
}

void QuadSSA::Print() {
    const char *op_str[] = {
        "(num)", "(dec)", "(str)",
        "and", "xor", "or", "not", "shl", "shr",
        "add", "sub", "mul", "div", "mod", "pow",
        "lt", "le", "gt", "ge", "eq", "neq"
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
    std::cout << " = ";
    PrintValue((*this)[0].value());
}
