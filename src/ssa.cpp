#include "ssa.h"

#include <iostream>
#include <utility>
#include <cstdio>
#include <cassert>

namespace {

std::string GetIndent(int indent_num) {
    std::string indent;
    indent.reserve(indent_num);
    while (indent_num--) {
        indent.push_back('\t');
    }
    return std::move(indent);
}

}

void ValueSSA::Print(int indent) {
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

void ArgGetterSSA::Print(int indent) {
    std::cout << name() << '(' << arg_id_ << ')';
}

void AsmSSA::Print(int indent) {
    std::string ind = GetIndent(indent + 1);
    std::cout << name() << std::endl << ind;
    for (const auto &i : text_) {
        if (i == '\n') {
            puts(ind.c_str());
        }
        else {
            putchar(i);
        }
    }
    fflush(stdout);
}

void UndefSSA::Print(int indent) {
    std::cout << name();
}

void PhiSSA::Print(int indent) {
    std::cout << name() << '(';
    for (const auto &it : *this) {
        it.value()->Print(indent);
        std::cout << ", ";
    }
    std::cout << "\b\b)";
}

void BlockSSA::Print(int indent) {
    ++indent;
    std::cout << name() << ' ' << id_ << std::endl;
    std::string ind = GetIndent(indent);
    for (const auto &it : *this) {
        std::cout << ind;
        it.value()->Print(indent);
        std::cout << std::endl;
    }
    std::cout << '\b';
}

void JumpSSA::Print(int indent) {
    std::cout << name() << "(block: ";
    std::cout << SSACast<BlockSSA>((*this)[0].value())->id();
    std::cout << ')';
    if (size() == 2) {
        std::cout << " if ";
        (*this)[1].value()->Print(indent);
    }
}

void ArgSetterSSA::Print(int indent) {
    std::cout << name() << '_' << arg_pos_ << " = ";
    (*this)[0].value()->Print(indent);
}

void QuadSSA::Print(int indent) {
    const char *op_str[] = {
        "(num)", "(dec)", "(str)",
        "and", "xor", "or", "not", "shl", "shr",
        "add", "sub", "mul", "div", "mod", "pow",
        "lt", "le", "gt", "ge", "eq", "neq",
        "ret"
    };
    std::cout << '[' << op_str[static_cast<int>(op_)] << ", ";
    (*this)[0].value()->Print(indent);
    if ((*this).size() == 2) {
        std::cout << ", ";
        (*this)[1].value()->Print(indent);
    }
    std::cout << ']';
}

void VariableSSA::Print(int indent) {
    std::cout << name() << '_' << id_;
    if ((*this).size()) {
        std::cout << " = ";
        (*this)[0].value()->Print(indent);
    }
}
