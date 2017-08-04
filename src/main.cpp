#include <vector>
#include <string>

#include "lexer.h"
#include "parser.h"
#include "analyzer.h"

int main(int argc, const char *argv[]) {
    std::vector<std::string> arg_list;
    for (int i = 1; i < argc; ++i) {
        arg_list.push_back(argv[i]);
    }
    return 0;
}
