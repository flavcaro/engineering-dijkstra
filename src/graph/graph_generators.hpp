#pragma once

#include "graph/graph.hpp"
#include <random>
#include <algorithm>
#include <stdexcept>

// Genera un grafo casuale Erdős-Rényi (G(V, M)) con pesi casuali sugli archi
inline Graph random_graph(int n, int m, unsigned int seed) {
    if (n <= 0) throw std::invalid_argument("Number of nodes must be positive");
    
    Graph g(static_cast<size_t>(n));
    std::mt19937 rng(seed);
    std::uniform_int_distribution<int> weight_dist(1, 100);
    std::uniform_int_distribution<int> node_dist(0, n - 1);

    int edges_created = 0;
    // Sicurezza per evitare loop infiniti se si richiedono troppi archi su un grafo piccolo
    long long max_edges = static_cast<long long>(n) * (n - 1);
    int target_edges = static_cast<int>(std::min(static_cast<long long>(m), max_edges));

    while (edges_created < target_edges) {
        int u = node_dist(rng);
        int v = node_dist(rng);

        if (u != v) {
            // Controlla se l'arco esiste già
            auto it = std::find_if(g[u].begin(), g[u].end(), [v](const Edge& e) {
                return e.target == v;
            });

            if (it == g[u].end()) {
                int w = weight_dist(rng);
                g[u].push_back(Edge{v, w});
                edges_created++;
            }
        }
    }
    return g;
}

// Genera un grafo Scale-Free usando il modello di Barabási-Albert
inline Graph barabasi_albert_graph(int n, int attachment, unsigned int seed) {
    if (n <= 0 || attachment <= 0 || attachment >= n) {
        throw std::invalid_argument("Invalid parameters for Barabasi-Albert graph");
    }

    Graph g(static_cast<size_t>(n));
    std::mt19937 rng(seed);
    std::uniform_int_distribution<int> weight_dist(1, 100);

    // Crea una cricca iniziale (collegamento completo) grande quanto il parametro attachment
    std::vector<int> degrees(static_cast<size_t>(n), 0);
    std::vector<int> repeated_nodes;

    for (int i = 0; i < attachment; ++i) {
        for (int j = i + 1; j < attachment; ++j) {
            int w1 = weight_dist(rng);
            g[i].push_back(Edge{j, w1});
            degrees[i]++;
            degrees[j]++;
            repeated_nodes.push_back(i);
            repeated_nodes.push_back(j);
        }
    }

    // Attaccamento preferenziale per i nodi rimanenti
    for (int source = attachment; source < n; ++source) {
        std::vector<int> targets;
        while (static_cast<int>(targets.size()) < attachment) {
            std::uniform_int_distribution<size_t> dist(0, repeated_nodes.size() - 1);
            int target = repeated_nodes[dist(rng)];

            if (target != source && std::find(targets.begin(), targets.end(), target) == targets.end()) {
                targets.push_back(target);
            }
        }

        for (int target : targets) {
            int w = weight_dist(rng);
            g[source].push_back(Edge{target, w});
            degrees[source]++;
            degrees[target]++;
            repeated_nodes.push_back(source);
            repeated_nodes.push_back(target);
        }
    }

    return g;
}

// Genera un grafo a griglia bidimensionale (Grid Graph) RxC
inline Graph grid_graph(int r, int c) {
    if (r <= 0 || c <= 0) throw std::invalid_argument("Grid dimensions must be positive");

    int n = r * c;
    Graph g(static_cast<size_t>(n));
    
    // Seed fisso interno per i pesi della griglia geometrica per consistenza
    std::mt19937 rng(1337);
    std::uniform_int_distribution<int> weight_dist(1, 100);

    auto get_node_id = [c](int i, int j) { return i * c + j; };

    for (int i = 0; i < r; ++i) {
        for (int j = 0; j < c; ++j) {
            int u = get_node_id(i, j);

            // Vicino Destro
            if (j + 1 < c) {
                int v = get_node_id(i, j + 1);
                g[u].push_back(Edge{v, weight_dist(rng)});
                g[v].push_back(Edge{u, weight_dist(rng)});
            }
            // Vicino Basso
            if (i + 1 < r) {
                int v = get_node_id(i + 1, j);
                g[u].push_back(Edge{v, weight_dist(rng)});
                g[v].push_back(Edge{u, weight_dist(rng)});
            }
        }
    }
    return g;
}