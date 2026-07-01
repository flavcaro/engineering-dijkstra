#pragma once

#include <vector>
#include <stdexcept>

// Arco del grafo con nomi uniformati per i generatori e Dijkstra
struct Edge {
    int target; // Nodo di destinazione (corrisponde a 'to')
    int weight; // Peso dell'arco (corrisponde a 'w')
};

// Grafo rappresentato come lista di adiacenza
using Graph = std::vector<std::vector<Edge>>;

// Grafo con n nodi ognuno con lista di adiacenza vuota
inline Graph make_graph(int n) {
    if (n < 0) {
        throw std::invalid_argument("Number of nodes cannot be negative");
    }
    return Graph(static_cast<size_t>(n));
}

inline void add_directed_edge(Graph& g, int u, int v, int w) {
    if (u < 0 || v < 0 || u >= static_cast<int>(g.size()) || v >= static_cast<int>(g.size())) {
        throw std::out_of_range("Invalid node index");
    }
    if (w < 0) {
        throw std::invalid_argument("Dijkstra requires non-negative weights");
    }
    g[static_cast<size_t>(u)].push_back(Edge{v, w});
}

inline void add_undirected_edge(Graph& g, int u, int v, int w) {
    add_directed_edge(g, u, v, w);
    add_directed_edge(g, v, u, w);
}