#pragma once

#include <functional>
#include <stdexcept>
#include <utility>
#include <vector>

// Implementazione di un Pairing Heap minimo.
// La struttura supporta:
// - inserimento di un elemento
// - accesso al minimo
// - estrazione del minimo
// - decrease-key tramite handle al nodo
template <typename Key, typename Value, typename Compare = std::less<Key>>
class PairingHeap {
public:
    //Nodo del pairing heap
    //ogni nodo contiene:
    // - una chiave
    // - un valore
    // - un puntatore al figlio più a sinistra
    // - un puntatore al fratello destro
    // - un puntatore al genitore
    struct Node {
        Key key;
        Value value;
        Node* child;
        Node* sibling;
        Node* parent;

        Node(const Key& k, const Value& v)
            : key(k), value(v), child(nullptr), sibling(nullptr), parent(nullptr) {}
    };

    // Handle per decrease-key, rappresentato come puntatore al nodo
    using Handle = Node*;

    PairingHeap() : root_(nullptr), size_(0), comp_(Compare()) {}

    ~PairingHeap() {
        destroy(root_);
    }

    // Controlla se il pairing heap è vuoto -> true
    bool empty() const {
        return root_ == nullptr;
    }

    // Restituisce il numero di elementi nel pairing heap
    std::size_t size() const {
        return size_;
    }

    // Restituisce la coppia (chiave, valore) del nodo con chiave minima
    std::pair<Key, Value> top() const {
        if (!root_) {
            throw std::runtime_error("PairingHeap is empty");
        }
        return {root_->key, root_->value};
    }

    // Inserisce un nuovo elemento con chiave e valore specificati
    // restituendo un handle al nodo creato
    Handle push(const Key& key, const Value& value) {
        Node* node = new Node(key, value);
        root_ = meld(root_, node);
        ++size_;
        return node;
    }

    //Estrae il nodo con chiave minima
    //Dopo la rimozione, i figli del nodo rimosso vengono uniti in un nuovo pairing heap
    void pop() {
        if (!root_) {
            throw std::runtime_error("PairingHeap is empty");
        }

        Node* old_root = root_;

        if (!root_->child) {
            root_ = nullptr;
        } else {
            root_ = two_pass_meld(root_->child);
            if (root_) {
                root_->parent = nullptr;
            }
        }

        delete old_root;
        --size_;
    }

    // Decrease-key: dato un handle al nodo e una nuova chiave, aggiorna la chiave del nodo
    // Se la nuova chiave è minore di quella attuale, il nodo viene spostato nella posizione corretta nel pairing heap
    void decrease_key(Handle node, const Key& new_key) {
        if (!node) {
            throw std::runtime_error("Invalid handle in decrease_key");
        }

        // Verifica che la nuova chiave sia effettivamente minore di quella attuale
        if (!comp_(new_key, node->key) && !equal_keys(new_key, node->key)) {
            throw std::runtime_error("New key is not smaller than current key");
        }

        node->key = new_key;

        // Se il nodo è già la radice, non è necessario fare nulla
        if (node == root_) {
            return;
        }

        // Se la nuova chiave è minore di quella del genitore, taglia il nodo e uniscilo alla radice
        if (node->parent && comp_(node->key, node->parent->key)) {
            cut(node);
            root_ = meld(root_, node);
        }
    }

private:
    Node* root_;
    std::size_t size_;
    Compare comp_;

    //Verifica se due chiavi sono considerate uguali secondo il comparatore
    bool equal_keys(const Key& a, const Key& b) const {
        return !comp_(a, b) && !comp_(b, a);
    }

    //libera ricorsivamente la memoria occupata dai nodi del pairing heap
    void destroy(Node* node) {
        if (!node) {
            return;
        }

        Node* child = node->child;
        while (child) {
            Node* next = child->sibling;
            destroy(child);
            child = next;
        }

        delete node;
    }

    // Unisce due alberi di pairing heap
    //restituisce la radice del nuovo albero risultante
    Node* meld(Node* a, Node* b) {
        if (!a) return b;
        if (!b) return a;

        if (comp_(b->key, a->key)) {
            std::swap(a, b);
        }

        b->parent = a;
        b->sibling = a->child;
        a->child = b;

        return a;
    }

    // Taglia un nodo dal suo albero e lo prepara per essere unito alla radice
    // Dopo il taglio, il nodo diventa una nuova radice del pairing heap
    //usato da decrease_key quando la chiave di un nodo viene aggiornata a un valore minore di quello attuale
    void cut(Node* node) {
        Node* parent = node->parent;
        if (!parent) {
            return;
        }

        if (parent->child == node) {
            parent->child = node->sibling;
        } else {
            Node* prev = parent->child;
            while (prev && prev->sibling != node) {
                prev = prev->sibling;
            }
            if (prev) {
                prev->sibling = node->sibling;
            }
        }

        node->parent = nullptr;
        node->sibling = nullptr;
    }

    // Esegue la procedura di "two-pass melding" sui figli di un nodo rimosso
    // Questa procedura unisce i figli in coppie e poi unisce i risultati in un unico albero
    
    Node* two_pass_meld(Node* first_sibling) {
        if (!first_sibling) {
            return nullptr;
        }

        std::vector<Node*> trees;
        Node* curr = first_sibling;

        while (curr) {
            Node* a = curr;
            Node* b = curr->sibling;

            curr = nullptr;
            if (b) {
                curr = b->sibling;
            }

            a->parent = nullptr;
            a->sibling = nullptr;

            if (b) {
                b->parent = nullptr;
                b->sibling = nullptr;
                trees.push_back(meld(a, b));
            } else {
                trees.push_back(a);
            }
        }

        Node* result = trees.back();
        for (int i = static_cast<int>(trees.size()) - 2; i >= 0; --i) {
            result = meld(trees[i], result);
        }

        return result;
    }
};