#pragma once

#include "graph/graph.hpp"
#include <fstream>
#include <string>
#include <stdexcept>

inline void save_graph_to_file(const Graph& g, const std::string& filename) {
    std::ofstream out(filename);
    if (!out) {
        throw std::runtime_error("Failed to open file for writing: " + filename);
    }

    // Scrive il numero di nodi totali nella prima riga
    out << g.size() << "\n";

    // Scrive la lista di adiacenza nel formato: sorgente destinazione peso
    for (size_t u = 0; u < g.size(); ++u) {
        for (const auto& edge : g[u]) {
            out << u << " " << edge.target << " " << edge.weight << "\n";
        }
    }
}