#include <iostream>
#include <algorithm>
#include <climits>
#include <vector>
#include <stdexcept>

#include "binary_heap.hpp"


class Graph {
  public:
    using W_TYPE = long long;
    static const W_TYPE W_MAX = LLONG_MAX;

    struct Edge {
        int from, to;

        bool operator==(const Edge& other) const {
            return from == other.from && to == other.to;
        }

        bool operator<(const Edge& other) const {
            return from < other.from || (from == other.from && to < other.to);
        }
    };

  protected:
    int size = 0;
    W_TYPE* adjacency_matrix = nullptr;

  public:
    Graph() {}

    ~Graph() {
        delete[] adjacency_matrix;
    }

    void ReadFromMatrix(std::istream& stream) {
        stream >> size;

        adjacency_matrix = new W_TYPE[size * size];

        for (int i = 0; i < size * size; ++i) {
            stream >> adjacency_matrix[i];
        }

        for (int i = 0; i < size; ++i) {
            SetWeight(i, i, 0);
        }
    }

    void ReadFromList(std::istream& stream) {
        int number_of_edges;
        stream >> size >> number_of_edges;

        adjacency_matrix = new W_TYPE[size * size];

        for (int i = 0; i < size * size; ++i) {
            adjacency_matrix[i] = W_MAX;
        }

        while (number_of_edges-- > 0) {
            int from, to;
            W_TYPE weight;
            stream >> from >> to >> weight;  // 1-indexation

            SetWeight(from - 1, to - 1, weight);
            SetWeight(to - 1, from - 1, weight);
        }

        for (int i = 0; i < size; ++i) {
            SetWeight(i, i, 0);
        }
    }

    int Size() const { return size; }

    W_TYPE GetWeight(int i, int j) const {
        return adjacency_matrix[i * size + j];
    }

    void SetWeight(int i, int j, W_TYPE weight) {
        adjacency_matrix[i * size + j] = weight;
    }

    void Print(std::ostream& stream) const {
        if (size == 0) {
            stream << "Graph is empty." << '\n';
            return;
        }

        stream << "Adjacency Matrix (" << size << 'x' << size << "):" << '\n';
        for (int i = 0; i < size; ++i) {
            for (int j = 0; j < size; ++j) {
                stream << GetWeight(i, j) << ' ';
            }
            stream << '\n';
        }
    }

    friend class MetricGraph;
};


class MetricGraph : public Graph {
  protected:
    const Graph& graph;
    int* next_node = nullptr;

    int UpdateNextNode(int* intermediate_node, int i, int j) {  // returns the next node in the shortest path between i and j
        if (next_node[i * size + j] != -1) { return next_node[i * size + j]; }
        int k = intermediate_node[i * size + j];
        if (k == -1) {
            next_node[i * size + j] = j;
        } else {
            next_node[i * size + j] = UpdateNextNode(intermediate_node, i, k);
        }
        return next_node[i * size + j];
    }

  public:
    MetricGraph(const Graph& graph) : graph(graph) {
        size = graph.size;
        adjacency_matrix = new W_TYPE[graph.size * graph.size];
        next_node = new int[graph.size * graph.size];

        // copying adjacency matrix
        for (int i = 0; i < size * size; ++i) {
            adjacency_matrix[i] = graph.adjacency_matrix[i];
        }
    }

    ~MetricGraph() {
        delete[] next_node;
    }

    void Convert() {
        // creating a routes table
        int* intermediate_node = new int[size * size];

        for (int i = 0; i < size * size; ++i) {
            next_node[i] = -1;
            intermediate_node[i] = -1;
        }

        // Floyd's algorithm
        for (int k = 0; k < size; ++k) {
            for (int i = 0; i < size; ++i) {
                for (int j = 0; j < size; ++j) {
                    W_TYPE new_weight;

                    if (GetWeight(i, k) ==  W_MAX || GetWeight(k, j) == W_MAX) {
                        continue;
                    }

                    new_weight = GetWeight(i, k) + GetWeight(k, j);

                    if (new_weight < GetWeight(i, j)) {
                        SetWeight(i, j, new_weight);

                        // updating a route
                        intermediate_node[i * size + j] = k;
                    }
                }
            }
        }

        // calculating the next node in the shortest path between each two nodes
        for (int i = 0; i < size; ++i) {
            for (int j = 0; j < size; ++j) {
                UpdateNextNode(intermediate_node, i, j);
            }
        }

        delete[] intermediate_node;
    }

    friend class MST;
    friend void AddEdgesBetweenTwoNodes(int from, int to, const MetricGraph& graph, std::vector<Graph::Edge>& edges);
    friend std::vector<Graph::Edge> ExpandEdgesFromMetricToInitial(const MetricGraph& graph, const std::vector<Graph::Edge>& edges);
};


class MST {  // Minimum Spanning Tree
  protected:
    using BH = BinaryHeap<Graph::W_TYPE>;
    using HN = BH::HeapNode;

    const Graph& graph;

  public:
    MST(const Graph& graph) : graph(graph) {}

    std::vector<Graph::Edge> Find(const std::vector<int>& terminals) {  // V_0 as an argument
        // Prim's algorithm
        BH heap;
        std::vector<Graph::Edge> edges;

        int m = terminals.size();
        if (m <= 1) { return edges; }

        int* vertices_map = new int[m - 1];  // vertex argmin of edge weight

        // initializing
        for (int i = 0; i < m - 1; ++i) {
            heap.Add(Graph::W_MAX);
        }
        for (int j = 0; j < m - 1; ++j) {
            heap.ChangeById(j, graph.GetWeight(terminals[j], terminals[m - 1]));
            vertices_map[j] = m - 1;
        }

        // adding edges one by one
        for (int k = 0; k < m - 1; ++k) {
            HN node = heap.Pop();
            edges.push_back(Graph::Edge{terminals[node.id], terminals[vertices_map[node.id]]});
            vertices_map[node.id] = -1;  // means deleted, already taken into tree

            // applying changes
            for (int j = 0; j < m - 1; ++j) {
                if (vertices_map[j] == -1) { continue; }

                Graph::W_TYPE current_min_edge = graph.GetWeight(terminals[j], terminals[vertices_map[j]]);
                Graph::W_TYPE new_min_edge = graph.GetWeight(terminals[j], terminals[node.id]);

                if (new_min_edge < current_min_edge) {
                    heap.ChangeById(j, new_min_edge);
                    vertices_map[j] = node.id;
                }
            }
        }

        delete[] vertices_map;

        return edges;
    }
};


void AddEdgesBetweenTwoNodes(int from, int to, const MetricGraph& graph, std::vector<Graph::Edge>& edges) {
    while (from != to) {
        int next_vertex = graph.next_node[from * graph.Size() + to];
        edges.push_back(Graph::Edge{from, next_vertex});

        from = next_vertex;
    }
}


std::vector<Graph::Edge> ExpandEdgesFromMetricToInitial(const MetricGraph& graph, const std::vector<Graph::Edge>& edges) {
    std::vector<Graph::Edge> result_edges;

    for (const Graph::Edge& edge : edges) {
        AddEdgesBetweenTwoNodes(edge.from, edge.to, graph, result_edges);
    }

    // removing repetitions
    std::sort(result_edges.begin(), result_edges.end());
    auto it = std::unique(result_edges.begin(), result_edges.end());
    result_edges.erase(it, result_edges.end());

    return result_edges;
}