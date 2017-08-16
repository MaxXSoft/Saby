#include "parser.h"

#include <cstdio>
#include <sstream>
#include <utility>

ASTPtr Parser::PrintError(const char *description) {
    fprintf(stderr, "\033[1mparser\033[0m(line %u): \033[31m\033[1merror:\033[0m %s\n", lexer_.line_pos(), description);
    ++error_num_;
    return nullptr;
}

ASTPtr Parser::ParseNumber() {
    auto ret = std::make_unique<NumberAST>(lexer_.num_val());
    NextToken();
    return std::move(ret);
}

ASTPtr Parser::ParseDecimal() {
    auto ret = std::make_unique<DecimalAST>(lexer_.dec_val());
    NextToken();
    return std::move(ret);
}

ASTPtr Parser::ParseString() {
    auto ret = std::make_unique<StringAST>(lexer_.str_val());
    NextToken();
    return std::move(ret);
}

// operator-precedence parser
ASTPtr Parser::ParseBinaryExpression(int op_prec, ASTPtr lhs) {
    for (;;) {
        auto cur_op_prec = lexer_.op_prec();
        if (cur_op_prec < op_prec) return lhs;

        auto op_val = lexer_.op_val();
        NextToken();

        auto rhs = ParseUnaryExpression();
        if (!rhs) return nullptr;

        if (cur_op_prec < lexer_.op_prec()) {
            rhs = ParseBinaryExpression(cur_op_prec + 1, std::move(rhs));
            if (!rhs) return nullptr;
        }

        lhs = std::make_unique<BinaryExpressionAST>(op_val, std::move(lhs), std::move(rhs));
    }
}

ASTPtr Parser::ParseUnaryExpression() {
    if (cur_token_ != kOperator) return ParsePrimary();
    auto op_val = lexer_.op_val();
    NextToken();
    // allow the usage of '!!x'
    if (auto operand = ParseUnaryExpression()) {
        return std::make_unique<UnaryExpressionAST>(op_val, std::move(operand));
    }
    return nullptr;
}

ASTPtr Parser::ParseTypeConv() {
    int op_val;
    switch (lexer_.key_val()) {
        case kNumber: op_val = kConvNum; break;
        case kFloat: op_val = kConvDec; break;
        case kString: op_val = kConvStr; break;
        default: return PrintError("invalid conversion");
    }
    cur_token_ = kOperator;
    lexer_.set_op_val(op_val);
    return ParseUnaryExpression();
}

ASTPtr Parser::ParseVarDefinition(int type) {
    VarDefList defs;
    while (cur_token_ != kSeparator && cur_token_ != kEOF) {
        if (cur_token_ != kId) return PrintError("expected identifier");
        auto id = lexer_.id_val();
        ASTPtr init_val = nullptr;
        if (NextToken() != ',') {
            if (cur_token_ != kOperator && lexer_.op_val() != kAssign) {
                return PrintError("invalid variable initialization");
            }
            NextToken();
            init_val = ParseExpression();
            if (cur_token_ == ',') NextToken();
        }
        else {
            NextToken();   // eat ','
        }
        defs.push_back(std::make_pair(id, std::move(init_val)));
    }
    return std::make_unique<VariableAST>(std::move(defs), type);
}

ASTPtr Parser::ParseFunctionDef() {
    ASTPtrList args;
    if (cur_token_ != ')') {
        for (;;) {
            auto arg_type = lexer_.key_val();
            if (arg_type < kNumber || arg_type > kList) {
                return PrintError("invalid argument type");
            }
            args.push_back(std::make_unique<IdentifierAST>(lexer_.id_val(), arg_type));

            NextToken();
            if (cur_token_ == ')') break;
            if (cur_token_ != ',') {
                return PrintError("expected ')' or  ',' in argument list");
            }

            if (NextToken() != kKeyword || NextToken() != kId) {
                return PrintError("invalid argument list");
            }
        }
    }

    if (NextToken() != kOperator || lexer_.op_val() != kRtn) {
        if (cur_token_ == '{') {
            auto body = ParseBlock();
            if (!body) return nullptr;
            return std::make_unique<FunctionAST>(std::move(args), kVoid, std::move(body));
        }
        return PrintError("expected '=>' operator");
    }
    if (NextToken() != kKeyword) return PrintError("expected type");
    auto return_type = lexer_.key_val();
    if (return_type < kNumber || return_type > kVoid) {
        return PrintError("invalid return value type");
    }

    if (NextToken() != '{') return PrintError("expected '{'");
    auto body = ParseBlock();
    if (!body) return nullptr;

    return std::make_unique<FunctionAST>(std::move(args), return_type, std::move(body));
}

ASTPtr Parser::ParseFunctionCall(ASTPtr callee) {
    NextToken();   // eat '('
    ASTPtrList args;
    if (cur_token_ != ')') {
        for (;;) {
            if (auto arg = ParseExpression()) {
                args.push_back(std::move(arg));
            }
            else {
                return nullptr;
            }
            if (cur_token_ == ')') break;
            if (cur_token_ != ',') {
                return PrintError("expected ')' or  ',' in argument list");
            }
            NextToken();   // eat ','
        }
    }
    auto call_ast = std::make_unique<CallAST>(std::move(callee), std::move(args));
    NextToken();   // eat ')'
    if (cur_token_ == '(') return ParseFunctionCall(std::move(call_ast));
    return std::move(call_ast);
}

ASTPtr Parser::ParseAsm() {
    if (NextToken() != '{') return PrintError("expected '{'");
    NextToken();   // eat '{'

    std::ostringstream oss;
    while (NextToken() != '}') {
        switch (cur_token_) {
            case kId: {
                oss << lexer_.id_val() << ' ';
                break;
            }
            case kNum: {
                oss << lexer_.num_val();
                break;
            }
            case kDecimal: {
                oss << lexer_.dec_val();
                break;
            }
            case kStr: {
                oss << '"' << lexer_.str_val() << '"';   // TODO: unknown error??
            }
            case ',': {
                oss << (char)cur_token_;
                break;
            }
            case ':': {
                oss.seekp(-1, std::ios_base::cur);   // TODO: Test the situation: test1: mov r1, 1
                oss << ':';
                break;
            }
            case kSeparator: {
                oss << '\n';
                break;
            }
            default: {
                return PrintError("invalid assembly");
            }
        }
    }
    NextToken();   // eat '}'

    return std::make_unique<AsmAST>(oss.str());
}

ASTPtr Parser::ParseIf() {
    NextToken();
    auto cond = ParseExpression();
    if (!cond) return nullptr;
    if (cur_token_ != '{') return PrintError("expected '{'");

    ASTPtr if_body = ParseBlock(), else_body = nullptr;
    if (cur_token_ == kSeparator) NextToken();
    if (cur_token_ == kKeyword && lexer_.key_val() == kElse) {
        NextToken();
        if (cur_token_ == kKeyword && lexer_.key_val() == kIf) {
            else_body = ParseIf();
        }
        else {
            if (cur_token_ != '{') return PrintError("expected '{'");
            else_body = ParseBlock();
        }
        if (!else_body) return nullptr;
    }

    return std::make_unique<IfAST>(std::move(cond), std::move(if_body), std::move(else_body));
}

ASTPtr Parser::ParseWhile() {
    NextToken();
    auto cond = ParseExpression();
    if (!cond) return nullptr;
    if (cur_token_ != '{') return PrintError("expected '{'");
    auto body = ParseBlock();
    if (!body) return nullptr;
    return std::make_unique<WhileAST>(std::move(cond), std::move(body));
}

ASTPtr Parser::ParseExternal() {
    auto type = lexer_.key_val();
    NextToken();
    LibList libs;
    if (type == kExport && cur_token_ == kOperator && lexer_.op_val() == kMul) {
        libs.push_back("*");
        NextToken();
    }
    else {
        std::string id;
        while (cur_token_ != kSeparator && cur_token_ != kEOF) {
            if (cur_token_ != kId) return PrintError("invalid import/export");
            id += lexer_.id_val();
            if (NextToken() == '.') {
                id += '/';
                NextToken();
                continue;
            }
            libs.push_back(id);
            id.clear();
            if (cur_token_ == ',') NextToken();
        }
    }
    return std::make_unique<ExternalAST>(type, std::move(libs));
}

ASTPtr Parser::ParseControlFlow() {
    auto type = lexer_.key_val();
    NextToken();
    if (type == kReturn && cur_token_ != kSeparator) {
        auto value = ParseExpression();
        if (!value) return nullptr;
        return std::make_unique<ControlFlowAST>(type, std::move(value));
    }
    return std::make_unique<ControlFlowAST>(type, nullptr);
}

ASTPtr Parser::ParseId() {
    std::string id = lexer_.id_val();
    auto id_ast = std::make_unique<IdentifierAST>(id, -1);
    NextToken();
    // variable reference, type -1 means reference
    if (cur_token_ != '(') return std::move(id_ast);
    // or is a function call expression
    return ParseFunctionCall(std::move(id_ast));
}

ASTPtr Parser::ParseBracket() {
    NextToken();   // eat '('
    if (cur_token_ == kKeyword && lexer_.key_val() >= 0 && lexer_.key_val() <= kString) {
        NextToken();
        // function definition
        if (cur_token_ == kId) return ParseFunctionDef();
        // conversion operator
        if (cur_token_ == ')') return ParseTypeConv();
        return PrintError("invalid bracket expression");
    }
    if (cur_token_ == ')') return ParseFunctionDef();

    auto expr = ParseExpression();
    if (!expr) return nullptr;
    if (cur_token_ != ')') return PrintError("expected ')'");

    NextToken();   // eat ')'
    return expr;
}

ASTPtr Parser::ParseBlock() {
    NextToken();   // eat '{'
    ASTPtrList list;

    while (cur_token_ != '}') {
        auto ast = ParseExpression();
        if (!ast) return nullptr;
        list.push_back(std::move(ast));
        if (cur_token_ == kSeparator) NextToken();
    }

    NextToken();   // eat '}'
    return std::make_unique<BlockAST>(std::move(list));
}

ASTPtr Parser::ParsePrimary() {
    switch (cur_token_) {
        case kKeyword: {
            switch (lexer_.key_val()) {
                case kNumber: case kFloat: case kFunction:
                case kString: case kList: case kVar: {
                    auto type = lexer_.key_val();
                    NextToken();
                    return ParseVarDefinition(type);
                }
                case kAsm: {
                    return ParseAsm();
                }
                case kIf: {
                    return ParseIf();
                }
                case kWhile: {
                    return ParseWhile();
                }
                case kImport: case kExport: {
                    return ParseExternal();
                }
                case kBreak: case kContinue: case kReturn: {
                    return ParseControlFlow();
                }
                default: {
                    return PrintError("invalid usage of keyword");
                }
            }
        }
        case kId: {
            return ParseId();
        }
        case kNum: {
            return ParseNumber();
        }
        case kDecimal: {
            return ParseDecimal();
        }
        case kStr: {
            return ParseString();
        }
        case kSeparator: {
            while (NextToken() == kSeparator) {}
            return ParsePrimary();
        }
        case kEOF: {
            return nullptr;   // TODO: notify that it's already EOF
        }
        case '(': {
            return ParseBracket();
        }
        // case '{': {   // bare 'code block' is not supported
        //     return ParseBlock();
        // }
        default: {
            return PrintError("unknown syntax");
        }
    }
}

ASTPtr Parser::ParseExpression() {
    auto lhs = ParseUnaryExpression();
    if (!lhs) return nullptr;
    return ParseBinaryExpression(0, std::move(lhs));
}
