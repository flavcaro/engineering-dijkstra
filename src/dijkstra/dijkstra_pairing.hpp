#pragma once

#include "graph/graph.hpp"
#include "dijkstra/dijkstra_binary.hpp"
#include "dijkstra/pairing_heap.hpp"

#include <limits>
#include <stdexcept>
#include <vector>

inline DijkstraResult dijkstra_pairing_heap(const Graph& g, int source) {
    int n = static_cast<int>(g.size());

    if (source < 0 || source >= n) {
        throw std::invalid_argument("Invalid source node");
    }

    const double INF = std::numeric_limits<double>::infinity();

    std::vector<double> dist(n, INF);
    std::vector<int> parent(n, -1);
    std::vector<bool> visited(n, false);

    using Heap = PairingHeap<double, int>;
    Heap pq;

    std::vector<typename Heap::Handle> handles(n, nullptr);

    dist[source] = 0.0;
    handles[source] = pq.push(0.0, source);

    while (!pq.empty()) {
        auto [du, u] = pq.top();
        pq.pop();
        handles[u] = nullptr;

        if (visited[u]) {
            continue;
        }
        visited[u] = true;

        for (const auto& e : g[u]) {
            int v = e.to;
            double nd = du + e.w;

            if (nd < dist[v]) {
                dist[v] = nd;
                parent[v] = u;

                if (handles[v] == nullptr) {
                    handles[v] = pq.push(nd, v);
                } else {
                    pq.decrease_key(handles[v], nd);
                }
            }
        }
    }

    return {dist, parent};
}