
#ifndef LEXER_HPP
#define LEXER_HPP
#include <string>
#include <fstream>
#include <iostream>

namespace toy {

// The lexer returns tokens [0-255] if it is an unknown character, otherwise one
// of these for known things.
enum Token {
    tok_eof = -1,
    tok_def = -2,
    tok_var = -3,
    tok_number = -4,
    tok_identifier = -5,
    tok_return = -6,

    tok_semicolon = ';',
    tok_parenthese_open = '(',
    tok_parenthese_close = ')',
    tok_bracket_open = '{',
    tok_bracket_close = '}',
    tok_sbracket_open = '[',
    tok_sbracket_close = ']',
    tok_brace_open = '<',
    tok_brace_close = '>',
};

struct Location {
    std::shared_ptr<std::string> Filename;
    size_t Line;
    size_t Column;
};

class Lexer {

public:
    Lexer(std::string filename) {
        IdentifierStr = "";
        NumVal = 0;
        Line = 1;
        Column = 0;
        CurTok = 0;
        Filename = filename;
        file = new std::ifstream(filename);
        if (!file->is_open()) {
            std::cerr << "Error opening file: " << filename << std::endl;
            exit(1);
        }
    }

    int CurToken() { return CurTok; }


    int LineNumber() { return Line; }
    int ColumnNumber() { return Column; }
    Location GetLocation() { return location; }

    std::string GetIdentifier() { return IdentifierStr; }
    double GetNumber() { return NumVal; }

    void NextToken() { CurTok = gettokn(); }

    int GetTokPrecedence() {
        switch(CurTok) {
            case '<':
                return 10;
            case '+':
                return 20;
            case '-':
                return 20;
            case '*':
                return 40;
            case '/':
                return 40;
            default:
                return -1;
        }
    }

private:
    std::string IdentifierStr;
    double NumVal;
    size_t Line;
    size_t Column;
    int CurTok;
    std::string Filename;
    std::ifstream *file;
    int LastChar = ' ';
    Location location;

    int gettokn() {
        // Skip any whitespace.
        while (isspace(LastChar)) {
            LastChar = readChar();
        }
        location.Line = Line;
        location.Column = Column;

        if (isalpha(LastChar)) { // identifier: [a-zA-Z][a-zA-Z0-9]*
            IdentifierStr = LastChar;
            while (isalnum((LastChar = readChar())) || LastChar == '_') {
                IdentifierStr += LastChar;
            }

            if (IdentifierStr == "def") {
                return tok_def;
            }
            if (IdentifierStr == "var") {
                return tok_var;
            }
            if (IdentifierStr == "return") {
                return tok_return;
            }
            return tok_identifier;
        }

        // Check if the character is a number
        if (isdigit(LastChar) || LastChar == '.') {
            std::string NumStr;
            do {
                NumStr += LastChar;
                LastChar = readChar();
            } while (isdigit(LastChar) || LastChar == '.');
            // TODO: validate number (e.g. 1.2.3)
            NumVal = strtod(NumStr.c_str(), 0);
            return tok_number;
        }

        // Check if the character is a comment
        if (LastChar == '#') {
            // Comment until end of line.
            do
                LastChar = readChar();
            while (LastChar != EOF && LastChar != '\n' && LastChar != '\r');

            if (LastChar != EOF)
                return gettokn();
        }

        // Check for end of file. Don't eat the EOF.
        if (LastChar == EOF)
            return tok_eof;

        // Otherwise, just return the character as its ASCII value.
        int ThisChar = LastChar;
        LastChar = readChar();
        return ThisChar;
    }

    int readChar() {
        char c = file->get();
        Column++;
        if (c == '\n') {
            Line++;
            Column = 0;
        }
        return c;
    }
};
};

#endif // LEXER_HPP