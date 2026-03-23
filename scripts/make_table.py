import pandas as pd
import matplotlib.pyplot as plt
from pathlib import Path

# cartella output
Path("results").mkdir(parents=True, exist_ok=True)

# leggi risultati benchmark
df = pd.read_csv("results/benchmark_results.csv")

# numero di archi per il grafico di scaling
dataset_sizes = {
    "random_n1000_m5000": 5000,
    "random_n10000_m50000": 50000,
    "grid_50x50": 4900,
    "grid_100x100": 19800,
    "com_youtube": 3000000
}

df["edges"] = df["dataset"].map(dataset_sizes)

# nomi più leggibili per gli algoritmi
algorithm_labels = {
    "binary_heap_lazy": "Binary heap (lazy)",
    "dary_heap": "4-ary heap",
    "pairing_heap": "Pairing heap"
}

df["algorithm_label"] = df["algorithm"].map(
    lambda x: algorithm_labels.get(x, x)
)

# riepilogo statistico
summary = df.groupby(["dataset", "algorithm_label"])["time_ms"].agg(["mean", "std", "count"])

print("\nBenchmark summary:\n")
print(summary)

with open("results/benchmark_summary.txt", "w", encoding="utf-8") as f:
    f.write("Benchmark summary:\n\n")
    f.write(summary.to_string())

# tabella tempi medi
table = pd.pivot_table(
    df,
    index="dataset",
    columns="algorithm_label",
    values="time_ms",
    aggfunc="mean"
)

# ordine fisso delle colonne
desired_order = [
    "Binary heap (lazy)",
    "4-ary heap",
    "Pairing heap"
]
table = table.reindex(columns=[c for c in desired_order if c in table.columns])

# nomi dataset più leggibili
table = table.rename(index={
    "random_n1000_m5000": "Random (1k, 5k)",
    "random_n10000_m50000": "Random (10k, 50k)",
    "grid_50x50": "Grid 50x50",
    "grid_100x100": "Grid 100x100",
    "com_youtube": "YouTube"
})

print("\nMean execution times:\n")
print(table)

with open("results/benchmark_table.txt", "w", encoding="utf-8") as f:
    f.write("Mean execution times:\n\n")
    f.write(table.to_string())

# tabella markdown
markdown_table = table.round(4).to_markdown()

print("\nMarkdown table:\n")
print(markdown_table)

with open("results/benchmark_table.md", "w", encoding="utf-8") as f:
    f.write(markdown_table)

# grafico confronto dataset
table.plot(kind="bar", figsize=(10, 6))
plt.ylabel("Execution time (ms)")
plt.xlabel("Dataset")
plt.title("Dijkstra benchmark results")
plt.xticks(rotation=45)
plt.tight_layout()
plt.savefig("results/benchmark_plot.png", dpi=200)
plt.show()

# grafico scaling
plt.figure(figsize=(8, 6))

for algorithm, group in df.groupby("algorithm_label"):
    group = group.sort_values("edges")

    plt.plot(
        group["edges"],
        group["time_ms"],
        marker="o",
        label=algorithm
    )

plt.xlabel("Number of edges")
plt.ylabel("Execution time (ms)")
plt.title("Dijkstra scaling behavior")
plt.xscale("log")
plt.yscale("log")
plt.legend()
plt.grid(True)
plt.tight_layout()
plt.savefig("results/scaling_plot.png", dpi=200)
plt.show()