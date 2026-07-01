# engineering-dijkstra

Studio sperimentale su Dijkstra: **quanto incide la struttura della priority queue** sulle prestazioni, confrontando implementazioni diverse su grafi sintetici Barabasi-Albert.

## 1) Obiettivo del progetto

Domanda di ricerca:

Come cambia il tempo di esecuzione di Dijkstra (e l'overhead strutturale associato) quando si sostituisce la priority queue?

Implementazioni confrontate:

- Binary heap con approccio lazy (niente decrease-key esplicito, si reinseriscono candidati e si scartano entry obsolete).
- 4-ary heap con supporto a decrease-key tramite handle.
- Pairing heap con supporto a decrease-key tramite handle.

## 2) Struttura del repository

- `src/graph/`: rappresentazione del grafo, I/O, generatori.
- `src/dijkstra/`: implementazioni di Dijkstra e strutture heap.
- `src/benchmark.cpp`: benchmark principale su grafi Barabasi-Albert generati al volo.
- `src/generate_datasets.cpp`: generazione dataset sintetici.
- `tests/`: test di correttezza (confronto distanze tra implementazioni).
- `scripts/`: conversione dataset SNAP e analisi risultati.
- `data/`: dataset generati.
- `results/`: CSV, tabelle, speedup e grafici.

## 3) Formato dei grafi

Formato testuale usato in tutto il progetto:

```
n m
u v w
u v w
...
```

- `n`: numero di nodi
- `m`: numero di archi
- `u v w`: arco diretto da `u` a `v` con peso `w`

Nota: i pesi devono essere non negativi (vincolo di Dijkstra).

## 4) Moduli principali

### 4.1 Rappresentazione grafo

- `graph.hpp`: lista di adiacenza (`Graph = vector<vector<Edge>>`), funzioni per aggiungere archi diretti/non diretti.
- `graph_io.hpp`: caricamento da file.
- `graph_export.hpp`: salvataggio su file.

### 4.2 Algoritmi Dijkstra

- `dijkstra_binary.hpp`:
	- usa `std::priority_queue` come min-heap.
	- strategia lazy: quando una distanza migliora, viene pushata una nuova coppia `(dist, nodo)`.
	- quando esce una entry non aggiornata (`du > dist[u]`), viene scartata.

- `dijkstra_dary.hpp`:
	- usa `DAryHeap<Key, Value, 4>`.
	- mantiene handle per nodo, così usa `decrease_key` quando trova una distanza migliore.

- `dijkstra_pairing.hpp`:
	- usa `PairingHeap<Key, Value>`.
	- anche qui handle + `decrease_key`.

### 4.3 Priority queue custom

- `d_ary_heap.hpp`:
	- min-heap d-ario (qui `D = 4`), con `push`, `top`, `pop`, `decrease_key`.
	- implementazione array-based, con `sift_up` / `sift_down`.

- `pairing_heap.hpp`:
	- pairing heap con `meld`, `cut`, `two_pass_meld`.
	- `decrease_key` tramite taglio e merge in radice.

## 5) Dataset usati

### 5.1 Sintetici (generati)

Generati da `src/generate_datasets.cpp`:

- Random scaling (n e m crescono insieme)
- Random density (n fisso, m cresce)
- Grid (`50x50`, `100x100`, `300x300`)

Il benchmark principale usa invece grafi Barabasi-Albert generati al volo per lo studio di performance:

- scaling: `n` cresce con attachment fisso
- density: `n` fisso, attachment che raddoppia
- per ogni configurazione vengono generati 10 grafi con seed diversi
- per ogni grafo ogni algoritmo viene eseguito 3 volte

I file vengono salvati in:

- `data/generated/random_scaling/`
- `data/generated/random_density/`
- `data/generated/grid/`

## 6) Come eseguire il progetto

## 6.1 Build benchmark

```bash
g++ -std=c++17 -O2 -Wall -Wextra -I./src src/benchmark.cpp -o benchmark.exe
```

## 6.2 Build generatore dataset

```bash
g++ -std=c++17 -O2 -Wall -Wextra -I./src src/generate_datasets.cpp -o generate_datasets.exe
```

## 6.3 Genera dataset sintetici

```bash
./generate_datasets.exe
```

## 6.4 Esegui benchmark

```bash
./benchmark.exe
```

Output principale:

- `results/benchmark_results.csv`

Il CSV contiene sia il tempo di esecuzione sia contatori indipendenti dalla macchina:

- `graph_id` e `seed`, per distinguere le 10 istanze generate per ogni configurazione
- `pq_pushes`
- `pq_pops`
- `decrease_key_calls`
- `edge_relax_attempts`
- `successful_relaxations`
- `stale_entries_discarded`
- `settled_nodes`

## 6.5 Analizza risultati e genera tabelle/grafici

Prerequisiti Python:

```bash
pip install pandas matplotlib tabulate
```

Esecuzione:

```bash
python scripts/make_table.py
```

Output in `results/`:

- Tabelle medie/deviazioni standard
- Tabelle speedup rispetto al binary heap
- Grafici bar/line/boxplot

## 7) Test di correttezza

I test confrontano le distanze calcolate rispetto alla versione binary heap:

- `tests/test_dary.cpp`
- `tests/test_pairing.cpp`
- `tests/test_heap_pilot.cpp`

Esempio build/esecuzione (d-ary):

```bash
g++ -std=c++17 -O2 -Wall -Wextra -I./src tests/test_dary.cpp -o test_dary.exe
./test_dary.exe
```

Esempio build/esecuzione (pairing):

```bash
g++ -std=c++17 -O2 -Wall -Wextra -I./src tests/test_pairing.cpp -o test_pairing.exe
./test_pairing.exe
```

Esempio build/esecuzione del pilot parametrico sulle heap:

```bash
g++ -std=c++17 -O2 -Wall -Wextra -I./src tests/test_heap_pilot.cpp -o test_heap_pilot.exe
./test_heap_pilot.exe 100 30 40 30 42
```

Il pilot usa una sequenza legale di 100 operazioni fondamentali: prima gli inserimenti, poi i decrease-key, infine i pop. Il confronto viene fatto tra binary heap lazy, 4-ary heap e pairing heap, verificando sia il minimo corrente sia il contenuto finale della coda.

## 8) Sintesi dei risultati ottenuti

Dai file già presenti in `results/`:

- `benchmark_table_mean.txt`
- `benchmark_speedup.txt`
- `benchmark_summary.txt`

Osservazioni principali:

1. **Binary heap (lazy)** è la baseline più solida e spesso la più veloce.
2. **4-ary heap** è competitivo e su alcuni casi più densi supera il binary heap.
	 - Esempio density `10k, 100k`: 3.54 ms vs 4.21 ms (speedup ~1.19x).
	 - Esempio density `10k, 200k`: 4.03 ms vs 4.95 ms (speedup ~1.23x).
3. **Pairing heap** in questo setup è quasi sempre più lento.
4. Il binary heap lazy resta la baseline di riferimento anche nei casi più densi.

Interpretazione pratica:

- Le complessità teoriche favorevoli al decrease-key non garantiscono automaticamente tempi migliori reali.
- Costanti nascoste, località di memoria e overhead delle strutture pointer-based incidono molto.

## 9) Limiti sperimentali

- Protocollo sintetico BA: 10 grafi per configurazione, 3 run per algoritmo su ogni grafo.
- Numero di run per grafo: 3 (aumentabile per maggiore robustezza statistica).
- Misura principale: tempo totale (non c'è profilazione dettagliata per fase).
- Hardware e ambiente influenzano i risultati assoluti.