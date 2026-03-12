#include "graph.hpp"
#include "graph_io.hpp"
#include "dijkstra.hpp"

#include <chrono>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <cmath>

struct BenchmarkCase {
    std::string dataset_name;
    std::string path;
    int source;
};

int main() {
    std::vector<BenchmarkCase> cases = {
        {"random_n1000_m5000", "data/generated/random/random_n1000_m5000.txt", 0},
        {"random_n10000_m50000", "data/generated/random/random_n10000_m50000.txt", 0},
        {"grid_50x50", "data/generated/grid/grid_50x50.txt", 0},
        {"grid_100x100", "data/generated/grid/grid_100x100.txt", 0}
    };

    std::ofstream csv("results/benchmark_results.csv");
    if (!csv) {
        std::cerr << "Cannot open results/benchmark_results.csv\n";
        return 1;
    }

    csv << "dataset,algorithm,source,time_ms,distance_to_10\n";

    for (const auto& test : cases) {
        std::cout << "Loading " << test.dataset_name << "...\n";
        Graph g = load_graph_from_file(test.path);

        auto start = std::chrono::steady_clock::now();
        DijkstraResult result = dijkstra_binary_heap_lazy(g, test.source);
        auto end = std::chrono::steady_clock::now();

        double time_ms =
            std::chrono::duration<double, std::milli>(end - start).count();

        double dist10 = (g.size() > 10) ? result.dist[10] : -1.0;

        csv << test.dataset_name << ","
            << g.size() << ","
            << "binary_heap_lazy" << ","
            << test.source << ","
            << time_ms << ","
            << dist10 << "\n";

        std::cout << "  time = " << time_ms << " ms\n";
    }

    std::cout << "Benchmark completed.\n";
    return 0;
}