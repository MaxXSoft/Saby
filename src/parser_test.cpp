#include "parser.h"

#include <iostream>

int main(int argc, const char *argv[]) {
    std::ifstream in(argv[1]);
    Lexer lexer(in);
    Parser parser(lexer);
    auto env = MakeEnvironment(nullptr);
    while (auto ast = parser.ParseNext()) {
        ast->CodeGen(env);
        std::cout << std::endl;
    }
    return 0;
}
