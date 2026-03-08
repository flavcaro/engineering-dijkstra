#pragma once

#include "graph.hpp"

#include <vector>
#include <queue>
#include <limits>
#include <stdexcept>
#include <functional>

struct DijkstraResult {
    std::vector<double> dist;
    std::vector<int> parent;
};

inline DijkstraResult dijkstra_binary_heap_lazy(const Graph& g, int source) {
    const int n = (int)g.size();

    if (source < 0 || source >= n) {
        throw std::out_of_range("Invalid source node");
    }

    const double INF = std::numeric_limits<double>::infinity();

    std::vector<double> dist(n, INF);
    std::vector<int> parent(n, -1);

    using PQItem = std::pair<double, int>; // (distance, node)
    std::priority_queue<PQItem, std::vector<PQItem>, std::greater<PQItem>> pq;

    dist[source] = 0.0;
    pq.push({0.0, source});

    while (!pq.empty()) {
        auto [du, u] = pq.top();
        pq.pop();

        // Lazy deletion: scarta entry obsolete
        if (du > dist[u]) {
            continue;
        }

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