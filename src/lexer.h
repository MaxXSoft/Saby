#ifndef SABY_LEXER_H_
#define SABY_LEXER_H_

#include <string>
#include <fstream>

enum Token {
    kError = 128,
    kEOF, kId,
    kNum, kDecimal, kStr,
    kKeyword, kOperator,
    kSeparator
};

enum Keyword {
    kNumber, kFloat, kFunction, kString, kList, kVoid, kVar,
    kImport, kExport, kAsm,
    kIf, kElse,
    kReturn,
    kWhile, kBreak, kContinue
};

enum Operator {
    kConvNum = -3, kConvDec, kConvStr,
    kAnd = 1, kXor, kOr, kNot, kShl, kShr,
    kAdd, kSub, kMul, kDiv, kMod, kPow,
    kInc, kDec,
    kLess, kLesEql, kGreat, kGreEql, kEql, kNeq,
    kRtn,
    kAssign
};

class Lexer {
public:
    Lexer(std::ifstream &in)
            : in_(in), line_pos_(1), error_num_(0), last_char_(' ') {
        in >> std::noskipws;
    }
    ~Lexer() {}

    int NextToken();

    unsigned int line_pos() const { return line_pos_; }
    unsigned int error_num() const { return error_num_; }
    const std::string &id_val() const { return id_val_; }
    long long  num_val() const { return num_val_; }
    double dec_val() const { return dec_val_; }
    const std::string &str_val() const { return str_val_; }
    int key_val() const { return key_val_; }
    int op_val() const { return op_val_; }
    int op_prec() const { return op_prec_; }

    void set_op_val(int op_val) {
        op_val_ = op_val;
        if (op_val >= 0) {
            op_prec_ = GetOperatorPrec();   // TODO: just made me unhappy
        }
        else {   // only type conversion operator's value less than 0
            op_prec_ = 200;
        }
    }

private:
    void NextChar() { in_ >> last_char_; }
    bool IsEndOfLine() {
        return in_.eof() || last_char_ == '\n' || last_char_ == '\r';
    }
    int PrintError(const char *description);
    int GetOperatorPrec();
    int HandleId();
    int HandleNum();
    int HandleString();
    int HandleChar();
    int HandleOperator();
    int HandleEOL();

    std::ifstream &in_;
    unsigned int line_pos_, error_num_;
    char last_char_;

    std::string id_val_;
    long long num_val_;
    double dec_val_;
    std::string str_val_;
    int key_val_;
    int op_val_;
    int op_prec_;
};

#endif // SABY_LEXER_H_
