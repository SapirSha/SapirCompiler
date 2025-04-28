#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

#include "ParserTableGenerator.h"
#include "Tokens.h"
#include "HashMap.h"
#include "HashSet.h"
#include "LinkedList.h"
#include "ErrorHandler.h"

#pragma warning(disable:4996)

int create_parser_tables() {
    if (parser_tables_initialized) return 0;
    parser_tables_initialized = 1;

    add_rules();

    build_states("PROGRAM");
    collect_symbols();
    set_nonterminals_position();
    compute_follow();
    build_parsing_tables();
    createAssociationMap();
    free_states();

    return 0;
}
