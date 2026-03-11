#pragma once

#include "graph.hpp"

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

// Implementazione "lazy" di Dijkstra con binary heap (priority_queue).
inline DijkstraResult dijkstra_binary_heap_lazy(const Graph& g, int source) {
    const int n = (int)g.size();

    if (source < 0 || source >= n) {
        throw std::out_of_range("Invalid source node");
    }

    const double INF = std::numeric_limits<double>::infinity();

    //inizializza distanze a infinito e predecessori a -1
    std::vector<double> dist(n, INF);
    std::vector<int> parent(n, -1);

    using PQItem = std::pair<double, int>; // elemento della priority queue (distance, node)
    //priotiry queue come min-heap basato su distanza
    std::priority_queue<PQItem, std::vector<PQItem>, std::greater<PQItem>> pq;

    dist[source] = 0.0;
    pq.push({0.0, source});

    while (!pq.empty()) {
        auto [du, u] = pq.top();
        pq.pop();

        // Lazy deletion: scarta entry obsolete
        //succede quando esiste gia una distanza migliore per u rispetto a du
        if (du > dist[u]) {
            continue;
        }

        //rilassamento degli archi uscenti da u
        for (const auto& e : g[u]) {
            int v = e.to;
            double nd = dist[u] + e.w;

            if (nd < dist[v]) {
                dist[v] = nd;
                parent[v] = u;
                pq.push({nd, v});
            }
        }
    }

    return {dist, parent};
}