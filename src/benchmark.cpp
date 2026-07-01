#include "graph/graph_generators.hpp"
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
#include <algorithm> // Aggiunto per std::max_element

// Definiamo i casi sperimentali per il benchmark
struct BenchmarkCase {
    std::string family;
    std::string dataset_name;
    int n;
    int attachment;
    int base_seed;
    int source;
};

// Struttura di supporto per configurare sia i test dinamici che da file statici
struct ExtendedCase {
    std::string family;
    std::string dataset_name;
    int n;
    int attachment;
    int base_seed;
    int source;
    std::string file_path; // Se vuoto, indica la generazione al volo
};

static std::size_t directed_edge_count(const Graph& g) {
    std::size_t edges = 0;
    for (const auto& adj : g) {
        edges += adj.size();
    }
    return edges;
}

template <typename Solver>
void run_algorithm(std::ofstream& csv,
                   const BenchmarkCase& test,
                   const Graph& g,
                   const std::string& algorithm,
                   int graph_id,
                   int seed,
                   int runs,
                   Solver&& solver) {
    const std::size_t edge_count = directed_edge_count(g);

    for (int run = 1; run <= runs; ++run) {
        DijkstraStats stats;

        auto start = std::chrono::steady_clock::now();
        DijkstraResult result = solver(g, test.source, &stats);
        auto end = std::chrono::steady_clock::now();

        double time_ms = std::chrono::duration<double, std::milli>(end - start).count();
        double dist_last = result.dist.empty() ? -1.0 : result.dist.back();

        csv << test.family << ","
            << test.dataset_name << ","
            << graph_id << ","
            << g.size() << ","
            << edge_count << ","
            << test.attachment << ","
            << seed << ","
            << algorithm << ","
            << run << ","
            << time_ms << ","
            << dist_last << ","
            << stats.pq_pushes << ","
            << stats.pq_pops << ","
            << stats.decrease_key_calls << ","
            << stats.edge_relax_attempts << ","
            << stats.successful_relaxations << ","
            << stats.stale_entries_discarded << ","
            << stats.settled_nodes << "\n";

        std::cout << "  [graph " << graph_id
                  << "][" << algorithm << "][run " << run << "] time = "
                  << time_ms << " ms"
                  << " | pushes=" << stats.pq_pushes
                  << " pops=" << stats.pq_pops
                  << " decrease=" << stats.decrease_key_calls
                  << " relax=" << stats.edge_relax_attempts
                  << " success=" << stats.successful_relaxations
                  << " stale=" << stats.stale_entries_discarded
                  << " settled=" << stats.settled_nodes
                  << "\n";
    }
}

int main() {
    const int runs = 3;
    const int graphs_per_case = 10;

    std::filesystem::create_directories("results");

    std::vector<ExtendedCase> cases = {
        // =========================================================================
        // FAMIGLIA 1: BARABÁSI-ALBERT (Generati dinamicamente al volo)
        // =========================================================================
        {"ba_scaling", "ba_scaling_n1000_a4", 1000, 4, 1001, 0, ""},
        {"ba_scaling", "ba_scaling_n2000_a4", 2000, 4, 2001, 0, ""},
        {"ba_scaling", "ba_scaling_n4000_a4", 4000, 4, 4001, 0, ""},
        {"ba_scaling", "ba_scaling_n8000_a4", 8000, 4, 8001, 0, ""},
        {"ba_scaling", "ba_scaling_n16000_a4", 16000, 4, 16001, 0, ""},

        // =========================================================================
        // FAMIGLIA 2: RANDOM SCALING & DENSITY (Caricati dai file generati)
        // =========================================================================
        {"rand_scaling", "rand_scale_n1000", 1000, -1, 42, 0, "data/generated/random_scaling/random_n1000_m5000.txt"},
        {"rand_scaling", "rand_scale_n5000", 5000, -1, 42, 0, "data/generated/random_scaling/random_n5000_m25000.txt"},
        {"rand_scaling", "rand_scale_n10000", 10000, -1, 42, 0, "data/generated/random_scaling/random_n10000_m50000.txt"},
        {"rand_scaling", "rand_scale_n20000", 20000, -1, 42, 0, "data/generated/random_scaling/random_n20000_m100000.txt"},
        {"rand_scaling", "rand_scale_n50000", 50000, -1, 42, 0, "data/generated/random_scaling/random_n50000_m250000.txt"},
        
        {"rand_density", "rand_dens_m20000", 10000, -1, 42, 0, "data/generated/random_density/random_n10000_m20000.txt"},
        {"rand_density", "rand_dens_m50000", 10000, -1, 42, 0, "data/generated/random_density/random_n10000_m50000.txt"},
        {"rand_density", "rand_dens_m100000", 10000, -1, 42, 0, "data/generated/random_density/random_n10000_m100000.txt"},
        {"rand_density", "rand_dens_m200000", 10000, -1, 42, 0, "data/generated/random_density/random_n10000_m200000.txt"},

        // =========================================================================
        // FAMIGLIA 3: GRID GRAPHS (Strutture geometriche caricate da file)
        // =========================================================================
        {"grid", "grid_50x50", 2500, -2, 42, 0, "data/generated/grid/grid_50x50.txt"},
        {"grid", "grid_100x100", 10000, -2, 42, 0, "data/generated/grid/grid_100x100.txt"},
        {"grid", "grid_300x300", 90000, -2, 42, 0, "data/generated/grid/grid_300x300.txt"}
    };

    std::ofstream csv("results/benchmark_results.csv");
    if (!csv) {
        std::cerr << "Cannot open results/benchmark_results.csv\n";
        return 1;
    }

    csv << "family,dataset,graph_id,n,directed_edges,attachment,seed,algorithm,run,time_ms,distance_to_last"
        << ",pq_pushes,pq_pops,decrease_key_calls,edge_relax_attempts,successful_relaxations"
        << ",stale_entries_discarded,settled_nodes\n";

    for (const auto& test : cases) {
        int actual_graphs = test.file_path.empty() ? graphs_per_case : 1;

        std::cout << "\n==================================================\n"
                  << "Processing case: " << test.dataset_name << " (Family: " << test.family << ")\n"
                  << "==================================================\n";

        for (int graph_id = 1; graph_id <= actual_graphs; ++graph_id) {
            int seed = test.base_seed + graph_id - 1;
            Graph g;

            if (!test.file_path.empty()) {
                std::cout << "Loading graph from file: " << test.file_path << "...\n";
                g = load_graph_from_file(test.file_path);
            } else {
                std::cout << "Generating dynamic BA graph " << graph_id << "/" << actual_graphs
                          << " (seed=" << seed << ")...\n";
                g = barabasi_albert_graph(test.n, test.attachment, seed);
            }

            std::cout << "  Graph ready: nodes=" << g.size()
                      << " directed_edges=" << directed_edge_count(g) << "\n";

            // Creazione del record standard
            BenchmarkCase core_case{test.family, test.dataset_name, test.n, test.attachment, test.base_seed, test.source};

            // =========================================================================
            // FIX ANALISI: SCELTA DINAMICA DELLA SORGENTE OTTIMALE (MAX OUT-DEGREE)
            // =========================================================================
            int best_source = 0;
            std::size_t max_out_degree = 0;
            for (std::size_t i = 0; i < g.size(); ++i) {
                if (g[i].size() > max_out_degree) {
                    max_out_degree = g[i].size();
                    best_source = static_cast<int>(i);
                }
            }
            core_case.source = best_source; // Aggiorna la sorgente con il vero hub del grafo
            // =========================================================================

            run_algorithm(csv, core_case, g, "binary_heap_lazy", graph_id, seed, runs, dijkstra_binary_heap_lazy);
            run_algorithm(csv, core_case, g, "pairing_heap", graph_id, seed, runs, dijkstra_pairing_heap);
            run_algorithm(csv, core_case, g, "dary_heap", graph_id, seed, runs, dijkstra_dary_heap);
        }
    }

    std::cout << "\nBenchmark completed. Results written to results/benchmark_results.csv\n";
    return 0;
}