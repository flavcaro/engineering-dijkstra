import pandas as pd
import matplotlib.pyplot as plt
from pathlib import Path

# crea cartella results se non esiste
Path("results").mkdir(parents=True, exist_ok=True)

# leggi file benchmark
df = pd.read_csv("results/benchmark_results.csv")

# dimensione dei dataset (numero archi)
dataset_sizes = {
    "random_n1000_m5000": 5000,
    "random_n10000_m50000": 50000,
    "grid_50x50": 4900,
    "grid_100x100": 19800,
    "com_youtube": 3000000
}

df["edges"] = df["dataset"].map(dataset_sizes)

# riepilogo statistico
summary = df.groupby(["dataset", "algorithm"])["time_ms"].agg(["mean", "std", "count"])

print("\nBenchmark summary:\n")
print(summary)

with open("results/benchmark_summary.txt", "w", encoding="utf-8") as f:
    f.write("Benchmark summary:\n\n")
    f.write(summary.to_string())

# tabella tempi medi
table = pd.pivot_table(
    df,
    index="dataset",
    columns="algorithm",
    values="time_ms",
    aggfunc="mean"
)

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
table.plot(kind="bar", figsize=(10,6))

plt.ylabel("Execution time (ms)")
plt.xlabel("Dataset")
plt.title("Dijkstra benchmark results")

plt.xticks(rotation=45)

plt.tight_layout()

plt.savefig("results/benchmark_plot.png", dpi=200)

plt.show()

# grafico scaling (tempo vs numero archi)
plt.figure(figsize=(8,6))

plt.scatter(df["edges"], df["time_ms"])

plt.xlabel("Number of edges")
plt.ylabel("Execution time (ms)")
plt.title("Dijkstra scaling behavior")

plt.xscale("log")
plt.yscale("log")

plt.tight_layout()

plt.savefig("results/scaling_plot.png", dpi=200)

plt.show()