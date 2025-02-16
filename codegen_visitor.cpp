#include "codegen_visitor.hpp"
#include <sstream>
#include <iostream>
#include <cassert>
#include "memory_manager.hpp"

// ------------------ Podstawy ------------------

std::string CodeGenVisitor::getCode() {
    std::ostringstream os;
    for (auto &line : instructions) {
        os << line << "\n";
    }
    return os.str();
}

void CodeGenVisitor::emit(const std::string &cmd) {
    instructions.push_back(cmd);
    lineCounter++;
}

void CodeGenVisitor::insertFirstJump() {
    long long k = lineCounter;
    instructions.insert(instructions.begin(), "JUMP " + std::to_string(k));
}

long long CodeGenVisitor::allocateTemp() {
    return memmgr.allocate(1);
}
void CodeGenVisitor::freeTemp(long long addr) {
}

SymbolInfo* CodeGenVisitor::getSymbol(const std::string &name) {

    SymbolInfo* si = symTab.lookup(name, currentProcedure);
    if (!si) {
        std::cerr << "CODEGEN ERROR: no symbol " << name << std::endl;
    }
    return si;
}

void CodeGenVisitor::fixupJump(size_t instrPos, long long offset) {
    // Podmieniamy w instructions[instrPos] "???" na offset
    std::string old = instructions[instrPos];
    size_t sp = old.find(' ');
    if (sp==std::string::npos) return;
    std::string opcode = old.substr(0, sp);
    std::ostringstream oss;
    oss << opcode << " " << offset;
    instructions[instrPos] = oss.str();
}

std::string CodeGenVisitor::makeLabel(const std::string &prefix) {
    std::ostringstream oss;
    oss << prefix << "_" << labelCounter++;
    return oss.str();
}

// ------------------ Pomocnicze: tablice ------------------

// ------------------ genArrOffset - final ------------------
void CodeGenVisitor::genArrOffset(long long base, long long lb, bool paramFlag) {
    // Założenie: w rejestrze p0 mamy "index".
    // Chcemy uzyskać: p0 = (baza tablicy) + (index - lb).

    // 1) Zachowujemy "index" w temp
    long long tmpIdx = allocateTemp();
    emit("STORE " + std::to_string(tmpIdx));
    emit("SET " + std::to_string(base));
    emit("ADD " + std::to_string(tmpIdx));


    // posprzątaj
    freeTemp(tmpIdx);
}


// ------------------ Logarytmiczne mnożenie i dzielenie ------------------

void CodeGenVisitor::genMultiply(long long memY)
{
    // 1) p0=x, memY => y
    // 2) obsługa znaku
    // 3) while (y>0)
    long long tmpX = allocateTemp();
    long long tmpY = allocateTemp();
    long long tmpRes = allocateTemp();
    long long tmpSign = allocateTemp();

    // store x => tmpX
    emit("STORE " + std::to_string(tmpX));
    emit("SET 0");
    emit("STORE " + std::to_string(tmpSign));
    emit("SET 0");
    emit("STORE " + std::to_string(tmpRes));
    emit("LOAD " + std::to_string(memY));
    emit("STORE " + std::to_string(tmpY));
    //Sprawdzanie znaku Y
    emit("JZERO 53"); // na sam koniec !!!!!!!!!!!!!!!!
    emit("JNEG 4");
    emit("SET 1");
    emit("ADD " + std::to_string(tmpSign));
    emit("JUMP 6");
    emit("SET 0");
    emit("SUB " + std::to_string(tmpY));
    emit("STORE " + std::to_string(tmpY));
    emit("SET -1");
    emit("ADD " + std::to_string(tmpSign));
    emit("STORE " + std::to_string(tmpSign));
    // Sprawdzanie znaku X
    emit("LOAD " + std::to_string(tmpX));
    emit("JZERO 41"); // na sam koniec !!!!!!!!!!!!!!!!
    emit("JNEG 4");
    emit("SET 1");
    emit("ADD " + std::to_string(tmpSign));
    emit("JUMP 6");
    emit("SET 0");
    emit("SUB " + std::to_string(tmpX));
    emit("STORE " + std::to_string(tmpX));
    emit("SET -1");
    emit("ADD " + std::to_string(tmpSign));
    emit("STORE " + std::to_string(tmpSign));
    // pierwszy krok
    emit("LOAD " + std::to_string(tmpX));
    emit("STORE " + std::to_string(tmpRes));
    emit("SET -1");
    emit("ADD " + std::to_string(tmpY));
    emit("STORE " + std::to_string(tmpY));
    // petla
    emit("JZERO 19");
    emit("HALF");
    emit("ADD 0");
    emit("SUB " + std::to_string(tmpY));
    emit("JZERO 8");
    emit("LOAD " + std::to_string(tmpX));
    emit("ADD " + std::to_string(tmpRes));
    emit("STORE " + std::to_string(tmpRes));
    emit("SET -1");
    emit("ADD " + std::to_string(tmpY));
    emit("STORE " + std::to_string(tmpY));
    emit("JUMP -11");
    emit("LOAD " + std::to_string(tmpX));
    emit("ADD " + std::to_string(tmpX));
    emit("STORE " + std::to_string(tmpX));
    emit("LOAD " + std::to_string(tmpY));
    emit("HALF");
    emit("STORE " + std::to_string(tmpY));
    emit("JUMP -18");
    // koniec pętli
    emit("LOAD " + std::to_string(tmpSign));
    emit("JZERO 3");
    emit("LOAD " + std::to_string(tmpRes));
    emit("JUMP 3");
    emit("SET 0");
    emit("SUB " + std::to_string(tmpRes));
}

void CodeGenVisitor::genDivision(long long memY, bool doMod)
{
    long long tmpX = allocateTemp();
    long long tmpY = allocateTemp();
    long long res = allocateTemp();
    long long mod = allocateTemp();

    long long sign = allocateTemp();
    long long signY = allocateTemp();

    long long midRes = allocateTemp();
    long long divShift = allocateTemp();
    long long divCounter = allocateTemp();
    long long sumCount = allocateTemp();

    //Ładowanie zmiennych i sprawdzanie 
    emit("JZERO 116"); // ??????????????????????????????????????????
    emit("STORE " + std::to_string(tmpX));
    emit("LOAD " + std::to_string(memY));
    emit("STORE " + std::to_string(tmpY));
    emit("JZERO 112"); // ?????????????????????????????????????????????
    emit("JPOS 3");
    emit("SET -1");
    emit("JUMP 2");
    emit("SET 1");
    emit("STORE " + std::to_string(signY));
    emit("SET 0");
    emit("STORE " + std::to_string(sign));

    emit("LOAD " + std::to_string(tmpX));
    emit("JPOS 7");
    emit("SET 0");
    emit("SUB " + std::to_string(tmpX));
    emit("STORE " + std::to_string(tmpX));
    emit("SET -1");
    emit("ADD " + std::to_string(sign));
    emit("JUMP 3");
    emit("SET 1");
    emit("ADD " + std::to_string(sign));
    emit("STORE " + std::to_string(sign));

    emit("LOAD " + std::to_string(tmpY));
    emit("JPOS 7");
    emit("SET 0");
    emit("SUB " + std::to_string(tmpY));
    emit("STORE " + std::to_string(tmpY));
    emit("SET -1");
    emit("ADD " + std::to_string(sign));
    emit("JUMP 3");
    emit("SET 1");
    emit("ADD " + std::to_string(sign));
    emit("STORE " + std::to_string(sign));

    // Przygotowanie zmiennych

    emit("SET 0");
    emit("STORE " + std::to_string(res));
    emit("STORE " + std::to_string(sumCount));
    emit("STORE " + std::to_string(mod));

    emit("LOAD " + std::to_string(tmpY));
    emit("STORE " + std::to_string(divShift));
    emit("STORE " + std::to_string(midRes));

    emit("SET 1");
    emit("STORE " + std::to_string(divCounter));

    emit("LOAD " + std::to_string(midRes));
    emit("SUB " + std::to_string(tmpX));


    // Pętla 1.

    emit("JZERO 60"); // ?????
    emit("JPOS 17"); // ??????

    emit("LOAD " + std::to_string(midRes));
    emit("STORE " + std::to_string(res));
    emit("LOAD " + std::to_string(divCounter));
    emit("ADD " + std::to_string(sumCount));
    emit("STORE " + std::to_string(sumCount));

    emit("LOAD " + std::to_string(divCounter));
    emit("ADD " + std::to_string(divCounter));
    emit("STORE " + std::to_string(divCounter));

    emit("LOAD " + std::to_string(divShift));
    emit("ADD " + std::to_string(divShift));
    emit("STORE " + std::to_string(divShift));

    emit("LOAD " + std::to_string(res));
    emit("ADD " + std::to_string(divShift));
    emit("STORE " + std::to_string(midRes));

    emit("SUB " + std::to_string(tmpX));

    emit("JUMP -17");

    // Pętla 2.

    emit("LOAD " + std::to_string(divCounter));
    emit("JZERO 20"); // ??????
    emit("LOAD " + std::to_string(divShift));
    emit("ADD " + std::to_string(res));
    emit("STORE " + std::to_string(midRes));

    emit("SUB " + std::to_string(tmpX));

    emit("JZERO 36"); // ??????
    emit("JPOS 7"); // ??????

    emit("LOAD " + std::to_string(res));
    emit("ADD " + std::to_string(divShift));
    emit("STORE " + std::to_string(res));

    emit("LOAD " + std::to_string(sumCount));
    emit("ADD " + std::to_string(divCounter));
    emit("STORE " + std::to_string(sumCount));

    emit("LOAD " + std::to_string(divShift));
    emit("HALF");
    emit("STORE " + std::to_string(divShift));

    emit("LOAD " + std::to_string(divCounter));
    emit("HALF");
    emit("STORE " + std::to_string(divCounter));

    emit("JUMP -20");

    // Koniec

    emit("LOAD " + std::to_string(sign));
    emit("JZERO 5");
    emit("LOAD " + std::to_string(tmpX));
    emit("SUB " + std::to_string(res));
    emit("STORE " + std::to_string(mod));
    emit("JUMP 10");

    emit("LOAD " + std::to_string(tmpX));
    emit("SUB " + std::to_string(res));
    emit("STORE " + std::to_string(mod));
    emit("LOAD " + std::to_string(tmpY));
    emit("SUB " + std::to_string(mod));
    emit("STORE " + std::to_string(mod));
    emit("SET -1");
    emit("SUB " + std::to_string(sumCount));
    emit("STORE " + std::to_string(sumCount));

    emit("LOAD " + std::to_string(signY));
    emit("JPOS 4");
    emit("SET 0");
    emit("SUB " + std::to_string(mod));
    emit("STORE " + std::to_string(mod));

    emit("JUMP 10");
    emit("LOAD " + std::to_string(divCounter));
    emit("ADD " + std::to_string(sumCount));
    emit("STORE " + std::to_string(sumCount));
    emit("LOAD " + std::to_string(sign));
    emit("JZERO 2");
    emit("JUMP 4");
    emit("SET 0");
    emit("SUB " + std::to_string(sumCount));
    emit("STORE " + std::to_string(sumCount));

    if (doMod) {
        emit("LOAD " + std::to_string(mod));
        emit("STORE " + std::to_string(memY));
    } else {
        emit("LOAD " + std::to_string(sumCount));
        emit("STORE " + std::to_string(memY));
    }

    emit("JUMP 2"); // ?????
    emit("SET 0");
}


// ------------------ Wizytory AST ------------------

void CodeGenVisitor::visit(ProgramAllNode &node) {
    memmgr.memSetNextAddress(1);
    lineCounter = 1;
    if (node.procedures) node.procedures->accept(*this);
    insertFirstJump();
    if (node.mainPart) node.mainPart->accept(*this);
    emit("HALT");
}

void CodeGenVisitor::visit(ProceduresNode &node) {
    for (auto* prc : node.procedureDecls) {
        prc->accept(*this);
    }
}

void CodeGenVisitor::visit(ProcHeadNode &node) {
    // nic
}

void CodeGenVisitor::visit(ProcedureDeclNode &node) {
    // Zapamiętaj poprzednią nazwę
    std::string oldProc = currentProcedure;
    // Ustaw aktualną
    currentProcedure = node.procName;
    
    SymbolInfo* si = getSymbol(node.procName);
    si->returnAddr = memmgr.allocate(1);
    si->addr = lineCounter;

    if (node.argsDecl) node.argsDecl->accept(*this);
    if (node.localDecls) node.localDecls->accept(*this);
    auto* si2 = dynamic_cast<ArgsDeclNode*>(node.argsDecl);
    long long argsSize = (si2 ? si2->argNames.size() : 0);
    for (long long i=0; i<argsSize; i++) {
        SymbolInfo* si3 = getSymbol(si2->argNames[i]);
        si->paramAddrs.push_back(si3->addr);
    }
    if (node.commands)   node.commands->accept(*this);

    emit("RTRN " + std::to_string(si->returnAddr));

    // paramAddrs

    // Przywróć poprzednią procedurę
    currentProcedure = oldProc;
}


void CodeGenVisitor::visit(MainNode &node) {
    // zapamiętaj
    std::string oldProc = currentProcedure;
    currentProcedure = ""; // main

    if (node.declarations) node.declarations->accept(*this);
    if (node.commands)     node.commands->accept(*this);

    // przywróć
    currentProcedure = oldProc;
}


void CodeGenVisitor::visit(DeclarationsNode &node) {
    for (auto* d : node.declList) {
        d->accept(*this);
    }
}

void CodeGenVisitor::visit(DeclarationVarNode &node) {
    // Alokujemy 1 komórkę (nie 0!)
    SymbolInfo* si = getSymbol(node.varName);
    si->addr = memmgr.allocate(1); // start from 1,2,3...
}

void CodeGenVisitor::visit(DeclarationArrNode &node) {
    SymbolInfo* si = getSymbol(node.arrName);
    // Jeśli to jest param (si->ifParam == true) => rezerwujemy 1 komórkę,
    // a faktyczny "size" będzie w run-time. 
    // Ale dla zwykłej tablicy:
        long long size = (node.upperBound - node.lowerBound + 1);
        si->addr = memmgr.allocate(size);
        si->addr -= node.lowerBound; // przesunięcie
        // debug:

}

void CodeGenVisitor::visit(CommandsNode &node) {
    for (auto* c: node.cmdList) {
        c->accept(*this);
    }
}

// --------------
// Główne "command" w switch
// --------------

void CodeGenVisitor::visit(CommandNode &node) {
    switch(node.cmdKind) {

    case CommandKind::ASSIGN: {
        // [0]=IdentifierNode, [1]=expression
        node.children[1]->accept(*this); // oblicz expr => p0
        auto* idn = dynamic_cast<IdentifierNode*>(node.children[0]);
        SymbolInfo* si = getSymbol(idn->name);



        if (!idn->indexExpr) {
            // zwykła zmienna
            emit(retStore(si->addr, si->ifParam));
        } else {
            // tablica: arr[i] := p0
            long long temp = allocateTemp();
            emit("STORE " + std::to_string(temp));  // p0 => temp
            idn->indexExpr->accept(*this);  // oblicz index => p0
            if (si->ifParam) {
                long long temp2 = allocateTemp();
                emit("STORE " + std::to_string(temp2));  // p0 => temp2
                emit("LOAD " + std::to_string(si->addr));
                emit("ADD " + std::to_string(temp2));
                emit("STORE " + std::to_string(temp2));
                emit("LOAD " + std::to_string(temp));
                emit("STOREI " + std::to_string(temp2));
                freeTemp(temp2);
            }
            else {
                genArrOffset(si->addr, si->lowerBound, si->ifParam);
                long long temp2 = allocateTemp();
                emit("STORE " + std::to_string(temp2));  // p0 => temp2
                emit("LOAD " + std::to_string(temp));  // temp => p0
                emit("STOREI " + std::to_string(temp2));  // p0 => memory[temp2] 
            }
            freeTemp(temp);
        }
        break;
    }


    case CommandKind::IF_THEN: {
        // children [0]=condition, [1]=commands
        node.children[0]->accept(*this); // condition => p0 (0 => false)
        emit("JZERO ???");
        size_t jzPos = instructions.size()-1;

        node.children[1]->accept(*this);

        long long offset = instructions.size() - jzPos;
        fixupJump(jzPos, offset);
        break;
    }

    case CommandKind::IF_THEN_ELSE: {
        // [0]=cond, [1]=then, [2]=else
        node.children[0]->accept(*this); 
        emit("JZERO ???");
        size_t jzPos = instructions.size()-1;

        node.children[1]->accept(*this);
        emit("JUMP ???");
        size_t jmpPos = instructions.size()-1;

        long long offsetElse = instructions.size() - jzPos;
        fixupJump(jzPos, offsetElse);

        node.children[2]->accept(*this);
        long long offsetEnd = instructions.size() - jmpPos;
        fixupJump(jmpPos, offsetEnd);
        break;
    }

    case CommandKind::WHILE: {
        // [0]=cond, [1]=body
        long long startLab = instructions.size();
        node.children[0]->accept(*this); 
        emit("JZERO ???");
        size_t jzPos = instructions.size()-1;

        node.children[1]->accept(*this);
        {
            long long dist = (long long)startLab - (long long)instructions.size();
            std::ostringstream oss; oss<<"JUMP "<<dist;
            emit(oss.str());
        }
        long long offsetEnd = instructions.size() - jzPos;
        fixupJump(jzPos, offsetEnd);
        break;
    }

    case CommandKind::REPEAT_UNTIL: {
        // [0]=body, [1]=cond
        long long startLab = instructions.size();
        node.children[0]->accept(*this);
        node.children[1]->accept(*this);
        {
            long long dist = (long long)startLab - (long long)instructions.size();
            std::ostringstream oss; 
            oss << "JZERO " << dist;
            emit(oss.str());
        }
        break;
    }

    case CommandKind::FOR_UP: {
        // [0]=iter, [1]=fromVal, [2]=toVal, [3]=body

        // 1. Zainicjalizuj iterator (z node.children[0])
        auto* idn = dynamic_cast<IdentifierNode*>(node.children[0]);
        SymbolInfo si1;
        if (idn) {
            si1.kind = SymbolKind::VAR;
            si1.name = idn->name;
            si1.isForIterator = true;
            si1.initialized = true;
            si1.ownerProcName = currentProcedure;
            symTab.addLocalSymbol(si1, idn->getLine());
        }
        SymbolInfo* si = getSymbol(idn->name);
        if (si->addr < 1) si->addr = memmgr.allocate(1);

        // 2. Załaduj wartość "from"
        //    p0 = fromVal
        node.children[1]->accept(*this);
        //    iter = fromVal
        emit(retStore(si->addr, si->ifParam));

        // 3. Załaduj wartość "to"
        long long limitAddr = memmgr.allocate(1);
        node.children[2]->accept(*this);
        {
            std::ostringstream oss;
            oss << "STORE " << limitAddr;
            emit(oss.str());
        }

        // 4. Oblicz różnicę: (limit = toVal - fromVal)
        //    p0 = memory[limitAddr]
        emit(retLoad(limitAddr, false));
        {
            std::ostringstream oss;
            // p0 = p0 - memory[iter]
            oss << "SUB " << si->addr;
            emit(oss.str());
        }

        // 5. Zacznij pętlę — zapisz różnicę do limitAddr i sprawdź, czy < 0
        long long startLab = instructions.size();
        {
            std::ostringstream oss;
            oss << "STORE " << limitAddr;
            emit(oss.str());
        }
        emit("JNEG ???");  // jeśli różnica < 0, wychodzimy z pętli
        size_t jnegPos = instructions.size() - 1;

        // 6. Ciało pętli
        node.children[3]->accept(*this);

        emit("SET 1");
        emit("ADD " + std::to_string(si->addr));
        emit(retStore(si->addr, false));

        // 7. Zmniejsz różnicę o 1 — (limit = limit - 1)
        emit("SET -1");
        {
            std::ostringstream oss;
            // p0 = -1 + memory[limitAddr]
            oss << "ADD " << limitAddr;
            emit(oss.str());
        }
        emit(retStore(limitAddr, false));

        // 8. Skocz do sprawdzania na początku pętli
        {
            long long dist = (long long)startLab - (long long)instructions.size() + 1;
            std::ostringstream oss; 
            oss << "JUMP " << dist;
            emit(oss.str());
        }

        // 9. Popraw skok warunku (JNEG ???)
        long long offsetEnd = instructions.size() - jnegPos;
        fixupJump(jnegPos, offsetEnd);

        if (idn) {
            symTab.removeLocalSymbol(idn->name, currentProcedure);
        }

        break;
    }

    case CommandKind::FOR_DOWN: {
        auto* idn = dynamic_cast<IdentifierNode*>(node.children[0]);
        SymbolInfo si1;
        if (idn) {
            si1.kind = SymbolKind::VAR;
            si1.name = idn->name;
            si1.isForIterator = true;
            si1.initialized = true;
            si1.ownerProcName = currentProcedure;
            symTab.addLocalSymbol(si1, idn->getLine());
        }
        SymbolInfo* si = getSymbol(idn->name);
        if (si->addr < 1) si->addr = memmgr.allocate(1);

        // 2. Załaduj wartość "from"
        //    p0 = fromVal
        node.children[1]->accept(*this);
        //    iter = fromVal
        emit(retStore(si->addr, si->ifParam));

        // 3. Załaduj wartość "to"
        long long limitAddr = memmgr.allocate(1);
        node.children[2]->accept(*this);
        {
            std::ostringstream oss;
            oss << "STORE " << limitAddr;
            emit(oss.str());
        }

        // 4. Oblicz różnicę: (limit = toVal - fromVal)
        //    p0 = memory[limitAddr]
        emit(retLoad(si->addr, false));
        {
            std::ostringstream oss;
            // p0 = p0 - memory[iter]
            oss << "SUB " << limitAddr;
            emit(oss.str());
        }

        // 5. Zacznij pętlę — zapisz różnicę do limitAddr i sprawdź, czy < 0
        long long startLab = instructions.size();
        // {
        //     std::ostringstream oss;
        //     oss << "STORE " << si->addr;
        //     emit(oss.str());
        // }
        emit("JNEG ???");  // jeśli różnica < 0, wychodzimy z pętli
        size_t jnegPos = instructions.size() - 1;

        // 6. Ciało pętli
        node.children[3]->accept(*this);

        // 7. Zmniejsz różnicę o 1 — (limit = limit - 1)
        emit("SET -1");
        {
            std::ostringstream oss;
            // p0 = -1 + memory[limitAddr]
            oss << "ADD " << si->addr;
            emit(oss.str());
        }
        emit(retStore(si->addr, false));

        // 8. Skocz do sprawdzania na początku pętli
        {
            long long dist = (long long)startLab - (long long)instructions.size() - 1;
            std::ostringstream oss; 
            oss << "JUMP " << dist;
            emit(oss.str());
        }

        // 9. Popraw skok warunku (JNEG ???)
        long long offsetEnd = instructions.size() - jnegPos;
        fixupJump(jnegPos, offsetEnd);

        if (idn) {
            symTab.removeLocalSymbol(idn->name, currentProcedure);
        }

        break;
    }

    case CommandKind::PROC_CALL: {
        // [0] = ProcCallNode
        auto* pc = dynamic_cast<ProcCallNode*>(node.children[0]);
        if (pc) pc->accept(*this);
        break;
    }

    case CommandKind::READ: {
        auto* idn = dynamic_cast<IdentifierNode*>(node.children[0]);
        emit("GET 0"); // read into p0
        SymbolInfo* si = getSymbol(idn->name);
        if (!idn->indexExpr) {
            // Use retStore for simple var
            // emit(retStore(si->addr, si->ifParam));
            emit("STORE " + std::to_string(si->addr));
        } else {
            long long temp = allocateTemp();
            emit("STORE " + std::to_string(temp));  // p0 => temp
            idn->indexExpr->accept(*this);  // oblicz index => p0
            if (si->ifParam) {
                long long temp2 = allocateTemp();
                emit("STORE " + std::to_string(temp2));  // p0 => temp2
                emit("LOAD " + std::to_string(si->addr));
                emit("ADD " + std::to_string(temp2));
                emit("STORE " + std::to_string(temp2));
                emit("LOAD " + std::to_string(temp));
                emit("STOREI " + std::to_string(temp2));
                freeTemp(temp2);
            }
            else {
                genArrOffset(si->addr, si->lowerBound, si->ifParam);
                long long temp2 = allocateTemp();
                emit("STORE " + std::to_string(temp2));  // p0 => temp2
                emit("LOAD " + std::to_string(temp));  // temp => p0
                emit("STOREI " + std::to_string(temp2));  // p0 => memory[temp2] 
            }
            freeTemp(temp);
        }
        break;
    }

    case CommandKind::WRITE: {
        // [0]= value => p0
        node.children[0]->accept(*this);
        emit("PUT 0");
        break;
    }

    default:
        // fallback
        for (auto* c: node.children) {
            c->accept(*this);
        }
    }
}

void CodeGenVisitor::visit(ArgsDeclNode &node) {
    // Dla formalnych parametrów: jeśli parametr jest tablicą, alokujemy tylko jedną komórkę
    // (bo będziemy przekazywać tylko wskaźnik).
    for (size_t i = 0; i < node.argNames.size(); i++) {
        SymbolInfo* si = getSymbol(node.argNames[i]);
        if (si->addr < 1) {
            if (si->kind == SymbolKind::ARR) {
                si->addr = memmgr.allocate(1); // tylko jedna komórka – pass-by-reference
            } else {
                si->addr = memmgr.allocate(1);
            }
        }
    }
}

void CodeGenVisitor::visit(ArgsNode &node) {
    // paramy aktualne => "SET paramVal", "STORE 2", etc.
    // brak definicji
}

void CodeGenVisitor::visit(ProcCallNode &node) {
    SymbolInfo* si = getSymbol(node.procName);

    if (node.args) node.args->accept(*this);

    long long argsSize = si->paramAddrs.size();
    ArgsNode* an = dynamic_cast<ArgsNode*>(node.args);

    
    for (long long i = 0; i < argsSize; i++) {
        SymbolInfo* si2 = getSymbol(an->varNames[i]);
        if (si2->kind == SymbolKind::VAR) {
            // Dla zwykłych zmiennych – kopiujemy wartość
            emit("LOAD " + std::to_string(si2->addr));
            emit(retStore(si->paramAddrs[i], false));
        } else if (si2->kind == SymbolKind::ARR) {
            // Dla tablic przekazywanych przez referencję – kopiujemy tylko wskaźnik (adres)
            emit("SET " + std::to_string(si2->addr));
            // emit(retStore(si->paramAddrs[i], true));
            emit("STORE " + std::to_string(si->paramAddrs[i]));
        }
    }
    long long ret = lineCounter + 3;
    emit("SET " + std::to_string(ret));
    emit(retStore(si->returnAddr, false));
    long long procStartPos = si->addr;
    long long jumpDist = procStartPos - lineCounter;
    emit("JUMP " + std::to_string(jumpDist));
    // Copy-back: dla zwykłych zmiennych kopiujemy wynik z formalnych parametrów z powrotem,
    // dla tablic (pass-by-reference) nie kopiujemy, bo zmiany są widoczne
    for (long long i = 0; i < argsSize; i++) {
        SymbolInfo* si2 = getSymbol(an->varNames[i]);
        if (si2->kind == SymbolKind::VAR) {
            emit(retLoad(si->paramAddrs[i], false));
            emit("STORE " + std::to_string(si2->addr));
        }
        // Dla tablic (ARR) nic nie kopiujemy z powrotem.
    }
}

void CodeGenVisitor::visit(ExpressionNode &node) {
    // Różnica: jeżeli op= "==", "!=", "<", ">", "<=", ">=", 
    // => generujemy p0= left-right, 
    // => W CommandNode (IF) sprawdzamy p0 ==0 itp. 
    // Lub bezpośrednio tu: p0=0 => eq, etc. 
    node.left->accept(*this);
    long long tmpA = allocateTemp();
    emit("STORE " + std::to_string(tmpA));

    node.right->accept(*this);
    long long tmpB = allocateTemp();
    emit("STORE " + std::to_string(tmpB));

    // load left => p0
    emit("LOAD " + std::to_string(tmpA));

    // Zakładamy, że condition => p0= 0 => false, !=0 => true
    // Gdy op= +, -, * / % => standard
    if (node.op == "+") {
        std::ostringstream oss; 
        oss<<"ADD "<< tmpB;
        emit(oss.str());
    } else if (node.op == "-") {
        std::ostringstream oss;
        oss<<"SUB "<<tmpB;
        emit(oss.str());
    } else if (node.op == "*") {
        genMultiply(tmpB);
    } else if (node.op == "/") {
        genDivision(tmpB, false);
    } else if (node.op == "%") {
        genDivision(tmpB, true);
    }
    else if (node.op == "==") {
        // p0= (left-right)
        // 0 => eq, !=0 => !eq
        std::ostringstream oss;
        oss<<"SUB "<<tmpB;
        emit(oss.str());
        emit("JZERO 3");
        emit("SET 0");
        emit("JUMP 2");
        emit("SET 1");

    }
    else if (node.op == "!=") {
        // p0= (left-right)
        // 0 => eq, !=0 => !eq
        // final interpretacja: p0=0 => eq => false, p0!=0 => true => (IF "JZERO skip")
        // w CommandNode(IF) => "JZERO" => means eq => skip
        std::ostringstream oss; 
        oss<<"SUB "<<tmpB;
        emit(oss.str());
        emit("JZERO 3");
        emit("SET 1");
        emit("JUMP 2");
        emit("SET 0");
    }
    else if (node.op == "<") {
        // p0= (left-right)
        // p0<0 => true, p0>=0 => false
        // w Command IF => "JNEG ???"
        std::ostringstream oss; 
        oss<<"SUB "<<tmpB;
        emit(oss.str());
        emit("JNEG 3");
        emit("SET 0");
        emit("JUMP 2");
        emit("SET 1");
    }
    else if (node.op == ">") {
        // p0= (left-right)
        // p0>0 => true => "JPOS ???"
        std::ostringstream oss;
        oss<<"SUB "<<tmpB;
        emit(oss.str());
        emit("JPOS 3");
        emit("SET 0");
        emit("JUMP 2");
        emit("SET 1");
    }
    else if (node.op == "<=") {
        // p0= (left-right)
        // p0<=0 => true => "JNEG ??? or JZERO ???"
        // final interpretacja w CommandNode jest tricky
        std::ostringstream oss; 
        oss<<"SUB "<<tmpB;
        emit(oss.str());
        emit("JPOS 3");
        emit("SET 1");
        emit("JUMP 2");
        emit("SET 0");
    }
    else if (node.op == ">=") {
        // p0= (left-right)
        // p0>=0 => true => "JPOS ??? or JZERO ???"
        std::ostringstream oss;
        oss<<"SUB "<<tmpB;
        emit(oss.str());
        emit("JNEG 3");
        emit("SET 1");
        emit("JUMP 2");
        emit("SET 0");
    }

    freeTemp(tmpA);
    freeTemp(tmpB);
}

void CodeGenVisitor::visit(ValueNode &node) {
    std::ostringstream oss; 
    oss << "SET " << node.val;
    emit(oss.str());
}

void CodeGenVisitor::visit(IdentifierNode &node) {
    SymbolInfo* si = getSymbol(node.name);
    if (!node.indexExpr) {
        // Zwykła zmienna
        emit(retLoad(si->addr, si->ifParam));
    } else {
        if(si->ifParam){
            emit("LOAD " + std::to_string(si->addr));
            long long temp = allocateTemp();
            emit("STORE " + std::to_string(temp));
            node.indexExpr->accept(*this);
            emit("ADD " + std::to_string(temp));
            emit("STORE " + std::to_string(temp));
            emit("LOADI " + std::to_string(temp));
            freeTemp(temp);
        }
        else {
            long long temp = allocateTemp();
            node.indexExpr->accept(*this);
            genArrOffset(si->addr, si->lowerBound, si->ifParam);
            emit("STORE " + std::to_string(temp));
            emit("LOADI " + std::to_string(temp));
            freeTemp(temp);
        }
        // Tablica: arr[i]
        //node.indexExpr->accept(*this);
    }
}


std::string CodeGenVisitor::retLoad(long long addr, bool param) {
    return "LOAD " + std::to_string(addr);
    if (param)
        return "LOADI " + std::to_string(addr);
    else
        return "LOAD " + std::to_string(addr);
}

std::string CodeGenVisitor::retStore(long long addr, bool param) {
    return "STORE " + std::to_string(addr);
    if (param)
        return "STOREI " + std::to_string(addr);
    else
        return "STORE " + std::to_string(addr);
}
std::string CodeGenVisitor::retAdd(long long addr, bool param) {
    return param ? ("ADDI " + std::to_string(addr)) : ("ADD " + std::to_string(addr));
}

std::string CodeGenVisitor::retSub(long long addr, bool param) {
    return param ? ("SUBI " + std::to_string(addr)) : ("SUB " + std::to_string(addr));
}