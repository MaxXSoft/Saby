// Semantic Analysis

#include "sema.h"

#include <cstdio>

namespace {

//

} // namespace

int Analyzer::PrintError(const char *description) {
    fprintf(stderr, "\033[1manalyzer\033[0m(line %u): \033[31m\033[1merror:\033[0m %s\n", parser_.line_pos(), description);
    ++error_num_;
    return kStatusError;
}
