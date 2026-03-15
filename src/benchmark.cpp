#include "graph.hpp"
#include "graph_io.hpp"
#include "dijkstra.hpp"

#include <chrono>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <cmath>

// Singolo test di benchmark:
// - nome del dataset
// - percorso del file da caricare
// - nodo sorgente da cui eseguire Dijkstra
struct BenchmarkCase {
    std::string dataset_name;
    std::string path;
    int source;
};

int main() {

    // numero di esecuzioni per ogni benchmark
    const int runs = 5;

    // Elenco dei casi di test da eseguire
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

    // intestazione file csv
    csv << "dataset,algorithm,source,time_ms,distance_to_10\n";

    for (const auto& test : cases) {

        std::cout << "Loading " << test.dataset_name << "...\n";

        // carica il grafo dal file specificato nel caso di test
        Graph g = load_graph_from_file(test.path);

        double total_time = 0.0;
        DijkstraResult result;

        // esegue l'algoritmo più volte per ottenere una misura più stabile
        for (int i = 0; i < runs; ++i) {

            auto start = std::chrono::steady_clock::now();

            result = dijkstra_binary_heap_lazy(g, test.source);

            auto end = std::chrono::steady_clock::now();

            total_time +=
                std::chrono::duration<double, std::milli>(end - start).count();
        }

        // tempo medio di esecuzione
        double time_ms = total_time / runs;

        // valore di controllo per verificare la correttezza del risultato
        double dist10 = (g.size() > 10) ? result.dist[10] : -1.0;

        // registra i risultati nel file csv
        csv << test.dataset_name << ","
            << "binary_heap_lazy" << ","
            << test.source << ","
            << time_ms << ","
            << dist10 << "\n";

        std::cout << "  avg time = " << time_ms << " ms\n";
        std::cout << "  dist[10] = " << dist10 << "\n";
    }

    std::cout << "Benchmark completed.\n";
    return 0;
}