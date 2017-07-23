#include "parser.h"

#include <iostream>

int main(int argc, const char *argv[]) {
    std::ifstream in(argv[1]);
    Lexer lexer(in);
    Parser parser(lexer);
    while (auto ast = parser.ParseNext()) {
        ast->CodeGen();
        std::cout << std::endl;
    }
    return 0;
}
