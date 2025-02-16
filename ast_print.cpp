#include "ast.hpp"
#include <iostream>

static void printIndent(int indent) {
    for (int i = 0; i < indent; i++) {
        std::cout << "  ";
    }
}

// Deklarujemy rekurencyjną funkcję:
void printAST(ASTNode* node, int indent = 0) {
    if (!node) {
        printIndent(indent);
        std::cout << "(null)\n";
        return;
    }

    // Sprawdź typ węzła przez dynamic_cast:

    // --- ProgramAllNode ---
    if (auto* pan = dynamic_cast<ProgramAllNode*>(node)) {
        printIndent(indent);
        std::cout << "ProgramAllNode\n";

        printIndent(indent + 1);
        std::cout << "procedures =>\n";
        printAST(pan->procedures, indent + 2);

        printIndent(indent + 1);
        std::cout << "mainPart =>\n";
        printAST(pan->mainPart, indent + 2);
    }

    // --- ProceduresNode ---
    else if (auto* procs = dynamic_cast<ProceduresNode*>(node)) {
        printIndent(indent);
        std::cout << "ProceduresNode (count=" << procs->procedureDecls.size() << ")\n";
        for (auto* p : procs->procedureDecls) {
            printAST(p, indent + 1);
        }
    }

    // --- ProcedureDeclNode ---
    else if (auto* pd = dynamic_cast<ProcedureDeclNode*>(node)) {
        printIndent(indent);
        std::cout << "ProcedureDeclNode: name=" << pd->procName << "\n";

        printIndent(indent + 1);
        std::cout << "argsDecl =>\n";
        printAST(pd->argsDecl, indent + 2);

        printIndent(indent + 1);
        std::cout << "localDecls =>\n";
        printAST(pd->localDecls, indent + 2);

        printIndent(indent + 1);
        std::cout << "commands =>\n";
        printAST(pd->commands, indent + 2);
    }

    // --- ProcHeadNode (jeśli w ogóle występuje w drzewie) ---
    else if (auto* ph = dynamic_cast<ProcHeadNode*>(node)) {
        printIndent(indent);
        std::cout << "ProcHeadNode name=" << ph->procName << "\n";

        printIndent(indent + 1);
        std::cout << "argsDecl =>\n";
        printAST(ph->argsDecl, indent + 2);
    }

    // --- MainNode ---
    else if (auto* mn = dynamic_cast<MainNode*>(node)) {
        printIndent(indent);
        std::cout << "MainNode\n";

        printIndent(indent + 1);
        std::cout << "declarations =>\n";
        printAST(mn->declarations, indent + 2);

        printIndent(indent + 1);
        std::cout << "commands =>\n";
        printAST(mn->commands, indent + 2);
    }

    // --- DeclarationsNode ---
    else if (auto* decls = dynamic_cast<DeclarationsNode*>(node)) {
        printIndent(indent);
        std::cout << "DeclarationsNode (count=" << decls->declList.size() << ")\n";
        for (auto* d : decls->declList) {
            printAST(d, indent + 1);
        }
    }

    // --- DeclarationVarNode ---
    else if (auto* dv = dynamic_cast<DeclarationVarNode*>(node)) {
        printIndent(indent);
        std::cout << "DeclarationVarNode name=" << dv->varName << "\n";
    }

    // --- DeclarationArrNode ---
    else if (auto* da = dynamic_cast<DeclarationArrNode*>(node)) {
        printIndent(indent);
        std::cout << "DeclarationArrNode name=" << da->arrName
                  << " range=[" << da->lowerBound << ":" << da->upperBound << "]\n";
    }

    // --- CommandsNode ---
    else if (auto* cmdList = dynamic_cast<CommandsNode*>(node)) {
        printIndent(indent);
        std::cout << "CommandsNode (count=" << cmdList->cmdList.size() << ")\n";
        for (auto* c : cmdList->cmdList) {
            printAST(c, indent + 1);
        }
    }

    // --- CommandNode ---
    else if (auto* cmd = dynamic_cast<CommandNode*>(node)) {
        printIndent(indent);
        std::cout << "CommandNode: kind=" << (int)cmd->cmdKind << "\n"; 
        // (int) – bo to enum class

        for (auto* child : cmd->children) {
            printAST(child, indent + 1);
        }
    }

    // --- ArgsDeclNode ---
    else if (auto* ad = dynamic_cast<ArgsDeclNode*>(node)) {
        printIndent(indent);
        std::cout << "ArgsDeclNode (count=" << ad->argNames.size() << ")\n";
        for (size_t i = 0; i < ad->argNames.size(); i++) {
            printIndent(indent + 1);
            std::cout << ad->argNames[i] << (ad->isArray[i] ? "[T]" : "") << "\n";
        }
    }

    // --- ArgsNode ---
    else if (auto* an = dynamic_cast<ArgsNode*>(node)) {
        printIndent(indent);
        std::cout << "ArgsNode (count=" << an->varNames.size() << ")\n";
        for (auto& s : an->varNames) {
            printIndent(indent + 1);
            std::cout << s << "\n";
        }
    }

    // --- ProcCallNode ---
    else if (auto* pc = dynamic_cast<ProcCallNode*>(node)) {
        printIndent(indent);
        std::cout << "ProcCallNode: name=" << pc->procName << "\n";
        if (pc->args) {
            printIndent(indent + 1);
            std::cout << "args =>\n";
            printAST(pc->args, indent + 2);
        }
    }

    // --- ExpressionNode ---
    else if (auto* expr = dynamic_cast<ExpressionNode*>(node)) {
        printIndent(indent);
        std::cout << "ExpressionNode op='" << expr->op << "'\n";
        printIndent(indent + 1);
        std::cout << "left =>\n";
        printAST(expr->left, indent + 2);

        printIndent(indent + 1);
        std::cout << "right =>\n";
        printAST(expr->right, indent + 2);
    }

    // --- ValueNode ---
    else if (auto* val = dynamic_cast<ValueNode*>(node)) {
        printIndent(indent);
        std::cout << "ValueNode: " << val->val << "\n";
    }

    // --- IdentifierNode ---
    else if (auto* idn = dynamic_cast<IdentifierNode*>(node)) {
        printIndent(indent);
        std::cout << "IdentifierNode: " << idn->name << "\n";
        if (idn->indexExpr) {
            printIndent(indent + 1);
            std::cout << "index =>\n";
            printAST(idn->indexExpr, indent + 2);
        }
    }

    // Gdy węzeł nie pasuje do żadnej z powyższych klas:
    else {
        printIndent(indent);
        std::cout << "Unknown ASTNode type!\n";
    }
}
