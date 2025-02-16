%{
#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <string>

#include "ast.hpp"

// Deklaracja funkcji do obsługi błędów i skanera:
static void yyerror(const char* msg);
int yylex(void);

// Globalny wskaźnik do korzenia AST:
ASTNode* g_root = nullptr;

extern FILE *yyin;
%}

%code requires
{
    #include <string>
    #include <cstdint>
    #include "ast.hpp"

    
    // Informacje o tokenie: wartość liczbową, numer linii i ewentualny string
    typedef struct Parser_token {
        uint64_t value;
        uint64_t line;
        std::string* str;
    } Parser_token;
}

%union
{
    Parser_token ptoken;  // dla tokenów
    ASTNode* node;        // dla węzłów AST
}

%locations

%token YY_SEMICOLON YY_COMMA
%token YY_ADD YY_SUB YY_MUL YY_DIV YY_MOD
%token YY_ASSIGN
%token YY_EQ YY_NEQ YY_LT YY_GT YY_LEQT YY_GEQT
%token YY_PROGRAM YY_IS YY_END YY_PROCEDURE YY_BEGIN
%token YY_IF YY_THEN YY_ELSE YY_ENDIF
%token YY_WHILE YY_DO YY_ENDWHILE YY_REPEAT YY_UNTIL
%token YY_FOR YY_FROM YY_TO YY_DOWNTO YY_ENDFOR
%token YY_READ YY_WRITE
%token YY_T
%token YY_L_BRACKET_R YY_R_BRACKET_R
%token YY_L_BRACKET_S YY_R_BRACKET_S
%token YY_ARR_RANGE
%token YY_VARIABLE
%token YY_NUM

%type <ptoken> YY_SEMICOLON YY_COMMA
%type <ptoken> YY_ADD YY_SUB YY_MUL YY_DIV YY_MOD
%type <ptoken> YY_ASSIGN
%type <ptoken> YY_EQ YY_NEQ YY_LT YY_GT YY_LEQT YY_GEQT
%type <ptoken> YY_PROGRAM YY_IS YY_END YY_PROCEDURE YY_BEGIN
%type <ptoken> YY_IF YY_THEN YY_ELSE YY_ENDIF
%type <ptoken> YY_WHILE YY_DO YY_ENDWHILE YY_REPEAT YY_UNTIL
%type <ptoken> YY_FOR YY_FROM YY_TO YY_DOWNTO YY_ENDFOR
%type <ptoken> YY_READ YY_WRITE
%type <ptoken> YY_T
%type <ptoken> YY_L_BRACKET_R YY_R_BRACKET_R
%type <ptoken> YY_L_BRACKET_S YY_R_BRACKET_S
%type <ptoken> YY_ARR_RANGE
%type <ptoken> YY_VARIABLE
%type <ptoken> YY_NUM

%type <node> program_all procedures proc_head main
%type <node> commands command
%type <node> declarations args_decl args
%type <node> expression condition value identifier
%type <node> proc_call
%type <node> number

%%

// ==================== G R A M A T Y K A ==================== //

// program_all -> procedures main | main
program_all:
      procedures main
        {
          ProgramAllNode* pan = new ProgramAllNode(@1.first_line, $1, $2);
          g_root = pan;
          $$ = pan;
        }
    | main
        {
          ProgramAllNode* pan = new ProgramAllNode(@1.first_line, nullptr, $1);
          g_root = pan;
          $$ = pan;
        }
    ;

/*
   procedures -> procedures YY_PROCEDURE proc_head YY_IS declarations YY_BEGIN commands YY_END
               | procedures YY_PROCEDURE proc_head YY_IS YY_BEGIN commands YY_END
               | empty
*/
procedures:
      procedures YY_PROCEDURE proc_head YY_IS declarations YY_BEGIN commands YY_END
        {
          ProceduresNode* ps = dynamic_cast<ProceduresNode*>($1);
          ProcHeadNode* ph = dynamic_cast<ProcHeadNode*>($3);
          DeclarationsNode* locDecls = dynamic_cast<DeclarationsNode*>($5);
          CommandsNode* cmds = dynamic_cast<CommandsNode*>($7);

          ProcedureDeclNode* pd = new ProcedureDeclNode(
              $2.line,
              ph->procName,
              ph->argsDecl,
              locDecls,
              cmds
          );
          ps->procedureDecls.push_back(pd);


          pd->argsDecl = ph->argsDecl;
          ph->argsDecl = nullptr;
          delete ph;
          $$ = ps;
        }
    | procedures YY_PROCEDURE proc_head YY_IS YY_BEGIN commands YY_END
        {
          ProceduresNode* ps = dynamic_cast<ProceduresNode*>($1);
          ProcHeadNode* ph = dynamic_cast<ProcHeadNode*>($3);
          CommandsNode* cmds = dynamic_cast<CommandsNode*>($6);

          ProcedureDeclNode* pd = new ProcedureDeclNode(
              $2.line,
              ph->procName,
              ph->argsDecl,
              nullptr,
              cmds
          );
          ps->procedureDecls.push_back(pd);


          pd->argsDecl = ph->argsDecl;
          ph->argsDecl = nullptr;

          delete ph;
          $$ = ps;
        }
    | %empty
        {
          ProceduresNode* ps = new ProceduresNode(@$.first_line);
          $$ = ps;
        }
    ;

/* proc_head -> YY_VARIABLE ( args_decl ) */
proc_head:
    YY_VARIABLE YY_L_BRACKET_R args_decl YY_R_BRACKET_R
    {
      ProcHeadNode* ph = new ProcHeadNode(
          $1.line,
          *($1.str),  // nazwa
          $3          // wskaźnik do args_decl
      );
      delete $1.str;
      $$ = ph;
    }
    ;

/*
   main -> YY_PROGRAM YY_IS declarations YY_BEGIN commands YY_END
         | YY_PROGRAM YY_IS YY_BEGIN commands YY_END
*/
main:
      YY_PROGRAM YY_IS declarations YY_BEGIN commands YY_END
        {
          MainNode* mn = new MainNode($1.line, $3, $5);
          $$ = mn;
        }
    | YY_PROGRAM YY_IS YY_BEGIN commands YY_END
        {
          MainNode* mn = new MainNode($1.line, nullptr, $4);
          $$ = mn;
        }
    ;

/*
   declarations -> rekurencyjna lista
*/
declarations:
      declarations YY_COMMA YY_VARIABLE
        {
          DeclarationsNode* dn = dynamic_cast<DeclarationsNode*>($1);
          DeclarationVarNode* dv = new DeclarationVarNode($3.line, *($3.str));
          dn->declList.push_back(dv);
          delete $3.str;
          $$ = dn;
        }
    | declarations YY_COMMA YY_VARIABLE YY_L_BRACKET_S YY_NUM YY_ARR_RANGE YY_NUM YY_R_BRACKET_S
        {
          DeclarationsNode* dn = dynamic_cast<DeclarationsNode*>($1);
          DeclarationArrNode* da = new DeclarationArrNode(
              $3.line,
              *($3.str),
              (long long)$5.value,
              (long long)$7.value
          );
          dn->declList.push_back(da);
          delete $3.str;
          $$ = dn;
        }
    | YY_VARIABLE
        {
          DeclarationsNode* dn = new DeclarationsNode($1.line);
          DeclarationVarNode* dv = new DeclarationVarNode($1.line, *($1.str));
          dn->declList.push_back(dv);
          delete $1.str;
          $$ = dn;
        }
    | YY_VARIABLE YY_L_BRACKET_S YY_NUM YY_ARR_RANGE YY_NUM YY_R_BRACKET_S
        {
          DeclarationsNode* dn = new DeclarationsNode($1.line);
          DeclarationArrNode* da = new DeclarationArrNode(
              $1.line,
              *($1.str),
              (long long)$3.value,
              (long long)$5.value
          );
          dn->declList.push_back(da);
          delete $1.str;
          $$ = dn;
        }
    ;

/*
  args_decl -> rekurencyjna lista argumentów (zwykłych lub tablicowych T)
*/
args_decl:
      args_decl YY_COMMA YY_VARIABLE
        {
          ArgsDeclNode* ad = dynamic_cast<ArgsDeclNode*>($1);
          ad->argNames.push_back(*($3.str));
          ad->isArray.push_back(false);
          delete $3.str;
          $$ = ad;
        }
    | args_decl YY_COMMA YY_T YY_VARIABLE
        {
          ArgsDeclNode* ad = dynamic_cast<ArgsDeclNode*>($1);
          ad->argNames.push_back(*($4.str));
          ad->isArray.push_back(true);
          delete $4.str;
          $$ = ad;
        }
    | YY_VARIABLE
        {
          ArgsDeclNode* ad = new ArgsDeclNode($1.line);
          ad->argNames.push_back(*($1.str));
          ad->isArray.push_back(false);
          delete $1.str;
          $$ = ad;
        }
    | YY_T YY_VARIABLE
        {
          ArgsDeclNode* ad = new ArgsDeclNode($1.line);
          ad->argNames.push_back(*($2.str));
          ad->isArray.push_back(true);
          delete $2.str;
          $$ = ad;
        }
    ;

/*
  args -> rekurencyjna lista identyfikatorów (tu w przykładzie nazwy)
*/
args:
      args YY_COMMA YY_VARIABLE
        {
          ArgsNode* an = dynamic_cast<ArgsNode*>($1);
          an->varNames.push_back(*($3.str));
          delete $3.str;
          $$ = an;
        }
    | YY_VARIABLE
        {
          ArgsNode* an = new ArgsNode($1.line);
          an->varNames.push_back(*($1.str));
          delete $1.str;
          $$ = an;
        }
    ;


/*
  commands -> commands command | command
*/
commands:
      commands command
        {
          CommandsNode* cmdList = dynamic_cast<CommandsNode*>($1);
          cmdList->cmdList.push_back($2);
          $$ = cmdList;
        }
    | command
        {
          CommandsNode* cmdList = new CommandsNode(@1.first_line);
          cmdList->cmdList.push_back($1);
          $$ = cmdList;
        }
    ;

/*
  command -> ...
   Uwaga na indexy w regule IF condition THEN commands ELSE commands ENDIF:
    tokeny: 1=IF, 2=condition, 3=THEN, 4=commands, 5=ELSE, 6=commands, 7=ENDIF
*/
command:
      identifier YY_ASSIGN expression YY_SEMICOLON
        {
          CommandNode* cmd = new CommandNode($2.line, CommandKind::ASSIGN);
          cmd->children.push_back($1);  // ident
          cmd->children.push_back($3);  // expr
          $$ = cmd;
        }
    | YY_IF condition YY_THEN commands YY_ELSE commands YY_ENDIF
        {
          CommandNode* cmd = new CommandNode($1.line, CommandKind::IF_THEN_ELSE);
          cmd->children.push_back($2); // condition
          cmd->children.push_back($4); // THEN-block
          cmd->children.push_back($6); // ELSE-block (uwaga: $6!)
          $$ = cmd;
        }
    | YY_IF condition YY_THEN commands YY_ENDIF
        {
          CommandNode* cmd = new CommandNode($1.line, CommandKind::IF_THEN);
          cmd->children.push_back($2); // condition
          cmd->children.push_back($4); // THEN-block
          $$ = cmd;
        }
    | YY_WHILE condition YY_DO commands YY_ENDWHILE
        {
          CommandNode* cmd = new CommandNode($1.line, CommandKind::WHILE);
          cmd->children.push_back($2); // condition
          cmd->children.push_back($4); // block
          $$ = cmd;
        }
    | YY_REPEAT commands YY_UNTIL condition YY_SEMICOLON
        {
          CommandNode* cmd = new CommandNode($1.line, CommandKind::REPEAT_UNTIL);
          cmd->children.push_back($2); // commands
          cmd->children.push_back($4); // condition
          $$ = cmd;
        }
    | YY_FOR YY_VARIABLE YY_FROM value YY_TO value YY_DO commands YY_ENDFOR
        {
          CommandNode* cmd = new CommandNode($1.line, CommandKind::FOR_UP);
          IdentifierNode* iter = new IdentifierNode($2.line, *($2.str), nullptr);
          cmd->children.push_back(iter); // iterator
          cmd->children.push_back($4);   // from
          cmd->children.push_back($6);   // to
          cmd->children.push_back($8);   // body
          delete $2.str;
          $$ = cmd;
        }
    | YY_FOR YY_VARIABLE YY_FROM value YY_DOWNTO value YY_DO commands YY_ENDFOR
        {
          CommandNode* cmd = new CommandNode($1.line, CommandKind::FOR_DOWN);
          IdentifierNode* iter = new IdentifierNode($2.line, *($2.str), nullptr);
          cmd->children.push_back(iter);
          cmd->children.push_back($4);   // from
          cmd->children.push_back($6);   // downto
          cmd->children.push_back($8);   // body
          delete $2.str;
          $$ = cmd;
        }
    | proc_call YY_SEMICOLON
        {
          CommandNode* cmd = new CommandNode($2.line, CommandKind::PROC_CALL);
          cmd->children.push_back($1);
          $$ = cmd;
        }
    | YY_READ identifier YY_SEMICOLON
        {
          CommandNode* cmd = new CommandNode($1.line, CommandKind::READ);
          cmd->children.push_back($2);
          $$ = cmd;
        }
    | YY_WRITE value YY_SEMICOLON
        {
          CommandNode* cmd = new CommandNode($1.line, CommandKind::WRITE);
          cmd->children.push_back($2);
          $$ = cmd;
        }
    ;

/* proc_call -> YY_VARIABLE '(' args ')' */
proc_call:
    YY_VARIABLE YY_L_BRACKET_R args YY_R_BRACKET_R
    {
      // Załóżmy, że ProcCallNode ma konstruktor (line, nazwa, wskaźnik do args):
      ProcCallNode* call = new ProcCallNode($1.line, *($1.str), $3);
      delete $1.str;
      $$ = call;
    }
    ;

/* expression -> value | expression + expression | ... */
expression:
      value
        {
          $$ = $1;
        }
    | expression YY_ADD expression
        {
          ExpressionNode* expr = new ExpressionNode($2.line, "+", $1, $3);
          $$ = expr;
        }
    | expression YY_SUB expression
        {
          ExpressionNode* expr = new ExpressionNode($2.line, "-", $1, $3);
          $$ = expr;
        }
    | expression YY_MUL expression
        {
          ExpressionNode* expr = new ExpressionNode($2.line, "*", $1, $3);
          $$ = expr;
        }
    | expression YY_DIV expression
        {
          ExpressionNode* expr = new ExpressionNode($2.line, "/", $1, $3);
          $$ = expr;
        }
    | expression YY_MOD expression
        {
          ExpressionNode* expr = new ExpressionNode($2.line, "%", $1, $3);
          $$ = expr;
        }
    ;

/* condition -> value op value */
condition:
      value YY_EQ value
        {
          ExpressionNode* expr = new ExpressionNode($2.line, "==", $1, $3);
          $$ = expr;
        }
    | value YY_NEQ value
        {
          ExpressionNode* expr = new ExpressionNode($2.line, "!=", $1, $3);
          $$ = expr;
        }
    | value YY_GT value
        {
          ExpressionNode* expr = new ExpressionNode($2.line, ">", $1, $3);
          $$ = expr;
        }
    | value YY_LT value
        {
          ExpressionNode* expr = new ExpressionNode($2.line, "<", $1, $3);
          $$ = expr;
        }
    | value YY_GEQT value
        {
          ExpressionNode* expr = new ExpressionNode($2.line, ">=", $1, $3);
          $$ = expr;
        }
    | value YY_LEQT value
        {
          ExpressionNode* expr = new ExpressionNode($2.line, "<=", $1, $3);
          $$ = expr;
        }
    ;

/* value -> YY_NUM | identifier */
value:
      number
        {
          $$ = $1;
        }
    | identifier
        {
          $$ = $1;
        }
    ;

number:
      YY_NUM
      {
        ValueNode* vn = new ValueNode($1.line, (long long) $1.value);
        $$ = vn;
      }
    | YY_SUB YY_NUM
      {
        ValueNode* vn = new ValueNode($2.line, -(long long) $2.value);
        $$ = vn;
      }
    ;
        

/* identyfikator -> var [ expr ] ? w uproszczeniu */
identifier:
      YY_VARIABLE
        {
          IdentifierNode* idn = new IdentifierNode($1.line, *($1.str), nullptr);
          delete $1.str;
          $$ = idn;
        }
    | YY_VARIABLE YY_L_BRACKET_S YY_VARIABLE YY_R_BRACKET_S
        {
          // index = kolejny identifier
          IdentifierNode* idx = new IdentifierNode($3.line, *($3.str), nullptr);
          IdentifierNode* idn = new IdentifierNode($1.line, *($1.str), idx);
          delete $1.str; delete $3.str;
          $$ = idn;
        }
    | YY_VARIABLE YY_L_BRACKET_S YY_NUM YY_R_BRACKET_S
        {
          // index = ValueNode
          ValueNode* vn = new ValueNode($3.line, (long long)$3.value);
          IdentifierNode* idn = new IdentifierNode($1.line, *($1.str), vn);
          delete $1.str;
          $$ = idn;
        }
    ;

%%

static void yyerror(const char* msg)
{
    std::cerr << "Parse error: " << msg << std::endl;
}
