#ifndef SEMANTIC_VISITOR_HPP
#define SEMANTIC_VISITOR_HPP

#include "ast.hpp"
#include "symtable.hpp"
#include <string>

// Klasa SemanticVisitor dziedziczy po ASTVisitor
class SemanticVisitor : public ASTVisitor {
public:
    SymbolTable symTab;

    std::vector<SymbolKind> tmpParamKinds;
    bool collectingParams = false;


    std::string currentProcedure;


    std::string currentForIterator;
    bool inFor = false; // flaga, czy jesteśmy w pętli for



    // Metoda startowa
    void analyze(ASTNode* root);

    // Implementacje wizyt:
    void visit(ProgramAllNode&) override;
    void visit(ProceduresNode&) override;
    void visit(ProcHeadNode&) override;
    void visit(ProcedureDeclNode&) override;
    void visit(MainNode&) override;
    void visit(DeclarationsNode&) override;
    void visit(DeclarationVarNode&) override;
    void visit(DeclarationArrNode&) override;
    void visit(CommandsNode&) override;
    void visit(CommandNode&) override;
    void visit(ArgsDeclNode&) override;
    void visit(ArgsNode&) override;
    void visit(ProcCallNode&) override;
    void visit(ExpressionNode&) override;
    void visit(ValueNode&) override;
    void visit(IdentifierNode&) override;

private:
    // Metody pomocnicze:
    void visitNode(ASTNode* node);
    void checkExpression(ASTNode* node);
};

#endif // SEMANTIC_VISITOR_HPP
