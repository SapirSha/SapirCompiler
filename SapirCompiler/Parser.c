#include "Parser.h"
#include "LinkedList.h"
#include "ParserTableGenerator.h"
#include <stdlib.h>
#include "Queue.h"
#include "ArrayList.h"

static const int ZERO = 0;

char* actiontypetostring2(int action) {
    switch (action)
    {
    case ACCEPT: return "A";
    case REDUCE: return "R";
    case SHIFT:  return "S";
    case ERROR:  return "E";

    default:
        return NULL;
    }
}
void print_actioncell(ActionCell* cell) {
    printf("%s%d", actiontypetostring2(cell->type), cell->value);
}

void printINT2(int* in) {
    printf("%d", *in);
}

#define NextAction \
    actionTable[*(int*)linkedlist_peek(States)][associationArray[((Token*)queue_peek(tokens))->type]]

// might need to change to static stack for faster removal
void commit_parser(Queue* tokens) {
	LinkedList* States = linkedlist_init(sizeof(unsigned int));
    linkedlist_push(States, &ZERO);
    ActionCell current = NextAction;
    queue_dequeue(tokens);

    linkedlist_push(States, &current.value);
    
    print_actioncell(&current);
    printf(" < --- Doing This\n ");

    current = NextAction;
    while (current.type != ERROR && current.type != ACCEPT) {
        linkedlist_print(States, printINT2);


        printf(" < --- Doing This\n ");

        if (current.type == REDUCE) {
            Rule* reduce_rule = arraylist_get(rules, current.value);
            printf("REDUCE RULE %d\n", current.value);

            printf("REDUCE COUNT: %d\n", reduce_rule->ruleTerminalCount);

            for (int i = 0; i < reduce_rule->ruleTerminalCount; i++)
                *(int*)linkedlist_pop(States);

            int pos = *(int*)linkedlist_peek(States);
            printf("GOTO %d: %d %d\n", gotoTable[pos][reduce_rule->nonterminal_position], pos, reduce_rule->nonterminal_position);
            linkedlist_push(States, &gotoTable[pos][reduce_rule->nonterminal_position]);


            //break;
        }
        else {
            linkedlist_push(States, &current.value);
            queue_dequeue(tokens);
        }
            printf("NEXT TOKEN: %d-%s\n", ((Token*)queue_peek(tokens))->type, ((Token*)queue_peek(tokens))->lexeme);
            printf("Row: %d Col: %d\n", *(unsigned int*)linkedlist_peek(States), associationArray[((Token*)queue_peek(tokens))->type]);
            current = NextAction;
            print_actioncell(&current);

    }

    printf("\nENDED IN: ");
    print_actioncell(&current);
    printf("\nEND PARSER");
}