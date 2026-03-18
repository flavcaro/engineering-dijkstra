#pragma once

#include "graph/graph.hpp"
#include "dijkstra/dijkstra_binary.hpp"
#include "dijkstra/pairing_heap.hpp"

#include <limits>
#include <stdexcept>
#include <vector>

// Implementazione di Dijkstra utilizzando un pairing heap.
// A differenza della versione con binary heap "lazy",
// qui viene utilizzata l'operazione decrease-key per aggiornare
// le distanze in modo efficiente.
inline DijkstraResult dijkstra_pairing_heap(const Graph& g, int source) {
    int n = static_cast<int>(g.size());

    //controlla che il nodo sorgente sia valido
    if (source < 0 || source >= n) {
        throw std::invalid_argument("Invalid source node");
    }

    const double INF = std::numeric_limits<double>::infinity();

    // Inizializza le distanze, i genitori e il vettore di visitati
    // dist[i] = distanza minima da source a i
    // parent[i] = nodo genitore di i nel percorso minimo da source a i
    std::vector<double> dist(n, INF);
    std::vector<int> parent(n, -1);

    // Vettore per tenere traccia dei nodi già visitati
    std::vector<bool> visited(n, false);

    using Heap = PairingHeap<double, int>;
    Heap pq;

    // Vettore di handle per tenere traccia dei nodi nel pairing heap
    //permette di aggiornare le chiavi in modo efficiente con decrease_key
    std::vector<typename Heap::Handle> handles(n, nullptr);

    dist[source] = 0.0;
    handles[source] = pq.push(0.0, source);

    // Loop principale di Dijkstra
    while (!pq.empty()) {
        // Estrae il nodo con distanza minima
        auto [du, u] = pq.top();
        pq.pop();
        handles[u] = nullptr;

        // Se il nodo è già stato visitato, salta
        if (visited[u]) {
            continue;
        }
        visited[u] = true;

        //Rilassamento degli archi uscenti da u
        for (const auto& e : g[u]) {
            int v = e.to;
            double nd = du + e.w;

            // Se viene trovato un percorso più breve per v
            //aggiorna la distanza e il genitore
            if (nd < dist[v]) {
                dist[v] = nd;
                parent[v] = u;

                //caso 1: v non è ancora nel pairing heap, quindi lo inserisce
                if (handles[v] == nullptr) {
                    handles[v] = pq.push(nd, v);
                //caso2: v è già nel pairing heap, quindi aggiorna la chiave con decrease_key
                } else {
                    pq.decrease_key(handles[v], nd);
                }
            }
        }
    }

    //restituisce le distanze minime e i genitori per ogni nodo raggiungibile da source
    return {dist, parent};
}