#pragma once

#include "graph/graph.hpp"
#include "dijkstra/dijkstra_binary.hpp"
#include "dijkstra/d_ary_heap.hpp"

#include <limits>
#include <stdexcept>
#include <vector>

// Esegue l'algoritmo di Dijkstra usando una coda di priorità basata su 4-ary heap.
// L'heap supporta decrease-key, quindi quando trova un cammino migliore
// aggiorna direttamente la priorità del nodo già presente nella coda.
inline DijkstraResult dijkstra_dary_heap(const Graph& g, int source) {
    int n = static_cast<int>(g.size());

    // Controllo validità del nodo sorgente
    if (source < 0 || source >= n) {
        throw std::invalid_argument("Invalid source node");
    }


    const double INF = std::numeric_limits<double>::infinity();

    // dist[v]   = distanza minima nota da source a v
    // parent[v] = predecessore di v nel cammino minimo
    // visited[v]= true se v è già stato estratto definitivamente dall'heap
    std::vector<double> dist(n, INF);
    std::vector<int> parent(n, -1);
    std::vector<bool> visited(n, false);

    // Heap d-ario con d = 4:
    // - chiave   = distanza
    // - valore   = id del nodo
    using Heap = DAryHeap<double, int, 4>;
    Heap pq;

    // Per ogni nodo mantiene un handle all'elemento nell'heap.
    // Serve per poter chiamare decrease-key in O(log n) senza reinserire duplicati.
    std::vector<typename Heap::Handle> handles(n, nullptr);

    //inizializza la coda con il nodo sorgente
    dist[source] = 0.0;
    handles[source] = pq.push(0.0, source);

    //finché la coda non è vuota
    while (!pq.empty()) {
        // Estrai il nodo con distanza minima corrente
        auto [du, u] = pq.top();
        pq.pop();
        // Segna il nodo come visitato e rimuovi il suo handle dall'heap
        handles[u] = nullptr;

        // Se il nodo è già stato visitato, salta 
        //(può succedere se abbiamo più entry per lo stesso nodo)
        if (visited[u]) {
            continue;
        }
        visited[u] = true;

        // Per ogni arco (u, v) rilassa l'arco
        for (const auto& e : g[u]) {
            int v = e.to;
            // Calcola la distanza del cammino da source a v passando per u
            double nd = du + e.w;

            // Se il cammino trovato è migliore di quello noto finora, aggiorna distanza e padre
            if (nd < dist[v]) {
                dist[v] = nd;
                parent[v] = u;

                // Se v non è ancora in coda all'heap, inseriscilo. 
                // Altrimenti aggiorna la sua chiave.
                if (handles[v] == nullptr) {
                    handles[v] = pq.push(nd, v);
                } else {
                    pq.decrease_key(handles[v], nd);
                }
            }
        }
    }

    return {dist, parent};
}