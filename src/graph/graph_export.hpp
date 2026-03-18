#pragma once

#include "graph.hpp"

#include <fstream>
#include <stdexcept>
#include <string>

//formato di output:
//n m
//u v w
inline void save_graph_to_file(const Graph& g, const std::string& path) {
    //apri file di output
    std::ofstream out(path);
    if (!out) {
        throw std::runtime_error("Impossibile aprire il file di output: " + path);
    }

    //calcola numero totale di archi m
    //se il grafo è orientato
    // m è la somma delle lunghezze delle liste di adiacenza
    //ogni lista di adiacenza contiene solo gli archi uscenti da un nodo
    long long m = 0;
    for (const auto& adj : g) {
        m += static_cast<long long>(adj.size());
    }

    out << g.size() << " " << m << "\n";

    for (int u = 0; u < static_cast<int>(g.size()); ++u) {
        //per ogni nodo u, visita tutti gli archi della sua lista di adiacenza
        for (const auto& e : g[u]) {
            out << u << " " << e.to << " " << e.w << "\n";
        }
    }
}