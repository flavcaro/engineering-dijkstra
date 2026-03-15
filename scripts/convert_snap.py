#!/usr/bin/env python3

from __future__ import annotations

import argparse
import random
from pathlib import Path


def parse_args() -> argparse.Namespace:
    # Definisce gli argomenti da linea di comando
    parser = argparse.ArgumentParser(
        description="Converte un dataset SNAP edge-list in un formato pesato per Dijkstra."
    )

    # file di input in formato SNAP
    parser.add_argument(
        "--input",
        required=True,
        help="Percorso del file input SNAP (edge list).",
    )

    # file di output nel formato usato dal progetto
    parser.add_argument(
        "--output",
        required=True,
        help="Percorso del file output convertito.",
    )

    # se specificato, il grafo viene trattato come orientato;
    # altrimenti ogni arco u v viene duplicato come v u
    parser.add_argument(
        "--directed",
        action="store_true",
        help="Tratta il grafo come orientato. Se non specificato, ogni arco u v diventa anche v u.",
    )

    # modalità di assegnazione dei pesi:
    # unit = tutti i pesi uguali a 1
    # random = pesi casuali nell'intervallo specificato
    parser.add_argument(
        "--weight-mode",
        choices=["unit", "random"],
        default="unit",
        help="Assegna peso 1 oppure un peso casuale.",
    )

    parser.add_argument(
        "--min-weight",
        type=int,
        default=1,
        help="Peso minimo per la modalità random.",
    )

    parser.add_argument(
        "--max-weight",
        type=int,
        default=10,
        help="Peso massimo per la modalità random.",
    )

    # seed del generatore pseudo-casuale per rendere la conversione riproducibile
    parser.add_argument(
        "--seed",
        type=int,
        default=42,
        help="Seed del generatore casuale.",
    )

    return parser.parse_args()


def choose_weight(rng: random.Random, mode: str, min_w: int, max_w: int) -> int:
    # Restituisce il peso di un arco in base alla modalità scelta
    if mode == "unit":
        return 1
    return rng.randint(min_w, max_w)


def convert_snap(
    input_path: Path,
    output_path: Path,
    directed: bool,
    weight_mode: str,
    min_weight: int,
    max_weight: int,
    seed: int,
) -> None:
    # Inizializza il generatore casuale con seed fisso
    rng = random.Random(seed)

    # lista degli archi nel formato (u, v, w)
    edges: list[tuple[int, int, int]] = []

    # insieme dei nodi presenti nel dataset
    nodes: set[int] = set()

    # legge il file SNAP riga per riga
    with input_path.open("r", encoding="utf-8", errors="ignore") as fin:
        for line_no, raw_line in enumerate(fin, start=1):
            line = raw_line.strip()

            # salta righe vuote e commenti
            if not line or line.startswith("#"):
                continue

            parts = line.split()
            if len(parts) < 2:
                print(f"[warning] Riga {line_no} ignorata: formato non valido")
                continue

            try:
                u = int(parts[0])
                v = int(parts[1])
            except ValueError:
                print(f"[warning] Riga {line_no} ignorata: nodi non interi")
                continue

            # ignora self-loop
            if u == v:
                continue

            # genera il peso dell'arco
            w = choose_weight(rng, weight_mode, min_weight, max_weight)

            # aggiunge l'arco diretto u -> v
            edges.append((u, v, w))
            nodes.add(u)
            nodes.add(v)

            # Se il grafo è non orientato, aggiunge anche l'arco inverso
            # mantenendo lo stesso peso, in modo da rappresentare
            # correttamente un arco non orientato pesato.
            if not directed:
                edges.append((v, u, w))

    if not nodes:
        raise ValueError("Nessun nodo trovato nel file di input.")

    # Numero di nodi calcolato come max_id + 1
    n = max(nodes) + 1
    m = len(edges)

    # crea la cartella di output se non esiste
    output_path.parent.mkdir(parents=True, exist_ok=True)

    # Scrive il file nel formato:
    # n m
    # u v w
    with output_path.open("w", encoding="utf-8") as fout:
        fout.write(f"{n} {m}\n")
        for u, v, w in edges:
            fout.write(f"{u} {v} {w}\n")

    print(f"[ok] File convertito: {output_path}")
    print(f"[ok] Nodi: {n}")
    print(f"[ok] Archi: {m}")


def main() -> None:
    # legge gli argomenti da linea di comando
    args = parse_args()

    input_path = Path(args.input)
    output_path = Path(args.output)

    # verifica che il file di input esista
    if not input_path.exists():
        raise FileNotFoundError(f"File input non trovato: {input_path}")

    # controlla che l'intervallo dei pesi casuali sia valido
    if args.weight_mode == "random" and args.min_weight > args.max_weight:
        raise ValueError("min-weight non può essere maggiore di max-weight")

    # esegue la conversione del dataset
    convert_snap(
        input_path=input_path,
        output_path=output_path,
        directed=args.directed,
        weight_mode=args.weight_mode,
        min_weight=args.min_weight,
        max_weight=args.max_weight,
        seed=args.seed,
    )


if __name__ == "__main__":
    main()