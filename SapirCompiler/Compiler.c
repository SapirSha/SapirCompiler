#include "Compiler.h"

#include "Lexer.h"
#include "Tokens.h"
#include "ParserTableGenerator.h"
#include "Parser.h"
#include "Queue.h"
#include "Sementic.h"
#include "SyntaxTree.h"
#include "IR_CFG.h"
#include "IR_Liveness.h"
#include "CodeGeneration.h"
#include "ErrorHandler.h"

bool compile(char *code) {

    Queue* tokens = tokenize(code);

    if (current_error_state != NO_ERROR) return false;

    create_parser_tables();

    SyntaxTree* syntax_tree = commit_parser(tokens);

    if (current_error_state != NO_ERROR) return false;
    return true;


    sementic_analysis(syntax_tree);

    if (current_error_state != NO_ERROR) return false;

    CodeBlock* mainblock = mainCFG(syntax_tree);

    mainblock = computeLiveness(mainblock);

    generate_code(mainblock);

    return true;
}