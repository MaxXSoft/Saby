#include <iostream>
#include <sstream>
#include <string>

#include "lexer.h"
#include "parser.h"
#include "analyzer.h"
// #include "generator.h"
#include "xstl/argh.h"

namespace {

enum FontColor {
    kColorRed = 31,
    kColorGreen, kColorYellow, kColorBlue,
    kColorPurple, kColorDeepGreen
};

void PrintText(const std::string &str, int color = 0, bool bold = false) {
    std::string temp;
    if (bold || color) {
        if (bold) temp = "\033[1m";
        if (color) {
            temp += "\033[";
            std::ostringstream oss;
            oss << temp << color;
            temp = oss.str() + "m";
        }
        temp += str;
        temp += "\033[0m";
    }
    else {
        temp = str;
    }
    std::cout << temp;
}

void PrintError(const std::string &err_msg) {
    std::cout << "sabyc: ";
    PrintText("error: ", kColorRed, true);
    std::cout << (err_msg.empty() ? "invalid argument" : err_msg) << std::endl;
}

} // namespace

int main(int argc, const char *argv[]) {
    std::string lib_path(argv[0]), sym_path(argv[1]);
    lib_path = lib_path.substr(0, lib_path.rfind("/") + 1) + "../lib/";
    sym_path += ".sym";
    std::string input_file, output_file;

    xstl::ArgumentHandler argh;

    if (argc < 2) PrintError("");

    argh.SetErrorHandler([](xstl::StrRef v) {
        PrintError("invalid argument '" + v + "'");
        return 1;
    });
    argh.AddHandler("", [&input_file](xstl::StrRef v) { input_file = v; return 0; });
    argh.AddHandler("o", [&output_file](xstl::StrRef v) { output_file = v; return 0; });
    // -h -v --help --version

    argh.ParseArguments(argc, argv);

    return 0;
}
