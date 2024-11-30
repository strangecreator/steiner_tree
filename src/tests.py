import pytest
import numpy as np

from utils import *


def test_simple_1():
    number_of_vertices = 5
    edges = [
        (1, 2, 1),
        (2, 3, 1),
        (3, 4, 1),
        (4, 5, 1),
        (5, 1, 1),
    ]
    terminals = [1, 3, 5]

    run_and_compare_results(number_of_vertices, edges, terminals, include_optimization_algorithm=True)


def test_simple_2():
    number_of_vertices = 11
    edges = [
        (1, 2, 1),
        (2, 3, 1),
        (2, 4, 1),
        (4, 5, 1),
        (5, 6, 1),
        (5, 7, 1),
        (7, 8, 1),
        (7, 9, 1),
        (9, 10, 1),
        (9, 11, 1),
    ]
    terminals = [1, 3, 4, 6, 7, 8, 10, 11]

    run_and_compare_results(number_of_vertices, edges, terminals, include_optimization_algorithm=True)


@pytest.mark.parametrize("n", [10])
@pytest.mark.parametrize("edge_p", [0.1, 0.3, 0.7, 0.9])
@pytest.mark.parametrize("terminal_p", [0.1, 0.3, 0.7, 0.9])
def test_Gnp_graphs_with_optimization_algorithm(n: int, edge_p: float, terminal_p: float):
    NUMBER_OF_ITERATIONS = 1
    WEIGHTS_RANGE = (1, 10)

    Gnp_graphs = generate_Gnp_weighted_graphs(n, edge_p, NUMBER_OF_ITERATIONS, WEIGHTS_RANGE)
    terminals_boolean_array = generate_terminal_boolean_array(NUMBER_OF_ITERATIONS, n, terminal_p)

    for i in range(NUMBER_OF_ITERATIONS):
        edges = Gnp_graphs[i]
        terminals = list(np.where(terminals_boolean_array[i])[0] + 1)

        run_and_compare_results(n, edges, terminals, include_optimization_algorithm=True)


@pytest.mark.parametrize("n", [15, 20])
@pytest.mark.parametrize("edge_p", [0.1, 0.3, 0.4, 0.5, 0.7, 0.9, 1.0])
@pytest.mark.parametrize("terminal_p", [0.1, 0.3])
def test_Gnp_graphs(n: int, edge_p: float, terminal_p: float):
    NUMBER_OF_ITERATIONS = 10
    WEIGHTS_RANGE = (1, 5)

    Gnp_graphs = generate_Gnp_weighted_graphs(n, edge_p, NUMBER_OF_ITERATIONS, WEIGHTS_RANGE)
    terminals_boolean_array = generate_terminal_boolean_array(NUMBER_OF_ITERATIONS, n, terminal_p)

    for i in range(NUMBER_OF_ITERATIONS):
        edges = Gnp_graphs[i]
        terminals = list(np.where(terminals_boolean_array[i])[0] + 1)

        run_and_compare_results(n, edges, terminals, include_optimization_algorithm=False)