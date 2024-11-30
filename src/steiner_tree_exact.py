from datetime import datetime
from pyscipopt import Model, quicksum


def get_current_timestamp():
    return int(datetime.now().timestamp() * 1000)


def find_steiner_tree(
        number_of_vertices: int,
        edges: list[tuple[int, int, int]],
        terminals: list[int]
    ):
    number_of_terminals = len(terminals)

    # checking for boundary cases
    if number_of_terminals <= 1:
        return (True, 0, []), (get_current_timestamp(), get_current_timestamp())
    
    if len(edges) == 0:
        return (False, -1, []), (get_current_timestamp(), get_current_timestamp())
    
    root = terminals[0]

    edges = list(map(lambda x: (min(x[0], x[1]), max(x[0], x[1]), x[2]), edges))

    # building an adjacency list
    adjacency_list = {v: [] for v in range(1, number_of_vertices + 1)}
    for (u, v, w) in edges:
        adjacency_list[u].append(v)
        adjacency_list[v].append(u)


    model = Model("Steiner Tree Finder")

    # reducing verbosity
    model.setIntParam('display/verblevel', 0)
    model.setParam('display/freq', 0)


    timestamp_start = get_current_timestamp()  # means the start of the main algorithm


    x, f = {}, {}
    for (u, v, w) in edges:
        x[(u, v)] = model.addVar(vtype="B")

    for (u, v, w) in edges:
        f[(u, v)] = model.addVar(lb=0, ub=number_of_terminals - 1, vtype="C")
        f[(v, u)] = model.addVar(lb=0, ub=number_of_terminals - 1, vtype="C")

        model.addCons(f[(u, v)] <= x[(min(u, v), max(u, v))] * (number_of_terminals - 1))
        model.addCons(f[(v, u)] <= x[(min(u, v), max(u, v))] * (number_of_terminals - 1))

    for (u, v, w) in edges:
        model.addConsSOS1([f[(u, v)], f[(v, u)]])


    model.setObjective(quicksum(w * x[(u, v)] for (u, v, w) in edges), "minimize")

    
    # nonterminals
    for nonterminal in range(1, number_of_vertices + 1):
        if nonterminal in terminals:
            continue

        inflow = quicksum(f[(u, nonterminal)] for u in adjacency_list[nonterminal])
        outflow = quicksum(f[(nonterminal, u)] for u in adjacency_list[nonterminal])
        model.addCons(inflow - outflow == 0)

    # terminals
    for terminal in terminals[1:]:
        inflow = quicksum(f[(u, terminal)] for u in adjacency_list[terminal])
        outflow = quicksum(f[(terminal, u)] for u in adjacency_list[terminal])
        model.addCons(inflow - outflow == 1)

    # root terminal
    root_inflow = quicksum(f[(u, root)] for u in adjacency_list[root])
    root_outflow = quicksum(f[(root, u)] for u in adjacency_list[root])
    model.addCons(root_inflow == 0)
    model.addCons(root_outflow == number_of_terminals - 1)


    # optimizing
    model.optimize()

    timestamp_end = get_current_timestamp()  # means the end of the main algorithm

    # retrieving the selected edges
    if model.getStatus() == "optimal":
        steiner_edges = []

        weight_of_the_tree = 0

        for (u, v, w) in edges:
            if model.getVal(x[(u, v)]) > 0.5:
                steiner_edges.append((u, v))
                weight_of_the_tree += w
        
        return (True, weight_of_the_tree, steiner_edges), (timestamp_start, timestamp_end)
    else:
        return (False, -1, []), (timestamp_start, timestamp_end)


if __name__ == "__main__":
    edges = [
        (1, 2, 1),
        (2, 3, 1),
        (3, 4, 1),
        (4, 5, 1),
        (5, 1, 1),
    ]
    terminals = [1, 3, 5]

    # finding steiner tree
    (status, weight, steiner_tree_edges), timestamps = find_steiner_tree(5, edges, terminals)

    # printing the result
    if status:
        print(weight, steiner_tree_edges)
    else:
        print("Could not find optimal solution.")    