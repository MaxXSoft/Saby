#include "ssa.h"

#include <iostream>
#include <cassert>

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

void ArgHolderSSA::Print() {
    std::cout << name() << '(' << arg_id_ << ')';
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
    for (const auto &it : *this) {
        std::cout << '\t';
        it.value()->Print();
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
        (*this)[1].value()->Print();
    }
}

void CallSSA::Print() {
    std::cout << name() << "{(block: ";
    std::cout << SSACast<BlockSSA>((*this)[0].value())->id();
    std::cout << "), (";
    if (size() > 1) {
        for (int i = 1; i < size(); ++i) {
            (*this)[i].value()->Print();
            std::cout << ", ";
        }
    }
    std::cout << "\b\b)}";
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
    (*this)[0].value()->Print();
    if ((*this).size() == 2) {
        std::cout << ", ";
        (*this)[1].value()->Print();
    }
    std::cout << ']';
}

void VariableSSA::Print() {
    std::cout << name() << '_' << id_;
    if ((*this).size()) {
        std::cout << " = ";
        (*this)[0].value()->Print();
    }
}
