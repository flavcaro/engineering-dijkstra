#include "graph/graph.hpp"
#include "dijkstra/dijkstra_binary.hpp"
#include "dijkstra/dijkstra_pairing.hpp"

#include <cmath>
#include <iostream>

int main() {
    // Grafo di test:
    //
    // 0 -> 1 (2)
    // 0 -> 2 (5)
    // 1 -> 2 (1)
    // 1 -> 3 (2)
    // 2 -> 3 (1)
    // 3 -> 4 (3)

    /* Output atteso:
    Distances from source 0:
    node    binary_heap     pairing_heap
    0       0               0
    1       2               2
    2       3               3
    3       4               4
    4       7               7

    [OK] Le distanze coincidono.
    */

    Graph g = make_graph(5);

    add_directed_edge(g, 0, 1, 2.0);
    add_directed_edge(g, 0, 2, 5.0);
    add_directed_edge(g, 1, 2, 1.0);
    add_directed_edge(g, 1, 3, 2.0);
    add_directed_edge(g, 2, 3, 1.0);
    add_directed_edge(g, 3, 4, 3.0);

    int source = 0;

    DijkstraResult res_binary = dijkstra_binary_heap_lazy(g, source);
    DijkstraResult res_pairing = dijkstra_pairing_heap(g, source);

    std::cout << "Distances from source " << source << ":\n";
    std::cout << "node\tbinary_heap\tpairing_heap\n";

    bool ok = true;
    const double EPS = 1e-9;

    for (int i = 0; i < static_cast<int>(g.size()); ++i) {
        std::cout << i << "\t"
                  << res_binary.dist[i] << "\t\t"
                  << res_pairing.dist[i] << "\n";

        if (std::fabs(res_binary.dist[i] - res_pairing.dist[i]) > EPS) {
            ok = false;
        }
    }

    if (ok) {
        std::cout << "\n[OK] Le distanze coincidono.\n";
        return 0;
    } else {
        std::cout << "\n[ERROR] Le distanze non coincidono.\n";
        return 1;
    }
}