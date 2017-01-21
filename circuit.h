#ifndef PWZADANIE3_CIRCUIT_H
#define PWZADANIE3_CIRCUIT_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>

#include "circuit.h"
#include "err.h"

#define VARIABLE_CODE   2137
#define VALUE_CODE      1488

#define ACTIVE          1
#define LEAF            10
#define BUFF_SIZE       64

typedef struct enode enode;
typedef struct cnode cnode;
typedef struct dependency_dag_node dnode;
typedef struct dependency_list dlist;
typedef struct dependency_dag ddag;

struct enode {
    // todo pipe-in
    // todo pipe-out

    int operation_code;
    long value;
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
    int variable_index;
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

void parse_single_equation(char *expression, size_t expression_length, ddag *variables, unsigned int variables_count);


void set_variables(unsigned int variables_count, dnode *variables);

ddag *initialize_dependency_graph(unsigned int count);

void read_parse_equation(unsigned int variables_count, ddag *dependency_graph);

int is_cycled(ddag *dependency_graph);

int dfs_cycle_search(int v, dnode *vertices, unsigned int vertices_visited[]);

int is_circuit_solvable(ddag *dependency_graph, int variables_initialized[], int active_variables[]);

int dfs_is_solvable(dnode *v, dnode *variables, int *variables_initialized, int *active_variables);

int read_resolve_initialization(ddag *dependency_graph,
                                int *variables_initialized,
                                int *active_circuits,
                                long *variables_values,
                                int *equation_number);

#endif //PWZADANIE3_CIRCUIT_H
