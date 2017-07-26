#ifndef SABY_SEMA_H_
#define SABY_SEMA_H_

#include "symbol.h"
#include "ast.h"
#include "parser.h"

enum ReturnStatus {
    kStatusError = -1,
    kDefault,
    kIdFound, kOuterIdFound, kIdAdded
};

class Analyzer {
public:
    Analyzer(Parser &parser)
            : parser_(parser), env_(MakeEnvironment(nullptr)) {}
    Analyzer(Parser &parser, const EnvPtr &env)
            : parser_(parser), env_(env) {}
    ~Analyzer() {}

    int Analyze();

    unsigned int error_num() const { return error_num_; }
    const EnvPtr &env() const { return env_; }

private:
    int PrintError(const char *description);

    Parser &parser_;
    unsigned int error_num_;
    EnvPtr env_;
};

#endif // SABY_SEMA_H_
