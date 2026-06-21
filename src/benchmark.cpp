#include "graph/graph_generators.hpp"
#include "dijkstra/dijkstra_binary.hpp"
#include "dijkstra/dijkstra_pairing.hpp"
#include "dijkstra/dijkstra_dary.hpp"

#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

// Definiamo i casi sperimentali per il benchmark
struct BenchmarkCase {
    // Nome della famiglia di grafi (es. "ba_scaling", "ba_density")
    std::string family;
    std::string dataset_name;
    // Numero di nodi nel grafo
    int n;
    // Numero di archi da attaccare per ogni nuovo nodo (per BA)
    int attachment;
    // Seed iniziale per la generazione del grafo
    int base_seed;
    // Nodo sorgente per l'algoritmo di Dijkstra
    int source;
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
    //per ogni configurazione genero 10 grafi diversi 
    //e per ognuno eseguo 3 volte ogni algoritmo
    const int runs = 3;
    const int graphs_per_case = 10;

    std::filesystem::create_directories("results");

    std::vector<BenchmarkCase> cases = {
        // BA scaling: n cresce, attachment fisso a 4
        {"ba_scaling", "ba_scaling_n1000_a4", 1000, 4, 1001, 0},
        {"ba_scaling", "ba_scaling_n2000_a4", 2000, 4, 2001, 0},
        {"ba_scaling", "ba_scaling_n3000_a4", 3000, 4, 3001, 0},
        {"ba_scaling", "ba_scaling_n4000_a4", 4000, 4, 4001, 0},
        {"ba_scaling", "ba_scaling_n5000_a4", 5000, 4, 5001, 0},
        {"ba_scaling", "ba_scaling_n6000_a4", 6000, 4, 6001, 0},
        {"ba_scaling", "ba_scaling_n7000_a4", 7000, 4, 7001, 0},
        {"ba_scaling", "ba_scaling_n8000_a4", 8000, 4, 8001, 0},
        {"ba_scaling", "ba_scaling_n9000_a4", 9000, 4, 9001, 0},
        {"ba_scaling", "ba_scaling_n10000_a4", 10000, 4, 10001, 0},

        // BA density: n fisso, attachment raddoppia.
        {"ba_density", "ba_density_n10000_a2", 10000, 2, 20001, 0},
        {"ba_density", "ba_density_n10000_a4", 10000, 4, 30001, 0},
        {"ba_density", "ba_density_n10000_a8", 10000, 8, 40001, 0},
        {"ba_density", "ba_density_n10000_a16", 10000, 16, 50001, 0},
        {"ba_density", "ba_density_n10000_a32", 10000, 32, 60001, 0},
    };

    std::ofstream csv("results/benchmark_results.csv");
    if (!csv) {
        std::cerr << "Cannot open results/benchmark_results.csv\n";
        return 1;
    }

    csv << "family,dataset,graph_id,n,directed_edges,attachment,seed,algorithm,run,time_ms,distance_to_last"
        << ",pq_pushes,pq_pops,decrease_key_calls,edge_relax_attempts,successful_relaxations"
        << ",stale_entries_discarded,settled_nodes\n";

    //per ogni configurazione genero 10 grafi diversi
    // e per ognuno eseguo 3 volte ogni algoritmo
    for (const auto& test : cases) {
        std::cout << "Case " << test.dataset_name
                  << " (n=" << test.n
                  << ", attachment=" << test.attachment
                  << ", graphs=" << graphs_per_case
                  << ", runs=" << runs << ")\n";

        for (int graph_id = 1; graph_id <= graphs_per_case; ++graph_id) {
            int seed = test.base_seed + graph_id - 1;

            std::cout << "Generating graph " << graph_id << "/" << graphs_per_case
                      << " (seed=" << seed << ")...\n";

            Graph g = barabasi_albert_graph(test.n, test.attachment, seed);

            std::cout << "  Graph ready: nodes=" << g.size()
                      << " directed_edges=" << directed_edge_count(g) << "\n";

            run_algorithm(csv, test, g, "binary_heap_lazy", graph_id, seed, runs, dijkstra_binary_heap_lazy);
            run_algorithm(csv, test, g, "pairing_heap", graph_id, seed, runs, dijkstra_pairing_heap);
            run_algorithm(csv, test, g, "dary_heap", graph_id, seed, runs, dijkstra_dary_heap);
        }
    }

    std::cout << "Benchmark completed. Results written to results/benchmark_results.csv\n";
    return 0;
}