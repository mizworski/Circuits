#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include "err.h"

int variables_in[2137];
int variables_out[2137];

typedef struct circuit_node circuit_node;

struct circuit_node {
    // todo pipe-in
    // todo pipe-out

    int operation_code;
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

circuit_node *parse_circuit(char *expression, size_t expression_length) {
    if (expression == NULL) {
        return NULL;
    }

    circuit_node *node = malloc(sizeof(circuit_node));

    size_t left_expression_length = 0;
    size_t right_expression_length = 0;

    char *left_expression = NULL;
    char *right_expression = NULL;

    if (expression[0] == '(' && expression[1] == '-') {
        node->operation_code = '-';
        left_expression_length = expression_length - 3;
        left_expression = malloc(left_expression_length * sizeof(char)); // todo sizeof(char) == 1?
        strncpy(left_expression, expression + 2, left_expression_length);
        node->left_son = parse_circuit(left_expression, left_expression_length);
        return node;
    }

    char *current_char = expression;

    if (*current_char == '(') {
        unsigned int opened_parentheses = 0;

        do {
            if (*current_char == '(') {
                ++opened_parentheses;
            } else if (*current_char == ')') {
                --opened_parentheses;
            }
            ++current_char;
            ++left_expression_length;
        } while (opened_parentheses != 0);

        // additional space for string terminator
        ++left_expression_length;

        left_expression = malloc(left_expression_length);
        strncpy(left_expression, expression + 1, left_expression_length);
    } else if (*current_char == 'x') {

    } else {

    }

    // moving to operator
    ++current_char;

    node->operation_code = *current_char;

    // moving to next expression
    ++current_char;
    expression = current_char;

    if (*current_char == '(') {
        unsigned int opened_parentheses = 0;

        do {
            if (*current_char == '(') {
                ++opened_parentheses;
            } else if (*current_char == ')') {
                --opened_parentheses;
            }
            ++current_char;
            ++right_expression_length;
        } while (opened_parentheses != 0);

        // additional space for string terminator
        ++right_expression_length;

        right_expression = malloc(right_expression_length);
        strncpy(right_expression, expression + 1, right_expression_length);
    } else {

    }

    node->left_son = parse_circuit(left_expression, left_expression_length);
    node->right_son = parse_circuit(right_expression, right_expression_length);

    return NULL;
}

int main() {
    // todo read input
    int input_length = 10;
    int circuit_equations_number = 10;
    int variables_number = 10;

    // PRE-PROCESSING

    // todo initialize pipes for variables
    // todo for now all variables pipes are visible to others, can be reduced to initialize them traversing dependency tree
    int vars_in[variables_number][2];
    int vars_out[variables_number][2];

    for (int i = 0; i < variables_number; ++i) {
        if (pipe(vars_in[i]) == -1) {
            syserr("Error in pipe\n");
        }

        if (pipe(vars_out[i]) == -1) {
            syserr("Error in pipe\n");
        }
    }

    // todo parse input to structure

    int which_variable = 1;

    char *expression = NULL;
    size_t len = 0;
    size_t expression_length;
    ssize_t read;

    read = getline(&expression, &len, stdin);

    // todo remove delimiter from ending, decrease input length

    expression_length = (size_t) read;

    circuit_node *circuit = parse_circuit(expression, expression_length);

    // todo create dependency tree

    // todo initialize processes from structure (create net)

    // REQUESTS

    // in loop

    // todo resolve dependencies, decide if circuit is resolvable and to which nodes send values

    // todo send values

    // todo print result

    // CLEANING STAGE

    // todo terminate all processes

    // todo free allocated memory

    return 0;
}