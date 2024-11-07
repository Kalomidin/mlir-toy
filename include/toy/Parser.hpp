#ifndef PARSER_HPP
#define PARSER_HPP
#include "AST.hpp"
#include "Lexer.hpp"

namespace toy {

class Parser {

public:
    Parser(Lexer &lexer) : lexer(lexer) {}

    std::unique_ptr<ModuleAST> parseModule() {
        lexer.NextToken();
        std::vector< FunctionExprAST> functions;
        while(lexer.CurToken() != tok_eof) {
            auto f = parseDefinition();
            if (!f) {
                return nullptr;
            }
            functions.push_back(std::move(*f));
        }

        return std::make_unique<ModuleAST>(std::move(functions));
    }
private:
    Lexer &lexer;

    // prototype ::= def id '(' decl_list ')'
    // decl_list ::= id | id, decl_list
    std::unique_ptr<PrototypeExprAST> parsePrototype() {
        auto loc = lexer.GetLocation();

        if (lexer.CurToken() != tok_def) {
            return parseError<PrototypeExprAST>("Expected 'def' in prototype");
        }
        lexer.NextToken(); // eat def
        if (lexer.CurToken() != tok_identifier) {
            return parseError<PrototypeExprAST>("Expected function name in prototype");
        }
        std::string name = lexer.GetIdentifier();
        lexer.NextToken(); // eat function name
        if (lexer.CurToken() != tok_parenthese_open) {
            return parseError<PrototypeExprAST>("Expected '(' in prototype");
        }
        lexer.NextToken(); // eat '('
        std::vector<VariableExprAST> args;
        while (lexer.CurToken() == tok_identifier) {
            args.push_back(VariableExprAST(lexer.GetLocation(), lexer.GetIdentifier()));
            lexer.NextToken(); // eat identifier
            if (lexer.CurToken() != ',') {
                break;
            }
            lexer.NextToken(); // eat ','
        }
        if (lexer.CurToken() != tok_parenthese_close) {
            return parseError<PrototypeExprAST>("Expected ')' in prototype");
        }
        lexer.NextToken(); // eat ')'
        return std::make_unique<PrototypeExprAST>(loc, name, std::move(args));
    }

    std::unique_ptr<ExprAST> parseIdentifier() {
        auto loc = lexer.GetLocation();
        if (lexer.CurToken() != tok_identifier) {
            return parseError<ExprAST>("Expected identifier");
        }
        std::string name = lexer.GetIdentifier();
        lexer.NextToken(); // eat identifier
        if (lexer.CurToken() != tok_parenthese_open) {
            return std::make_unique<VariableExprAST>(loc, name);
        }
        lexer.NextToken(); // eat '('
        std::vector<std::unique_ptr<ExprAST>> args;
        while (lexer.CurToken() != tok_parenthese_close) {
            auto arg = parseExpression();
            if (!arg) {
                return nullptr;
            }
            args.push_back(std::move(arg));
            if (lexer.CurToken() != ',') {
                break;
            }
            lexer.NextToken(); // eat ','
        }
        if (lexer.CurToken() != tok_parenthese_close) {
            return parseError<ExprAST>("Expected ')' in function call");
        }
        lexer.NextToken(); // eat ')'
        return std::make_unique<CallExprAST>(loc, name, std::move(args));    
    }

    std::unique_ptr<LiteralExprAST> parseLiteral() {
        auto loc = lexer.GetLocation();
        if (lexer.CurToken() != tok_number) {
            return parseError<LiteralExprAST>("Expected number");
        }
        double val = lexer.GetNumber();
        lexer.NextToken(); // eat number
        return std::make_unique<LiteralExprAST>(loc, val);
    }

    std::unique_ptr<ExprAST> parseParentExpr() {
        if (lexer.CurToken() != tok_parenthese_open) {
            return parseError<ExprAST>("Expected '(' in expression");
        }
        lexer.NextToken(); // eat '('
        auto expr = parseExpression();
        if (!expr) {
            return nullptr;
        }
        if (lexer.CurToken() != tok_parenthese_close) {
            return parseError<ExprAST>("Expected ')' in expression");
        }
        lexer.NextToken(); // eat ')'
        return expr;
    }

    std::unique_ptr<ExprAST> parseTensorLiteral() {
        // TODO: implement tensor literal
        return parseError<ExprAST>("Tensor literal not implemented");
    }

    std::unique_ptr<ExprAST> parsePrimary() {
        switch(lexer.CurToken()) {
            case tok_identifier:
                return parseIdentifier();
            case tok_number:
                return parseLiteral();
            case tok_parenthese_open:
                return parseParentExpr();
            case tok_sbracket_open:
                return parseTensorLiteral();
            default:
                return parseError<ExprAST>("Expected primary expression");
        }
    }

    // binoprhs ::= ('+' primary)*
    std::unique_ptr<ExprAST> parseBinOpRHS(int precedence, std::unique_ptr<ExprAST> left) {
        while (true) {
            auto loc = lexer.GetLocation();
            int tokPrecedence = lexer.GetTokPrecedence();
            if (tokPrecedence < precedence) {
                return left;
            }
            int binOp = lexer.CurToken();
            lexer.NextToken(); // eat binop
            auto right = parsePrimary();
            if (!right) {
                return nullptr;
            }
            int nextPrecedence = lexer.GetTokPrecedence();
            if (tokPrecedence < nextPrecedence) {
                right = parseBinOpRHS(tokPrecedence + 1, std::move(right));
                if (!right) {
                    return nullptr;
                }
            }
            left = std::make_unique<BinOpExprAST>(loc, binOp, std::move(left), std::move(right));
        }
    }

    // expr = prototype*expr
    // expr = identifier
    // expr = literal
    std::unique_ptr<ExprAST> parseExpression() {
        auto loc = lexer.GetLocation();
        auto left = parsePrimary();
        if (!left) {
            return nullptr;
        }
        return parseBinOpRHS(0, std::move(left));
    }

    std::unique_ptr<VarDeclExprAST> parseVarDecl() {
        auto loc = lexer.GetLocation();
        // read var
        if (lexer.CurToken() != tok_var) {
            return parseError<VarDeclExprAST>("Expected 'var' in variable declaration");
        }
        lexer.NextToken(); // eat var
        // read identifier
        if (lexer.CurToken() != tok_identifier) {
            return parseError<VarDeclExprAST>("Expected identifier after var");
        }
        std::string name = lexer.GetIdentifier();
        lexer.NextToken(); // eat identifier
        VarType type;
        // check if type specified
        if (lexer.CurToken() == tok_brace_open) {
            std::vector<int> shape;
            lexer.NextToken(); // eat '<'
            while(lexer.CurToken()!= tok_brace_close) {
                shape.push_back(lexer.GetNumber());
                lexer.NextToken(); // eat type
            }
            if (shape.size() != 2) {
                return parseError<VarDeclExprAST>("Expected 2 numbers in type");
            }
            lexer.NextToken(); // eat '>'
            type.shape = shape;
        }
        // read '='
        if (lexer.CurToken() != '=') {
            return parseError<VarDeclExprAST>("Expected '=' after var");
        }
        lexer.NextToken(); // eat '='
        // read expression
        auto expr = parseExpression();
        if (!expr) {
            return nullptr;
        }
        return std::make_unique<VarDeclExprAST>(loc, name, type, std::move(expr));
    }

    std::unique_ptr<ReturnExprAST>  parseReturn() {
        auto loc = lexer.GetLocation();
        // read return
        if (lexer.CurToken() != tok_return) {
            return parseError<ReturnExprAST>("Expected 'return' in return statement");
        }
        lexer.NextToken(); // eat return
        // check if return has expression
        if (lexer.CurToken() == tok_semicolon) {
            lexer.NextToken(); // eat ';'
            return parseError<ReturnExprAST>("Expected expression after return");
        }
        auto expr = parseExpression();
        if (!expr) {
            return nullptr;
        }
        return std::make_unique<ReturnExprAST>(loc, std::move(expr));
    }

    // { exprs }
    std::unique_ptr<ExprASTList> parseBlock() {
        // read opening bracket
        if (lexer.CurToken() != tok_bracket_open) {
            return parseError<ExprASTList>("Expected '{' in block");
        }
        lexer.NextToken(); // eat '{'
        std::vector<std::unique_ptr<ExprAST>> exprs;
        while (lexer.CurToken() != tok_bracket_close && lexer.CurToken() != tok_eof) {
            switch(lexer.CurToken()) {
                // var a = expr;
                case tok_var:
                    if (auto var = parseVarDecl()) {
                        exprs.push_back(std::move(var));
                    } else {
                        return nullptr;
                    }
                    break;
                // return expr;
                case tok_return:
                    if (auto ret = parseReturn()) {
                        exprs.push_back(std::move(ret));
                    } else {
                        return nullptr;
                    }
                    break;
                // expr;
                default:
                    if (auto expr = parseExpression()) {
                        exprs.push_back(std::move(expr));
                    } else {
                        return nullptr;
                    }
                    break;
            }
            // read semi colon
            if (lexer.CurToken() != tok_semicolon) {
                return parseError<ExprASTList>("Expected ';' after expression");
            }
            lexer.NextToken(); // eat ';'
        }
        if (lexer.CurToken() != tok_bracket_close) {
            return parseError<ExprASTList>("Expected '}' in block");
        }
        lexer.NextToken(); // eat '}'
        return std::make_unique<ExprASTList>(std::move(exprs));
    }

    std::unique_ptr<FunctionExprAST> parseDefinition() {
        auto loc = lexer.GetLocation();
        auto proto = parsePrototype();
        if (!proto) {
            return nullptr;
        }
        auto block = parseBlock();
        if (!block) {
            return nullptr;
        }
        return std::make_unique<FunctionExprAST>(loc, std::move(proto), std::move(block));
    };

    template<typename R>
    std::unique_ptr<R> parseError(const std::string &msg) {
        std::cerr << "Error: " << msg << " at line " << lexer.LineNumber() << " column " << lexer.ColumnNumber() << std::endl;
        return nullptr;
    }
};
};


 #endif // PARSER_HPP