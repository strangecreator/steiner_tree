#include <iostream>
#include <algorithm>
#include <stdexcept>
#include <string>

#include "utils.hpp"
#include "graph.hpp"


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

    print_current_time(std::cout);  // means the start of the main algorithm

    // converting into metric task
    MetricGraph metric_graph(graph);
    metric_graph.Convert();

    // finding minimum spanning tree
    MST mst(metric_graph);
    std::vector<Graph::Edge> metric_edges = mst.Find(terminals);

    // checking if terminals are even feasible to each other
    for (const Graph::Edge& edge : metric_edges) {
        if (metric_graph.GetWeight(edge.from, edge.to) == Graph::W_MAX) {
            print_current_time(std::cout);
            std::cout << "NO";
            return 0;
        }
    }

    // expanding our solution from metric back to initial problem
    std::vector<Graph::Edge> edges = ExpandEdgesFromMetricToInitial(metric_graph, metric_edges);

    // printing the result
    print_current_time(std::cout);  // means the end of the main algorithm
    std::cout << "YES\n";

    Graph::W_TYPE sum = 0;
    for (const Graph::Edge& edge : edges) {
        sum += graph.GetWeight(edge.from, edge.to);
    }
    std::cout << sum << '\n';

    for (const Graph::Edge& edge : edges) {
        std::cout << edge.from + 1 << ' ' << edge.to + 1 << '\n';
    }

    std::cout.flush();
}