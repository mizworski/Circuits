#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>

#include "circuit.h"
#include "err.h"

int variables_in[2137];
int variables_out[2137];

circuit_node *parse_expression(char *expression, size_t expression_length) {
    if (expression == NULL) {
        return NULL;
    }

    if (expression[0] == '(') {
        if (expression[1] == '-') {
            return parse_unary_operation(expression, expression_length);
        } else {
            return prase_binary_operation(expression, expression_length);
        }
    } else if (*expression == 'x') {
        return prase_variable(expression);
    } else {
        return parse_value(expression);
    }
}

circuit_node *prase_binary_operation(char *expression, size_t expression_length) {
    circuit_node *node = malloc(sizeof(circuit_node));

    char *current_char = expression;
    char *left_expression_begin = expression + 1;
    char *right_expression_begin;

    size_t left_expression_length = 0;
    size_t right_expression_length = 0;

    int operator_found = 0;
    unsigned int opened_parentheses = 0;

    do {
        ++current_char;
        if (*current_char == '(') {
            ++opened_parentheses;
        } else if (*current_char == ')') {
            --opened_parentheses;
        } else if (*current_char == '+' || *current_char == '*') {
            operator_found = 1;
        }
        ++left_expression_length;
    } while (opened_parentheses != 0 || operator_found == 0);

    if (*current_char == '+' || *current_char == '*') {
        --left_expression_length; /// Counted characters inclucing space and operator
    } else {
        ++left_expression_length; /// Need to add null at the end of the string.
        current_char += 2;
    }
    node->operation_code = *current_char;

    right_expression_begin = current_char + 2;
    right_expression_length = (expression_length - (left_expression_length + 2)) - 2;

    char *left_expression = malloc(left_expression_length * sizeof(char));
    char *right_expression = malloc(right_expression_length * sizeof(char));

    strncpy(left_expression, left_expression_begin, left_expression_length);
    strncpy(right_expression, right_expression_begin, right_expression_length);

    left_expression[left_expression_length - 1] = 0;
    right_expression[right_expression_length - 1] = 0;

    node->left_son = parse_expression(left_expression, left_expression_length);
    node->right_son = parse_expression(right_expression, right_expression_length);

    return node;
}

circuit_node *prase_variable(const char *expression) {
    circuit_node *node = malloc(sizeof(circuit_node));

    node->operation_code = VARIABLE_CODE;
    node->right_son = NULL;
    node->left_son = NULL;
    sscanf(expression, "x[%d]", &node->value);

    return node;
}

circuit_node *parse_value(const char *expression) {
    circuit_node *node = malloc(sizeof(circuit_node));

    node->operation_code = VALUE_CODE;
    node->right_son = NULL;
    node->left_son = NULL;
    sscanf(expression, "%d", &node->value);

    return node;
}

circuit_node *parse_unary_operation(const char *expression, size_t expression_length) {
    circuit_node *node = malloc(sizeof(circuit_node));

    char *new_expression = NULL;
    node->operation_code = '-';
    size_t new_expression_length = expression_length - 3;
    new_expression = malloc(new_expression_length * sizeof(char)); // todo sizeof(char) == 1?
    strncpy(new_expression, expression + 2, new_expression_length);
    node->left_son = parse_expression(new_expression, new_expression_length);
    return node;
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
    expression[expression_length - 1] = 0;

    circuit_node *circuit = parse_expression(expression, expression_length);

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