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
typedef struct dependency_dag_node dnode;
typedef struct dependency_list dlist;
typedef struct dependency_dag ddag;

struct enode {
    int operation_code;
    long value;
    enode *left_son; /// nodes that you are waiting for
    enode *right_son;
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

void spawn_tree(int var_index, dnode *variables, int var_pipes[][2]);

void put_val_into_pipe(const int var_index, const long *variables_values, int var_pipes[][2]);

void spawn_circuit_node(enode *node, int pipe_up[2], int var_pipes[][2]);

void spawn_variable_node(const enode *node, int var_pipes[][2], char *var_value);

int spawn_negate_node(const enode *node, int var_pipes[][2], char *var_string);

int spawn_binary_node(enode *node, int var_pipes[][2], char *var_string);

void process_single_input(unsigned int variables_count, const ddag *dependency_graph, const int *active_circuits,
                          const long *variables_values, int equation_number);

void release_memory(ddag *dependencies);

void release_variable(dnode variable);

void release_expression(enode *expression);

void release_dependency_list(dlist *list_element);

#endif //PWZADANIE3_CIRCUIT_H
