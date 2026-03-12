#include "graph.hpp"
#include "graph_generators.hpp"
#include "graph_export.hpp"

#include <iostream>

int main() {

    std::cout << "Generating datasets...\n";

    // RANDOM GRAPHS
    {
        Graph g = random_graph(1000, 5000, 42);
        save_graph_to_file(g, "data/generated/random/random_n1000_m5000.txt");
    }

    {
        Graph g = random_graph(10000, 50000, 42);
        save_graph_to_file(g, "data/generated/random/random_n10000_m50000.txt");
    }

    {
        Graph g = random_graph(50000, 250000, 42);
        save_graph_to_file(g, "data/generated/random/random_n50000_m250000.txt");
    }

    // GRID GRAPHS
    {
        Graph g = grid_graph(50, 50);
        save_graph_to_file(g, "data/generated/grid/grid_50x50.txt");
    }

    {
        Graph g = grid_graph(100, 100);
        save_graph_to_file(g, "data/generated/grid/grid_100x100.txt");
    }

    {
        Graph g = grid_graph(300, 300);
        save_graph_to_file(g, "data/generated/grid/grid_300x300.txt");
    }

    std::cout << "Datasets generated.\n";

    return 0;
}