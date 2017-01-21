#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "circuit.h"

int is_circuit_solvable(ddag *dependency_graph, int *variables_initialized);

int dfs_is_solvabe(dnode *v, dnode variables[], int *variables_initialized);

enode *parse_binary_operation(char *expression,
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

void parse_single_equation(char *expression, size_t expression_length, ddag *dgraph, unsigned int variables_count) {
    int variable_index;
    sscanf(expression, "x[%d]", &variable_index);

    unsigned int dependent_variables[variables_count];
    memset(dependent_variables, 0, sizeof dependent_variables);

    dnode *node = &dgraph->variables[variable_index];

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
            new_list->variable = &dgraph->variables[i];

            *tail = new_list;
            tail = &new_list->next;
        }
    }

    *tail = NULL;
}

int main() {
    // todo read input
    unsigned int rows_number;
    unsigned int circuit_equations_number;
    unsigned int variables_count;
    unsigned int initial_values_to_process;

    scanf("%d %d %d\n", &rows_number, &circuit_equations_number, &variables_count);

    initial_values_to_process = rows_number - circuit_equations_number;

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

    ddag *dependency_graph = initialize_dependency_graph(variables_count);

    for (int i = 0; i < circuit_equations_number; ++i) {
        unsigned int equation_number;
        scanf("%d ", &equation_number);

        read_parse_equation(variables_count, dependency_graph);

        if (is_cycled(dependency_graph) == 1) {
            fprintf(stdout, "%d F", equation_number);
            return 42;
        }
        fprintf(stdout, "%d P\n", equation_number);
    }

    // todo initialize processes from structure (create net)

    // REQUESTS

    fflush(stdout);

    if (dependency_graph->variables[0].expression == NULL) {
        fprintf(stdout, "%d F", circuit_equations_number + 1);
        return 42;
    }

    // in loop

    for (int i = 0; i < initial_values_to_process; ++i) {
        int equation_number;
        scanf("%d ", &equation_number);

        int variables_initialized[variables_count];
        memset(variables_initialized, 0, sizeof(variables_initialized));

        int variables_values[variables_count];

        char *expression = NULL;
        size_t len = 0;
        size_t expression_length;
        ssize_t read;

        read = getline(&expression, &len, stdin);

        expression_length = (size_t) read;
        expression[expression_length - 1] = 0;

        unsigned int var_index;
        int var_value;
        unsigned int chars_read;
        while (sscanf(expression, "x[%d] %d %n", &var_index, &var_value, &chars_read) == 2) {
            variables_initialized[var_index] = 1;
            variables_values[var_index] = var_value;
            expression += chars_read;
        }

        if (is_circuit_solvable(dependency_graph, variables_initialized) == 1) {
            fprintf(stdout, "%d P\n", equation_number);
        } else {
            fprintf(stdout, "%d F\n", equation_number);
        }
    }

    // todo resolve dependencies, decide if circuit is resolvable and to which nodes send values

    // todo send values

    // todo print result

    // CLEANING STAGE

    // todo terminate all processes

    // todo free allocated memory

    return 0;
}

int is_circuit_solvable(ddag *dependency_graph, int *variables_initialized) {
    dnode *root = dependency_graph->variables;

    return dfs_is_solvabe(root, dependency_graph->variables, variables_initialized);
}

int dfs_is_solvabe(dnode *v, dnode variables[], int *variables_initialized) {
    int variable_index = v->variable_index;

    if (variables_initialized[variable_index] == 1) {
        return 1;
    }

    if (variables[variable_index].expression == NULL) {
        return 0;
    }

    for (dlist *var = variables[variable_index].dependent_variables; var != NULL; var = var->next) {
        if (dfs_is_solvabe(var->variable, variables, variables_initialized) == 0) {
            return 0;
        }
    }

    return 1;
}

int is_cycled(ddag *dependency_graph) {
    unsigned int vertices_number = dependency_graph->variables_count;
    unsigned int vertex_visited[vertices_number];

    memset(vertex_visited, 0, sizeof(vertex_visited));

    for (int v = 0; v < vertices_number; ++v) {
        if (vertex_visited[v] == 0) {
            if (dfs_cycle_search(v, dependency_graph->variables, vertex_visited) == 1) {
                return 1;
            }
        }
    }

    return 0;
}

int dfs_cycle_search(int v, dnode *vertices, unsigned int vertices_visited[]) {
    vertices_visited[v] = 1;

    for (dlist *var = vertices[v].dependent_variables; var != NULL; var = var->next) {
        int var_index = var->variable->variable_index;
        if (vertices_visited[var_index] == 1) {
            return 1;
        } else if (dfs_cycle_search(var_index, vertices, vertices_visited) == 1) {
            return 1;
        }
    }

    vertices_visited[v] = 0;

    return 0;
}

void read_parse_equation(unsigned int variables_count, ddag *dependency_graph) {
    char *expression = NULL;
    size_t len = 0;
    size_t expression_length;
    ssize_t read;

    read = getline(&expression, &len, stdin);

    expression_length = (size_t) read;
    expression[expression_length - 1] = 0;


    parse_single_equation(expression, expression_length, dependency_graph, variables_count);
}

ddag *initialize_dependency_graph(unsigned int variables_count) {
    ddag *dag = malloc(sizeof(ddag));
    dag->variables = malloc(variables_count * sizeof(dnode));
    dag->variables_count = variables_count;
    set_variables(variables_count, dag->variables);

    return dag;
}

void set_variables(unsigned int variables_count, dnode *variables) {
    for (int i = 0; i < variables_count; ++i) {
        variables[i].variable_index = i;
        variables[i].dependent_variables = NULL;
        variables[i].expression = NULL;
    }
}