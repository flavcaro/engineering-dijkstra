#include "graph.hpp"
#include "dijkstra.hpp"
#include "graph_generators.hpp"
#include <chrono>

#include <iostream>
#include <iomanip>

int main() {

    int n = 10000;
    int m = 50000;

    std::cout << "Generating random graph\n";
    Graph g = random_graph(n, m);

    int source = 0;

    //misurazione del tempo di esecuzione di Dijkstra
    auto start = std::chrono::steady_clock::now();

    DijkstraResult result = dijkstra_binary_heap_lazy(g, source);

    auto end = std::chrono::steady_clock::now();

    double time_ms =
        std::chrono::duration<double, std::milli>(end - start).count();

    std::cout << "Dijkstra runtime: " << time_ms << " ms\n";

    std::cout << "Distance to node 10: " << result.dist[10] << "\n";

    return 0;
}
 