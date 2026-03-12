#pragma once

#include "graph.hpp"

#include <fstream>
#include <stdexcept>
#include <string>

inline void save_graph_to_file(const Graph& g, const std::string& path) {
    std::ofstream out(path);
    if (!out) {
        throw std::runtime_error("Impossibile aprire il file di output: " + path);
    }

    long long m = 0;
    for (const auto& adj : g) {
        m += static_cast<long long>(adj.size());
    }

    out << g.size() << " " << m << "\n";

    for (int u = 0; u < static_cast<int>(g.size()); ++u) {
        for (const auto& e : g[u]) {
            out << u << " " << e.to << " " << e.w << "\n";
        }
    }
}