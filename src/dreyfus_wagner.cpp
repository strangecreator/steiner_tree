#include <iostream>
#include <algorithm>
#include <stdexcept>
#include <string>
#include <vector>

#include <unordered_map>
#include <functional>
#include <utility>
#include <bitset>

#include "utils.hpp"
#include "graph.hpp"


/*
ONLY UP TO 32 TERMINALS ARE AVAILABLE
*/


struct Key {
    uint32_t s;     // subset of terminals
    uint32_t v;          // index of a vertex v

    bool operator==(const Key& other) const {
        return s == other.s && v == other.v;
    }
};


namespace std {
    template <>
    struct hash<Key> {
        std::size_t operator()(const Key& key) const {
            uint64_t combined = (static_cast<uint64_t>(key.s) << 32) | key.v;
            return std::hash<uint64_t>{}(combined);
        }
    };
}


struct Value {
    Graph::W_TYPE weight;  // total weight for making these nodes feasible to each other

    // for answer backtracking
    uint32_t s1;
    uint32_t s2;
    int u;
};

void _BacktrackEdges(
    int number_of_terminals,
    std::unordered_map<Key, Value>& map,
    const MetricGraph& metric_graph,
    std::vector<Graph::Edge>& edges,
    const std::vector<int>& terminals,
    const std::vector<int>& terminals_map,
    uint32_t s,
    uint32_t v
) {
    if (s == 0) { return; }

    // checking if v is already in s
    int is_terminal = terminals_map[v];  // gives an index of this terminal (if it's terminal)
    if (is_terminal != -1 && (s & (1 << static_cast<uint32_t>(is_terminal)))) {
        uint32_t s_previous = s & ~(1 << static_cast<uint32_t>(is_terminal));
        return _BacktrackEdges(
            number_of_terminals,
            map,
            metric_graph,
            edges,
            terminals,
            terminals_map,
            s_previous,
            v
        );
    }

    Value value = map[Key{s, v}];

    if (value.s1 == 0 && value.s2 == 0) {  // means s consists with one terminal vertex
        AddEdgesBetweenTwoNodes(value.u, v, metric_graph, edges);
        return;
    }

    AddEdgesBetweenTwoNodes(value.u, v, metric_graph, edges);

    _BacktrackEdges(
        number_of_terminals,
        map,
        metric_graph,
        edges,
        terminals,
        terminals_map,
        value.s1,
        value.u
    );

    return _BacktrackEdges(
        number_of_terminals,
        map,
        metric_graph,
        edges,
        terminals,
        terminals_map,
        value.s2,
        value.u
    );
}


std::vector<Graph::Edge> BacktrackEdges(
    int number_of_terminals,
    std::unordered_map<Key, Value>& map,
    const MetricGraph& metric_graph,
    const std::vector<int>& terminals,
    const std::vector<int>& terminals_map
) {
    std::vector<Graph::Edge> edges;

    uint32_t s = (1 << static_cast<uint64_t>(number_of_terminals)) - 1U;
    uint32_t v = static_cast<uint32_t>(terminals[0]);

    _BacktrackEdges(number_of_terminals, map, metric_graph, edges, terminals, terminals_map, s, v);
    return edges;
}


int main() {
    // speeding up
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);
    std::cout.tie(nullptr);

    // initialization
    std::string mode;
    std::cin >> mode;

    Graph graph;

    // reading graph
    if (mode == "list") {
        graph.ReadFromList(std::cin);
    } else if (mode == "matrix") {
        graph.ReadFromMatrix(std::cin);
    } else {
        throw std::runtime_error("Invalid mode argument!");
    }

    // reading all terminal vertices
    int number_of_terminals;
    std::cin >> number_of_terminals;

    if (number_of_terminals <= 1) {
        print_current_time(std::cout);
        print_current_time(std::cout);
        std::cout << "YES\n0";
        return 0;
    }

    std::vector<int> terminals(number_of_terminals, 0);
    for (int i = 0; i < number_of_terminals; ++i) {
        std::cin >> terminals[i];  // 1-indexation
        --terminals[i];
    }

    std::vector<int> terminals_map(graph.Size(), -1);
    for (int i = 0; i < number_of_terminals; ++i) {
        terminals_map[terminals[i]] = i;
    }

    print_current_time(std::cout);  // means the start of the main algorithm

    // converting into metric task
    MetricGraph metric_graph(graph);
    metric_graph.Convert();

    // dynamic programming
    std::unordered_map<Key, Value> map;

    // base
    for (uint32_t i = 0; i < static_cast<uint32_t>(number_of_terminals); ++i) {
        for (uint32_t v = 0; v < static_cast<uint32_t>(graph.Size()); ++v) {
            // S1=0, S2=0 is a special case when |S| = 1 or when total_weight=W_MAX
            map[Key{(1U << i), v}] = Value{metric_graph.GetWeight(terminals[i], v), 0, 0, terminals[i]};
        }
    }

    // transition
    for (uint32_t s = 1U; s < (1U << static_cast<uint64_t>(number_of_terminals)); ++s) {
        for (uint32_t v = 0U; v < static_cast<uint32_t>(graph.Size()); ++v) {
            Key key{s, v};
            // checking if (s, v) is already in map
            if (map.count(key) != 0) { continue; }

            // checking if v is already in s
            int is_terminal = terminals_map[v];  // gives an index of this terminal (if it's terminal)
            if (is_terminal != -1 && (s & (1 << static_cast<uint32_t>(is_terminal)))) {
                uint32_t s_previous = s & ~(1 << static_cast<uint32_t>(is_terminal));
                map[key] = map[Key{s_previous, v}];
                continue;
            }

            Graph::W_TYPE current_total_weight = Graph::W_MAX;
            uint32_t s1_current = 0U;
            uint32_t s2_current = 0U;
            int u_current = -1;

            for (uint32_t s1 = s; s1; s1 = (s1 - 1) & s) {
                uint32_t s2 = s ^ s1;

                // ensuring that these subsets are non-empty
                if (s1 && s2) {
                    for (uint32_t u = 0; u < static_cast<uint32_t>(graph.Size()); ++u) {
                        if (metric_graph.GetWeight(u, v) == Graph::W_MAX) { continue; }

                        Value s1_value = map[Key{s1, u}];
                        Value s2_value = map[Key{s2, u}];

                        if (s1_value.weight == Graph::W_MAX || s2_value.weight == Graph::W_MAX) { continue; }

                        Graph::W_TYPE new_total_weight = s1_value.weight + s2_value.weight + metric_graph.GetWeight(u, v);

                        if (new_total_weight < current_total_weight) {
                            current_total_weight = new_total_weight;
                            s1_current = s1;
                            s2_current = s2;
                            u_current = static_cast<int>(u);
                        }
                    }
                }
            }

            map[key] = Value{current_total_weight, s1_current, s2_current, u_current};
        }
    }

    // printing the result
    print_current_time(std::cout);  // means the end of the main algorithm

    uint32_t s = (1 << static_cast<uint64_t>(number_of_terminals)) - 1U;
    uint32_t v = static_cast<uint32_t>(terminals[0]);
    Graph::W_TYPE sum = map[Key{s, v}].weight;

    if (sum == Graph::W_MAX) {
        std::cout << "NO\n";
        return 0;
    }

    std::cout << "YES\n" << sum << '\n';

    std::vector<Graph::Edge> edges = BacktrackEdges(
        number_of_terminals,
        map,
        metric_graph,
        terminals,
        terminals_map
    );

    for (const Graph::Edge& edge : edges) {
        std::cout << edge.from + 1 << ' ' << edge.to + 1 << '\n';
    }

    std::cout.flush();
}