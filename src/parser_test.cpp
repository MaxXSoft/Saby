#include <iostream>

#include "parser.h"

int main(int argc, const char *argv[]) {
    std::string lib_path(argv[0]), sym_path(argv[1]);
    lib_path = lib_path.substr(0, lib_path.rfind("/") + 1) + "../lib/";
    sym_path += ".sym";

    std::ifstream in(argv[1]);
    Lexer lexer(in);
    Parser parser(lexer);
    Analyzer analyzer(lexer);

    analyzer.set_lib_path(lib_path);
    analyzer.set_sym_path(sym_path);

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
