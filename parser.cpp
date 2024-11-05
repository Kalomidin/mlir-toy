

#include "lexer.hpp"
#include "parser.hpp"

static std::unique_ptr<ExprAST> ParsePrimary();
static std::unique_ptr<ExprAST> ParseExpression();

static int getTokPrecedence() {
    switch (CurTok) {
        case '*':
            return 100;
        case '/':
            return 100;
        case '+':
            return 90;
        case '-':
            return 90;
        case '=':
            return 0;
        default:
            return -1;
    }
}

static std::unique_ptr<ArrayExprAST> ParseArray() {
    if (CurTok != '[') {
        LogErrorP("Expected '[' in array");
        return nullptr;
    }
    getNextToken(); // eat '['
    std::vector<std::unique_ptr<ExprAST>> Elements;
    while (CurTok != ']') {
        if (CurTok == tok_number) {
            double Val = NumVal;
            getNextToken(); // eat number
            Elements.push_back(std::make_unique<LiteralExprAST>(Val));
        } else {
            auto arr = ParseArray();
            if (arr == nullptr) {
                return nullptr;
            }
            Elements.push_back(std::move(arr));
        }
        if (CurTok == ',') {
            getNextToken(); // eat ','
        }
    }
    getNextToken(); // eat ']'
    return std::make_unique<ArrayExprAST>(std::move(Elements));
}

static std::unique_ptr<ExprAST> ParseVarDecl() {
    getNextToken(); // eat var
    if (CurTok != tok_identifier) {
        return LogError("Expected identifier after var");
    }
    std::string Name = IdentifierStr;
    getNextToken(); // eat identifier
    int Row, Column = 0;
    if (CurTok == '<') {
        getNextToken(); // eat '<'
        if (CurTok != tok_number) {
            return LogError("Expected number after '<'");
        }
        Row = NumVal;
        getNextToken(); // eat number
        if (CurTok != ',') {
            return LogError("Expected ',' after number");
        }
        getNextToken(); // eat ','
        if (CurTok != tok_number) {
            return LogError("Expected number after ','");
        }
        Column = NumVal;
        getNextToken(); // eat number
        if (CurTok != '>') {
            return LogError("Expected '>' after number");
        }
        getNextToken(); // eat '>'
    }
    if (CurTok != '=') {
        return LogError("Expected '=' after var");
    }
    getNextToken(); // eat '='

    auto Literal = ParseExpression();
    if (Literal == nullptr) {
        return nullptr;
    }
    return std::make_unique<VarDeclExprAST>(Name, Row, Column, std::move(Literal));
}

static std::unique_ptr<ExprAST> ParseReturnExpr() {
    getNextToken(); // eat return
    if (CurTok == ';') {
        return std::make_unique<ReturnExprAST>(nullptr);
    }
    auto E = ParseExpression();
    if (E == nullptr) {
        return nullptr;
    }
    return std::make_unique<ReturnExprAST>(std::move(E));
}

static std::unique_ptr<ExprAST> ParseIdentifierExpr() {
    std::string callee = IdentifierStr;
    getNextToken(); // eat identifier
    if (CurTok != '(') {
        return std::make_unique<VariableExprAST>(callee);
    }
    getNextToken(); // eat '('
    std::vector<std::unique_ptr<ExprAST>> Args;
    while(CurTok != ')') {
        if (auto Arg = ParseExpression()) {
            Args.push_back(std::move(Arg));
        } else {
            return nullptr;
        }
        if (CurTok == ',') {
            getNextToken(); // eat ','
        }
    }
    getNextToken(); // eat ')'
    return std::make_unique<CallExprAST>(callee, std::move(Args));
}

static std::unique_ptr<ExprAST> ParseNumberExpr() {
    auto Result = std::make_unique<LiteralExprAST>(NumVal);
    getNextToken(); // eat number
    return std::move(Result);
}

static std::unique_ptr<ExprAST> ParseParentExpr() {
    getNextToken(); // eat '('
    auto V = ParseExpression();
    if (!V) {
        return nullptr;
    }
    if (CurTok != ')') {
        return LogError("Expected ')'");
    }
    getNextToken(); // eat ')'
    return V;
}

// read line by line and each line ends with a semicolon
// each line one of 
// 1. var
// 2. return 
// 3. function call
// 4. expression with number
static std::unique_ptr<ExprAST> ParsePrimary() {
    switch (CurTok) {
    case tok_var:
        return ParseVarDecl();
    case tok_identifier:
        return ParseIdentifierExpr();
    case tok_number:
        return ParseNumberExpr();
    case tok_return:
        return ParseReturnExpr();
    case '[':
        return ParseArray();
    case '(':
        return ParseParentExpr();
    default:
        return LogError("unknown token when expecting an expression");
    }
}

static std::unique_ptr<ExprAST> ParseBinOPRHS(int precedence, std::unique_ptr<ExprAST> LHS) {
    while (true) {
        int TokPrec = getTokPrecedence();
        if (TokPrec <= precedence) {
            return LHS;
        }
        int BinOp = CurTok;
        getNextToken(); // eat binop

        auto RHS = ParsePrimary();
        if (!RHS) {
            return nullptr;
        }

        int NextPrec = getTokPrecedence();
        if (TokPrec < NextPrec) {
            RHS = ParseBinOPRHS(TokPrec + 1, std::move(RHS));
            if (!RHS) {
                return nullptr;
            }
        }

        LHS = std::make_unique<BinOpExprAST>(BinOp, std::move(LHS), std::move(RHS));
    }
}

static std::unique_ptr<ExprAST> ParseExpression() {
    auto LHS = ParsePrimary();
    if (!LHS) {
        return nullptr;
    }
    return ParseBinOPRHS(0, std::move(LHS));
}

static std::unique_ptr<FunctionExprAST> ParseDefinition() {
    getNextToken(); // eat def
    if (CurTok != tok_identifier) {
        return LogErrorP("Expected function name in definition");
    }

    std::string FnName = IdentifierStr;
    getNextToken(); // eat function name
    if (CurTok != '(') {
        return LogErrorP("Expected '(' in function definition");
    }
    getNextToken(); // eat '('
    std::vector<std::unique_ptr<ExprAST>> Params;
    while (CurTok == tok_identifier) {
        Params.push_back(std::make_unique<VarDeclExprAST>(IdentifierStr));
        getNextToken(); // eat identifier
        if (CurTok != ',') {
            break;
        }
        getNextToken(); // eat ','
    }
    if (CurTok != ')') {
        return LogErrorP("Expected ')' in function definition");
    }

    getNextToken(); // eat ')'

    if (CurTok != '{') {
        return LogErrorP("Expected '{' in function definition");
    }
    getNextToken(); // eat '{'

    std::vector<std::unique_ptr<ExprAST>> Block;
    while (CurTok != '}' && CurTok != tok_eof) {
        if (auto E = ParseExpression()) {
            // E->print(1);
            Block.push_back(std::move(E));
        } else {
            return nullptr;
        }
        // read semi colon
        if (CurTok != ';') {
            return LogErrorP("Expected ';' after expression");
        }
        getNextToken(); // eat ';'
    }
    if (CurTok != '}') {
        return LogErrorP("Expected '}' in function definition");
    }
    getNextToken(); // eat '}'

    return std::make_unique<FunctionExprAST>(FnName, std::move(Params), std::move(Block));
}

static std::unique_ptr<ExprAST> HandleDefinition() {
    if (auto FnAST = ParseDefinition()) {
        printf("Parsed a function definition.\n");
        return FnAST;
    } else {
        return nullptr;
    }
}

void MainLoop() {
    // every time before get next token, print the prompt
    getNextToken();
    while (true) {
        if (!isFileSet()) {
            LogErrorP("No file set");
            return;
        }
        std::unique_ptr<ExprAST> expression;
        switch (CurTok) {
        case tok_eof:
            return;
        case '\n':
            break;
        case tok_def:
            expression = HandleDefinition();
            if (expression == nullptr) {
                return;
            }
            expression->print(1);
            break;
        default:
            LogErrorP("unknown token when expecting a definition");
            return;
        }
    }
}