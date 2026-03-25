#include "graph/graph.hpp"
#include "graph/graph_generators.hpp"
#include "graph/graph_export.hpp"

#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

int main() {
    std::filesystem::create_directories("data/generated/random_scaling");
    std::filesystem::create_directories("data/generated/random_density");
    std::filesystem::create_directories("data/generated/grid");

    std::cout << "Generating datasets...\n";

    // RANDOM GRAPHS - SCALING
    {
        std::vector<std::pair<int, int>> scaling_cases = {
            {1000, 5000},
            {5000, 25000},
            {10000, 50000},
            {20000, 100000},
            {50000, 250000}
        };

        for (const auto& [n, m] : scaling_cases) {
            std::cout << "Generating random scaling graph: n=" << n
                      << ", m=" << m << "...\n";

            Graph g = random_graph(n, m, 42);

            std::string filename =
                "data/generated/random_scaling/random_n" +
                std::to_string(n) + "_m" +
                std::to_string(m) + ".txt";

            save_graph_to_file(g, filename);
        }
    }

    // RANDOM GRAPHS - DENSITY STUDY
    {
        int n = 10000;
        int seed = 42;

        std::vector<int> edges = {
            20000,
            50000,
            100000,
            200000
        };

        for (int m : edges) {
            std::cout << "Generating random density graph: n=" << n
                      << ", m=" << m << "...\n";

            Graph g = random_graph(n, m, seed);

            std::string filename =
                "data/generated/random_density/random_n" +
                std::to_string(n) + "_m" +
                std::to_string(m) + ".txt";

            save_graph_to_file(g, filename);
        }
    }

    // GRID GRAPHS
    {
        std::vector<std::pair<int, int>> grid_cases = {
            {50, 50},
            {100, 100},
            {300, 300}
        };

        for (const auto& [r, c] : grid_cases) {
            std::cout << "Generating grid graph: " << r << "x" << c << "...\n";
            Graph g = grid_graph(r, c);

            std::string filename =
                "data/generated/grid/grid_" +
                std::to_string(r) + "x" +
                std::to_string(c) + ".txt";

            save_graph_to_file(g, filename);
        }
    }

    std::cout << "Datasets generated.\n";
    return 0;
}