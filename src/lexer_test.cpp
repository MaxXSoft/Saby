#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

#include "lexer.h"

enum FontColor {
    kColorRed = 31,
    kColorGreen, kColorYellow, kColorBlue,
    kColorPurple, kColorDeepGreen
};

const char *keyword_str[] = {
    "number", "float", "function", "string", "list", "void",
    "import", "export", "asm",
    "if", "else",
    "return",
    "while", "break", "continue"
};

const char *operator_str[] = {
    "", "&", "^", "|", "~", "<<", ">>",
    "+", "-", "*", "/", "%", "**",
    "++", "--",
    "<", "<=", ">", ">=", "==", "!=",
    "=>",
    "=",
    "&=", "^=", "|=", "~=", "<<=", ">>=",
    "+=", "-=", "*=", "/=", "%=", "**="
};

void PrintText(const char *str, int color = 0, bool bold = false) {
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

void PrintTokens(Lexer &lexer) {
    int token, line_num = 1;
    std::cout << "1 ";
    do {
        if (lexer.line_pos() != line_num) {
            line_num = lexer.line_pos();
            std::cout << std::endl << line_num << " ";
        }
        switch (token = lexer.NextToken()) {
            case kError: {
                // PrintText("error", kColorRed);
                break;
            }
            case kEOF: {
                PrintText("end of file", 0, true);
                break;
            }
            case kId: {
                PrintText(lexer.id_val().c_str(), kColorYellow);
                break;
            }
            case kNum: {
                std::cout << lexer.num_val();
                break;
            }
            case kDecimal: {
                std::cout << lexer.dec_val();
                break;
            }
            case kStr: {
                std::cout << "\"" << lexer.str_val() << "\"";
                break;
            }
            case kKeyword: {
                PrintText(keyword_str[lexer.key_val()], kColorDeepGreen);
                break;
            }
            case kOperator: {
                PrintText(operator_str[lexer.op_val()], kColorRed);
                break;
            }
            case kSeparator: {
                PrintText("[SEP]", kColorBlue, true);
                break;
            }
            default: {
                std::cout << (char)token;
                break;
            }
        }
        std::cout << " ";
    } while (token != kEOF);
    std::cout << std::endl << "error(s): " << lexer.error_num() << std::endl;
}

int main(int argc, const char *argv[]) {
    std::ifstream in(argv[1]);
    Lexer lexer(in);
    PrintTokens(lexer);
    return 0;
}
