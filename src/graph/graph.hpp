#pragma once

#include <vector>
#include <stdexcept>

//arco del grafo
struct Edge {
    int to; //nodo di destinazione
    double w; //peso dell'arco, costo per attraversare l'arco da un nodo all'altro
};

//grafo rappresentato come lista di adiacenza
using Graph = std::vector<std::vector<Edge>>;

//grafo con n nodi ognuno con lista di adiacenza vuota
inline Graph make_graph(int n) {
    if (n < 0) {
        throw std::invalid_argument("Number of nodes cannot be negative");
    }
    return Graph(n);
}

inline void add_directed_edge(Graph& g, int u, int v, double w) {
    if (u < 0 || v < 0 || u >= (int)g.size() || v >= (int)g.size()) {
        throw std::out_of_range("Invalid node index");
    }
    if (w < 0) {
        throw std::invalid_argument("Dijkstra requires non-negative weights");
    }
    g[u].push_back({v, w});
}

inline void add_undirected_edge(Graph& g, int u, int v, double w) {
    add_directed_edge(g, u, v, w);
    add_directed_edge(g, v, u, w);
}