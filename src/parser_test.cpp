#include "parser.h"

#include <iostream>

int main(int argc, const char *argv[]) {
    std::ifstream in(argv[1]);
    Lexer lexer(in);
    Parser parser(lexer);
    Analyzer analyzer(parser);

    while (auto ast = parser.ParseNext()) {
        if (ast->SemaAnalyze(analyzer) == kTypeError) break;
        ast->CodeGen();
        std::cout << std::endl;
    }

    auto err_num = lexer.error_num() + parser.error_num() + analyzer.error_num();
    if (err_num == 1) {
        std::cout << err_num << " error generated." << std::endl;
    }
    else if (err_num > 1) {
        std::cout << err_num << " errors generated." << std::endl;
    }
    
    return 0;
}
