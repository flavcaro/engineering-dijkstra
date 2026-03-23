#include "graph/graph_io.hpp"
#include "dijkstra/dijkstra_binary.hpp"
#include "dijkstra/dijkstra_pairing.hpp"
#include "dijkstra/dijkstra_dary.hpp"

#include <chrono>
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
    const int runs = 5;

    std::vector<BenchmarkCase> cases = {
        {"random_n1000_m5000", "data/generated/random/random_n1000_m5000.txt", 0},
        {"random_n10000_m50000", "data/generated/random/random_n10000_m50000.txt", 0},
        {"grid_50x50", "data/generated/grid/grid_50x50.txt", 0},
        {"grid_100x100", "data/generated/grid/grid_100x100.txt", 0},
        {"com_youtube", "data/social/com-youtube.ungraph-weighted.txt", 0}
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

        // -------------------------------
        // Binary heap lazy
        // -------------------------------
        {
            double total_time = 0.0;
            DijkstraResult result;

            for (int i = 0; i < runs; ++i) {
                auto start = std::chrono::steady_clock::now();
                result = dijkstra_binary_heap_lazy(g, test.source);
                auto end = std::chrono::steady_clock::now();

                total_time +=
                    std::chrono::duration<double, std::milli>(end - start).count();
            }

            double time_ms = total_time / runs;
            double dist10 = (g.size() > 10) ? result.dist[10] : -1.0;

            csv << test.dataset_name << ","
                << "binary_heap_lazy" << ","
                << test.source << ","
                << time_ms << ","
                << dist10 << "\n";

            std::cout << "  [binary_heap_lazy] time = " << time_ms << " ms\n";
            std::cout << "  [binary_heap_lazy] dist[10] = " << dist10 << "\n";
        }

        // -------------------------------
        // Pairing heap
        // -------------------------------
        {
            double total_time = 0.0;
            DijkstraResult result;

            for (int i = 0; i < runs; ++i) {
                auto start = std::chrono::steady_clock::now();
                result = dijkstra_pairing_heap(g, test.source);
                auto end = std::chrono::steady_clock::now();

                total_time +=
                    std::chrono::duration<double, std::milli>(end - start).count();
            }

            double time_ms = total_time / runs;
            double dist10 = (g.size() > 10) ? result.dist[10] : -1.0;

            csv << test.dataset_name << ","
                << "pairing_heap" << ","
                << test.source << ","
                << time_ms << ","
                << dist10 << "\n";

            std::cout << "  [pairing_heap] time = " << time_ms << " ms\n";
            std::cout << "  [pairing_heap] dist[10] = " << dist10 << "\n";
        }

        // -------------------------------
        // 4-ary heap
        // -------------------------------
        {
            double total_time = 0.0;
            DijkstraResult result;

            for (int i = 0; i < runs; ++i) {
                auto start = std::chrono::steady_clock::now();
                result = dijkstra_dary_heap(g, test.source);
                auto end = std::chrono::steady_clock::now();

                total_time +=
                    std::chrono::duration<double, std::milli>(end - start).count();
            }

            double time_ms = total_time / runs;
            double dist10 = (g.size() > 10) ? result.dist[10] : -1.0;

            csv << test.dataset_name << ","
                << "dary_heap" << ","
                << test.source << ","
                << time_ms << ","
                << dist10 << "\n";

            std::cout << "  [dary_heap] time = " << time_ms << " ms\n";
            std::cout << "  [dary_heap] dist[10] = " << dist10 << "\n";
        }
    }

    std::cout << "Benchmark completed.\n";
    return 0;
}