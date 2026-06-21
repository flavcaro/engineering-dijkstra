#pragma once

#include "graph.hpp"

#include <algorithm>
#include <random>
#include <set>
#include <unordered_set>
#include <utility>
#include <stdexcept>

// Genera un grafo casuale basato sul modello Erdős-Rényi (G(V,M)). 
// n nodi, m archi e i pesi degli archi sono reali casuali tra 1.0 e 10.0.
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

    // massimo numero di archi orientati distinti in un grafo semplice senza self-loop
    long long max_edges = 1LL * n * (n - 1);
    if (m > max_edges) {
        throw std::invalid_argument("Too many edges requested");
    }

    Graph g = make_graph(n);

    //generatore Marsenne Twister con seed specificato
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

        //.second restituisce true se l'inserimento è avvenuto, false se l'elemento era già presente
        if (!used_edges.insert({u,v}).second) {
            continue; // duplicato
        }

        double w = weight_dist(rng);

        add_directed_edge(g, u, v, w);
    }

    return g;
}

// Genera un grafo Barabasi-Albert non orientato con pesi casuali sugli archi.
// Ogni nuovo nodo si collega a `attachment` (num archi) nodi esistenti scelti con
// probabilita proporzionale al grado corrente.
inline Graph barabasi_albert_graph(int n, int attachment, int seed = 42, int initial_nodes = -1) {

    if (n <= 0) {
        throw std::invalid_argument("Number of nodes must be positive");
    }

    if (attachment <= 0) {
        throw std::invalid_argument("Attachment parameter must be positive");
    }

    //dimensione del nucleo iniziale
    if (initial_nodes < 0) {
        initial_nodes = std::max(2, attachment + 1);
    }

    if (initial_nodes > n) {
        throw std::invalid_argument("Initial node count cannot exceed total nodes");
    }

    if (attachment >= initial_nodes) {
        throw std::invalid_argument("Attachment parameter must be smaller than initial node count");
    }

    Graph g = make_graph(n);

    std::mt19937 rng(seed);
    std::uniform_real_distribution<double> weight_dist(1.0, 10.0);

    // `targets` contiene ogni nodo tante volte quanto il suo grado corrente.
    // Questo permette di campionare con probabilita proporzionale al grado.
    std::vector<int> targets;
    targets.reserve(static_cast<std::size_t>(2 * attachment * n));

    // Nucleo iniziale: grafo completo sui primi `initial_nodes` nodi.
    for (int u = 0; u < initial_nodes; ++u) {
        for (int v = u + 1; v < initial_nodes; ++v) {
            double w = weight_dist(rng);
            add_undirected_edge(g, u, v, w);
            targets.push_back(u);
            targets.push_back(v);
        }
    }

    std::uniform_int_distribution<std::size_t> pick_target;

    for (int new_node = initial_nodes; new_node < n; ++new_node) {
        std::unordered_set<int> chosen;
        chosen.reserve(static_cast<std::size_t>(attachment) * 2);

        while (static_cast<int>(chosen.size()) < attachment) {
            pick_target = std::uniform_int_distribution<std::size_t>(0, targets.size() - 1);
            int candidate = targets[pick_target(rng)];
            if (candidate == new_node) {
                continue;
            }
            chosen.insert(candidate);
        }

        for (int target : chosen) {
            double w = weight_dist(rng);
            add_undirected_edge(g, new_node, target, w);
            targets.push_back(new_node);
            targets.push_back(target);
        }
    }

    return g;
}

// Genera una griglia rows x cols.
// Ogni nodo è collegato ai vicini destra e sotto (grafo non orientato)
inline Graph grid_graph(int rows, int cols, double weight = 1.0) {

    int n = rows * cols;
    Graph g = make_graph(n);

    // Funzione lambda per calcolare l'ID del nodo in base alla riga e colonna
    auto node_id = [cols](int r, int c) {
        return r * cols + c;
    };

    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {

            int u = node_id(r, c);

            // Collega il nodo corrente al vicino sotto (se esiste) e a destra (se esiste)
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