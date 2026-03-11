#pragma once

#include "graph.hpp"

#include <random>
#include <set>
#include <utility>
#include <stdexcept>

// Genera un grafo casuale con n nodi e m archi. I pesi degli archi sono reali casuali tra 1.0 e 10.0.
inline Graph random_graph(int n, int m, int seed = 42) {

    if (n <= 0) {
        throw std::invalid_argument("Number of nodes must be positive");
    }

    if (m < 0) {
        throw std::invalid_argument("Number of edges cannot be negative");
    }

    // massimo numero di archi distinti senza self-loop
    if (m > n * (n - 1)) {
        throw std::invalid_argument("Too many edges requested");
    }

    Graph g = make_graph(n);

    std::mt19937 rng(seed);

    std::uniform_int_distribution<int> node_dist(0, n - 1);
    std::uniform_real_distribution<double> weight_dist(1.0, 10.0);

    //set per evitare dulicati (u,v)
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