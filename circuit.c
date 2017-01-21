#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "circuit.h"

void spawn_circuit_node(enode *node, int pipe_up[2], int var_pipes[][2]);

void spawn_variable_node(const enode *node, int var_pipes[][2], char *var_value);

void spawn_negate_node(const enode *node, int var_pipes[][2], char *var_string);

void spawn_binary_node(enode *node, int var_pipes[][2], char *var_string);

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
    sscanf(expression, "x[%ld]", &node->value);

    dependent_variables[node->value] = 1;

    return node;
}

enode *parse_value(const char *expression) {
    enode *node = malloc(sizeof(enode));

    node->operation_code = VALUE_CODE;
    node->right_son = NULL;
    node->left_son = NULL;
    sscanf(expression, "%ld", &node->value);

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
    unsigned int rows_number;
    unsigned int circuit_equations_number;
    unsigned int variables_count;
    unsigned int initial_values_to_process;

    scanf("%d %d %d\n", &rows_number, &circuit_equations_number, &variables_count);

    initial_values_to_process = rows_number - circuit_equations_number;

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

    fflush(stdout);

    if (dependency_graph->variables[0].expression == NULL) {
        fprintf(stdout, "%d F", circuit_equations_number + 1);
        return 42;
    }

    for (int i = 0; i < initial_values_to_process; ++i) {
        int variables_initialized[variables_count];
        int active_circuits[variables_count];
        long variables_values[variables_count];

        memset(variables_initialized, 0, sizeof(variables_initialized));
        memset(active_circuits, 0, sizeof(active_circuits));
        memset(variables_values, 0, sizeof(variables_values));

        int equation_number;
        int is_solvable = read_resolve_initialization(dependency_graph,
                                                      variables_initialized,
                                                      active_circuits,
                                                      variables_values,
                                                      &equation_number);

        if (is_solvable == 1) {

            // todo this whole method has to be done in another process.

            unsigned int childs_count = 0;
            int var_pipes[variables_count][2];

//            int **var_pipes = malloc(sizeof(2*variables_count*sizeof(int)));

//            int **var_pipes = calloc(0, variables_count * sizeof(int *));
//
//            for (int j = 0; j < variables_count; ++j) {
//                var_pipes[j] = calloc(0, 2 * sizeof(int));
//            }
//
            for (int j = 0; j < variables_count; ++j) {
                if (pipe(var_pipes[j]) == -1) {
                    syserr("Error in pipe\n");
                }
            }

//            for (int j = 0; j < variables_count; ++j) {
//                printf("%d, ", variables_initialized[j]);
//            }
//            printf("\n");
//            for (int j = 0; j < variables_count; ++j) {
//                printf("%d, ", active_circuits[j]);
//            }
//            printf("\n");
//            for (int j = 0; j < variables_count; ++j) {
//                printf("%ld, ", variables_values[j]);
//            }

            for (int j = 0; j < variables_count; ++j) {
                if (active_circuits[j] == LEAF) {
                    put_val_into_pipe(j, variables_values, var_pipes);
                } else if (active_circuits[j] == ACTIVE) {
                    switch (fork()) {
                        case -1:
                            syserr("Error in fork\n");
                        case 0:
//                            printf("Spawned (%p)\n", dependency_graph->variables);
                            spawn_tree(j, dependency_graph->variables, var_pipes);
                            return 1;
                        default:
                            ++childs_count;
                    }
                }
            }

            char result_string[BUFF_SIZE];
            if (read(var_pipes[0][0], result_string, BUFF_SIZE) == -1) {
                syserr("Error while reading\n");
            }

            for (int p = 0; p < childs_count; ++p) {
                if (wait(0) == -1)
                    syserr("Error in wait\n");
            }

            for (int v = 0; v < variables_count; ++v) {
                if (close(var_pipes[v][0]) == -1) {
                    syserr("Error while closing pipe\n");
                }
                if (close(var_pipes[v][1]) == -1) {
                    syserr("Error while closing pipe\n");
                }
            }

            long result_val;
            sscanf(result_string, "%ld", &result_val);
            fprintf(stdout, "%d P %ld\n", equation_number, result_val);
        } else {
            fprintf(stdout, "%d F\n", equation_number);
        }
    }

    // todo free allocated memory

    return 0;
}

void put_val_into_pipe(const int var_index, const long *variables_values, int var_pipes[][2]) {
    char var_value[BUFF_SIZE];
    sprintf(var_value, "%ld", variables_values[var_index]);

    if (write(var_pipes[var_index][1], var_value, BUFF_SIZE) == -1) {
        syserr("Error while writing\n");
    }
}

void spawn_tree(int var_index, dnode *variables, int var_pipes[][2]) {
    int pipe_down[2];
    if (pipe(pipe_down) == -1) {
        syserr("Error in pipe\n");
    }

    char var_value[BUFF_SIZE];

    enode *root = variables[var_index].expression;
    switch (fork()) {
        case -1:
            syserr("Error in fork\n");
        case 0:
//            printf("Spawned main node\n");
//            printf("Spawned main node (%d), (%p)\n", var_index, root);
            spawn_circuit_node(root, pipe_down, var_pipes);
            return;
        default:
            if (close(pipe_down[1]) == -1) {
                syserr("Error while closing pipe_down[1]\n");
            }

            if (read(pipe_down[0], var_value, BUFF_SIZE) == -1) {
                syserr("Error while reading\n");
            }

            if (close(pipe_down[0]) == -1) {
                syserr("Error while closing pipe_down[0]\n");
            }

            if (write(var_pipes[var_index][1], var_value, BUFF_SIZE) == -1) {
                syserr("Error while writing\n");
            }

            if (wait(0) == -1)
                syserr("Error in wait\n");
    }
}

void spawn_circuit_node(enode *node, int pipe_up[2], int var_pipes[][2]) {
    // todo close grandfather pipe

    char var_value[BUFF_SIZE];

    if (close(pipe_up[0]) == -1) {
        syserr("Error while closing pipe_up[0]\n");
    }

    if (node->operation_code == VALUE_CODE) {
        sprintf(var_value, "%ld", node->value);
    } else if (node->operation_code == VARIABLE_CODE) {
        spawn_variable_node(node, var_pipes, var_value);
    } else if (node->operation_code == '-') {
        spawn_negate_node(node, var_pipes, var_value);
    } else if (node->operation_code == '*') {
        spawn_binary_node(node, var_pipes, var_value);
    }

//    printf("Waiting for results main node\n");
    /// Writing results to pipe_up.
    if (write(pipe_up[1], var_value, BUFF_SIZE) == -1) {
        syserr("Error while writing ddd\n");
    }

    if (close(pipe_up[1]) == -1) {
        syserr("Error while closing pipe_up[1]\n");
    }
}

void spawn_binary_node(enode *node, int var_pipes[][2], char *var_string) {
    int pipe_down_left[2];
    if (pipe(pipe_down_left) == -1) {
        syserr("Error in pipe\n");
    }
    int pipe_down_right[2];
    if (pipe(pipe_down_right) == -1) {
        syserr("Error in pipe\n");
    }
    switch (fork()) {
        case -1:
            syserr("Error in fork\n");
        case 0:
            spawn_circuit_node(node->left_son, pipe_down_left, var_pipes);
            return;
        default:;
    }

    switch (fork()) {
        case -1:
            syserr("Error in fork\n");
        case 0:
            spawn_circuit_node(node->right_son, pipe_down_right, var_pipes);
            return;
        default:;
    }

    if (close(pipe_down_left[1]) == -1) {
        syserr("Error while closing pipe_down[1]\n");
    }

    char left_val_string[BUFF_SIZE];

    if (read(pipe_down_left[0], left_val_string, BUFF_SIZE) == -1) {
        syserr("Error while reading\n");
    }

    if (close(pipe_down_left[0]) == -1) {
        syserr("Error while closing pipe_down[0]\n");
    }

    long left_val;
    sscanf(var_string, "%ld", &left_val);

    char right_val_string[BUFF_SIZE];

    if (read(pipe_down_right[0], right_val_string, BUFF_SIZE) == -1) {
        syserr("Error while reading\n");
    }

    if (close(pipe_down_right[0]) == -1) {
        syserr("Error while closing pipe_down[0]\n");
    }

    long right_val;
    sscanf(right_val_string, "%ld", &right_val);

    long res_val;

    if (node->operation_code == '*') {
        res_val = left_val * right_val;
    } else {
        res_val = left_val + right_val;
    }

    sprintf(var_string, "%ld", res_val);

    if (wait(0) == -1)
        syserr("Error in wait\n");

    if (wait(0) == -1)
        syserr("Error in wait\n");
}

void spawn_negate_node(const enode *node, int var_pipes[][2], char *var_string) {
    int pipe_down[2];
    if (pipe(pipe_down) == -1) {
        syserr("Error in pipe\n");
    }

    switch (fork()) {
        case -1:
            syserr("Error in fork\n");
        case 0:
            spawn_circuit_node(node->left_son, pipe_down, var_pipes);
            return;
        default:
            if (close(pipe_down[1]) == -1) {
                syserr("Error while closing pipe_down[1]\n");
            }

            if (read(pipe_down[0], var_string, BUFF_SIZE) == -1) {
                syserr("Error while reading\n");
            }

            if (close(pipe_down[0]) == -1) {
                syserr("Error while closing pipe_down[0]\n");
            }

            long var_value;// = atoi(result_string);
            sscanf(var_string, "%ld", &var_value);

            var_value *= -1;

            sprintf(var_string, "%ld", var_value);

            if (wait(0) == -1) {
                syserr("Error in wait\n");
            }
    }
}

void spawn_variable_node(const enode *node, int var_pipes[][2], char *var_value) {
    long var_index = node->value;

//    printf("Waiting for results variable %ld (%p)\n ", var_index, node);
    if (read(var_pipes[var_index][0], var_value, BUFF_SIZE) == -1) {
        syserr("Error while reading\n");
    }

//    printf("%s\n", var_value);
//    printf("Writing in variable\n");
    //todo do i have to write back value to buffer? (multiple access)
    if (write(var_pipes[var_index][1], var_value, BUFF_SIZE) == -1) {
        syserr("Error while writing\n");
    }
//    printf("%s\n", var_value);
}

int read_resolve_initialization(ddag *dependency_graph,
                                int *variables_initialized,
                                int *active_circuits,
                                long *variables_values,
                                int *equation_number) {
    char *expression = NULL;
    size_t len = 0;
    size_t expression_length;
    ssize_t read;

    read = getline(&expression, &len, stdin);

    expression_length = (size_t) read;
    expression[expression_length - 1] = 0;

    unsigned int chars_read;
    sscanf(expression, "%d %n", equation_number, &chars_read);

    expression += chars_read;

    unsigned int var_index;
    long var_value;
    while (sscanf(expression, "x[%d] %ld %n", &var_index, &var_value, &chars_read) == 2) {
        variables_initialized[var_index] = 1;
        variables_values[var_index] = var_value;
        expression += chars_read;
    }

    return is_circuit_solvable(dependency_graph, variables_initialized, active_circuits);
}

int is_circuit_solvable(ddag *dependency_graph, int variables_initialized[], int active_variables[]) {
    dnode *root = dependency_graph->variables;

    return dfs_is_solvable(root, dependency_graph->variables, variables_initialized, active_variables);
}

int dfs_is_solvable(dnode *v, dnode *variables, int *variables_initialized, int *active_variables) {
    int variable_index = v->variable_index;

    if (variables_initialized[variable_index] == 1) {
        active_variables[variable_index] = LEAF;
        return 1;
    }

    if (variables[variable_index].expression == NULL) {
        return 0;
    }

    for (dlist *var = variables[variable_index].dependent_variables; var != NULL; var = var->next) {
        if (dfs_is_solvable(var->variable, variables, variables_initialized, active_variables) == 0) {
            return 0;
        }
    }

    active_variables[variable_index] = ACTIVE;
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