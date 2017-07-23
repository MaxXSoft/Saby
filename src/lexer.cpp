#include "lexer.h"

#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <cctype>

namespace {

const char *keyword_str[] = {
    "number", "float", "function", "string", "list", "void",
    "import", "export", "asm",
    "if", "else",
    "return",
    "while", "break", "continue"
};

const char *operator_str[] = {
    "", "&", "^", "|", "~", "<<", ">>",
    "+", "-", "*", "/", "%", "**",
    "++", "--",
    "<", "<=", ">", ">=", "==", "!=",
    "=>",
    "=",
    "&=", "^=", "|=", "~=", "<<=", ">>=",
    "+=", "-=", "*=", "/=", "%=", "**="
};

const int operator_prec[] = {
    -1, 40, 30, 20, 100, 70, 70,
    80, 80, 90, 90, 90, 110,
    100, 100,
    60, 60, 60, 60, 50, 50,
    120,
    10
};

template <typename T>
int GetIndex(const char *str, T &str_array) {
    auto len = sizeof(str_array) / sizeof(str_array[0]);
    for (int i = 0; i < len; ++i) {
        if (!strcmp(str, str_array[i])) return i;
    }
    return kError;
}

int GetDLE(const std::string &str) {
    switch (str[0]) {
        case 'a': return '\a';
        case 'b': return '\b';
        case 'f': return '\f';
        case 'n': return '\n';
        case 'r': return '\r';
        case 't': return '\t';
        case 'v': return '\v';
        case '\\': return '\\';
        case '\'': return '\'';
        case '\"': return '\"';
        case '0': return '\0';
        case 'x': {
            auto hex = str.substr(1);
            char *end_pos = nullptr;
            auto ret = (int)strtol(hex.c_str(), &end_pos, 16);
            return end_pos - hex.c_str() == hex.length() ? ret : kError;
        }
        default: return kError;
    }
}

bool IsOperatorChar(char op_char) {
    const char op_char_arr[] = "&^|~<>+-*/%=!";
    for (const auto &temp : op_char_arr) {
        if (temp == op_char) return true;
    }
    return false;
}

} // namespace

int Lexer::PrintError(const char *description) {
    fprintf(stderr, "\033[1mlexer\033[0m(line %u): \033[31m\033[1merror:\033[0m %s\n", line_pos_, description);
    ++error_num_;
    return kError;
}

int Lexer::GetOperatorPrec() {   // TODO: just made me unhappy
    return operator_prec[op_val_ <= kAssign ? op_val_ : kAssign];
}

int Lexer::HandleId() {
    std::string id;

    do {
        id += last_char_;
        NextChar();
    } while (!in_.eof() && (isalnum(last_char_) || last_char_ == '_'));

    auto temp = GetIndex(id.c_str(), keyword_str);
    if (temp != kError) {   // keyword
        key_val_ = temp;
        return kKeyword;
    }
    else {                  // id
        id_val_ = id;
        return kId;
    }
}

int Lexer::HandleNum() {
    std::string num_str;
    char *end_pos = nullptr;
    auto is_double = (last_char_ == '.');
    auto IsValidConv = [&end_pos, &num_str]() {
        return end_pos - num_str.c_str() == num_str.length();
    };

    if (last_char_ == '0') {   // start with 0
        NextChar();
        if (last_char_ == 'X' || last_char_ == 'x') {   // hex
            NextChar();
            while(isalnum(last_char_)) {
                num_str += last_char_;
                NextChar();
            }
            num_val_ = strtoll(num_str.c_str(), &end_pos, 16);
            return IsValidConv() ? kNum : PrintError("invalid hex");
        }
        else if (isspace(last_char_) || IsOperatorChar(last_char_) ||
                 last_char_ == ';' || last_char_ == ',' ||
                 last_char_ == ')' || IsEndOfLine()) {
            num_val_ = 0;   // just zero
            return kNum;
        }
        else if (last_char_ != '.') {
            return PrintError("invalid number");
        }
    }
    do {   // read number string
        if (!is_double && last_char_ == '.') is_double = true;
        num_str += last_char_;
        NextChar();
    } while (isdigit(last_char_) || last_char_ == '.' || last_char_ == 'e');

    if (is_double) {   // decimal
        dec_val_ = strtod(num_str.c_str(), &end_pos);
        return IsValidConv() ? kDecimal : PrintError("invalid floating point");
    }
    else {   // number
        num_val_ = strtoll(num_str.c_str(), &end_pos, 0);
        return IsValidConv() ? kNum : PrintError("invalid number");
    }
}

int Lexer::HandleString() {
    std::string str, temp;
    NextChar();

    while(last_char_ != '\"') {   // start with double quotes
        if (last_char_ == '\\') {   // escaped character
            NextChar();
            if (IsEndOfLine()) return PrintError("expected \'\"\'");
            temp += last_char_;
            if (last_char_ == 'x') {   // hex ascii
                for (int i = 0; i < 2; ++i) {
                    NextChar();
                    if (IsEndOfLine()) return PrintError("expected \'\"\'");
                    temp += last_char_;
                }
            }
            auto ret = GetDLE(temp);
            if (ret != kError) {
                last_char_ = ret;
            }
            else {
                return PrintError("unknown escaped character");
            }
        }
        str += last_char_;
        NextChar();
        if (IsEndOfLine()) return PrintError("expected \'\"\'");
    }
    NextChar();   // eat right bracket

    str_val_ = str;
    return kStr;
}

int Lexer::HandleOperator() {
    std::string op;

    do {   // read operator
        op += last_char_;
        NextChar();
    } while (IsOperatorChar(last_char_));

    auto temp = GetIndex(op.c_str(), operator_str);
    if (temp != kError) {
        set_op_val(temp);
        return kOperator;
    }
    else {
        return PrintError("unknown operator");
    }
}

int Lexer::HandleEOL() {
    do {
        ++line_pos_;
        NextChar();
    } while (IsEndOfLine() && !in_.eof());
    return kSeparator;
}

int Lexer::NextToken() {
    // reset operator
    set_op_val(0);
    // key_val_ = -1;
    
    // end of file
    if (in_.eof()) return kEOF;

    // skip spaces
    while (!IsEndOfLine() && isspace(last_char_)) NextChar();

    // skip comment
    if (last_char_ == '#') {
        do {
            NextChar();
        } while (!IsEndOfLine());
		if (!in_.eof()) return NextToken();
    }

    // separator
    if (last_char_ == ';') {
        NextChar();
        return kSeparator;
    }

    // id or keyword, format: [A-Za-z]([A-Za-z0-9]|_)*
    if (isalpha(last_char_)) return HandleId();
    if (last_char_ == '@') {   // regard '@' as identifier
        NextChar();
        id_val_ = "@";
        return kId;
    }

    // number or decimal
    if (isdigit(last_char_) || last_char_ == '.') return HandleNum();

    // string
    if (last_char_ == '\"') return HandleString();

    // operator
    if (IsOperatorChar(last_char_)) return HandleOperator();
    
    // end of line
    if (IsEndOfLine()) return HandleEOL();

    // other characters
    auto cur_char = last_char_;
    NextChar();
    return cur_char;
}
