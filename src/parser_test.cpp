#include <iostream>

#include "fs/dir.h"
#include "lexer.h"
#include "parser.h"
#include "analyzer.h"
#include "irbuilder.h"

int main(int argc, const char *argv[]) {
    std::string lib_path(argv[0]), sym_path(argv[1]);
    lib_path = lib_path.substr(0, lib_path.rfind("/") + 1) + "../lib";
    lib_path = GetRealPath(lib_path);
    sym_path = GetRealPath(sym_path);
    sym_path += ".sym";

    std::ifstream in(argv[1]);
    Lexer lexer(in);
    Parser parser(lexer);
    Analyzer analyzer(lexer);
    IRBuilder irb;

    analyzer.set_lib_path(lib_path);
    analyzer.set_sym_path(sym_path);

    auto entry = irb.NewBlock();
    irb.SealBlock(entry);
    while (auto ast = parser.ParseNext()) {
        if (ast->SemaAnalyze(analyzer) == kTypeError) break;
        ast->GenIR(irb);
    }

    // print all of the blocks
    for (const auto &i : irb.blocks()) {
        i->Print();
        std::cout << std::endl;
    }

    auto err_num = lexer.error_num() + parser.error_num() + analyzer.error_num();
    if (err_num == 1) {
        std::cout << err_num << " error generated. ";
    }
    else if (err_num > 1) {
        std::cout << err_num << " errors generated. ";
    }

    auto war_num = analyzer.warning_num();
    if (war_num == 1) {
        std::cout << war_num << " warning generated.";
    }
    else if (war_num > 1) {
        std::cout << war_num << " warnings generated.";
    }

    if (err_num + war_num) std::cout << std::endl;
    return err_num;
}
