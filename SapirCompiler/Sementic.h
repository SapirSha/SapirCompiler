#ifndef SEMENTIC_H
#define SEMENTIC_H

#include "SyntaxTree.h"

typedef enum {
	NONE = 0,
	INT,
	STRING,
	BOOL,
	FLOAT,
} Data_Type;



int sementic_analysis(SyntaxTree* tree);

#endif