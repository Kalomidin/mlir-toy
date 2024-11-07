#ifndef AST_HPP
#define AST_HPP
#include <vector>
#include "Lexer.hpp"

namespace toy {

struct VarType {
    std::vector<int> shape;
};

class ExprAST {
public:
    enum ExprASTKind {
        Expr_VarDecl,
        Expr_Return,
        Expr_Num,
        Expr_Literal,
        Expr_Var,
        Expr_BinOp,
        Expr_Call,
        Expr_Print,
        Expr_Prototype,
    };

    ExprAST(Location Loc, ExprASTKind kind) : Loc(Loc), kind(kind) {}


    // Virtual destructor to ensure proper cleanup of derived class objects
    virtual ~ExprAST() {}

    const Location &loc() { return Loc; }

    ExprASTKind getKind() const { return kind; }

private:
    Location Loc;
    const ExprASTKind kind;
};

class VariableExprAST: public ExprAST {
    std::string Name;
public:
    VariableExprAST(Location Loc, std::string Name) : ExprAST(Loc, Expr_Var), Name(Name) {}

    static bool classof(const ExprAST *E) {
        return E->getKind() == Expr_Var;
    }
};

class PrototypeExprAST: public ExprAST {
    std::string Name;
    std::vector<VariableExprAST> Args;
public:
    PrototypeExprAST(Location Loc, std::string Name, std::vector<VariableExprAST> Args)
        : ExprAST(Loc, Expr_Prototype), Name(Name), Args(Args) {}

    static bool classof(const ExprAST *E) {
        return E->getKind() == Expr_Prototype;
    }
};

class VarDeclExprAST: public ExprAST {
    std::string Name;
    VarType Type;
    std::unique_ptr<ExprAST> Expr;
public:
    VarDeclExprAST(Location Loc, std::string Name, VarType Type, std::unique_ptr<ExprAST> Expr)
        : ExprAST(Loc, Expr_VarDecl), Name(Name), Type(Type), Expr(std::move(Expr)) {}
    
    static bool classof(const ExprAST *E) {
        return E->getKind() == Expr_VarDecl;
    }
};

class ReturnExprAST: public ExprAST {
    std::unique_ptr<ExprAST> Value;
public:
    ReturnExprAST(Location Loc, std::unique_ptr<ExprAST> Value)
    : ExprAST(Loc, Expr_Return), Value(std::move(Value)) {}

    static bool classof(const ExprAST *E) {
        return E->getKind() == Expr_Return;
    }
};

class LiteralExprAST: public ExprAST {
    double Val;
public:
    LiteralExprAST(Location Loc, double Val)
        : ExprAST(Loc, Expr_Literal), Val(Val) {}

    static bool classof(const ExprAST *E) {
        return E->getKind() == Expr_Literal;
    } 
};

class BinOpExprAST: public ExprAST {
    char Op;
    std::unique_ptr<ExprAST> LHS, RHS;
public:
    BinOpExprAST(Location Loc, char Op, std::unique_ptr<ExprAST> LHS, std::unique_ptr<ExprAST> RHS)
        : ExprAST(Loc, Expr_BinOp), Op(Op), LHS(std::move(LHS)), RHS(std::move(RHS)) {}

    static bool classof(const ExprAST *E) {
        return E->getKind() == Expr_BinOp;
    }
};

class CallExprAST: public ExprAST {
    std::string Callee;
    std::vector<std::unique_ptr<ExprAST>> Args;
public:
    CallExprAST(Location Loc, const std::string &Callee, std::vector<std::unique_ptr<ExprAST>> Args)
        : ExprAST(Loc, Expr_Call), Callee(Callee), Args(std::move(Args)) {}

    static bool classof(const ExprAST *E) {
        return E->getKind() == Expr_Call;
    }
};

class ExprASTList {
    std::vector<std::unique_ptr<ExprAST>> Exprs;
public:
    ExprASTList(std::vector<std::unique_ptr<ExprAST>> Exprs) : Exprs(std::move(Exprs)) {}
};

// Expression class for numeric literals like "1.0".
class FunctionExprAST {
    Location Loc;
    std::unique_ptr<PrototypeExprAST> Proto;
    std::unique_ptr<ExprASTList> Block;

public:
    FunctionExprAST(Location Loc, std::unique_ptr<PrototypeExprAST> Proto, std::unique_ptr<ExprASTList> Block)
        : Loc(Loc), Proto(std::move(Proto)), Block(std::move(Block)) {}

};

class ModuleAST {
    std::vector<FunctionExprAST> Functions;
public:
    // Use rvalue reference to move the vector
    ModuleAST(std::vector<FunctionExprAST> Functions)
        : Functions(std::move(Functions)) {}  // Move the vector
};
};

#endif // AST_HPP
