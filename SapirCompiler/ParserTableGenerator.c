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

#include <time.h>

#pragma warning(disable:4996)

int create_parser_tables() {
    if (parser_tables_initialized) return 0;
    parser_tables_initialized = 1;

    add_rules();


    clock_t t0 = clock();
    build_states("PROGRAM");
    clock_t t1 = clock();
    collect_symbols();
    clock_t t2 = clock();
    set_nonterminals_position();
    clock_t t3 = clock();
    compute_follow();
    clock_t t4 = clock();
    build_parsing_tables();
    clock_t t5 = clock();
    createAssociationMap();
    free_states();

    double secs;
    secs = (double)(t1 - t0) / CLOCKS_PER_SEC;
    printf("states took %.6f s\n", secs);
    secs = (double)(t2 - t1) / CLOCKS_PER_SEC;
    printf("symbols took %.6f s\n", secs);
    secs = (double)(t3 - t2) / CLOCKS_PER_SEC;
    printf("set positions took %.6f s\n", secs);
    secs = (double)(t4 - t3) / CLOCKS_PER_SEC;
    printf("follow took %.6f s\n", secs);
    secs = (double)(t5 - t4) / CLOCKS_PER_SEC;
    printf("tables took %.6f s\n", secs);

    return 0;
}
