#ifndef AST_HPP
#define AST_HPP

#include <string>
#include <vector>
#include <cstdint>
#include <iostream>

// Forward declaration interfejsu
class ASTVisitor;

//////////////////////////////////////
// Bazowa klasa węzłów AST
//////////////////////////////////////
class ASTNode {
protected:
    uint64_t line; // nr linii w kodzie
public:
    ASTNode(uint64_t ln) : line(ln) {}
    virtual ~ASTNode() = default;

    uint64_t getLine() const { return line; }

    // Każdy węzeł zdefiniuje accept, który woła visitor.visit(*this)
    virtual void accept(ASTVisitor& visitor) = 0;
};

//////////////////////////////////////
// Deklaracja enumeracji CommandKind
//////////////////////////////////////
enum class CommandKind {
    ASSIGN, IF_THEN, IF_THEN_ELSE, WHILE, REPEAT_UNTIL,
    FOR_UP, FOR_DOWN, PROC_CALL, READ, WRITE
};

//////////////////////////////////////
// Deklaracja enumeracji ExprKind
//////////////////////////////////////
enum class ExprKind {
    BINOP,
    UNOP,
    LITERAL,
    IDENT,
};

//////////////////////////////////////
// Interfejs ASTVisitor
//////////////////////////////////////
class ProgramAllNode;
class ProceduresNode;
class ProcHeadNode;
class ProcedureDeclNode;
class MainNode;
class DeclarationsNode;
class DeclarationVarNode;
class DeclarationArrNode;
class CommandsNode;
class CommandNode;
class ArgsDeclNode;
class ArgsNode;
class ProcCallNode;
class ExpressionNode;
class ValueNode;
class IdentifierNode;

class ASTVisitor {
public:
    virtual ~ASTVisitor() = default;

    virtual void visit(ProgramAllNode&) = 0;
    virtual void visit(ProceduresNode&) = 0;
    virtual void visit(ProcHeadNode&) = 0;
    virtual void visit(ProcedureDeclNode&) = 0;
    virtual void visit(MainNode&) = 0;
    virtual void visit(DeclarationsNode&) = 0;
    virtual void visit(DeclarationVarNode&) = 0;
    virtual void visit(DeclarationArrNode&) = 0;
    virtual void visit(CommandsNode&) = 0;
    virtual void visit(CommandNode&) = 0;
    virtual void visit(ArgsDeclNode&) = 0;
    virtual void visit(ArgsNode&) = 0;
    virtual void visit(ProcCallNode&) = 0;
    virtual void visit(ExpressionNode&) = 0;
    virtual void visit(ValueNode&) = 0;
    virtual void visit(IdentifierNode&) = 0;
};

// Każdy węzeł AST implementuje accept(...)
//////////////////////////////////////
// ProgramAllNode
//////////////////////////////////////
class ProgramAllNode : public ASTNode {
public:
    ASTNode* procedures;  // moze byc nullptr
    ASTNode* mainPart;    // moze byc nullptr

    ProgramAllNode(uint64_t ln, ASTNode* procs, ASTNode* mainN)
        : ASTNode(ln), procedures(procs), mainPart(mainN) {}
    ~ProgramAllNode() override {
        delete procedures;
        delete mainPart;
    }
    void accept(ASTVisitor& visitor) override {
        visitor.visit(*this);
    }
};

//////////////////////////////////////
// ProceduresNode
//////////////////////////////////////
class ProceduresNode : public ASTNode {
public:
    std::vector<ASTNode*> procedureDecls;

    ProceduresNode(uint64_t ln)
        : ASTNode(ln) {}
    ~ProceduresNode() override {
        for (auto* p : procedureDecls) delete p;
    }
    void accept(ASTVisitor& visitor) override {
        visitor.visit(*this);
    }
};

//////////////////////////////////////
// ProcHeadNode
//////////////////////////////////////
class ProcHeadNode : public ASTNode {
public:
    std::string procName;
    ASTNode* argsDecl; // np. ArgsDeclNode

    ProcHeadNode(uint64_t ln, const std::string& name, ASTNode* args)
        : ASTNode(ln), procName(name), argsDecl(args) {}
    ~ProcHeadNode() override {
        delete argsDecl;
    }
    void accept(ASTVisitor& visitor) override {
        visitor.visit(*this);
    }
};

//////////////////////////////////////
// ProcedureDeclNode
//////////////////////////////////////
class ProcedureDeclNode : public ASTNode {
public:
    std::string procName;
    ASTNode* argsDecl;   // ArgsDeclNode
    ASTNode* localDecls; // DeclarationsNode
    ASTNode* commands;   // CommandsNode

    ProcedureDeclNode(uint64_t ln,
                      const std::string &pName,
                      ASTNode* args,
                      ASTNode* decls,
                      ASTNode* cmds)
        : ASTNode(ln), procName(pName),
          argsDecl(args), localDecls(decls), commands(cmds) {}

    ~ProcedureDeclNode() override {
        delete argsDecl;
        delete localDecls;
        delete commands;
    }
    void accept(ASTVisitor& visitor) override {
        visitor.visit(*this);
    }
};

//////////////////////////////////////
// MainNode
//////////////////////////////////////
class MainNode : public ASTNode {
public:
    ASTNode* declarations;  // DeclarationsNode
    ASTNode* commands;      // CommandsNode

    MainNode(uint64_t ln, ASTNode* decls, ASTNode* cmds)
        : ASTNode(ln), declarations(decls), commands(cmds) {}

    ~MainNode() override {
        delete declarations;
        delete commands;
    }
    void accept(ASTVisitor& visitor) override {
        visitor.visit(*this);
    }
};

//////////////////////////////////////
// DeclarationsNode
//////////////////////////////////////
class DeclarationsNode : public ASTNode {
public:
    std::vector<ASTNode*> declList; // DeclarationVarNode lub DeclarationArrNode

    DeclarationsNode(uint64_t ln)
        : ASTNode(ln) {}
    ~DeclarationsNode() override {
        for (auto* d : declList) delete d;
    }
    void accept(ASTVisitor& visitor) override {
        visitor.visit(*this);
    }
};

//////////////////////////////////////
// DeclarationVarNode
//////////////////////////////////////
class DeclarationVarNode : public ASTNode {
public:
    std::string varName;

    DeclarationVarNode(uint64_t ln, const std::string &nm)
        : ASTNode(ln), varName(nm) {}

    void accept(ASTVisitor& visitor) override {
        visitor.visit(*this);
    }
};

//////////////////////////////////////
// DeclarationArrNode
//////////////////////////////////////
class DeclarationArrNode : public ASTNode {
public:
    std::string arrName;
    long long lowerBound;
    long long upperBound;

    DeclarationArrNode(uint64_t ln,
                       const std::string &nm,
                       long long lb, long long ub)
        : ASTNode(ln), arrName(nm), lowerBound(lb), upperBound(ub) {}

    void accept(ASTVisitor& visitor) override {
        visitor.visit(*this);
    }
};

//////////////////////////////////////
// CommandsNode
//////////////////////////////////////
class CommandsNode : public ASTNode {
public:
    std::vector<ASTNode*> cmdList; // CommandNode

    CommandsNode(uint64_t ln)
        : ASTNode(ln) {}
    ~CommandsNode() override {
        for (auto* c : cmdList) delete c;
    }
    void accept(ASTVisitor& visitor) override {
        visitor.visit(*this);
    }
};

//////////////////////////////////////
// CommandNode
//////////////////////////////////////
class CommandNode : public ASTNode {
public:
    CommandKind cmdKind;
    std::vector<ASTNode*> children; // np. [ IdentifierNode, ExpressionNode ]

    CommandNode(uint64_t ln, CommandKind ck)
        : ASTNode(ln), cmdKind(ck) {}
    ~CommandNode() override {
        for (auto* c : children) delete c;
    }
    void accept(ASTVisitor& visitor) override {
        visitor.visit(*this);
    }
};

//////////////////////////////////////
// ArgsDeclNode
//////////////////////////////////////
class ArgsDeclNode : public ASTNode {
public:
    std::vector<std::string> argNames;
    std::vector<bool> isArray; // param jest tablicowy (T) lub nie

    ArgsDeclNode(uint64_t ln)
        : ASTNode(ln) {}
    void accept(ASTVisitor& visitor) override {
        visitor.visit(*this);
    }
};

//////////////////////////////////////
// ArgsNode
//////////////////////////////////////
class ArgsNode : public ASTNode {
public:
    std::vector<std::string> varNames; // nazwy przekazywanych parametrów

    ArgsNode(uint64_t ln)
        : ASTNode(ln) {}
    void accept(ASTVisitor& visitor) override {
        visitor.visit(*this);
    }
};

//////////////////////////////////////
// ProcCallNode
//////////////////////////////////////
class ProcCallNode : public ASTNode {
public:
    std::string procName;
    ASTNode* args; // np. ArgsNode

    ProcCallNode(uint64_t ln, const std::string& nm, ASTNode* a)
        : ASTNode(ln), procName(nm), args(a) {}

    // Konstruktor, gdy brak wskaźnika do args
    ProcCallNode(uint64_t ln, const std::string& nm)
        : ASTNode(ln), procName(nm), args(nullptr) {}

    ~ProcCallNode() override {
        delete args;
    }
    void accept(ASTVisitor& visitor) override {
        visitor.visit(*this);
    }
};

//////////////////////////////////////
// ExpressionNode
//////////////////////////////////////
class ExpressionNode : public ASTNode {
public:
    ExprKind exprKind;
    std::string op;   // np. "+", "-", "/", "=="
    ASTNode* left;
    ASTNode* right;

    ExpressionNode(uint64_t ln,
                   const std::string& oper,
                   ASTNode* l,
                   ASTNode* r)
        : ASTNode(ln),
          exprKind(ExprKind::BINOP),
          op(oper), left(l), right(r) {}

    ~ExpressionNode() override {
        delete left;
        delete right;
    }

    void accept(ASTVisitor& visitor) override {
        visitor.visit(*this);
    }
};

//////////////////////////////////////
// ValueNode
//////////////////////////////////////
class ValueNode : public ASTNode {
public:
    long long val;

    ValueNode(uint64_t ln, long long v)
        : ASTNode(ln), val(v) {}

    void accept(ASTVisitor& visitor) override {
        visitor.visit(*this);
    }
};

//////////////////////////////////////
// IdentifierNode
//////////////////////////////////////
class IdentifierNode : public ASTNode {
public:
    std::string name;
    ASTNode* indexExpr; // nullptr, jeśli to zwykła zmienna

    IdentifierNode(uint64_t ln, const std::string &nm, ASTNode* idx)
        : ASTNode(ln), name(nm), indexExpr(idx) {}

    ~IdentifierNode() override {
        delete indexExpr;
    }

    void accept(ASTVisitor& visitor) override {
        visitor.visit(*this);
    }
};

#endif // AST_HPP
