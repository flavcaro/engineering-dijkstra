#include "graph/graph_io.hpp"
#include "dijkstra/dijkstra_binary.hpp"
#include "dijkstra/dijkstra_pairing.hpp"
#include "dijkstra/dijkstra_dary.hpp"

#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

struct BenchmarkCase {
    std::string dataset_name;
    std::string path;
    int source;
};

int main() {
    const int runs = 3;

    std::filesystem::create_directories("results");

    std::vector<BenchmarkCase> cases = {
        // Random scaling
        {"random_n1000_m5000", "data/generated/random_scaling/random_n1000_m5000.txt", 0},
        {"random_n5000_m25000", "data/generated/random_scaling/random_n5000_m25000.txt", 0},
        {"random_n10000_m50000", "data/generated/random_scaling/random_n10000_m50000.txt", 0},
        {"random_n20000_m100000", "data/generated/random_scaling/random_n20000_m100000.txt", 0},
        {"random_n50000_m250000", "data/generated/random_scaling/random_n50000_m250000.txt", 0},

        // Random density
        {"random_density_n10000_m20000", "data/generated/random_density/random_n10000_m20000.txt", 0},
        {"random_density_n10000_m50000", "data/generated/random_density/random_n10000_m50000.txt", 0},
        {"random_density_n10000_m100000", "data/generated/random_density/random_n10000_m100000.txt", 0},
        {"random_density_n10000_m200000", "data/generated/random_density/random_n10000_m200000.txt", 0},

        // Grid
        {"grid_50x50", "data/generated/grid/grid_50x50.txt", 0},
        {"grid_100x100", "data/generated/grid/grid_100x100.txt", 0},
        {"grid_300x300", "data/generated/grid/grid_300x300.txt", 0},

        // Real-world
        {"com_youtube", "data/social/com-youtube.ungraph-weighted.txt", 0},
        {"roadNet_CA", "data/road/roadNet-CA-weighted.txt", 0},
    };

    std::ofstream csv("results/benchmark_results.csv");
    if (!csv) {
        std::cerr << "Cannot open results/benchmark_results.csv\n";
        return 1;
    }

    csv << "dataset,algorithm,run,source,time_ms,distance_to_10\n";

    for (const auto& test : cases) {
        std::cout << "Loading " << test.dataset_name << "...\n";

        Graph g = load_graph_from_file(test.path);

        // Binary heap lazy
        for (int run = 1; run <= runs; ++run) {
            auto start = std::chrono::steady_clock::now();
            DijkstraResult result = dijkstra_binary_heap_lazy(g, test.source);
            auto end = std::chrono::steady_clock::now();

            double time_ms =
                std::chrono::duration<double, std::milli>(end - start).count();
            double dist10 = (g.size() > 10) ? result.dist[10] : -1.0;

            csv << test.dataset_name << ","
                << "binary_heap_lazy" << ","
                << run << ","
                << test.source << ","
                << time_ms << ","
                << dist10 << "\n";

            std::cout << "  [binary_heap_lazy][run " << run
                      << "] time = " << time_ms << " ms\n";
        }

        // Pairing heap
        for (int run = 1; run <= runs; ++run) {
            auto start = std::chrono::steady_clock::now();
            DijkstraResult result = dijkstra_pairing_heap(g, test.source);
            auto end = std::chrono::steady_clock::now();

            double time_ms =
                std::chrono::duration<double, std::milli>(end - start).count();
            double dist10 = (g.size() > 10) ? result.dist[10] : -1.0;

            csv << test.dataset_name << ","
                << "pairing_heap" << ","
                << run << ","
                << test.source << ","
                << time_ms << ","
                << dist10 << "\n";

            std::cout << "  [pairing_heap][run " << run
                      << "] time = " << time_ms << " ms\n";
        }

        // 4-ary heap
        for (int run = 1; run <= runs; ++run) {
            auto start = std::chrono::steady_clock::now();
            DijkstraResult result = dijkstra_dary_heap(g, test.source);
            auto end = std::chrono::steady_clock::now();

            double time_ms =
                std::chrono::duration<double, std::milli>(end - start).count();
            double dist10 = (g.size() > 10) ? result.dist[10] : -1.0;

            csv << test.dataset_name << ","
                << "dary_heap" << ","
                << run << ","
                << test.source << ","
                << time_ms << ","
                << dist10 << "\n";

            std::cout << "  [dary_heap][run " << run
                      << "] time = " << time_ms << " ms\n";
        }
    }

    std::cout << "Benchmark completed.\n";
    return 0;
}