#ifndef SYMTABLE_HPP
#define SYMTABLE_HPP

#include <string>
#include <unordered_map>
#include <vector>
#include <iostream>
#include <cstdint>

enum class SymbolKind {
    VAR,
    ARR,
    PROC
};

struct SymbolInfo {
    SymbolKind kind;
    std::string name;
    bool initialized = false; 
    bool isForIterator = false;
    bool ifParam = false; 

    long long addr = -1;
    long long lowerBoundOffset = 0; 

    // Dla tablic
    long long lowerBound;
    long long upperBound;

    // Dla procedur
    std::vector<SymbolKind> paramKinds; 
    std::vector<long long> paramAddrs; 
    std::vector<std::string> paramNames;
    std::vector<SymbolInfo*> paramSymbols;
    long long returnAddr = -1;

    // nazwa procedury, do której należy symbol ("" dla main / global)
    std::string ownerProcName; 
};

class SymbolTable {
public:
    // Zostaje globalScope dla symboli globalnych
    std::unordered_map<std::string, SymbolInfo> globalScope;

    // Zamiast single 'currentScope' mamy mapę <nazwa_procedury, map<nazwa_symbolu, SymbolInfo>>
    std::unordered_map<std::string, std::unordered_map<std::string, SymbolInfo>> localScopes;

    int errors = 0;

    void reportError(const std::string &msg, uint64_t line) {
        std::cerr << "Semantyczny błąd w linii " << line << ": " << msg << std::endl;
        errors++;
    }

    // -------------------------
    // Dodawanie do globalScope:
    // -------------------------
    bool addGlobalSymbol(const SymbolInfo &info, uint64_t line) {
        // klucz w globalScope: info.name
        auto it = globalScope.find(info.name);
        if (it != globalScope.end()) {
            reportError("Druga deklaracja symbolu (global) \"" + info.name + "\"", line);
            return false;
        }
        globalScope[info.name] = info;
        return true;
    }

    // --------------------------------------------
    // Dodawanie do localScopes[ownerProcName] itd.
    // --------------------------------------------
    bool addLocalSymbol(const SymbolInfo &info, uint64_t line) {
        // bierzemy ownerProcName z przekazanego info
        const std::string &owner = info.ownerProcName;
        auto &procMap = localScopes[owner]; 
        // sprawdzamy, czy w obrębie danej procedury nazwa jest wolna
        if (procMap.find(info.name) != procMap.end()) {
            reportError("Druga deklaracja lokalna symbolu \"" + info.name 
                        + "\" w procedurze [" + owner + "]", line);
            return false;
        }
        // wrzucamy do localScopes[owner][info.name]
        procMap[info.name] = info;
        return true;
    }

    // Metoda lookup bez podania nazwy procedury
    // - sprawdza w localScopes[currentProcedure] i w globalu
    SymbolInfo* lookup(const std::string &name, const std::string &currentProc) {
        // 1) localScopes[currentProc]
        auto itProc = localScopes.find(currentProc);
        if (itProc != localScopes.end()) {
            auto &symMap = itProc->second;
            auto itSym = symMap.find(name);
            if (itSym != symMap.end()) {
                return &itSym->second;
            }
        }
        // 2) globalScope
        auto itGlob = globalScope.find(name);
        if (itGlob != globalScope.end()) {
            return &itGlob->second;
        }
        // 3) brak
        return nullptr;
    }

    // Używane, jeśli chcesz odwoływać się do symboli procedury innej niż currentProc:
    // (albo w semantyce, gdy "ownerProcName" jest w info)
    SymbolInfo* lookupOwned(const std::string &name, const std::string &owner) {
        // najpierw localScopes[owner]
        auto itOwner = localScopes.find(owner);
        if (itOwner != localScopes.end()) {
            auto &symMap = itOwner->second;
            auto itSym = symMap.find(name);
            if (itSym != symMap.end()) {
                return &itSym->second;
            }
        }
        // jak nie ma => global
        auto itGlob = globalScope.find(name);
        if (itGlob != globalScope.end()) {
            return &itGlob->second;
        }
        return nullptr;
    }

    // Dodatkowo, do for-iterator remove (jeśli chcesz usuwać):
    bool removeLocalSymbol(const std::string &name, const std::string &owner) {
        auto itOwner = localScopes.find(owner);
        if (itOwner != localScopes.end()) {
            auto &symMap = itOwner->second;
            auto it = symMap.find(name);
            if (it != symMap.end()) {
                symMap.erase(it);
                return true;
            }
        }
        return false;
    }


    void clearLocalScope() {

    }
};

#endif // SYMTABLE_HPP
