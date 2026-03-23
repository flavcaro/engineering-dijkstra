#pragma once

#include <stdexcept>
#include <utility>
#include <vector>

// Min-heap d-ario.
// Ogni nodo ha fino a D figli. (default D=4)
// Supporta push, top, pop e decrease-key tramite handle
template <typename Key, typename Value, int D = 4>
class DAryHeap {
    static_assert(D >= 2, "D must be at least 2");

public:
// Rappresenta un nodo nell'heap
// Contiene la chiave, il valore e l'indice corrente nel vettore heap
    struct Node {
        Key key;
        Value value;
        int index; // posizione corrente nel vettore heap
    };

    // Handle: un puntatore al nodo nell'heap
    // usato per decrease-key in O(log n) senza dover reinserire duplicati
    using Handle = Node*;

    DAryHeap() = default;

    // Distruttore: libera la memoria allocata dinamicamente per i nodi
    ~DAryHeap() {
        for (Node* node : heap_) {
            delete node;
        }
    }

    // Non permette copie o assegnazioni per evitare problemi di gestione della memoria
    DAryHeap(const DAryHeap&) = delete;
    DAryHeap& operator=(const DAryHeap&) = delete;

    //verifica se l'heap è vuoto
    bool empty() const {
        return heap_.empty();
    }

    //ritorna il numero di elementi nell'heap
    std::size_t size() const {
        return heap_.size();
    }

    //restituisce la coppia (chiave, valore) del nodo con chiave minima 
    //senza rimuoverlo dall'heap
    std::pair<Key, Value> top() const {
        if (heap_.empty()) {
            throw std::runtime_error("DAryHeap is empty");
        }
        return {heap_[0]->key, heap_[0]->value};
    }

    // Inserisce un nuovo nodo con chiave e valore specificati.
    // Restituisce un handle al nodo inserito, che può essere usato per decrease-key
    // La complessità è O(log n) nel caso peggiore, ma in media è più veloce grazie alla struttura d-aria
    Handle push(const Key& key, const Value& value) {
        Node* node = new Node{key, value, static_cast<int>(heap_.size())};
        heap_.push_back(node);
        //ripristina la proprietà di heap dopo l'inserimento
        sift_up(node->index);
        return node;
    }

    // Rimuove il nodo con chiave minima dall'heap
    void pop() {
        if (heap_.empty()) {
            throw std::runtime_error("DAryHeap is empty");
        }

        Node* root = heap_[0];

        // Se l'heap ha solo un elemento, basta rimuoverlo e liberare la memoria
        if (heap_.size() == 1) {
            heap_.pop_back();
            delete root;
            return;
        }

        // Sostituisce la radice con l'ultimo elemento
        heap_[0] = heap_.back();
        heap_[0]->index = 0;
        heap_.pop_back();

        delete root;

        //ripristina la proprietà di heap dopo la rimozione
        sift_down(0);
    }

    //diminuisce la chiave di un nodo gia presente nell'heap tramite il suo handle
    void decrease_key(Handle node, const Key& new_key) {
        if (!node) {
            throw std::runtime_error("Invalid handle in decrease_key");
        }
        if (new_key > node->key) {
            throw std::runtime_error("New key is greater than current key");
        }

        node->key = new_key;
        //ripristina la proprietà di heap dopo la modifica della chiave
        sift_up(node->index);
    }

private:
//rappresentazione dell'heap come vettore di puntatori a nodi
    std::vector<Node*> heap_;

    //indice del padre dell'i-esimo nodo
    static int parent(int i) {
        return (i - 1) / D;
    }

    //k-esimo figlio dell'i-esimo nodo (0<=k<D)
    static int child(int i, int k) {
        return D * i + k + 1;
    }

    //scambia due nodi nell'heap e aggiorna i loro indici
    void swap_nodes(int i, int j) {
        std::swap(heap_[i], heap_[j]);
        heap_[i]->index = i;
        heap_[j]->index = j;
    }

    //risale l'heap a partire dall'i-esimo nodo per ripristinare la proprietà di heap
    void sift_up(int i) {
        while (i > 0) {
            int p = parent(i);
            if (heap_[i]->key < heap_[p]->key) {
                swap_nodes(i, p);
                i = p;
            } else {
                break;
            }
        }
    }

    //scende l'heap a partire dall'i-esimo nodo per ripristinare la proprietà di heap
    void sift_down(int i) {
        while (true) {
            int best = i;

            for (int k = 0; k < D; ++k) {
                int c = child(i, k);
                if (c < static_cast<int>(heap_.size()) &&
                    heap_[c]->key < heap_[best]->key) {
                    best = c;
                }
            }

            //se il nodo corrente è già minore di tutti i suoi figli, la proprietà di heap è soddisfatta
            if (best == i) {
                break;
            }

            swap_nodes(i, best);
            i = best;
        }
    }
};
