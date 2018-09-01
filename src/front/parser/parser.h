#ifndef SABY_DEFINE_PARSER_PARSER_H_
#define SABY_DEFINE_PARSER_PARSER_H_

#include "../lexer/lexer.h"
#include "../../define/ast/ast.h"

class Parser {
public:
    Parser(Lexer &lexer) : lexer_(lexer), cur_token_(0) {
        NextToken();
    }
    ~Parser() {}

    ASTPtr ParseNext() { return ParseExpression(); }

    unsigned int error_num() const { return error_num_; }

private:
    int NextToken() { return cur_token_ = lexer_.NextToken(); }
    ASTPtr PrintError(const char *description);

    ASTPtr ParseNumber();
    ASTPtr ParseDecimal();
    ASTPtr ParseString();
    ASTPtr ParseBinaryExpression(int op_prec, ASTPtr lhs);
    ASTPtr ParseUnaryExpression();
    ASTPtr ParseTypeConv();
    ASTPtr ParseVarDefinition(int type);
    ASTPtr ParseFunctionDef();
    ASTPtr ParseFunctionCall(ASTPtr callee);
    ASTPtr ParseAsm();
    ASTPtr ParseIf();
    ASTPtr ParseWhile();
    ASTPtr ParseExternal();
    ASTPtr ParseControlFlow();
    ASTPtr ParseId();
    ASTPtr ParseBracket();
    ASTPtr ParseBlock();
    ASTPtr ParsePrimary();
    ASTPtr ParseExpression();

    Lexer &lexer_;
    unsigned int error_num_;
    int cur_token_;
};

#endif // SABY_DEFINE_PARSER_PARSER_H_
