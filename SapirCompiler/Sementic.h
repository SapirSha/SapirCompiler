#ifndef SEMENTIC_H
#define SEMENTIC_H

#include "SyntaxTree.h"

typedef enum {
	INT,
	STRING,
	BOOL,
	FLOAT
} Data_Type;



int sementic_analysis(SyntaxTree* tree);

#endif