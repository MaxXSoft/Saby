#ifndef SABY_SEMA_H_
#define SABY_SEMA_H_

#include <string>

#include "symbol.h"
#include "lexer.h"

class Analyzer {
public:
    Analyzer(Lexer &lexer)
            : lexer_(lexer), env_(MakeEnvironment(nullptr)),
              error_num_(0), warning_num_(0) {}
    Analyzer(Lexer &lexer, const EnvPtr &env)
            : lexer_(lexer), env_(env), error_num_(0), warning_num_(0) {}
    ~Analyzer() {}

    TypeValue AnalyzeId(const std::string &id, TypeValue type);
    TypeValue AnalyzeVar(const VarTypeList &defs, TypeValue type);
    TypeValue AnalyzeBinExpr(int op, TypeValue l_type, TypeValue r_type, bool is_lvalue);
    TypeValue AnalyzeUnaExpr(int op, TypeValue type, bool is_lvalue);
    TypeValue AnalyzeCall(TypeValue callee, const TypeList &args);
    TypeValue AnalyzeFunc(const TypeList &args, TypeValue ret_type);
    TypeValue AnalyzeFuncReturn(TypeValue return_type);
    TypeValue AnalyzeBreakCont(int ast_type);
    TypeValue AnalyzeCtrlFlow(int ctrlflow_type, TypeValue value);
    TypeValue AnalyzeExtern(int ext_type, const LibList &libs);

    void NewEnvironment() {
        nested_env_ = MakeEnvironment(env_);
        env_ = nested_env_;
    }
    void RestoreEnvironment() {
        env_ = env_->outer();
        if (nested_env_->outer() != env_) nested_env_ = nested_env_->outer();
    }

    // NOTE: absolute path requited!
    void set_lib_path(const std::string &lib_path) {
        lib_path_ = lib_path;
        if (lib_path_.back() != '/') lib_path_.push_back('/');
    }
    void set_sym_path(const std::string &sym_path) { sym_path_ = sym_path; }
    void set_has_return(bool has_return) { has_return_ = has_return; }

    unsigned int error_num() const { return error_num_; }
    unsigned int warning_num() const { return warning_num_; }
    const EnvPtr &env() const { return env_; }
    const EnvPtr &nested_env() const { return nested_env_; }

private:
    TypeValue PrintError(const char *description, const char *id = nullptr);
    void PrintWarning(const char *description, const char *id);

    Lexer &lexer_;
    unsigned int error_num_, warning_num_;
    EnvPtr env_, nested_env_;
    // lib_path: run_path/lib/; sym_path: file_path/file_name.saby.sym
    std::string lib_path_, sym_path_;
    bool has_return_;
};

#endif // SABY_SEMA_H_
