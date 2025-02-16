#include "semantic_visitor.hpp"
#include <iostream>
#include <cassert>

// Metoda startowa
void SemanticVisitor::analyze(ASTNode* root) {
    if (!root) return;
    root->accept(*this); // wywołanie visit(...) w ramach visitora
}

// Pomocnicza metoda
void SemanticVisitor::visitNode(ASTNode* node) {
    if (node) {
        node->accept(*this);
    }
}

// Pomocnicza metoda do odwiedzania wyrażeń
void SemanticVisitor::checkExpression(ASTNode* node) {
    if (node) {
        node->accept(*this); // to wywoła visit(ExpressionNode|ValueNode|IdentifierNode)
    }
}

//////////////////////////////
// Implementacje wizyt:
//////////////////////////////

void SemanticVisitor::visit(ProgramAllNode &node) {
    // Odwiedzamy listę procedur i część main
    visitNode(node.procedures);
    visitNode(node.mainPart);
}

void SemanticVisitor::visit(ProceduresNode &node) {
    for (auto* p : node.procedureDecls) {
        visitNode(p);
    }
}

void SemanticVisitor::visit(ProcHeadNode &node) {
    // Rzadko kiedy Parser zachowuje ProcHeadNode w drzewie, ale jeśli tak - odwiedź argsDecl
    visitNode(node.argsDecl);
}

void SemanticVisitor::visit(ProcedureDeclNode &node) {
    // 1. Sprawdź, czy procedura już istnieje w globalScope
    SymbolInfo* si = symTab.lookup(node.procName, "");
    if (si && si->kind == SymbolKind::PROC) {
        symTab.reportError("Procedura \"" + node.procName + "\" już zadeklarowana", node.getLine());
    } else if (si && si->kind != SymbolKind::PROC) {
        symTab.reportError("Symbol o nazwie \"" + node.procName + "\" nie jest procedurą", node.getLine());
    } else {
        // Dodajemy do globalScope informację o nowej procedurze
        SymbolInfo info;
        info.kind = SymbolKind::PROC;
        info.name = node.procName;
        info.ownerProcName = ""; // bo procedura jest zapisywana w globalScope
        symTab.addGlobalSymbol(info, node.getLine());
        // Pobieramy wskaźnik, żeby uzupełnić paramKinds itp.
        si = symTab.lookup(node.procName, "");
    }

    if (!si) {
        return; // przerwanie w razie braku
    }

    // 2. Chronimy przed rekurencją
    std::string oldProc = currentProcedure;
    currentProcedure = node.procName;

    // 3. (Nie czyścimy localScope!)
    // symTab.clearLocalScope();  // USUWAMY

    // 4. Zbieranie paramKinds
    collectingParams = true;
    tmpParamKinds.clear();

    // 5. Odwiedzamy definicję parametrów (ArgsDeclNode)
    if (node.argsDecl) {
        node.argsDecl->accept(*this);
    }
    collectingParams = false;

    // 6. Zapisujemy paramKinds do symbolu procedury (np. [VAR, VAR] itp.)
    si->paramKinds = tmpParamKinds;

    // 7. Deklaracje lokalne
    visitNode(node.localDecls);

    // 8. Ciało
    visitNode(node.commands);

    // 9. Przywrócenie
    currentProcedure = oldProc;
}




void SemanticVisitor::visit(MainNode &node) {
    // Deklaracje globalne, potem komendy
    visitNode(node.declarations);
    visitNode(node.commands);
}

void SemanticVisitor::visit(DeclarationsNode &node) {
    // Każdy element to DeclarationVarNode albo DeclarationArrNode
    for (auto* d : node.declList) {
        visitNode(d);
    }
}

void SemanticVisitor::visit(DeclarationVarNode &node) {
    SymbolInfo info;
    info.kind = SymbolKind::VAR;
    info.name = node.varName;
    info.initialized = false;  // domyślnie

    // NOWE: ustaw ownerProcName
    if (currentProcedure.empty()) {
        info.ownerProcName = ""; 
    } else {
        info.ownerProcName = currentProcedure;
    }

    if (currentProcedure.empty()) {
        // global
        symTab.addGlobalSymbol(info, node.getLine());
    } else {
        // lokal
        symTab.addLocalSymbol(info, node.getLine());
    }
}


void SemanticVisitor::visit(DeclarationArrNode &node) {
    if (node.lowerBound > node.upperBound) {
        symTab.reportError("Deklaracja tablicy ...", node.getLine());
    }

    SymbolInfo info;
    info.kind = SymbolKind::ARR;
    info.name = node.arrName;
    info.lowerBound = node.lowerBound;
    info.upperBound = node.upperBound;
    info.initialized = false;

    // NOWE: ustaw ownerProcName
    if (currentProcedure.empty()) {
        info.ownerProcName = "";
    } else {
        info.ownerProcName = currentProcedure;
    }

    if (currentProcedure.empty()) {
        symTab.addGlobalSymbol(info, node.getLine());
    } else {
        symTab.addLocalSymbol(info, node.getLine());
    }
}


void SemanticVisitor::visit(CommandsNode &node) {
    for (auto* c : node.cmdList) {
        visitNode(c);
    }
}

void SemanticVisitor::visit(CommandNode &node) {
    switch (node.cmdKind) {

        case CommandKind::ASSIGN: {
            // node.children[0] = IdentifierNode (lewa strona)
            // node.children[1] = expression (prawa strona)
            if (node.children.size() >= 2) {
                // 1) Odwiedzamy prawe wyrażenie
                visitNode(node.children[1]);
                // 2) Sprawdzamy/lewy ident
                auto* leftId = dynamic_cast<IdentifierNode*>(node.children[0]);
                if (leftId) {
                    SymbolInfo* si = symTab.lookup(leftId->name, currentProcedure);
                    if (!si) {
                        symTab.reportError(
                            "Przypisanie do niezadeklarowanego identyfikatora: " 
                            + leftId->name,
                            leftId->getLine());
                    } else {
                        // Sprawdzamy, czy to iterator FOR
                        if (si->isForIterator) {
                            symTab.reportError("Modyfikacja iteratora pętli FOR: " 
                                               + leftId->name,
                                               leftId->getLine());
                        }
                        // Oznaczamy zmienną/ tablicę za zainicjalizowaną
                        si->initialized = true;
                    }
                }
                // 3) Na koniec odwiedzamy lewy ident 
                visitNode(node.children[0]);
            }
            break;
        }

        case CommandKind::PROC_CALL: {
            // node.children[0] = ProcCallNode
            if (!node.children.empty()) {
                auto* pc = dynamic_cast<ProcCallNode*>(node.children[0]);
                if (pc) pc->accept(*this);
            }
            break;
        }

        case CommandKind::READ: {
            // node.children[0] = IdentifierNode
            if (!node.children.empty()) {
                auto* idn = dynamic_cast<IdentifierNode*>(node.children[0]);
                if (idn) {
                    SymbolInfo* si = symTab.lookup(idn->name, currentProcedure);
                    if (!si) {
                        symTab.reportError("READ niezadeklarowanego identyfikatora: " 
                                           + idn->name,
                                           node.getLine());
                    } else {
                        // Odczyt do ident -> inicjalizujemy
                        si->initialized = true;
                    }
                }
                // odwiedzamy ident, by sprawdzić ARR vs VAR
                visitNode(node.children[0]);
            }
            break;
        }

        case CommandKind::WRITE: {
            // node.children[0] = wartość / ident / expression
            if (!node.children.empty()) {
                visitNode(node.children[0]);
            }
            break;
        }

        case CommandKind::FOR_UP: {
            // node.children: [0] = iterator, [1]= from, [2]= to, [3]= body
            if (node.children.size() == 4) {
                auto* iter = dynamic_cast<IdentifierNode*>(node.children[0]);
                if (iter) {
                    SymbolInfo info;
                    info.kind = SymbolKind::VAR;
                    info.name = iter->name;
                    info.isForIterator = true;
                    info.initialized   = true;
                    info.ownerProcName = currentProcedure;
                    // Dodajemy do currentScope
                    symTab.addLocalSymbol(info, iter->getLine());

                    currentForIterator = iter->name;
                    inFor = true;
                }
                // odwiedzamy from, to, body
                visitNode(node.children[1]);
                visitNode(node.children[2]);
                visitNode(node.children[3]); // ciało pętli

                // usunięcie iteratora 
                if (iter) {
                    symTab.removeLocalSymbol(iter->name, currentProcedure);
                }
                currentForIterator.clear();
                inFor = false;
            }
            break;
        }
        case CommandKind::FOR_DOWN: {
            // analogicznie
            if (node.children.size() == 4) {
                auto* iter = dynamic_cast<IdentifierNode*>(node.children[0]);
                if (iter) {
                    SymbolInfo info;
                    info.kind = SymbolKind::VAR;
                    info.name = iter->name;
                    info.isForIterator = true;
                    info.initialized   = true;
                    info.ownerProcName = currentProcedure;
                    
                    symTab.addLocalSymbol(info, iter->getLine());

                    currentForIterator = iter->name;
                    inFor = true;
                }
                visitNode(node.children[1]);
                visitNode(node.children[2]);
                visitNode(node.children[3]);

                if (iter) {
                    symTab.removeLocalSymbol(iter->name, currentProcedure);
                }
                currentForIterator.clear();
                inFor = false;
            }
            break;
        }

        case CommandKind::IF_THEN: {
            // children: [0] = condition, [1] = then-commands
            if (node.children.size() == 2) {
                visitNode(node.children[0]); // condition
                visitNode(node.children[1]); // then commands
            }
            break;
        }

        case CommandKind::IF_THEN_ELSE: {
            // children: [0] = condition, [1] = then-commands, [2] = else-commands
            if (node.children.size() == 3) {
                visitNode(node.children[0]); 
                visitNode(node.children[1]); 
                visitNode(node.children[2]);
            }
            break;
        }

        case CommandKind::WHILE: {
            // children: [0] = condition, [1] = body
            if (node.children.size() == 2) {
                visitNode(node.children[0]); // condition
                visitNode(node.children[1]); // body
            }
            break;
        }

        case CommandKind::REPEAT_UNTIL: {
            // children: [0] = body (commands), [1] = condition
            if (node.children.size() == 2) {
                visitNode(node.children[0]); // body
                visitNode(node.children[1]); // condition
            }
            break;
        }

        default: {
            // na wypadek, gdyby w grammarze były inne commandy
            // odwiedzamy węzły children
            for (auto* c : node.children) {
                visitNode(c);
            }
            break;
        }
    }
}


void SemanticVisitor::visit(ArgsDeclNode &node) {
    // Dla każdego parametru formalnego
    for (size_t i = 0; i < node.argNames.size(); i++) {
        SymbolInfo info;
        info.name = node.argNames[i];
        // Jeśli parametr jest tablicą, ustawiamy typ na ARR i flagę ifParam na true.
        if (node.isArray[i]) {
            info.kind = SymbolKind::ARR;
            info.ifParam = true; // tablica przekazywana przez referencję
        } else {
            info.kind = SymbolKind::VAR;
            info.ifParam = false; // zwykła zmienna – przekazywana przez wartość
        }
        // Parametr formalny jest inicjalizowany (przekazywany z wywołania procedury)
        info.initialized = true;
        // Ustawiamy właściciela (nazwę procedury) – dla parametrów formalnych zawsze currentProcedure
        info.ownerProcName = currentProcedure;
        // Dodajemy symbol do lokalnego scope procedury
        symTab.addLocalSymbol(info, node.getLine());
        // Jeśli zbieramy typy parametrów (np. do sprawdzenia liczby i typów przy wywołaniach procedury)
        if (collectingParams) {
            tmpParamKinds.push_back(info.kind);
        }
    }
}



void SemanticVisitor::visit(ArgsNode &node) {
    // W parserze: varNames = {"n", "k", "w"}, itp.
    // Nic tu nie deklarujemy, ewentualnie sprawdzamy, czy zadeklarowane?
    // Ale tu mamy same stringi, a nie IdentifierNode.
}

void SemanticVisitor::visit(ProcCallNode &node) {
    // 1. Znajdź symbol procedury
    SymbolInfo* si = symTab.lookup(node.procName, "");
    if (!si || si->kind != SymbolKind::PROC) {
        symTab.reportError("Wywołanie nieznanej procedury: " + node.procName, node.getLine());
    } else {
        // 2. Sprawdź paramKinds vs node.args (jeśli parser daje varNames)
        auto* an = dynamic_cast<ArgsNode*>(node.args);
        if (an) {
            size_t expectedCount = si->paramKinds.size();
            size_t givenCount = an->varNames.size();
            if (expectedCount != givenCount) {
                symTab.reportError("Błędna liczba argumentów w wywołaniu procedury „" + node.procName + "”",
                                   node.getLine());
            } else {
                for (size_t i = 0; i < givenCount; i++) {
                    SymbolInfo* argSi = symTab.lookup(an->varNames[i], currentProcedure);
                    if (!argSi) {
                        symTab.reportError("Argument nr " + std::to_string(i+1) + 
                                           " („" + an->varNames[i] + "”) niezadeklarowany",
                                           node.getLine());
                    } else {
                        // sprawdzamy typ
                        SymbolKind expected = si->paramKinds[i];
                        if (argSi->kind != expected) {
                            symTab.reportError("Argument nr " + std::to_string(i+1) + 
                                               " w wywołaniu procedury „" + node.procName +
                                               "” ma zły typ (oczekiwano " +
                                               (expected==SymbolKind::ARR ? "ARR" : "VAR") + ")",
                                               node.getLine());
                        }
                        // param in-out => argument staje się zainicjalizowany
                        argSi->initialized = true;
                    }
                }
            }
        }
    }
    // 3. Sprawdzenie rekurencji
    if (!currentProcedure.empty() && node.procName == currentProcedure) {
        symTab.reportError("Wywołanie niezdefiniowanej procedury: " + node.procName, node.getLine());
    }

    // 4. Odwiedzamy node.args - w parserze jest to ArgsNode*, raczej nic
    visitNode(node.args);
}

void SemanticVisitor::visit(ExpressionNode &node) {
    // odwiedzamy lewy, prawy
    visitNode(node.left);
    visitNode(node.right);
}

void SemanticVisitor::visit(ValueNode &node) {
    // stała liczba - nic nie robimy
}

void SemanticVisitor::visit(IdentifierNode &node) {
    // Sprawdzamy symbol
    SymbolInfo* si = symTab.lookup(node.name, currentProcedure);
    if (!si) {
        symTab.reportError("Użycie niezadeklarowanego identyfikatora \"" + node.name + "\"", node.getLine());
        return;
    }
    else if (!si->initialized) {
        symTab.reportError("Użycie niezainicjalizowanej zmiennej/tablicy: " + node.name, node.getLine());
    }

    // Jeżeli indexExpr != nullptr => to musi być ARR
    if (node.indexExpr) {
        if (si->kind != SymbolKind::ARR) {
            symTab.reportError("Użycie indeksu przy obiekcie, który nie jest tablicą: " + node.name,
                               node.getLine());
        }
        // odwiedzamy indexExpr
        visitNode(node.indexExpr);
    } else {
        // brak indeksu => musi być VAR (jeśli jest ARR => błąd)
        if (si->kind == SymbolKind::ARR) {
            symTab.reportError("Użycie tablicy bez indeksu: " + node.name, node.getLine());
        }
    }
}
