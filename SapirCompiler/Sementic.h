#ifndef SEMENTIC_H
#define SEMENTIC_H

#include "SyntaxTree.h"
#include "SymbolTable.h"
#include <stdlib.h>

int sementic_analysis(SyntaxTree* tree);

#define CURRENT_FUNCTION_SYMBOL "function"


#endif