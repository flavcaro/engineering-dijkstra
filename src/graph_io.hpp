#pragma once

#include "graph.hpp"

#include <fstream>
#include <stdexcept>
#include <string>

inline Graph load_graph_from_file(const std::string& path) {
    std::ifstream in(path);
    if (!in) {
        throw std::runtime_error("Cannot open input file: " + path);
    }

    int n;
    long long m;
    in >> n >> m;

    if (!in || n < 0 || m < 0) {
        throw std::runtime_error("Invalid graph file header: " + path);
    }

    Graph g = make_graph(n);

    for (long long i = 0; i < m; ++i) {
        int u, v;
        double w;
        in >> u >> v >> w;

        if (!in) {
            throw std::runtime_error("Invalid edge data in file: " + path);
        }

        add_directed_edge(g, u, v, w);
    }

    return g;
}