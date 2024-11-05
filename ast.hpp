#include <string>
#include <vector>
#include  <iostream>
#include "lexer.hpp"

class ExprAST {
public:
    // Virtual destructor to ensure proper cleanup of derived class objects
    virtual ~ExprAST() = default;
    // Static single assignment (SSA) -> every variable is assigned only once
    virtual void print(int spacing) const = 0;
};

// Expression class for numeric literals like "1.0".
class FunctionExprAST : public ExprAST {
    std::vector<std::unique_ptr<ExprAST>> Params;
    std::vector<std::unique_ptr<ExprAST>> Block;
    std::string Name;

public:
    FunctionExprAST(std::string Name, std::vector<std::unique_ptr<ExprAST>> Params, std::vector<std::unique_ptr<ExprAST>> Block)
        : Name(Name), Params(std::move(Params)), Block(std::move(Block)) {}

    void print(int spacing) const override {
        std::string spaces = "";
        for (int i = 0; i < spacing; i++) {
            spaces += "  ";
        }
        std::cout << spaces << "Proto: '" << Name << "'" << std::endl;
        std::cout << spaces << "Params: [ " << std::endl;
        for (auto &param : Params) {
            param->print(spacing+1);
        }
        std::cout << spaces << "]" << std::endl;
        std::cout << spaces << "Block: [ " << std::endl;
        for (auto &expr : Block) {
            expr->print(spacing+1);
        }
        std::cout << spaces << "]" << std::endl;
    }
};


class BinOpExprAST : public ExprAST {
    char Op;
    std::unique_ptr<ExprAST> LHS, RHS;
public:
    BinOpExprAST(char Op, std::unique_ptr<ExprAST> LHS, std::unique_ptr<ExprAST> RHS)
        : Op(Op), LHS(std::move(LHS)), RHS(std::move(RHS)) {}

    void print(int spacing) const override {
        std::string spaces = "";
        for (int i = 0; i < spacing; i++) {
            spaces += "  ";
        }
        std::cout << spaces << "BinOp: " << Op << std::endl;
        std::cout << spaces << "LHS:[ ";
        LHS->print(spacing+1);
        std::cout << "]" << std::endl;
        std::cout << spaces << "RHS:[ ";
        RHS->print(spacing+1);
        std::cout << "]" << std::endl;
    }
};

class VarDeclExprAST : public ExprAST {
    std::string Name;
    int Row, Column;
    std::unique_ptr<ExprAST> Literal;
public:
    VarDeclExprAST(std::string Name, int Row, int Column, std::unique_ptr<ExprAST> Literal)
        : Name(Name), Row(Row), Column(Column), Literal(std::move(Literal)) {}
    VarDeclExprAST(std::string Name)
        : Name(Name), Row(0), Column(0), Literal(std::move(nullptr)) {}

    void print(int spacing) const override {
        std::string spaces = "";
        for (int i = 0; i < spacing; i++) {
            spaces += "  ";
        }
        std::cout << spaces << "VarDecl: " << Name << std::endl;
         spaces += "  ";
        if (Row != 0 && Column != 0) {
            std::cout << spaces << "Row: " << Row << std::endl;
            std::cout << spaces << "Column: " << Column << std::endl;
        }
        if (Literal != nullptr) {
            std::cout << spaces << "Literal:[";
            Literal->print(spacing+1);
            std::cout << "]" << std::endl;
        }
    }
};

class ArrayExprAST : public ExprAST {
    std::vector<std::unique_ptr<ExprAST>> Elements;
public:
    ArrayExprAST(std::vector<std::unique_ptr<ExprAST>> Elements)
        : Elements(std::move(Elements)) {}
    size_t Size() { return Elements.size(); }
    std::vector<std::unique_ptr<ExprAST>> &GetElements() { return Elements; }

    void print(int spacing) const override {
        std::string spaces = "";
        for (int i = 0; i < spacing; i++) {
            spaces += "  ";
        }
        std::cout << "Array:[";
        for (auto &element : Elements) {
            element->print(spacing+1);
            if (&element != &Elements.back()) {
                std::cout << ", ";
            }
        }
        std::cout << "]";
    }
};

class LiteralExprAST : public ExprAST {
    double Val;
public:
    LiteralExprAST(double Val) : Val(Val) {}

    void print(int spacing) const override {
        std::cout << Val;
    }
};

class ReturnExprAST : public ExprAST {
    std::unique_ptr<ExprAST> Value;
public:
    ReturnExprAST(std::unique_ptr<ExprAST> Value) : Value(std::move(Value)) {}

    void print(int spacing) const override {
        std::string spaces = "";
        for (int i = 0; i < spacing; i++) {
            spaces += "  ";
        }
        std::cout << spaces << "Return: " << std::endl;
        if (Value != nullptr) {
            Value->print(spacing+1);
        }
    }
};

class CallExprAST : public ExprAST {
    std::string Callee;
    std::vector<std::unique_ptr<ExprAST>> Params;
public:
    CallExprAST(const std::string &Callee,
                std::vector<std::unique_ptr<ExprAST>> Params)
        : Callee(Callee), Params(std::move(Params)) {}

    void print(int spacing) const override {
        std::cout << "Call: " << Callee << ", Params:[ ";
        for (auto &param : Params) {
            param->print(spacing+1);
            if (&param != &Params.back()) {
                std::cout << ", ";
            }
        }
        std::cout << "]";
    }
};

// Expression class for referencing a variable, like "a".
class VariableExprAST : public ExprAST {
    std::string Name;

public:
    VariableExprAST(const std::string &Name) : Name(Name) {}
    void print(int spacing) const override {
        std::cout << "Variable: " << Name;
    }
};

extern bool EXIT_ON_ERROR;

/// LogError* - These are little helper functions for error handling.
inline std::unique_ptr<ExprAST> LogError(const char *Str) {
  fprintf(stderr, "Error in line %zu: %s\n", Line, Str);
  // if exit on error is enabled, exit the program
  if (EXIT_ON_ERROR) {
    exit(1);
  }
  return nullptr;
}
inline std::unique_ptr<FunctionExprAST> LogErrorP(const char *Str) {
  LogError(Str);
  return nullptr;
}
