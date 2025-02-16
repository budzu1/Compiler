# Compiler
Developed a compiler for a custom imperative language created by my lecturer, utilizing FLEX for lexical analysis and BISON for parsing. This university project involved implementing syntax and semantic checks, and generating intermediate code. Still needs optimalization.

Tomasz Budzyński

main.cpp: zawiera funkcję main() i główną logikę uruchamiania kompilatora.
parser.y: plik gramatyki Bison, definiujący parser.
lexer.l: plik Flex, definiujący analizator leksykalny.
ast.hpp: definicje abstrakcyjnego drzewa składniowego (AST).
semantic_visitor.cpp/hpp: implementacja analizy semantycznej.
codegen_visitor.cpp/hpp: implementacja generowania kodu.
symtable.hpp: definicje tabeli symboli.
memory_manager.hpp: zarządzanie pamięcią wykorzystywane przez generowanie kodu.
Makefile: plik budowania projektu.

g++ (Ubuntu 13.3.0-6ubuntu2~24.04) 13.3.0
bison (GNU Bison) 3.8.2
flex 2.6.4

Moja implementacja dzielenia (/) działa jak opertor // w pythonie, moje modulo (%) działa jak % w pythonie.
Np. -5/2 = -3, -5%2 = 1
