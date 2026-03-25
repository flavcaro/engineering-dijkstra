import pandas as pd
import matplotlib.pyplot as plt
from pathlib import Path

# ============================================================
# Setup
# ============================================================

RESULTS_DIR = Path("results")
RESULTS_DIR.mkdir(parents=True, exist_ok=True)

csv_path = RESULTS_DIR / "benchmark_results.csv"

if not csv_path.exists():
    raise FileNotFoundError(
        "results/benchmark_results.csv non esiste. Esegui prima ./benchmark.exe"
    )

if csv_path.stat().st_size == 0:
    raise ValueError(
        "results/benchmark_results.csv è vuoto. Il benchmark non ha scritto risultati."
    )

df = pd.read_csv(csv_path)

required_columns = {"dataset", "algorithm", "time_ms"}
missing = required_columns - set(df.columns)
if missing:
    raise ValueError(f"Colonne mancanti nel CSV: {sorted(missing)}")

# ============================================================
# Labels e metadati
# ============================================================

algorithm_labels = {
    "binary_heap_lazy": "Binary heap (lazy)",
    "dary_heap": "4-ary heap",
    "pairing_heap": "Pairing heap",
}

algorithm_order = [
    "Binary heap (lazy)",
    "4-ary heap",
    "Pairing heap",
]

dataset_info = {
    # Random scaling
    "random_n1000_m5000": {
        "label": "Random (1k, 5k)",
        "edges": 5000,
        "category": "random_scaling",
        "order": 1,
    },
    "random_n5000_m25000": {
        "label": "Random (5k, 25k)",
        "edges": 25000,
        "category": "random_scaling",
        "order": 2,
    },
    "random_n10000_m50000": {
        "label": "Random (10k, 50k)",
        "edges": 50000,
        "category": "random_scaling",
        "order": 3,
    },
    "random_n20000_m100000": {
        "label": "Random (20k, 100k)",
        "edges": 100000,
        "category": "random_scaling",
        "order": 4,
    },
    "random_n50000_m250000": {
        "label": "Random (50k, 250k)",
        "edges": 250000,
        "category": "random_scaling",
        "order": 5,
    },

    # Random density
    "random_density_n10000_m20000": {
        "label": "Density (10k, 20k)",
        "edges": 20000,
        "category": "random_density",
        "order": 1,
    },
    "random_density_n10000_m50000": {
        "label": "Density (10k, 50k)",
        "edges": 50000,
        "category": "random_density",
        "order": 2,
    },
    "random_density_n10000_m100000": {
        "label": "Density (10k, 100k)",
        "edges": 100000,
        "category": "random_density",
        "order": 3,
    },
    "random_density_n10000_m200000": {
        "label": "Density (10k, 200k)",
        "edges": 200000,
        "category": "random_density",
        "order": 4,
    },

    # Grid
    "grid_50x50": {
        "label": "Grid 50x50",
        "edges": 4900,
        "category": "grid",
        "order": 1,
    },
    "grid_100x100": {
        "label": "Grid 100x100",
        "edges": 19800,
        "category": "grid",
        "order": 2,
    },
    "grid_300x300": {
        "label": "Grid 300x300",
        "edges": 179400,
        "category": "grid",
        "order": 3,
    },

    # Real
    "com_youtube": {
        "label": "YouTube",
        "edges": 3000000,
        "category": "real",
        "order": 1,
    },
    "roadNet_CA": {
        "label": "roadNet-CA",
        "edges": 2700000,
        "category": "real",
        "order": 2,
    },
}

unknown_datasets = sorted(set(df["dataset"]) - set(dataset_info.keys()))
if unknown_datasets:
    raise ValueError(
        "Questi dataset compaiono nel CSV ma non sono definiti in dataset_info:\n"
        + "\n".join(unknown_datasets)
    )

df["algorithm_label"] = df["algorithm"].map(lambda x: algorithm_labels.get(x, x))
df["dataset_label"] = df["dataset"].map(lambda x: dataset_info[x]["label"])
df["edges"] = df["dataset"].map(lambda x: dataset_info[x]["edges"])
df["category"] = df["dataset"].map(lambda x: dataset_info[x]["category"])
df["dataset_order"] = df["dataset"].map(lambda x: dataset_info[x]["order"])

# ============================================================
# Summary statistico
# ============================================================

summary = (
    df.groupby(
        ["dataset", "dataset_label", "category", "dataset_order", "edges", "algorithm_label"],
        as_index=False
    )["time_ms"]
    .agg(["mean", "std", "count"])
    .reset_index()
)

summary = summary.sort_values(["category", "dataset_order", "algorithm_label"])

print("\nBenchmark summary:\n")
print(summary)

with open(RESULTS_DIR / "benchmark_summary.txt", "w", encoding="utf-8") as f:
    f.write(summary.to_string(index=False))

# ============================================================
# Funzioni utili
# ============================================================

def build_table(value_col: str, category: str | None = None) -> pd.DataFrame:
    data = summary.copy()
    if category is not None:
        data = data[data["category"] == category]

    data = data.sort_values(["dataset_order", "algorithm_label"])

    table = data.pivot(
        index="dataset_label",
        columns="algorithm_label",
        values=value_col
    )

    existing_columns = [c for c in algorithm_order if c in table.columns]
    table = table.reindex(columns=existing_columns)
    return table


def save_table_files(table: pd.DataFrame, base_name: str, round_digits: int = 4) -> None:
    with open(RESULTS_DIR / f"{base_name}.txt", "w", encoding="utf-8") as f:
        f.write(table.to_string())

    with open(RESULTS_DIR / f"{base_name}.md", "w", encoding="utf-8") as f:
        f.write(table.round(round_digits).to_markdown())


def plot_bar_table(
    table: pd.DataFrame,
    title: str,
    filename: str,
    ylabel: str = "Execution time (ms)",
    rotation: int = 35
) -> None:
    ax = table.plot(kind="bar", figsize=(10, 5))
    ax.set_title(title)
    ax.set_xlabel("Dataset")
    ax.set_ylabel(ylabel)
    plt.xticks(rotation=rotation, ha="right")
    plt.tight_layout()
    plt.savefig(RESULTS_DIR / filename, dpi=200)
    plt.show()


def plot_line_by_category(
    category: str,
    title: str,
    filename: str,
    x_label: str = "Number of edges",
    y_label: str = "Execution time (ms)",
    logx: bool = True,
    logy: bool = True
) -> None:
    data = summary[summary["category"] == category].copy()
    data = data.sort_values(["edges", "algorithm_label"])

    plt.figure(figsize=(8, 6))

    for algorithm in algorithm_order:
        group = data[data["algorithm_label"] == algorithm].sort_values("edges")
        if group.empty:
            continue

        plt.plot(
            group["edges"],
            group["mean"],
            marker="o",
            label=algorithm
        )

    plt.xlabel(x_label)
    plt.ylabel(y_label)
    plt.title(title)

    if logx:
        plt.xscale("log")
    if logy:
        plt.yscale("log")

    plt.legend()
    plt.grid(True)
    plt.tight_layout()
    plt.savefig(RESULTS_DIR / filename, dpi=200)
    plt.show()


# ============================================================
# Tabelle globali
# ============================================================

mean_table = build_table("mean")
std_table = build_table("std")

print("\nMean execution times:\n")
print(mean_table)

print("\nStandard deviation of execution times:\n")
print(std_table)

save_table_files(mean_table, "benchmark_table_mean")
save_table_files(std_table, "benchmark_table_std")

# ============================================================
# Tabelle separate per categoria
# ============================================================

for category in ["random_scaling", "random_density", "grid", "real"]:
    mean_cat = build_table("mean", category=category)
    std_cat = build_table("std", category=category)

    save_table_files(mean_cat, f"{category}_table_mean")
    save_table_files(std_cat, f"{category}_table_std")

# ============================================================
# Grafico generale
# ============================================================

plot_bar_table(
    mean_table,
    title="Dijkstra benchmark results",
    filename="benchmark_plot_all.png"
)

# ============================================================
# Grafici separati per categoria
# ============================================================

random_scaling_table = build_table("mean", category="random_scaling")
random_density_table = build_table("mean", category="random_density")
grid_table = build_table("mean", category="grid")
real_table = build_table("mean", category="real")

plot_bar_table(
    random_scaling_table,
    title="Random graphs: scaling study",
    filename="benchmark_plot_random_scaling.png"
)

plot_bar_table(
    random_density_table,
    title="Random graphs: density study",
    filename="benchmark_plot_random_density.png"
)

plot_bar_table(
    grid_table,
    title="Grid graphs",
    filename="benchmark_plot_grid.png"
)

plot_bar_table(
    real_table,
    title="Real datasets",
    filename="benchmark_plot_real.png"
)

# ============================================================
# Grafici lineari: scaling e density
# ============================================================

plot_line_by_category(
    category="random_scaling",
    title="Scaling on random graphs",
    filename="scaling_plot.png"
)

plot_line_by_category(
    category="random_density",
    title="Effect of graph density",
    filename="density_plot.png"
)

# ============================================================
# Speedup rispetto al binary heap
# ============================================================

speedup_rows = []

for dataset_name in summary["dataset"].unique():
    dataset_subset = summary[summary["dataset"] == dataset_name].copy()

    binary_row = dataset_subset[
        dataset_subset["algorithm_label"] == "Binary heap (lazy)"
    ]

    if binary_row.empty:
        continue

    binary_time = binary_row["mean"].iloc[0]

    for _, row in dataset_subset.iterrows():
        speedup_rows.append({
            "dataset": dataset_name,
            "dataset_label": row["dataset_label"],
            "category": row["category"],
            "dataset_order": row["dataset_order"],
            "algorithm_label": row["algorithm_label"],
            "speedup_vs_binary": binary_time / row["mean"],
        })

speedup_df = pd.DataFrame(speedup_rows)
speedup_df = speedup_df.sort_values(["category", "dataset_order", "algorithm_label"])

speedup_table = speedup_df.pivot(
    index="dataset_label",
    columns="algorithm_label",
    values="speedup_vs_binary"
)

existing_columns = [c for c in algorithm_order if c in speedup_table.columns]
speedup_table = speedup_table.reindex(columns=existing_columns)

print("\nSpeedup vs Binary Heap:\n")
print(speedup_table.round(4))

save_table_files(speedup_table, "benchmark_speedup")

# Speedup separato per categoria
for category in ["random_scaling", "random_density", "grid", "real"]:
    cat_df = speedup_df[speedup_df["category"] == category]
    cat_table = cat_df.pivot(
        index="dataset_label",
        columns="algorithm_label",
        values="speedup_vs_binary"
    )
    cat_table = cat_table.reindex(columns=[c for c in algorithm_order if c in cat_table.columns])
    save_table_files(cat_table, f"{category}_speedup")

# Grafico speedup generale
ax = speedup_table.plot(kind="bar", figsize=(12, 6))
ax.set_title("Speedup relative to Binary heap (lazy)")
ax.set_xlabel("Dataset")
ax.set_ylabel("Speedup")
plt.xticks(rotation=35, ha="right")
plt.axhline(1.0, linestyle="--")
plt.tight_layout()
plt.savefig(RESULTS_DIR / "speedup_plot.png", dpi=200)
plt.show()

# ============================================================
# Boxplot dei tempi per run
# Serve a far vedere meglio la variabilità tra esecuzioni
# ============================================================

for category in ["random_scaling", "random_density", "grid", "real"]:
    cat_df = df[df["category"] == category].copy()
    if cat_df.empty:
        continue

    labels_in_order = [
        dataset_info[name]["label"]
        for name in dataset_info
        if dataset_info[name]["category"] == category
        and name in set(cat_df["dataset"])
    ]

    # Faccio un boxplot per ogni algoritmo separatamente
    for algorithm in algorithm_order:
        algo_df = cat_df[cat_df["algorithm_label"] == algorithm].copy()
        if algo_df.empty:
            continue

        data_for_box = []
        valid_labels = []

        for dataset_name in dataset_info:
            info = dataset_info[dataset_name]
            if info["category"] != category:
                continue

            subset = algo_df[algo_df["dataset"] == dataset_name]["time_ms"]
            if not subset.empty:
                data_for_box.append(subset.values)
                valid_labels.append(info["label"])

        if not data_for_box:
            continue

        plt.figure(figsize=(10, 5))
        plt.boxplot(data_for_box, tick_labels=valid_labels)
        plt.title(f"Run variability - {algorithm} - {category.replace('_', ' ').title()}")
        plt.ylabel("Execution time (ms)")
        plt.xticks(rotation=35, ha="right")
        plt.tight_layout()

        safe_algorithm_name = (
            algorithm.lower()
            .replace(" ", "_")
            .replace("(", "")
            .replace(")", "")
            .replace("-", "")
        )
        plt.savefig(
            RESULTS_DIR / f"boxplot_{category}_{safe_algorithm_name}.png",
            dpi=200
        )
        plt.show()

print("\nAnalisi completata. File salvati nella cartella results/.")