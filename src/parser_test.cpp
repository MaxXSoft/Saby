#include "parser.h"

#include <iostream>

int main(int argc, const char *argv[]) {
    std::ifstream in(argv[1]);
    Lexer lexer(in);
    Parser parser(lexer);
    Analyzer analyzer(parser);
    while (auto ast = parser.ParseNext()) {
        ast->SemaAnalyze(analyzer);
        ast->CodeGen();
        std::cout << std::endl;
    }
    // for (const auto &i : analyzer.env()->table()) {
    //     std::cout << i.first << ": ";
    //     std::cout << i.second.type() << std::endl;
    // }
    return 0;
}
