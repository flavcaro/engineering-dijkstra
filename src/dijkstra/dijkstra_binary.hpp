#pragma once

#include "graph/graph.hpp"

#include <cstddef>
#include <vector>
#include <queue>
#include <limits>
#include <stdexcept>
#include <functional>

/*
    Risultato dell'algoritmo di Dijkstra.

    dist[v]   -> distanza minima dalla sorgente al nodo v
    parent[v] -> predecessore di v nel cammino minimo
*/
struct DijkstraResult {
    std::vector<double> dist;
    std::vector<int> parent;
};

struct DijkstraStats {
    std::size_t pq_pushes = 0;
    std::size_t pq_pops = 0;
    std::size_t decrease_key_calls = 0;
    std::size_t edge_relax_attempts = 0;
    std::size_t successful_relaxations = 0;
    std::size_t stale_entries_discarded = 0;
    std::size_t settled_nodes = 0;
};

// Implementazione "lazy" di Dijkstra con binary heap (priority_queue).
inline DijkstraResult dijkstra_binary_heap_lazy(const Graph& g, int source, DijkstraStats* stats = nullptr) {
    const int n = (int)g.size();

    if (source < 0 || source >= n) {
        throw std::out_of_range("Invalid source node");
    }

    const double INF = std::numeric_limits<double>::infinity();

    //inizializza distanze a infinito e predecessori a -1
    std::vector<double> dist(n, INF);
    std::vector<int> parent(n, -1);

    if (stats) {
        *stats = {};
    }

    using PQItem = std::pair<double, int>; // elemento della priority queue (distance, node)
    //priotiry queue come min-heap basato su distanza
    std::priority_queue<PQItem, std::vector<PQItem>, std::greater<PQItem>> pq;

    dist[source] = 0.0;
    pq.push({0.0, source});
    if (stats) {
        ++stats->pq_pushes;
    }

    while (!pq.empty()) {
        auto [du, u] = pq.top();
        pq.pop();
        if (stats) {
            ++stats->pq_pops;
        }

        // Lazy deletion: scarta entry obsolete
        //succede quando esiste gia una distanza migliore per u rispetto a du
        if (du > dist[u]) {
            if (stats) {
                ++stats->stale_entries_discarded;
            }
            continue;
        }

        if (stats) {
            ++stats->settled_nodes;
        }

        //rilassamento degli archi uscenti da u
        for (const auto& e : g[u]) {
            int v = e.target;
            if (stats) {
                ++stats->edge_relax_attempts;
            }
            double nd = dist[u] + e.weight;

            if (nd < dist[v]) {
                if (stats) {
                    ++stats->successful_relaxations;
                }
                dist[v] = nd;
                parent[v] = u;
                pq.push({nd, v});
                if (stats) {
                    ++stats->pq_pushes;
                }
            }
        }
    }

    return {dist, parent};
}