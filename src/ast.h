#ifndef SABY_AST_H_
#define SABY_AST_H_

#include <string>
#include <memory>
#include <utility>
#include <vector>

#include "symbol.h"
#include "analyzer.h"
#include "ssa.h"

class ExpressionAST {
public:
    virtual ~ExpressionAST() = default;

    virtual TypeValue SemaAnalyze(Analyzer &ana) = 0;
    virtual SSAPtr GenIR() = 0;

    EnvPtr env;
};

using ASTPtr = std::unique_ptr<ExpressionAST>;
using ASTPtrList = std::vector<ASTPtr>;
using VarDef = std::pair<std::string, ASTPtr>;
using VarDefList = std::vector<VarDef>;

class IdentifierAST : public ExpressionAST {
public:
    IdentifierAST(const std::string &id, int type) : id_(id), type_(type) {}
    
    TypeValue SemaAnalyze(Analyzer &ana) override;
    SSAPtr GenIR() override;

private:
    std::string id_;
    int type_;
};

class VariableAST : public ExpressionAST {
public:
    VariableAST(VarDefList defs, int type)
            : defs_(std::move(defs)), type_(type) {}
    
    TypeValue SemaAnalyze(Analyzer &ana) override;
    SSAPtr GenIR() override;

private:
    VarDefList defs_;
    int type_;
};

class NumberAST : public ExpressionAST {
public:
    NumberAST(long long value) : value_(value) {}
    
    TypeValue SemaAnalyze(Analyzer &ana) override;
    SSAPtr GenIR() override;

private:
    long long value_;
};

class DecimalAST : public ExpressionAST {
public:
    DecimalAST(double value) : value_(value) {}
    
    TypeValue SemaAnalyze(Analyzer &ana) override;
    SSAPtr GenIR() override;

private:
    double value_;
};

class StringAST : public ExpressionAST {
public:
    StringAST(const std::string &str) : str_(str) {}
    
    TypeValue SemaAnalyze(Analyzer &ana) override;
    SSAPtr GenIR() override;

private:
    std::string str_;
};


class BinaryExpressionAST : public ExpressionAST {
public:
    BinaryExpressionAST(int operator_id, ASTPtr lhs, ASTPtr rhs)
            : operator_id_(operator_id), lhs_(std::move(lhs)), rhs_(std::move(rhs)) {}
    
    TypeValue SemaAnalyze(Analyzer &ana) override;
    SSAPtr GenIR() override;

private:
    int operator_id_;
    ASTPtr lhs_, rhs_;
};

class UnaryExpressionAST : public ExpressionAST {
public:
    UnaryExpressionAST(int operator_id, ASTPtr operand)
            : operator_id_(operator_id), operand_(std::move(operand)) {}
    
    TypeValue SemaAnalyze(Analyzer &ana) override;
    SSAPtr GenIR() override;

private:
    int operator_id_;
    ASTPtr operand_;
};

class CallAST : public ExpressionAST {
public:
    CallAST(ASTPtr callee, ASTPtrList args)
            : callee_(std::move(callee)), args_(std::move(args)) {}
    
    TypeValue SemaAnalyze(Analyzer &ana) override;
    SSAPtr GenIR() override;

private:
    ASTPtr callee_;
    ASTPtrList args_;
};

class BlockAST : public ExpressionAST {
public:
    BlockAST(ASTPtrList expr_list)
            : expr_list_(std::move(expr_list)) {}
    
    TypeValue SemaAnalyze(Analyzer &ana) override;
    SSAPtr GenIR() override;

private:
    ASTPtrList expr_list_;
};

class FunctionAST : public ExpressionAST {
public:
    FunctionAST(ASTPtrList args, int return_type, ASTPtr body)
            : args_(std::move(args)), return_type_(return_type),
              body_(std::move(body)) {}
    
    TypeValue SemaAnalyze(Analyzer &ana) override;
    SSAPtr GenIR() override;

private:
    ASTPtrList args_;
    int return_type_;
    ASTPtr body_;
};

class AsmAST : public ExpressionAST {
public:
    AsmAST(const std::string &asm_str) : asm_str_(asm_str) {}
    
    TypeValue SemaAnalyze(Analyzer &ana) override;
    SSAPtr GenIR() override;

private:
    std::string asm_str_;
};

class IfAST : public ExpressionAST {
public:
    IfAST(ASTPtr cond, ASTPtr then, ASTPtr else_then)
            : cond_(std::move(cond)), then_(std::move(then)),
              else_then_(std::move(else_then)) {}
    
    TypeValue SemaAnalyze(Analyzer &ana) override;
    SSAPtr GenIR() override;

private:
    ASTPtr cond_, then_, else_then_;
};

class WhileAST : public ExpressionAST {
public:
    WhileAST(ASTPtr cond, ASTPtr body)
            : cond_(std::move(cond)), body_(std::move(body)) {}
    
    TypeValue SemaAnalyze(Analyzer &ana) override;
    SSAPtr GenIR() override;

private:
    ASTPtr cond_, body_;
};

// break, continue, return
class ControlFlowAST : public ExpressionAST {
public:
    ControlFlowAST(int type, ASTPtr value)
            : type_(type), value_(std::move(value)) {}
    
    TypeValue SemaAnalyze(Analyzer &ana) override;
    SSAPtr GenIR() override;

private:
    int type_;
    ASTPtr value_;
};

// import, export
class ExternalAST : public ExpressionAST {
public:
    ExternalAST(int type, LibList libs)
            : type_(type), libs_(std::move(libs)) {}
    
    TypeValue SemaAnalyze(Analyzer &ana) override;
    SSAPtr GenIR() override;

private:
    int type_;
    LibList libs_;
};

#endif // SABY_AST_H_
