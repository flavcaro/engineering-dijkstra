import pandas as pd
import matplotlib.pyplot as plt
from pathlib import Path

Path("results").mkdir(parents=True, exist_ok=True)

csv_path = Path("results/benchmark_results.csv")

if not csv_path.exists():
    raise FileNotFoundError("results/benchmark_results.csv non esiste. Esegui prima ./benchmark.exe")

if csv_path.stat().st_size == 0:
    raise ValueError("results/benchmark_results.csv è vuoto. Il benchmark non ha scritto risultati.")
df = pd.read_csv(csv_path)

algorithm_labels = {
    "binary_heap_lazy": "Binary heap (lazy)",
    "dary_heap": "4-ary heap",
    "pairing_heap": "Pairing heap"
}
df["algorithm_label"] = df["algorithm"].map(lambda x: algorithm_labels.get(x, x))

dataset_info = {
    "random_n1000_m5000": {"label": "Random (1k, 5k)", "edges": 5000, "category": "random_scaling"},
    "random_n5000_m25000": {"label": "Random (5k, 25k)", "edges": 25000, "category": "random_scaling"},
    "random_n10000_m50000": {"label": "Random (10k, 50k)", "edges": 50000, "category": "random_scaling"},
    "random_n20000_m100000": {"label": "Random (20k, 100k)", "edges": 100000, "category": "random_scaling"},
    "random_n50000_m250000": {"label": "Random (50k, 250k)", "edges": 250000, "category": "random_scaling"},

    "random_density_n10000_m20000": {"label": "Density (10k, 20k)", "edges": 20000, "category": "random_density"},
    "random_density_n10000_m50000": {"label": "Density (10k, 50k)", "edges": 50000, "category": "random_density"},
    "random_density_n10000_m100000": {"label": "Density (10k, 100k)", "edges": 100000, "category": "random_density"},
    "random_density_n10000_m200000": {"label": "Density (10k, 200k)", "edges": 200000, "category": "random_density"},

    "grid_50x50": {"label": "Grid 50x50", "edges": 4900, "category": "grid"},
    "grid_100x100": {"label": "Grid 100x100", "edges": 19800, "category": "grid"},
    "grid_300x300": {"label": "Grid 300x300", "edges": 179400, "category": "grid"},

    "com_youtube": {"label": "YouTube", "edges": 3000000, "category": "real"}
}

df["dataset_label"] = df["dataset"].map(lambda x: dataset_info[x]["label"])
df["edges"] = df["dataset"].map(lambda x: dataset_info[x]["edges"])
df["category"] = df["dataset"].map(lambda x: dataset_info[x]["category"])

summary = (
    df.groupby(["dataset", "dataset_label", "category", "algorithm_label"])["time_ms"]
    .agg(["mean", "std", "count"])
    .reset_index()
)

print("\nBenchmark summary:\n")
print(summary)

with open("results/benchmark_summary.txt", "w", encoding="utf-8") as f:
    f.write(summary.to_string(index=False))

# Tabella medie
mean_table = summary.pivot(
    index="dataset_label",
    columns="algorithm_label",
    values="mean"
)

desired_order = ["Binary heap (lazy)", "4-ary heap", "Pairing heap"]
mean_table = mean_table.reindex(columns=[c for c in desired_order if c in mean_table.columns])

print("\nMean execution times:\n")
print(mean_table)

with open("results/benchmark_table_mean.txt", "w", encoding="utf-8") as f:
    f.write(mean_table.to_string())

with open("results/benchmark_table_mean.md", "w", encoding="utf-8") as f:
    f.write(mean_table.round(4).to_markdown())

# Tabella deviazioni standard
std_table = summary.pivot(
    index="dataset_label",
    columns="algorithm_label",
    values="std"
)
std_table = std_table.reindex(columns=[c for c in desired_order if c in std_table.columns])

with open("results/benchmark_table_std.txt", "w", encoding="utf-8") as f:
    f.write(std_table.to_string())

with open("results/benchmark_table_std.md", "w", encoding="utf-8") as f:
    f.write(std_table.round(4).to_markdown())

# Grafico generale a barre
mean_table.plot(kind="bar", figsize=(12, 6))
plt.ylabel("Execution time (ms)")
plt.xlabel("Dataset")
plt.title("Dijkstra benchmark results")
plt.xticks(rotation=45, ha="right")
plt.tight_layout()
plt.savefig("results/benchmark_plot.png", dpi=200)
plt.show()

# Scaling plot solo sui random_scaling
plt.figure(figsize=(8, 6))
scaling_df = summary[summary["category"] == "random_scaling"]

for algorithm, group in scaling_df.groupby("algorithm_label"):
    group = group.sort_values("dataset")
    x = group["dataset"].map(lambda x: dataset_info[x]["edges"])
    y = group["mean"]

    plt.plot(x, y, marker="o", label=algorithm)

plt.xlabel("Number of edges")
plt.ylabel("Execution time (ms)")
plt.title("Scaling on random graphs")
plt.xscale("log")
plt.yscale("log")
plt.legend()
plt.grid(True)
plt.tight_layout()
plt.savefig("results/scaling_plot.png", dpi=200)
plt.show()

# Density plot
plt.figure(figsize=(8, 6))
density_df = summary[summary["category"] == "random_density"]

for algorithm, group in density_df.groupby("algorithm_label"):
    group = group.sort_values("dataset")
    x = group["dataset"].map(lambda x: dataset_info[x]["edges"])
    y = group["mean"]

    plt.plot(x, y, marker="o", label=algorithm)

plt.xlabel("Number of edges")
plt.ylabel("Execution time (ms)")
plt.title("Effect of graph density")
plt.xscale("log")
plt.yscale("log")
plt.legend()
plt.grid(True)
plt.tight_layout()
plt.savefig("results/density_plot.png", dpi=200)
plt.show()

# Speedup rispetto al binary heap
speedup_rows = []
for dataset in summary["dataset"].unique():
    dataset_subset = summary[summary["dataset"] == dataset]
    binary_time = dataset_subset.loc[
        dataset_subset["algorithm_label"] == "Binary heap (lazy)", "mean"
    ].values[0]

    for _, row in dataset_subset.iterrows():
        speedup = binary_time / row["mean"]
        speedup_rows.append({
            "dataset": dataset,
            "dataset_label": row["dataset_label"],
            "algorithm_label": row["algorithm_label"],
            "speedup_vs_binary": speedup
        })

speedup_df = pd.DataFrame(speedup_rows)

speedup_table = speedup_df.pivot(
    index="dataset_label",
    columns="algorithm_label",
    values="speedup_vs_binary"
)
speedup_table = speedup_table.reindex(columns=[c for c in desired_order if c in speedup_table.columns])

with open("results/benchmark_speedup.md", "w", encoding="utf-8") as f:
    f.write(speedup_table.round(4).to_markdown())

print("\nSpeedup vs Binary Heap:\n")
print(speedup_table.round(4))