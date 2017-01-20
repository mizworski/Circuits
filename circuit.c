#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "circuit.h"

int variables_in[2137];
int variables_out[2137];

void set_variables(unsigned int variables_count, dnode *variables);

enode *
parse_binary_operation(char *expression,
                       size_t expression_length,
                       unsigned int dependent_variables[],
                       unsigned int variables_count) {
    enode *node = malloc(sizeof(enode));

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
        --left_expression_length; /// Counted characters includes space and operator
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

    node->left_son = parse_expression(left_expression, left_expression_length, dependent_variables, variables_count);
    node->right_son = parse_expression(right_expression, right_expression_length, dependent_variables, variables_count);

    return node;
}

enode *parse_variable(const char *expression,
                      unsigned int dependent_variables[]) {
    enode *node = malloc(sizeof(enode));

    node->operation_code = VARIABLE_CODE;
    node->right_son = NULL;
    node->left_son = NULL;
    sscanf(expression, "x[%d]", &node->value);

    dependent_variables[node->value] = 1;

    return node;
}

enode *parse_value(const char *expression) {
    enode *node = malloc(sizeof(enode));

    node->operation_code = VALUE_CODE;
    node->right_son = NULL;
    node->left_son = NULL;
    sscanf(expression, "%d", &node->value);

    return node;
}

enode *parse_unary_operation(char *expression,
                             size_t expression_length,
                             unsigned int dependent_variables[],
                             unsigned int variables_count) {
    enode *node = malloc(sizeof(enode));

    char *new_expression = NULL;
    node->operation_code = '-';
    size_t new_expression_length = expression_length - 3;
    new_expression = malloc(new_expression_length * sizeof(char));
    strncpy(new_expression, expression + 2, new_expression_length);
    node->left_son = parse_expression(new_expression, new_expression_length, dependent_variables, variables_count);
    return node;
}

enode *parse_expression(char *expression,
                        size_t expression_length,
                        unsigned int dependent_variables[],
                        unsigned int variables_count) {
    if (expression == NULL) {
        return NULL;
    }

    if (expression[0] == '(') {
        if (expression[1] == '-') {
            return parse_unary_operation(expression, expression_length, dependent_variables, variables_count);
        } else {
            return parse_binary_operation(expression, expression_length, dependent_variables, variables_count);
        }
    } else if (*expression == 'x') {
        return parse_variable(expression, dependent_variables);
    } else {
        return parse_value(expression);
    }
}

dnode *parse_single_equation(char *expression, size_t expression_length, dnode *variables, unsigned int variables_count) {
    unsigned int dependent_variables[variables_count];
    memset(dependent_variables, 0, sizeof dependent_variables);
    int variable_index;
    sscanf(expression, "x[%d]", &variable_index);

    dnode *node = malloc(sizeof(dnode));

    expression += 7;
    expression_length -= 7;

    node->expression = parse_expression(expression,
                                              expression_length,
                                              dependent_variables,
                                              variables_count);

    dlist **tail = &node->dependent_variables;

    for (int i = 0; i < variables_count; ++i) {
        if (dependent_variables[i] == 1) {
            dlist *new_list = malloc(sizeof(dlist));
            new_list->variable = &variables[i];

            *tail = new_list;
            tail = &new_list->next;
        }
    }

    *tail = NULL;

    return node;
}

int main() {
    // todo read input
    unsigned int input_length = 10;
    unsigned int circuit_equations_number = 10;
    unsigned int variables_count = 10;

    // PRE-PROCESSING

    // todo initialize pipes for variables
    // todo for now all variables pipes are visible to others, can be reduced to initialize them traversing dependency tree
    int vars_in[variables_count][2];
    int vars_out[variables_count][2];

    for (int i = 0; i < variables_count; ++i) {
        if (pipe(vars_in[i]) == -1) {
            syserr("Error in pipe\n");
        }

        if (pipe(vars_out[i]) == -1) {
            syserr("Error in pipe\n");
        }
    }

    // todo parse input to structure

    dnode variables[variables_count];

    set_variables(variables_count, variables);

    int which_variable = 1;

    char *expression = NULL;
    size_t len = 0;
    size_t expression_length;
    ssize_t read;

    read = getline(&expression, &len, stdin);

    // todo remove delimiter from ending, decrease input length

    expression_length = (size_t) read;
    expression[expression_length - 1] = 0;

    dnode *circuit = parse_single_equation(expression, expression_length, variables, variables_count);

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

void set_variables(unsigned int variables_count, dnode *variables) {
    for (int i = 0; i < variables_count; ++i) {
        variables[i].variable_name = i;
        variables[i].dependent_variables = NULL;
        variables[i].expression = NULL;
    }
}