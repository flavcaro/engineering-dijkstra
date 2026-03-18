#pragma once

#include "graph.hpp"

#include <random>
#include <set>
#include <utility>
#include <stdexcept>

// Genera un grafo casuale con n nodi e m archi. 
// I pesi degli archi sono reali casuali tra 1.0 e 10.0.
// il seed è il valore iniziale per il generatore di numeri casuali (default 42)
// Usando lo stesso seed si ottiene sempre lo stesso grafo,
// utile per rendere riproducibili i benchmark.
inline Graph random_graph(int n, int m, int seed = 42) {

    if (n <= 0) {
        throw std::invalid_argument("Number of nodes must be positive");
    }

    if (m < 0) {
        throw std::invalid_argument("Number of edges cannot be negative");
    }

    // massimo numero di archi distinti senza self-loop
    long long max_edges = 1LL * n * (n - 1);
    if (m > max_edges) {
        throw std::invalid_argument("Too many edges requested");
    }

    Graph g = make_graph(n);

    std::mt19937 rng(seed);

    std::uniform_int_distribution<int> node_dist(0, n - 1);
    std::uniform_real_distribution<double> weight_dist(1.0, 10.0);

    // std::set viene usato per evitare archi duplicati.
    // Garantisce unicità ma ha costo O(log m).
    // Per grafi molto grandi si potrebbe usare std::unordered_set
    // per ottenere inserimenti medi O(1).
    std::set<std::pair<int,int>> used_edges;

    while ((int)used_edges.size() < m) {

        int u = node_dist(rng);
        int v = node_dist(rng);

        if (u == v) {
            continue; // evita self-loop
        }

        if (!used_edges.insert({u,v}).second) {
            continue; // duplicato
        }

        double w = weight_dist(rng);

        add_directed_edge(g, u, v, w);
    }

    return g;
}

// Genera una griglia rows x cols.
// Ogni nodo è collegato ai vicini destra e sotto (grafo non orientato)
inline Graph grid_graph(int rows, int cols, double weight = 1.0) {

    int n = rows * cols;
    Graph g = make_graph(n);

    auto node_id = [cols](int r, int c) {
        return r * cols + c;
    };

    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {

            int u = node_id(r, c);

            if (r + 1 < rows) {
                int v = node_id(r + 1, c);
                add_undirected_edge(g, u, v, weight);
            }

            if (c + 1 < cols) {
                int v = node_id(r, c + 1);
                add_undirected_edge(g, u, v, weight);
            }
        }
    }

    return g;
}