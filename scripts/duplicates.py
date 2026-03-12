import pandas as pd

df = pd.read_csv("results/benchmark_results.csv")
print(df.groupby(["dataset", "algorithm"]).size())