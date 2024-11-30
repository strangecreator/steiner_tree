import os
import subprocess
from pathlib import Path

import numpy as np

import steiner_tree_exact


BASE_DIR = Path(__file__).parent.parent
EXECUTABLE = {
    "2-approximation": {
        "nt": str(BASE_DIR / "bin/steiner_tree_two_approximation.exe"),
        "posix": str(BASE_DIR / "bin/steiner_tree_two_approximation")
    },
    "dreyfus-wagner": {
        "nt": str(BASE_DIR / "bin/dreyfus_wagner.exe"),
        "posix": str(BASE_DIR / "bin/dreyfus_wagner")
    }
}


def generate_terminal_boolean_array(number_of_iterations: int, n: int, terminal_p: float):
    terminals_boolean_array = np.zeros((number_of_iterations, n), dtype=bool)
    for i in range(number_of_iterations):
        row = np.array([True] * int(n * terminal_p) + [False] * (n - int(n * terminal_p)))
        np.random.shuffle(row)
        terminals_boolean_array[i] = row

    return terminals_boolean_array


def convert_to_input(
        number_of_vertices: int,
        edges: list[tuple[int, int, int]],
        terminals: list[int]
    ):
    edges_string = "\n".join(map(lambda x: ' '.join(map(str, x)), edges))
    return f"list\n{number_of_vertices} {len(edges)}\n{edges_string}\n{len(terminals)}\n{' '.join(map(str, terminals))}"


def parse_response(response: str):
    lines = response.strip().split('\n')

    timestamps = (int(lines[0]), int(lines[1]))

    if lines[2].strip() == "NO":
        return ((False, -1, []), timestamps)
    
    try:
        return ((
            True,
            int(lines[3]),
            list(map(lambda x: list(map(int, x.split())), lines[4:]))
        ), timestamps)
    except Exception as e:
        raise Exception from e  # TODO


def run_executable(
        executable: str,
        number_of_vertices: int,
        edges: list[tuple[int, int, int]],
        terminals: list[int]
    ):
    input_data = convert_to_input(number_of_vertices, edges, terminals)

    process = subprocess.run(
        [executable],
        input=input_data,
        text=True,
        capture_output=True
    )

    if process.returncode != 0:
        raise Exception(process.stderr)
    else:
        return parse_response(process.stdout)


def compute_2_approximation(*args, **kwargs):
    return run_executable(EXECUTABLE["2-approximation"][os.name], *args, **kwargs)


def compute_dreyfus_wagner(*args, **kwargs):
    return run_executable(EXECUTABLE["dreyfus-wagner"][os.name], *args, **kwargs)
    

def is_solutions_match(optimization_solution, dreyfus_wagner_solution, approximate_solution) -> None:
    assert dreyfus_wagner_solution[0][0] == approximate_solution[0][0], "Dreyfus Wagner does not match with the approximate algorithm."
    assert (
        not dreyfus_wagner_solution[0][0] or
        (
            dreyfus_wagner_solution[0][1] <= approximate_solution[0][1] <= 2 * dreyfus_wagner_solution[0][1]
        )
    ), "Approximate algorithm result is not in segment, provided by Dreyfus Wagner."
    
    if optimization_solution is None:
        return
    
    assert optimization_solution[0][0] == approximate_solution[0][0], "Optimization solution does not match with the approximate algorithm."
    assert (
        not optimization_solution[0][0] or
        (
            optimization_solution[0][1] <= approximate_solution[0][1] <= 2 * optimization_solution[0][1]
        )
    ), "Approximate algorithm result is not in segment, provided by optimization algorithm."

    assert optimization_solution[0][0] == dreyfus_wagner_solution[0][0], "Optimization solution does not match with the Dreyfus Wagner."
    assert (
        not optimization_solution[0][0] or
        (
            dreyfus_wagner_solution[0][1] == optimization_solution[0][1]
        )
    ), "Optimization algorithm does not match with the Dreyfus Wagner (by minimum weight)."
    

def run_and_compare_results(
        number_of_vertices: int,
        edges: list[tuple[int, int, int]],
        terminals: list[int],
        include_optimization_algorithm=False
    ):
    if include_optimization_algorithm:
        optimization_solution = steiner_tree_exact.find_steiner_tree(
            number_of_vertices,
            edges,
            terminals
        )
    else:
        optimization_solution = None

    dreyfus_wagner_solution = compute_dreyfus_wagner(
        number_of_vertices,
        edges,
        terminals
    )
    
    approximate_solution = compute_2_approximation(
        number_of_vertices,
        edges,
        terminals
    )

    is_solutions_match(optimization_solution, dreyfus_wagner_solution, approximate_solution)


def boolean_array_to_edges(n: int, edge_array) -> list:
    triu_indices = np.triu_indices(n, k=1)
    edge_pairs = zip(triu_indices[0], triu_indices[1])
    edges = [(i + 1, j + 1) for (i, j), exists in zip(edge_pairs, edge_array) if exists]
    return edges


def generate_Gnp_graphs(n: int, p: float, size: int) -> list:
    number_of_edges = (n * (n - 1)) // 2
    edges_boolean = np.random.choice([True, False], size=(size, number_of_edges), p=[p, 1 - p])

    edges = [boolean_array_to_edges(n, boolean_edge_array) for boolean_edge_array in edges_boolean]
    return edges


def generate_Gnp_weighted_graphs(n: int, p: float, size: int, weight_range: tuple[int, int]) -> list:
    number_of_edges = (n * (n - 1)) // 2
    unweighted_edges = generate_Gnp_graphs(n, p, size)
    weights = np.random.randint(weight_range[0], weight_range[1] + 1, size=(size, number_of_edges))
    return [
        [(u, v, weights[i][j]) for j, (u, v) in enumerate(edges)]
        for i, edges in enumerate(unweighted_edges)
    ]