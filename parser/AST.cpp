#include "toy/AST.hpp"

#include <iostream> 
#include <string>

using namespace toy;

namespace toy {

struct Indent {
    Indent(int &level) : level(level) { level++; }
    ~Indent() { level--; }
    int &level;
};

class ASTDumper {
public:
    void dump(ModuleAST *node);
private:
    void dump(FunctionExprAST *node);
    void dump(PrototypeExprAST *node);
    void dump(ExprAST *node);
    void dump(VariableExprAST *node);
    void dump(VarDeclExprAST *node);
    void dump(BinOpExprAST *node);
    void dump(CallExprAST *node);
    void dump(ExprASTList *node);
    void dump(ReturnExprAST *node);
    void dump(LiteralExprAST *node);
    void dump(NumberExprAST *node);

    void indent() {
        for (int i = 0; i < curIndent; i++) {
            std::cout << "  ";
        }
    }

    template <typename T>
    static std::string loc(T *node) {
        const Location &loc = node->loc();
        return " @" + *loc.Filename + " " + std::to_string(loc.Line) + ":" + std::to_string(loc.Column); 
    }

    int curIndent = 0;
};

};

#define INDENT() \
    Indent level_(curIndent);   \
    indent();               


void ASTDumper::dump(FunctionExprAST *node) {
    dump(node->getProto().get());
    dump(node->getBlock().get());
}

void ASTDumper::dump(PrototypeExprAST *node) {
    std::cout << "Prototype: " << node->getName();
    std::cout << loc(node) << std::endl;
    for (auto &arg : node->getArgs()) {
        dump(arg.get());
    }
}

void ASTDumper::dump(ExprASTList *node) {
    for (auto &expr : node->getExprs()) {
        std::cout << "Expr: " << std::endl;
        dump(expr.get());
    }
}

void ASTDumper::dump(VariableExprAST *node) {
    INDENT();
    std::cout << "Variable: " << node->getName() << loc(node) << std::endl;
}

void ASTDumper::dump(ExprAST *node) {
    switch(node->getKind()) {
        case ExprAST::Expr_Var:
            dump(static_cast<VariableExprAST *>(node));
            break;
        case ExprAST::Expr_BinOp:
            dump(static_cast<BinOpExprAST *>(node));
            break;
        case ExprAST::Expr_Call:
            dump(static_cast<CallExprAST *>(node));
            break;
        case ExprAST::Expr_VarDecl:
            dump(static_cast<VarDeclExprAST *>(node));
            break;
        case ExprAST::Expr_Return:
            dump(static_cast<ReturnExprAST *>(node));
            break;
        case ExprAST::Expr_Literal:
            dump(static_cast<LiteralExprAST *>(node));
            break;
        case ExprAST::Expr_Num:
            dump(static_cast<NumberExprAST *>(node));
            break;
        default:
            std::cerr << "Unknown AST node: " << node->getKind() << std::endl;
            break;
    }
}

void ASTDumper::dump(BinOpExprAST *node) {
    INDENT();
    std::cout << "BinOp: " << node->getOp() << loc(node) << std::endl;
    // INDENT();
    dump(node->getLHS().get());
    dump(node->getRHS().get());
}

void ASTDumper::dump(CallExprAST *node) {
    INDENT();
    std::cout << "Call: " << node->getCallee() << loc(node) << std::endl;
    for (auto &arg : node->getArgs()) {
        dump(arg.get());
    }
}

void ASTDumper::dump(ModuleAST *node) {
    for (auto &func : node->getFunctions()) {
        dump(func.get());
    }
}

void ASTDumper::dump(VarDeclExprAST *node) {
    INDENT();
    std::cout << "VarDecl: " << node->getName() << loc(node) << std::endl;
    dump(node->getExpr().get());
}

void ASTDumper::dump(ReturnExprAST *node) {
    INDENT();
    std::cout << "Return:" << loc(node) << std::endl;
    dump(node->getValue().get());
}

void ASTDumper::dump(LiteralExprAST *node) {
    INDENT();
    auto dims = node->getType();
    std::cout << "Literal: " << "<" << dims.shape[0] << "," << dims.shape[1] << ">" << loc(node) << std::endl;
    for (auto &value : node->getValues()) {
        dump(value.get());
    }
}

void ASTDumper::dump(NumberExprAST *node) {
    INDENT();
    std::cout << "Number: " << node->getVal() << loc(node) << std::endl;
}

namespace toy {

// Public API
void dump(ModuleAST &module) { ASTDumper().dump(&module); }

} // namespace toy