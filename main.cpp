#include <iostream>
#include <fstream>
#include "ast.hpp"
#include "semantic_visitor.hpp"
#include "ast_print.cpp"
#include "codegen_visitor.hpp"

// Deklaracja parsera:
int yyparse();
extern ASTNode* g_root; // globalny wskaźnik do AST
extern FILE* yyin;

int main(int argc, char** argv) {
    if (argc < 3) {
        std::cerr << "Użycie: " << argv[0] << " <plik_zrodlowy> <plik_wyjsciowy>\n";
        return 1;
    }
    
    // Otwieramy plik wejściowy
    FILE* f = fopen(argv[1], "r");
    if (!f) {
        std::cerr << "Nie można otworzyć pliku " << argv[1] << "\n";
        return 2;
    }
    yyin = f;
    
    // Parsowanie
    int parseRes = yyparse();
    fclose(f);
    
    if (parseRes != 0) {
        std::cerr << "Błąd parsowania!\n";
        return 3;
    }
    if (!g_root) {
        std::cerr << "AST nie powstało\n";
        return 4;
    }
    
    // Wypisujemy AST (opcjonalnie)
    // printAST(g_root);
    
    std::cout << "Parsowanie OK. Teraz analiza semantyczna...\n";
    
    // Analiza semantyczna
    SemanticVisitor visitor;
    visitor.analyze(g_root);
    
    if (visitor.symTab.errors > 0) {
        std::cerr << "Wykryto " << visitor.symTab.errors 
                  << " błędów semantycznych.\n";
        return 5;
    }
    
    std::cout << "Analiza semantyczna OK. Możemy generować kod.\n";
    
    // Generacja kodu
    SymbolTable& symTab = visitor.symTab;
    CodeGenVisitor codeGen(symTab);
    g_root->accept(codeGen);
    std::string finalCode = codeGen.getCode();
    
    // Zapisujemy kod do pliku wyjściowego (podanego jako argv[2])
    std::ofstream outFile(argv[2]);
    if (outFile.is_open()) {
        outFile << finalCode;
        outFile.close();
        std::cout << "Kod został zapisany do " << argv[2] << std::endl;
    } else {
        std::cerr << "Błąd: Nie można otworzyć pliku do zapisu." << std::endl;
        return 6;
    }
    
    return 0;
}
