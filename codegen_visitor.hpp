#ifndef CODEGEN_VISITOR_HPP
#define CODEGEN_VISITOR_HPP

#include "ast.hpp"
#include "symtable.hpp"
#include "memory_manager.hpp"

#include <vector>
#include <string>
#include <cstdint>

class CodeGenVisitor : public ASTVisitor {
public:
    SymbolTable &symTab;       ///< Twoja tablica symboli z polami: kind, addr, lowerBound, upperBound ...
    MemoryManager memmgr;      ///< menedżer pamięci (przydział/zwolnienie)
    std::string currentProcedure;

    long long lineCounter = 1; ///< licznik linii kodu

    std::vector<std::string> instructions; ///< finalny kod maszynowy (w wierszach)

    // Konstruktor:
    CodeGenVisitor(SymbolTable &st)
      : symTab(st), lineCounter(1), currentProcedure("")
    {std::cout << memmgr.getNextAddress() << " start constr value" << std::endl;}

    // Zwraca finalny kod w formie jednego stringa
    std::string getCode();

    // Dodajemy jedną linię kodu
    void emit(const std::string &cmd);
    void insertFirstJump();

    // ========== Metody alokacji / zwalniania =============
    long long allocateTemp();
    void freeTemp(long long addr);

    // ========== Metody pomocnicze do generowania logarytmicznej arytmetyki =============
    void genMultiply(long long memY);        // p0 *= memY => p0
    void genDivision(long long memY, bool doMod); // p0 = p0 / memY lub p0 = p0 % memY

    // ========== Obsługa tablic (dynamiczny offset) =========
    void genArrOffset(long long base, long long lb, bool ifParam); // w p0 index => p0= base + (p0-lb)

    // ========== Naprawianie relatywnych skoków =============
    // Typowy schemat: generujemy "JZERO ???", zapamiętujemy pos, ...
    void fixupJump(size_t instrPos, long long offset);

    std::string retLoad(long long addr, bool param);
    std::string retStore(long long addr, bool param);
    std::string retAdd(long long addr, bool param);
    std::string retSub(long long addr, bool param);

    // ========== Metody odwiedzające AST (pełna obsługa) =============
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
    // Funkcja do pobrania SymbolInfo:
    SymbolInfo* getSymbol(const std::string &name);

    // Pomocnicza do generowania unikalnych etykiet:
    std::string makeLabel(const std::string &prefix);

    long long labelCounter = 0;
};

#endif
