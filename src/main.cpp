#include "toy/Lexer.hpp"
#include "toy/Parser.hpp"
#include "toy/AST.hpp"

int main(int argc, char *argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <filename>" << std::endl;
        return 1;
    }

    toy::Lexer lexer(argv[1]);
    toy::Parser parser(lexer);
    auto module = parser.parseModule();
    if (!module) {
        return 1;
    }
    return 0;
}