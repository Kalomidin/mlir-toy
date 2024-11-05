// lexer.cpp
#include "lexer.hpp"
#include <iostream>
#include <string>
#include <fstream>

using namespace std; // Safe to use in cpp files

// Define global variables here, to avoid multiple definitions
string IdentifierStr;
size_t Line = 1;
double NumVal;
int CurTok;
bool EXIT_ON_ERROR = false;

std::ifstream *file = nullptr;

char readChar();

// Helper functions
bool is_alpha(int c) { return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'); }

bool is_underscore(int c) { return c == '_'; }

bool is_alnum(int c) { return is_alpha(c) || (c >= '0' && c <= '9') || is_underscore(c); }

// gettokn - Return the next token from standard input.
int gettokn() {
    static int LastChar = ' ';

    // Skip any whitespace.
    while (isspace(LastChar))
        LastChar = readChar();
    // Check if the character is an alphabet
    if (is_alpha(LastChar)) { // identifier: [a-zA-Z][a-zA-Z0-9]*
        IdentifierStr = LastChar;
        while (is_alnum((LastChar = readChar())))
            IdentifierStr += LastChar;

        if (IdentifierStr == "def")
            return tok_def;
        if (IdentifierStr == "var")
            return tok_var;
        if (IdentifierStr == "return")
            return tok_return;
        return tok_identifier;
    }

    // Check if the character is a number
    if (isdigit(LastChar) || LastChar == '.') {
        string NumStr;
        do {
            NumStr += LastChar;
            LastChar = readChar();
        } while (isdigit(LastChar) || LastChar == '.');

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

    if (LastChar == '\n' || LastChar == '\r') {
        Line++;
    }

    // Check for end of file. Don't eat the EOF.
    if (LastChar == EOF)
        return tok_eof;

    // Otherwise, just return the character as its ASCII value.
    int ThisChar = LastChar;
    LastChar = readChar();
    return ThisChar;
}

// Define getNextToken here instead of in the header
int getNextToken() { return CurTok = gettokn(); }

void readFile(const std::string &filename) {
    file = new std::ifstream(filename);
    if (!file->is_open()) {
        std::cerr << "Error opening file: " << filename << std::endl;
        exit(1);
    }
    EXIT_ON_ERROR = true;
}

bool isFileSet() { return file != nullptr; }

void closeFile() {
    if (file != nullptr) {
        file->close();
        delete file; // free the memory
    }
}

char readChar() {
    if (file == nullptr) {
        return getchar();
    }
    return file->get();
}