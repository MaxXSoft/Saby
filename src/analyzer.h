#ifndef SABY_SEMA_H_
#define SABY_SEMA_H_

#include <string>

class Analyzer;

#include "symbol.h"
#include "ast.h"
#include "parser.h"

class Analyzer {
public:
    Analyzer(Parser &parser)
            : parser_(parser), env_(MakeEnvironment(nullptr)) {}
    Analyzer(Parser &parser, const EnvPtr &env)
            : parser_(parser), env_(env) {}
    ~Analyzer() {}

    TypeValue AnalyzeId(const std::string &id, TypeValue type);
    TypeValue AnalyzeVar(const VarDefList &defs, TypeValue type);
    TypeValue AnalyzeBinExpr(int op, TypeValue l_type, TypeValue r_type);
    TypeValue AnalyzeUnaExpr(int op, TypeValue type);
    TypeValue AnalyzeCall(const ASTPtr &callee, const ASTPtrList &args);
    TypeValue AnalyzeBlock(const ASTPtrList &expr_list);
    TypeValue AnalyzeFunc(const ASTPtrList &args, TypeValue ret_type, const ASTPtr &body);
    TypeValue AnalyzeIf(const ASTPtr &cond, const ASTPtr &then, const ASTPtr &else_then);
    TypeValue AnalyzeWhile(const ASTPtr &cond, const ASTPtr &body);
    TypeValue AnalyzeCtrlFlow(int ctrlflow_type, const ASTPtr &value);
    TypeValue AnalyzeExtern(int ext_type, const LibList &libs);

    unsigned int error_num() const { return error_num_; }
    const EnvPtr &env() const { return env_; }
    const EnvPtr &nested_env() const { return nested_env_; }

    void set_lib_path(const std::string lib_path) { lib_path_ = lib_path; }
    void set_sym_path(const std::string sym_path) { sym_path_ = sym_path; }

private:
    TypeValue PrintError(const char *description, const char *id = nullptr);
    void NewEnvironment() {
        nested_env_ = MakeEnvironment(env_);
        env_ = nested_env_;
    }
    void RestoreEnvironment() { env_ = env_->outer(); }

    Parser &parser_;
    unsigned int error_num_;
    EnvPtr env_, nested_env_;
    // lib_path: run_path/lib/; sym_path: file_path/file_name.saby.sym
    std::string lib_path_, sym_path_;
};

#endif // SABY_SEMA_H_
