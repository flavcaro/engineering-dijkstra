from pathlib import Path

import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
import pandas as pd

RESULTS_DIR = Path("results")
RESULTS_DIR.mkdir(parents=True, exist_ok=True)

csv_path = RESULTS_DIR / "benchmark_results.csv"
if not csv_path.exists():
    raise FileNotFoundError("results/benchmark_results.csv non esiste. Esegui prima il benchmark.")
if csv_path.stat().st_size == 0:
    raise ValueError("results/benchmark_results.csv è vuoto.")

df = pd.read_csv(csv_path)

required_columns = {
    "family", "dataset", "graph_id", "n", "directed_edges", "attachment",
    "seed", "algorithm", "run", "time_ms", "pq_pushes", "pq_pops",
    "decrease_key_calls", "edge_relax_attempts", "successful_relaxations",
    "stale_entries_discarded", "settled_nodes"
}
missing = required_columns - set(df.columns)
if missing:
    raise ValueError(f"Colonne mancanti nel CSV: {sorted(missing)}")

algorithm_labels = {
    "binary_heap_lazy": "Binary heap (lazy)",
    "dary_heap": "4-ary heap",
    "pairing_heap": "Pairing heap",
}
algorithm_order = ["Binary heap (lazy)", "4-ary heap", "Pairing heap"]

counter_columns = [
    "pq_pushes", "pq_pops", "decrease_key_calls", "edge_relax_attempts",
    "successful_relaxations", "stale_entries_discarded", "settled_nodes"
}

# Genera automaticamente etichette pulite per la tesi/relazione
def dataset_label(row: pd.Series) -> str:
    fam = row["family"]
    if fam == "ba_scaling":
        return f"BA scaling (n={int(row['n'] / 1000)}k)"
    if fam == "ba_density":
        # Nel vecchio benchmark era mappato sotto random_density ma calcolato come BA density
        return f"BA density (a={int(row['attachment'])})" if row['attachment'] > 0 else f"Rand Dens (e={int(row['directed_edges'] / 1000)}k)"
    if fam == "rand_scaling":
        return f"Rand scaling (n={int(row['n'] / 1000)}k)"
    if fam == "rand_density":
        return f"Rand density (m={int(row['directed_edges'] / 1000)}k)"
    if fam == "grid":
        import math
        side = int(math.sqrt(row["n"]))
        return f"Grid {side}x{side}"
    return str(row["dataset"])

# Fornisce la chiave numerica corretta per ordinare l'asse X nei grafici
def dataset_order_key(row: pd.Series) -> int:
    fam = row["family"]
    if fam in ["ba_scaling", "rand_scaling", "grid"]:
        return int(row["n"])
    # Per le densità usiamo il numero di archi come coordinata X crescente
    return int(row["directed_edges"])

def save_table(table: pd.DataFrame, base_name: str, digits: int = 4) -> None:
    table.to_csv(RESULTS_DIR / f"{base_name}.csv")
    with open(RESULTS_DIR / f"{base_name}.txt", "w", encoding="utf-8") as f:
        f.write(table.to_string())
    with open(RESULTS_DIR / f"{base_name}.md", "w", encoding="utf-8") as f:
        f.write(table.round(digits).to_markdown())

df["algorithm_label"] = df["algorithm"].map(lambda x: algorithm_labels.get(x, x))

metadata = (
    df.groupby("dataset", as_index=False)
    .agg(
        family=("family", "first"),
        n=("n", "first"),
        attachment=("attachment", "first"),
        directed_edges=("directed_edges", "mean"),
        graphs=("graph_id", "nunique"),
        runs=("run", "nunique"),
    )
)
metadata["dataset_label"] = metadata.apply(dataset_label, axis=1)
metadata["dataset_order"] = metadata.apply(dataset_order_key, axis=1)

df = df.merge(metadata[["dataset", "dataset_label", "dataset_order"]], on="dataset", how="left")

metric_columns = ["time_ms", *counter_columns]
summary = (
    df.groupby(
        ["dataset", "dataset_label", "family", "dataset_order", "n", "attachment", "algorithm_label"],
        as_index=False
    )[metric_columns]
    .agg(["mean", "std", "count"])
)
summary.columns = ["_".join(col).strip("_") for col in summary.columns]
summary = summary.sort_values(["family", "dataset_order", "algorithm_label"])

save_table(metadata.sort_values(["family", "dataset_order"]), "benchmark_metadata", digits=2)
summary.to_csv(RESULTS_DIR / "benchmark_summary.csv", index=False)

def build_pivot(value_col: str, family: str | None = None) -> pd.DataFrame:
    data = summary.copy()
    if family is not None:
        data = data[data["family"] == family]
    table = data.pivot(index="dataset_label", columns="algorithm_label", values=value_col)
    ordered_index = (
        data[["dataset_label", "dataset_order"]]
        .drop_duplicates()
        .sort_values("dataset_order")["dataset_label"]
    )
    return table.reindex(index=ordered_index, columns=[c for c in algorithm_order if c in table.columns])

def plot_line(family: str, filename: str, title: str, x_col: str, x_label: str) -> None:
    data = summary[summary["family"] == family].copy().sort_values(x_col)
    if data.empty:
        return
    plt.figure(figsize=(8, 5))
    for algorithm in algorithm_order:
        group = data[data["algorithm_label"] == algorithm].sort_values(x_col)
        if not group.empty:
            plt.plot(group[x_col], group["time_ms_mean"], marker="o", label=algorithm)
    plt.xlabel(x_label)
    plt.ylabel("Execution time (ms)")
    plt.title(title)
    plt.grid(True, alpha=0.35)
    plt.legend()
    plt.tight_layout()
    plt.savefig(RESULTS_DIR / filename, dpi=200)
    plt.close()

def plot_speedup(table: pd.DataFrame, filename: str, title: str) -> None:
    if table.empty:
        return
    ax = table.plot(kind="bar", figsize=(10, 5))
    ax.set_title(title)
    ax.set_xlabel("Dataset")
    ax.set_ylabel("Speedup vs binary")
    ax.axhline(1.0, linestyle="--", color="black", linewidth=1)
    plt.xticks(rotation=35, ha="right")
    plt.tight_layout()
    plt.savefig(RESULTS_DIR / filename, dpi=200)
    plt.close()

mean_table = build_pivot("time_ms_mean")
std_table = build_pivot("time_ms_std")
save_table(mean_table, "benchmark_table_mean")
save_table(std_table, "benchmark_table_std")

# Ciclo su tutte e 5 le famiglie reali presenti nel nuovo CSV dell'Opzione B
all_families = ["ba_scaling", "ba_density", "rand_scaling", "rand_density", "grid"]
for family in all_families:
    save_table(build_pivot("time_ms_mean", family), f"{family}_table_mean")
    save_table(build_pivot("time_ms_std", family), f"{family}_table_std")

for counter in counter_columns:
    save_table(build_pivot(f"{counter}_mean"), f"counter_{counter}_mean", digits=2)

speedup_rows = []
for dataset_name in summary["dataset"].unique():
    subset = summary[summary["dataset"] == dataset_name]
    binary = subset[subset["algorithm_label"] == "Binary heap (lazy)"]
    if binary.empty:
        continue
    binary_time = binary["time_ms_mean"].iloc[0]
    for _, row in subset.iterrows():
        speedup_rows.append({
            "dataset": dataset_name,
            "dataset_label": row["dataset_label"],
            "family": row["family"],
            "dataset_order": row["dataset_order"],
            "algorithm_label": row["algorithm_label"],
            "speedup_vs_binary": binary_time / row["time_ms_mean"],
        })

speedup_df = pd.DataFrame(speedup_rows)
if not speedup_df.empty:
    speedup_df = speedup_df.sort_values(["family", "dataset_order", "algorithm_label"])
    speedup_df.to_csv(RESULTS_DIR / "benchmark_speedup.csv", index=False)

    speedup_table = speedup_df.pivot(index="dataset_label", columns="algorithm_label", values="speedup_vs_binary")
    ordered_labels = speedup_df[["dataset_label", "family", "dataset_order"]].drop_duplicates().sort_values(["family", "dataset_order"])["dataset_label"]
    speedup_table = speedup_table.reindex(index=ordered_labels, columns=[c for c in algorithm_order if c in speedup_table.columns])
    save_table(speedup_table, "benchmark_speedup")
    plot_speedup(speedup_table, "speedup_plot.png", "Speedup relative to Binary heap (lazy)")

    for family in all_families:
        family_speedup = speedup_df[speedup_df["family"] == family]
        if family_speedup.empty:
            continue
        table = family_speedup.pivot(index="dataset_label", columns="algorithm_label", values="speedup_vs_binary")
        ordered = family_speedup[["dataset_label", "dataset_order"]].drop_duplicates().sort_values("dataset_order")["dataset_label"]
        table = table.reindex(index=ordered, columns=[c for c in algorithm_order if c in table.columns])
        save_table(table, f"{family}_speedup")
        plot_speedup(table, f"{family}_speedup_plot.png", f"Speedup on {family.replace('_', ' ').title()}")

# Tracciamento dei grafici a linee per l'evoluzione temporale
plot_line("ba_scaling", "ba_scaling_plot.png", "Scaling on Barabasi-Albert graphs", "n", "Number of nodes")
plot_line("rand_scaling", "rand_scaling_plot.png", "Scaling on Random graphs (Erdos-Renyi)", "n", "Number of nodes")
plot_line("rand_density", "rand_density_plot.png", "Effect of Edge Density (Fixed V=10k)", "directed_edges", "Number of Directed Edges")
plot_line("grid", "grid_plot.png", "Scaling on 2D Grid Graphs", "n", "Total nodes (R x C)")

# Boxplot della variabilità di computazione hardware
for family in all_families:
    for algorithm in algorithm_order:
        subset = df[(df["family"] == family) & (df["algorithm_label"] == algorithm)].copy()
        if subset.empty:
            continue
        labels = subset[["dataset_label", "dataset_order"]].drop_duplicates().sort_values("dataset_order")["dataset_label"].tolist()
        values = [subset[subset["dataset_label"] == label]["time_ms"].values for label in labels]
        plt.figure(figsize=(10, 5))
        plt.boxplot(values, tick_labels=labels)
        plt.title(f"Run variability - {algorithm} - {family.replace('_', ' ').title()}")
        plt.ylabel("Execution time (ms)")
        plt.xticks(rotation=35, ha="right")
        plt.tight_layout()
        safe_algorithm_name = algorithm.lower().replace(" ", "_").replace("(", "").replace(")", "").replace("-", "")
        plt.savefig(RESULTS_DIR / f"boxplot_{family}_{safe_algorithm_name}.png", dpi=200)
        plt.close()

print("Analisi completata con successo! Tabelle e grafici esportati in results/.")