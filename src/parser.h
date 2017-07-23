#ifndef SABY_PARSER_H_
#define SABY_PARSER_H_

#include "lexer.h"
#include "ast.h"

class Parser {
public:
    Parser(Lexer &lexer) : lexer_(lexer), cur_token_(0) {
        NextToken();
    }

    ASTPtr ParseNext() { return ParseExpression(); }

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
    ASTPtr ParseSingleWord();
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

#endif // SABY_PARSER_H_
