import pytest
import subprocess
from pathlib import Path

import steiner_tree_exact


BASE_DIR = Path(__file__).parent.parent
EXECUTABLE = str(BASE_DIR / "bin/steiner_tree_two_approximation")


def convert_to_input(
        number_of_vertices: int,
        edges: list[tuple[int, int, int]],
        terminals: list[int]
    ):
    edges_string = "\n".join(map(lambda x: ' '.join(map(str, x)), edges))
    return f"list\n{number_of_vertices} {len(edges)}\n{edges_string}\n{len(terminals)}\n{' '.join(map(str, terminals))}"


def parse_response(response: str):
    lines = response.strip().split('\n')

    if lines[0].strip() == "NO":
        return (False, -1, [])
    
    try:
        return (
            True,
            int(lines[1]),
            list(map(lambda x: list(map(int, x.split())), lines[2:]))
        )
    except Exception as e:
        raise Exception from e  # TODO


def compute_2_approximation(
        number_of_vertices: int,
        edges: list[tuple[int, int, int]],
        terminals: list[int]
    ):
    input_data = convert_to_input(number_of_vertices, edges, terminals)

    process = subprocess.run(
        [EXECUTABLE],
        input=input_data,
        text=True,
        capture_output=True
    )

    if process.returncode != 0:
        raise Exception(process.stderr)
    else:
        return parse_response(process.stdout)
    

def run_and_compare_results(
        number_of_vertices: int,
        edges: list[tuple[int, int, int]],
        terminals: list[int]
    ):
    actual_solution = steiner_tree_exact.find_steiner_tree(
        number_of_vertices,
        edges,
        terminals
    )
    approximate_solution = compute_2_approximation(
        number_of_vertices,
        edges,
        terminals
    )

    assert actual_solution[0] == approximate_solution[0]

    if actual_solution[0]:
        assert actual_solution[1] <= approximate_solution[1] <= 2 * actual_solution[1], "Approximation algorithm is incorrect."
    

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

    run_and_compare_results(number_of_vertices, edges, terminals)


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

    run_and_compare_results(number_of_vertices, edges, terminals)


def test_Gnp_graphs():
    pass