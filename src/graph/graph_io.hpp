#pragma once

#include "graph/graph.hpp"
#include <fstream>
#include <string>
#include <stdexcept>
#include <vector>
#include <algorithm>
#include <iostream>

inline Graph load_graph_from_file(const std::string& filename) {
    std::ifstream in(filename);
    if (!in) {
        throw std::runtime_error("Failed to open file for reading: " + filename);
    }

    size_t declared_nodes = 0;
    if (!(in >> declared_nodes)) {
        throw std::runtime_error("Invalid file format in: " + filename + " (missing node count)");
    }

    // Struttura temporanea per salvare gli archi prima di sapere il numero reale di nodi
    struct TempEdge { size_t u, v; int w; };
    std::vector<TempEdge> temp_edges;
    
    size_t u = 0, v = 0;
    int w = 0;
    size_t max_index = 0;
    bool uses_node_zero = false;

    // 1. Leggiamo tutti gli archi e analizziamo gli indici usati nel file
    while (in >> u >> v >> w) {
        temp_edges.push_back({u, v, w});
        if (u > max_index) max_index = u;
        if (v > max_index) max_index = v;
        if (u == 0 || v == 0) {
            uses_node_zero = true;
        }
    }

    // 2. Determiniamo se il grafo è 1-based (non usa lo zero e il max_index arriva a declared_nodes)
    bool is_1_based = !uses_node_zero && (max_index == declared_nodes);

    // 3. Calcoliamo la dimensione reale necessaria per il vettore del Grafo
    size_t real_nodes = declared_nodes;
    if (is_1_based) {
        // Se è 1-based, gli indici andranno da 1 a N. Sottraendo 1, entreranno perfettamente in [0, N-1]
        real_nodes = declared_nodes;
    } else {
        // Se è 0-based ma ci sono indici fuori scala, prendiamo il massimo trovato + 1
        real_nodes = std::max(declared_nodes, max_index + 1);
    }

    Graph g(real_nodes);

    // 4. Popoliamo il grafo applicando la correzione solo se necessario
    for (const auto& edge : temp_edges) {
        size_t final_u = edge.u;
        size_t final_v = edge.v;

        if (is_1_based) {
            final_u--;
            final_v--;
        }

        // Controllo di sicurezza finale (non dovrebbe mai fallire ora)
        if (final_u < real_nodes && final_v < real_nodes) {
            g[final_u].push_back(Edge{static_cast<int>(final_v), edge.w});
        }
    }

    std::cout << "  [File Loader] Loaded " << filename << ": declared=" << declared_nodes 
              << ", allocated_nodes=" << real_nodes << " (Detected " << (is_1_based ? "1-based" : "0-based") << ")\n";

    return g;
}