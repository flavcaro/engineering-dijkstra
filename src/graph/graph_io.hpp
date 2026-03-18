#pragma once

#include "graph.hpp"

#include <fstream>
#include <stdexcept>
#include <string>

//carica un grafo da un file di testo. Il formato del file è:
//n m
//u v w
//...
//dove n è il numero di nodi, m è il numero di archi
//e ogni riga successiva rappresenta un arco diretto da u a v con peso w.
inline Graph load_graph_from_file(const std::string& path) {
    std::ifstream in(path);
    if (!in) {
        throw std::runtime_error("Cannot open input file: " + path);
    }

    int n;
    long long m;
    //legge n e m dalla prima riga del file: intestazione del file
    in >> n >> m;

    if (!in || n < 0 || m < 0) {
        throw std::runtime_error("Invalid graph file header: " + path);
    }

    //crea un grafo con n nodi e nessun arco
    Graph g = make_graph(n);

    //legge m archi dal file e aggiungili al grafo con peso w
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