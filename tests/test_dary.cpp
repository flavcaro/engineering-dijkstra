#include "graph/graph.hpp"
#include "dijkstra/dijkstra_binary.hpp"
#include "dijkstra/dijkstra_dary.hpp"

#include <cmath>
#include <iostream>

int main() {
    Graph g = make_graph(5);

    add_directed_edge(g, 0, 1, 2.0);
    add_directed_edge(g, 0, 2, 5.0);
    add_directed_edge(g, 1, 2, 1.0);
    add_directed_edge(g, 1, 3, 2.0);
    add_directed_edge(g, 2, 3, 1.0);
    add_directed_edge(g, 3, 4, 3.0);

    int source = 0;

    DijkstraResult a = dijkstra_binary_heap_lazy(g, source);
    DijkstraResult b = dijkstra_dary_heap(g, source);

    bool ok = true;
    const double EPS = 1e-9;

    for (int i = 0; i < static_cast<int>(g.size()); ++i) {
        std::cout << i << "  " << a.dist[i] << "  " << b.dist[i] << "\n";
        if (std::fabs(a.dist[i] - b.dist[i]) > EPS) {
            ok = false;
        }
    }

    if (ok) {
        std::cout << "[OK] Distances match.\n";
        return 0;
    }

    std::cout << "[ERROR] Distances do not match.\n";
    return 1;
}