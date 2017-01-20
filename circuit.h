#ifndef PWZADANIE3_CIRCUIT_H
#define PWZADANIE3_CIRCUIT_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>

#include "circuit.h"
#include "err.h"

#define VARIABLE_CODE 2137
#define VALUE_CODE 1488

typedef struct enode enode;
typedef struct cnode cnode;
typedef struct dependency_dag_node dnode;
typedef struct dependency_list dlist;
typedef struct dependency_dag ddag;

struct enode {
    // todo pipe-in
    // todo pipe-out

    int operation_code;
    int value;
    enode *left_son; /// nodes that you are waiting for
    enode *right_son;
    enode *parent; /// nodes to receive your value
};

struct cnode {
    // todo pipe-in to receive signals
    // todo pipe-out to output value

    int variable_index;
    unsigned int *dependent_variables;
};

struct dependency_dag_node {
    int variable_name;
    dlist *dependent_variables;
    enode *expression;
};

struct dependency_list {
    dnode *variable;
    dlist *next;
};

struct dependency_dag {
    dnode *variables;
    unsigned int variables_count;
};

enode *parse_unary_operation(char *expression,
                             size_t expression_length,
                             unsigned int dependent_variables[],
                             unsigned int variables_count);

enode *parse_value(const char *expression);

enode *parse_variable(const char *expression, unsigned int dependent_variables[]);

enode *parse_binary_operation(char *expression,
                              size_t expression_length,
                              unsigned int dependent_variables[],
                              unsigned int variables_count);

enode *parse_expression(char *expression,
                        size_t expression_length,
                        unsigned int dependent_variables[],
                        unsigned int variables_count);

dnode *
parse_single_equation(char *expression, size_t expression_length, dnode variables[], unsigned int variables_count);

#endif //PWZADANIE3_CIRCUIT_H
