#include "dijkstra/d_ary_heap.hpp"
#include "dijkstra/pairing_heap.hpp"

#include <algorithm>
#include <cstddef>
#include <functional>
#include <iostream>
#include <limits>
#include <queue>
#include <random>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace {

using Key = int;
using Value = int;
using Item = std::pair<Key, Value>;

struct Operation {
    enum class Kind {
        Insert,
        DecreaseKey,
        Pop
    };

    Kind kind;
    Value id;
    Key key;
};

// 1. CODA DI RIFERIMENTO (La nostra certezza matematica)
class ReferenceQueue {
public:
    Value push(Key key) {
        const Value id = next_id_++;
        data_.push_back({key, id});
        return id;
    }

    void decrease_key(Value id, Key new_key) {
        auto it = std::find_if(data_.begin(), data_.end(), [id](const Item& item) {
            return item.second == id;
        });
        if (it == data_.end()) {
            throw std::runtime_error("ReferenceQueue decrease_key on missing id");
        }
        it->first = new_key;
    }

    Item top() const {
        if (data_.empty()) {
            throw std::runtime_error("ReferenceQueue is empty");
        }
        return *std::min_element(data_.begin(), data_.end(), [](const Item& a, const Item& b) {
            if (a.first != b.first) {
                return a.first < b.first;
            }
            return a.second < b.second;
        });
    }

    Item pop() {
        Item best = top();
        auto it = std::find_if(data_.begin(), data_.end(), [best](const Item& item) {
            return item == best;
        });
        data_.erase(it);
        return best;
    }

    bool empty() const {
        return data_.empty();
    }

    std::size_t size() const {
        return data_.size();
    }

private:
    Value next_id_ = 0;
    std::vector<Item> data_;
};

// 2. WRAPPER PER BINARY HEAP (LAZY)
class BinaryLazyQueue {
public:
    Value push(Key key) {
        const Value id = next_id_++;
        ensure_capacity(id);
        alive_[id] = true;
        current_key_[id] = key;
        pq_.push({key, id});
        ++size_;
        return id;
    }

    void decrease_key(Value id, Key new_key) {
        ensure_capacity(id);
        if (!alive_[id]) {
            throw std::runtime_error("BinaryLazyQueue decrease_key on inactive id");
        }
        current_key_[id] = new_key;
        pq_.push({new_key, id});
    }

    Item top() {
        prune();
        if (pq_.empty()) {
            throw std::runtime_error("BinaryLazyQueue is empty");
        }
        return pq_.top();
    }

    Item pop() {
        prune();
        if (pq_.empty()) {
            throw std::runtime_error("BinaryLazyQueue is empty");
        }

        Item best = pq_.top();
        pq_.pop();
        alive_[best.second] = false;
        --size_;
        return best;
    }

    bool empty() {
        prune();
        return size_ == 0;
    }

    std::size_t size() const {
        return size_;
    }

private:
    void ensure_capacity(Value id) {
        if (id >= static_cast<Value>(alive_.size())) {
            alive_.resize(static_cast<std::size_t>(id) + 1, false);
            current_key_.resize(static_cast<std::size_t>(id) + 1, 0);
        }
    }

    void prune() {
        while (!pq_.empty()) {
            const Item& best = pq_.top();
            const Value id = best.second;
            if (id < 0 || id >= static_cast<Value>(alive_.size())) {
                pq_.pop();
                continue;
            }
            if (!alive_[id] || current_key_[id] != best.first) {
                pq_.pop();
                continue;
            }
            break;
        }
    }

    Value next_id_ = 0;
    std::priority_queue<Item, std::vector<Item>, std::greater<Item>> pq_;
    std::vector<bool> alive_;
    std::vector<Key> current_key_;
    std::size_t size_ = 0;
};

// 3. WRAPPER PER HEAP STRUTTURATI (EAGER: D-ARY E PAIRING)
template <typename Heap>
class HeapQueue {
public:
    Value push(Key key) {
        const Value id = next_id_++;
        ensure_capacity(id);
        handles_[id] = heap_.push(key, id);
        return id;
    }

    void decrease_key(Value id, Key new_key) {
        if (id < 0 || id >= static_cast<Value>(handles_.size()) || handles_[id] == nullptr) {
            throw std::runtime_error("HeapQueue decrease_key on missing handle");
        }
        heap_.decrease_key(handles_[id], new_key);
    }

    Item top() {
        return heap_.top();
    }

    Item pop() {
        Item best = heap_.top();
        heap_.pop();
        handles_[best.second] = nullptr;
        return best;
    }

    bool empty() const {
        return heap_.empty();
    }

    std::size_t size() const {
        return heap_.size();
    }

private:
    void ensure_capacity(Value id) {
        if (id >= static_cast<Value>(handles_.size())) {
            handles_.resize(static_cast<std::size_t>(id) + 1, nullptr);
        }
    }

    Heap heap_;
    Value next_id_ = 0;
    std::vector<typename Heap::Handle> handles_;
};

struct Config {
    int total_ops = 100;
    int insert_ops = 30;
    int decrease_ops = 40;
    int pop_ops = 30;
    unsigned seed = 42;
};

Config parse_config(int argc, char** argv) {
    Config cfg;
    if (argc > 1) cfg.total_ops = std::stoi(argv[1]);
    if (argc > 2) cfg.insert_ops = std::stoi(argv[2]);
    if (argc > 3) cfg.decrease_ops = std::stoi(argv[3]);
    if (argc > 4) cfg.pop_ops = std::stoi(argv[4]);
    if (argc > 5) cfg.seed = static_cast<unsigned>(std::stoul(argv[5]));

    if (cfg.total_ops <= 0) throw std::invalid_argument("total_ops must be positive");
    if (cfg.insert_ops < 0 || cfg.decrease_ops < 0 || cfg.pop_ops < 0) 
        throw std::invalid_argument("operation counts must be non-negative");
    if (cfg.insert_ops + cfg.decrease_ops + cfg.pop_ops != cfg.total_ops) 
        throw std::invalid_argument("operation counts must sum to total_ops");
    if (cfg.insert_ops < cfg.pop_ops) 
        throw std::invalid_argument("insert_ops must be at least pop_ops to avoid empty pops");

    return cfg;
}

std::vector<Operation> build_plan(const Config& cfg) {
    std::mt19937 rng(cfg.seed);
    std::vector<Operation> plan;
    plan.reserve(static_cast<std::size_t>(cfg.total_ops));

    int rem_inserts = cfg.insert_ops;
    int rem_decreases = cfg.decrease_ops;
    int rem_pops = cfg.pop_ops;

    std::vector<Value> active_ids;
    std::vector<Key> current_keys;
    Value next_id = 0;

    std::uniform_int_distribution<Key> insert_key_dist(10000, 1'000'000);
    std::uniform_int_distribution<int> decrease_delta_dist(1, 100);

    for (int i = 0; i < cfg.total_ops; ++i) {
        std::vector<int> available_choices;
        
        if (rem_inserts > 0) available_choices.push_back(0);
        if (rem_decreases > 0 && !active_ids.empty()) available_choices.push_back(1);
        if (rem_pops > 0 && !active_ids.empty()) {
            const bool must_keep_one_item = (rem_inserts == 0 && rem_decreases > 0 && active_ids.size() == 1);
            if (!must_keep_one_item) available_choices.push_back(2);
        }

        if (available_choices.empty()) {
            if (rem_inserts > 0) available_choices.push_back(0);
            else throw std::runtime_error("Impossibile completare il piano senza violare la coerenza della coda.");
        }

        std::uniform_int_distribution<std::size_t> pick_choice(0, available_choices.size() - 1);
        int choice = available_choices[pick_choice(rng)];

        Operation op{};
        if (choice == 0) { 
            op.kind = Operation::Kind::Insert;
            op.id = next_id++;
            op.key = insert_key_dist(rng);
            active_ids.push_back(op.id);
            current_keys.push_back(op.key);
            rem_inserts--;
        } 
        else if (choice == 1) { 
            std::uniform_int_distribution<std::size_t> pick_idx(0, active_ids.size() - 1);
            size_t pos = pick_idx(rng);
            
            op.kind = Operation::Kind::DecreaseKey;
            op.id = active_ids[pos];
            Key delta = decrease_delta_dist(rng);
            op.key = std::max<Key>(0, current_keys[pos] - delta); 
            
            current_keys[pos] = op.key;
            rem_decreases--;
        } 
        else { 
            auto min_it = std::min_element(current_keys.begin(), current_keys.end());
            size_t pos = std::distance(current_keys.begin(), min_it);

            op.kind = Operation::Kind::Pop;
            op.id = active_ids[pos];
            op.key = current_keys[pos];

            active_ids.erase(active_ids.begin() + pos);
            current_keys.erase(current_keys.begin() + pos);
            rem_pops--;
        }
        plan.push_back(op);
    }

    return plan;
}

template <typename Q1, typename Q2, typename Q3>
bool check_all_equal(const std::string& phase, Q1& q1, Q2& q2, Q3& q3, const ReferenceQueue& ref) {
    if (q1.size() != ref.size() || q2.size() != ref.size() || q3.size() != ref.size()) {
        std::cerr << "[ERROR] size mismatch after " << phase << "\n";
        std::cerr << " Sizes -> Ref: " << ref.size() << " | Bin: " << q1.size() 
                  << " | DAry: " << q2.size() << " | Pair: " << q3.size() << "\n";
        return false;
    }

    if (!ref.empty()) {
        const Item expected = ref.top();
        const Item a = q1.top();
        const Item b = q2.top();
        const Item c = q3.top();

        // Tolleriamo ID diversi SE E SOLO SE le chiavi di priorità sono identiche
        if (expected.first != a.first || expected.first != b.first || expected.first != c.first) {
            std::cerr << "[ERROR] top key mismatch after " << phase << "\n";
            std::cerr << "  reference key=" << expected.first << "\n";
            std::cerr << "  binary key=" << a.first << "\n";
            std::cerr << "  dary key=" << b.first << "\n";
            std::cerr << "  pairing key=" << c.first << "\n";
            return false;
        }
    }
    return true;
}

} // namespace

int main(int argc, char** argv) {
    try {
        const Config cfg = parse_config(argc, argv);
        const std::vector<Operation> plan = build_plan(cfg);

        ReferenceQueue reference;
        BinaryLazyQueue binary;
        HeapQueue<DAryHeap<Key, Value, 4>> dary;
        HeapQueue<PairingHeap<Key, Value>> pairing;

        int op_count = 0;
        for (const Operation& op : plan) {
            op_count++;
            if (op.kind == Operation::Kind::Insert) {
                reference.push(op.key);
                binary.push(op.key);
                dary.push(op.key);
                pairing.push(op.key);

                if (!check_all_equal("insert (Op " + std::to_string(op_count) + ")", binary, dary, pairing, reference)) {
                    return 1;
                }
            }
            else if (op.kind == Operation::Kind::DecreaseKey) {
                reference.decrease_key(op.id, op.key);
                binary.decrease_key(op.id, op.key);
                dary.decrease_key(op.id, op.key);
                pairing.decrease_key(op.id, op.key);

                if (!check_all_equal("decrease-key (Op " + std::to_string(op_count) + ")", binary, dary, pairing, reference)) {
                    return 1;
                }
            }
            else if (op.kind == Operation::Kind::Pop) {
                const Item expected = reference.pop();
                const Item got_binary = binary.pop();
                const Item got_dary = dary.pop();
                const Item got_pairing = pairing.pop();

                // Tolleriamo la divergenza dell'ID estratto solo se l'elemento ha lo stesso identico costo (chiave)
                if (expected.first != got_binary.first || expected.first != got_dary.first || expected.first != got_pairing.first) {
                    std::cerr << "[ERROR] pop key mismatch at Op " << op_count << "\n";
                    std::cerr << "  reference key=" << expected.first << "\n";
                    std::cerr << "  binary key=" << got_binary.first << "\n";
                    std::cerr << "  dary key=" << got_dary.first << "\n";
                    std::cerr << "  pairing key=" << got_pairing.first << "\n";
                    return 1;
                }

                if (!check_all_equal("pop (Op " + std::to_string(op_count) + ")", binary, dary, pairing, reference)) {
                    return 1;
                }
            }
        }

        // =========================================================================
        // DRAINING FINALE TOLLERANTE AI DUPLICATI
        // =========================================================================
        std::cout << "Draining and verifying final heap contents...\n";
        int drain_step = 0;
        
        while (!reference.empty()) {
            drain_step++;
            Item ref_top = reference.pop();

            // 1. Verifica speculare Binary Heap
            if (!binary.empty()) {
                Item bin_top = binary.pop();
                if (bin_top.first != ref_top.first) {
                    std::cerr << "[CRITICAL ERROR] Binary Heap fallito al passo di drenaggio " << drain_step << "\n"
                              << "  Chiave Attesa (Ref): " << ref_top.first << "\n"
                              << "  Chiave Trovata (Bin): " << bin_top.first << "\n";
                    return 1;
                }
            } else {
                std::cerr << "[CRITICAL ERROR] Binary Heap vuoto in anticipo al passo " << drain_step << "\n";
                return 1;
            }

            // 2. Verifica speculare D-Ary Heap
            if (!dary.empty()) {
                Item dary_top = dary.pop();
                if (dary_top.first != ref_top.first) {
                    std::cerr << "[CRITICAL ERROR] D-Ary Heap fallito al passo di drenaggio " << drain_step << "\n"
                              << "  Chiave Attesa (Ref):  " << ref_top.first << "\n"
                              << "  Chiave Trovata (DAry): " << dary_top.first << "\n";
                    return 1;
                }
            } else {
                std::cerr << "[CRITICAL ERROR] D-Ary Heap vuoto in anticipo al passo " << drain_step << "\n";
                return 1;
            }

            // 3. Verifica speculare Pairing Heap
            if (!pairing.empty()) {
                Item pair_top = pairing.pop();
                if (pair_top.first != ref_top.first) {
                    std::cerr << "[CRITICAL ERROR] Pairing Heap fallito al passo di drenaggio " << drain_step << "\n"
                              << "  Chiave Attesa (Ref):   " << ref_top.first << "\n"
                              << "  Chiave Trovata (Pair):  " << pair_top.first << "\n";
                    return 1;
                }
            } else {
                std::cerr << "[CRITICAL ERROR] Pairing Heap vuoto in anticipo al passo " << drain_step << "\n";
                return 1;
            }
        }

        // Controllo elementi residui orfani
        if (!binary.empty() || !dary.empty() || !pairing.empty()) {
            std::cerr << "[CRITICAL ERROR] Elementi rimasti negli heap abusivamente dopo il drenaggio completo.\n";
            return 1;
        }

        std::cout << "[OK] Heap pilot passed successfully!\n"
                  << "Executed " << cfg.total_ops << " operations:\n"
                  << " -> " << cfg.insert_ops << " inserts\n"
                  << " -> " << cfg.decrease_ops << " decrease-keys\n"
                  << " -> " << cfg.pop_ops << " pops\n";
        return 0;

    } catch (const std::exception& ex) {
        std::cerr << "[ERROR] " << ex.what() << "\n";
        return 1;
    }
}