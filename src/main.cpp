#include "graph.hpp"
#include "dijkstra.hpp"
#include "graph_generators.hpp"
#include "graph_export.hpp"

#include <chrono>
#include <iostream>

int main() {
    int n = 10000;
    int m = 50000;
    int seed = 42;

    std::cout << "Generating random graph\n";
    Graph g = random_graph(n, m, seed);

    save_graph_to_file(g, "data/generated/random/random_n10000_m50000_seed42.txt");

    int source = 0;

    auto start = std::chrono::steady_clock::now();
    DijkstraResult result = dijkstra_binary_heap_lazy(g, source);
    auto end = std::chrono::steady_clock::now();

    double time_ms =
        std::chrono::duration<double, std::milli>(end - start).count();

    std::cout << "Dijkstra runtime: " << time_ms << " ms\n";
    std::cout << "Distance to node 10: " << result.dist[10] << "\n";

    return 0;
}