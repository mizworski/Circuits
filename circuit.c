#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include "err.h"

typedef struct circuit_node circuit_node;

struct circuit_node {
    // todo pipe-in
    // todo pipe-out

    int operation_code;
    circuit_node *left_son; /// nodes that you are waiting for
    circuit_node *right_son;
    circuit_node *parents[]; /// nodes to receive your value
};

typedef struct dependency_tree_node dt_node;

struct dependency_tree_node {
    int variable_name;
    dt_node *left_son;
    dt_node *right_son;
};

void initialize_circuit() {

}

int main() {
    // todo read input
    int input_length = 10;
    int circuit_equations_number = 10;
    int variables_number = 10;


    // PRE-PROCESSING

    // todo parse input to structure

    // todo create dependency tree

    // todo initialize pipes for variables

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