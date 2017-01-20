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

typedef struct circuit_node circuit_node;

struct circuit_node {
    // todo pipe-in
    // todo pipe-out

    int operation_code;
    int value;
    circuit_node *left_son; /// nodes that you are waiting for
    circuit_node *right_son;
    circuit_node *parent; /// nodes to receive your value
};

typedef struct circuit_variable_node cv_node;

struct circuit_variable_node {
    // todo
//    int parents_ids[];
};

typedef struct dependency_tree_node dt_node;

struct dependency_tree_node {
    int variable_name;
    dt_node *left_son;
    dt_node *right_son;
};

circuit_node *parse_unary_operation(const char *expression, size_t expression_length);

circuit_node *parse_value(const char *expression);

circuit_node *prase_variable(const char *expression);

circuit_node *prase_binary_operation(char *expression, size_t expression_length);

circuit_node *parse_expression(char *expression, size_t expression_length);

#endif //PWZADANIE3_CIRCUIT_H
