#ifndef LEXER_HPP
#define LEXER_HPP
#include <string>

// Declare external variables to avoid multiple definitions
extern std::string IdentifierStr; 
extern double NumVal;
extern size_t Line;
extern int CurTok;

// The lexer returns tokens [0-255] if it is an unknown character, otherwise one
// of these for known things.
enum Token {
    tok_eof = -1,
    tok_def = -2,
    tok_var = -3,
    tok_number = -4,
    tok_identifier = -5,
    tok_return = -6,
};


// Declare functions
int gettokn();
int getNextToken();
void readFile(const std::string &filename);
void closeFile();
bool isFileSet();

#endif // LEXER_HPP
