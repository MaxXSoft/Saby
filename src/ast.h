#ifndef SABY_AST_H_
#define SABY_AST_H_

#include <string>
#include <memory>
#include <utility>
#include <vector>

#include "analyzer.h"
#include "irbuilder.h"

class ExpressionAST {
public:
    enum class ASTType : char {
        Id, Var, Num, Dec, Str,
        Binary, Unary, Call, Block, Func, Asm,
        If, While, CtrlFlow, Extern
    };

    virtual ~ExpressionAST() = default;

    virtual TypeValue SemaAnalyze(Analyzer &ana) = 0;
    virtual SSAPtr GenIR(IRBuilder &irb) = 0;

    ASTType type() const { return type_; }
    EnvPtr env() const { return env_; }

protected:
    ExpressionAST(ASTType type) : type_(type) {}
    void set_env(EnvPtr env) { env_ = env; }

private:
    ASTType type_;
    EnvPtr env_;
};

using ASTPtr = std::unique_ptr<ExpressionAST>;
using ASTPtrList = std::vector<ASTPtr>;
using VarDef = std::pair<std::string, ASTPtr>;
using VarDefList = std::vector<VarDef>;

class IdentifierAST : public ExpressionAST {
public:
    IdentifierAST(const std::string &id, int type)
            : ExpressionAST(ASTType::Id), id_(id), type_(type) {}

    TypeValue SemaAnalyze(Analyzer &ana) override;
    SSAPtr GenIR(IRBuilder &irb) override;

    const std::string &id() const { return id_; }

private:
    std::string id_;
    int type_;
};

class VariableAST : public ExpressionAST {
public:
    VariableAST(VarDefList defs, int type)
            : ExpressionAST(ASTType::Var),
              defs_(std::move(defs)), type_(type) {}

    TypeValue SemaAnalyze(Analyzer &ana) override;
    SSAPtr GenIR(IRBuilder &irb) override;

private:
    VarDefList defs_;
    int type_;
};

class NumberAST : public ExpressionAST {
public:
    NumberAST(long long value)
            : ExpressionAST(ASTType::Num), value_(value) {}

    TypeValue SemaAnalyze(Analyzer &ana) override;
    SSAPtr GenIR(IRBuilder &irb) override;

private:
    long long value_;
};

class DecimalAST : public ExpressionAST {
public:
    DecimalAST(double value)
            : ExpressionAST(ASTType::Dec), value_(value) {}

    TypeValue SemaAnalyze(Analyzer &ana) override;
    SSAPtr GenIR(IRBuilder &irb) override;

private:
    double value_;
};

class StringAST : public ExpressionAST {
public:
    StringAST(const std::string &str)
            : ExpressionAST(ASTType::Str), str_(str) {}

    TypeValue SemaAnalyze(Analyzer &ana) override;
    SSAPtr GenIR(IRBuilder &irb) override;

private:
    std::string str_;
};


class BinaryExpressionAST : public ExpressionAST {
public:
    BinaryExpressionAST(int operator_id, ASTPtr lhs, ASTPtr rhs)
            : ExpressionAST(ASTType::Binary), operator_id_(operator_id),
              lhs_(std::move(lhs)), rhs_(std::move(rhs)) {}

    TypeValue SemaAnalyze(Analyzer &ana) override;
    SSAPtr GenIR(IRBuilder &irb) override;

private:
    int operator_id_;
    ASTPtr lhs_, rhs_;
};

class UnaryExpressionAST : public ExpressionAST {
public:
    UnaryExpressionAST(int operator_id, ASTPtr operand)
            : ExpressionAST(ASTType::Unary),
              operator_id_(operator_id), operand_(std::move(operand)) {}

    TypeValue SemaAnalyze(Analyzer &ana) override;
    SSAPtr GenIR(IRBuilder &irb) override;

    void set_operand_type(int operand_type) { operand_type_ = operand_type; }

private:
    int operator_id_, operand_type_;
    ASTPtr operand_;
};

class CallAST : public ExpressionAST {
public:
    CallAST(ASTPtr callee, ASTPtrList args)
            : ExpressionAST(ASTType::Call),
              callee_(std::move(callee)), args_(std::move(args)) {}

    TypeValue SemaAnalyze(Analyzer &ana) override;
    SSAPtr GenIR(IRBuilder &irb) override;

private:
    ASTPtr callee_;
    ASTPtrList args_;
};

class BlockAST : public ExpressionAST {
public:
    BlockAST(ASTPtrList expr_list)
            : ExpressionAST(ASTType::Block), expr_list_(std::move(expr_list)) {}

    TypeValue SemaAnalyze(Analyzer &ana) override;
    SSAPtr GenIR(IRBuilder &irb) override;

private:
    ASTPtrList expr_list_;
};

class FunctionAST : public ExpressionAST {
public:
    FunctionAST(ASTPtrList args, int return_type, ASTPtr body)
            : ExpressionAST(ASTType::Func), args_(std::move(args)),
              return_type_(return_type), body_(std::move(body)) {}

    TypeValue SemaAnalyze(Analyzer &ana) override;
    SSAPtr GenIR(IRBuilder &irb) override;

private:
    ASTPtrList args_;
    int return_type_;
    ASTPtr body_;
};

class AsmAST : public ExpressionAST {
public:
    AsmAST(const std::string &asm_str)
            : ExpressionAST(ASTType::Asm), asm_str_(asm_str) {}

    TypeValue SemaAnalyze(Analyzer &ana) override;
    SSAPtr GenIR(IRBuilder &irb) override;

private:
    std::string asm_str_;
};

class IfAST : public ExpressionAST {
public:
    IfAST(ASTPtr cond, ASTPtr then, ASTPtr else_then)
            : ExpressionAST(ASTType::If), cond_(std::move(cond)),
              then_(std::move(then)), else_then_(std::move(else_then)) {}

    TypeValue SemaAnalyze(Analyzer &ana) override;
    SSAPtr GenIR(IRBuilder &irb) override;

private:
    ASTPtr cond_, then_, else_then_;
};

class WhileAST : public ExpressionAST {
public:
    WhileAST(ASTPtr cond, ASTPtr body)
            : ExpressionAST(ASTType::While),
              cond_(std::move(cond)), body_(std::move(body)) {}

    TypeValue SemaAnalyze(Analyzer &ana) override;
    SSAPtr GenIR(IRBuilder &irb) override;

private:
    ASTPtr cond_, body_;
};

// break, continue, return
class ControlFlowAST : public ExpressionAST {
public:
    ControlFlowAST(int type, ASTPtr value)
            : ExpressionAST(ASTType::CtrlFlow),
              type_(type), value_(std::move(value)) {}

    TypeValue SemaAnalyze(Analyzer &ana) override;
    SSAPtr GenIR(IRBuilder &irb) override;

private:
    int type_;
    ASTPtr value_;
};

// import, export
class ExternalAST : public ExpressionAST {
public:
    ExternalAST(int type, LibList libs)
            : ExpressionAST(ASTType::Extern),
              type_(type), libs_(std::move(libs)) {}

    TypeValue SemaAnalyze(Analyzer &ana) override;
    SSAPtr GenIR(IRBuilder &irb) override;

private:
    int type_;
    LibList libs_;
};

#endif // SABY_AST_H_
