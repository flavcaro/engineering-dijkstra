#!/usr/bin/env python3

from __future__ import annotations

import argparse
import random
from pathlib import Path


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Converte un dataset SNAP edge-list in un formato pesato per Dijkstra."
    )

    parser.add_argument(
        "--input",
        required=True,
        help="Percorso del file input SNAP (edge list).",
    )
    parser.add_argument(
        "--output",
        required=True,
        help="Percorso del file output convertito.",
    )
    parser.add_argument(
        "--directed",
        action="store_true",
        help="Tratta il grafo come orientato. Se non specificato, ogni arco u v diventa anche v u.",
    )
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
    parser.add_argument(
        "--seed",
        type=int,
        default=42,
        help="Seed del generatore casuale.",
    )

    return parser.parse_args()


def choose_weight(rng: random.Random, mode: str, min_w: int, max_w: int) -> int:
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
    rng = random.Random(seed)

    edges: list[tuple[int, int, int]] = []
    nodes: set[int] = set()

    with input_path.open("r", encoding="utf-8", errors="ignore") as fin:
        for line_no, raw_line in enumerate(fin, start=1):
            line = raw_line.strip()

            # Salta righe vuote e commenti SNAP
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

            if u == v:
                # opzionale: salta self-loop
                continue

            w = choose_weight(rng, weight_mode, min_weight, max_weight)

            edges.append((u, v, w))
            nodes.add(u)
            nodes.add(v)

            if not directed:
                w2 = choose_weight(rng, weight_mode, min_weight, max_weight)
                edges.append((v, u, w2))
                nodes.add(v)
                nodes.add(u)

    if not nodes:
        raise ValueError("Nessun nodo trovato nel file di input.")

    # Numero nodi come max_id + 1: semplice da leggere in C++
    n = max(nodes) + 1
    m = len(edges)

    output_path.parent.mkdir(parents=True, exist_ok=True)

    with output_path.open("w", encoding="utf-8") as fout:
        fout.write(f"{n} {m}\n")
        for u, v, w in edges:
            fout.write(f"{u} {v} {w}\n")

    print(f"[ok] File convertito: {output_path}")
    print(f"[ok] Nodi: {n}")
    print(f"[ok] Archi: {m}")


def main() -> None:
    args = parse_args()

    input_path = Path(args.input)
    output_path = Path(args.output)

    if not input_path.exists():
        raise FileNotFoundError(f"File input non trovato: {input_path}")

    if args.weight_mode == "random" and args.min_weight > args.max_weight:
        raise ValueError("min-weight non può essere maggiore di max-weight")

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