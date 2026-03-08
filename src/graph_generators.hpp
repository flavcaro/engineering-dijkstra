#pragma once

#include "graph.hpp"

#include <random>

inline Graph random_graph(int n, int m, int seed = 42) {
    Graph g = make_graph(n);

    std::mt19937 rng(seed);
    std::uniform_int_distribution<int> node_dist(0, n - 1);
    std::uniform_real_distribution<double> weight_dist(1.0, 10.0);

    for (int i = 0; i < m; ++i) {
        int u = node_dist(rng);
        int v = node_dist(rng);

        if (u == v) continue;

        double w = weight_dist(rng);

        add_directed_edge(g, u, v, w);
    }

    return g;
}