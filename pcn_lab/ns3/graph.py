
import pandas as pd
import matplotlib.pyplot as plt
import sys
import os

# ---------------------------------
# 1. CHOOSE YOUR GRAPH
# ---------------------------------
graph_type = "bar"
# Options: bar, histogram, boxplot, line, scatter

# ---------------------------------
# 2. CHOOSE YOUR COLUMNS
# ---------------------------------
x_column = "Technology"
y_column = "Throughput"

# ---------------------------------
# 3. HARDCODED SAMPLE DATA
# Used when no CSV file is supplied
# ---------------------------------
data = {
    "Technology": [
        "WiFi", "WiFi", "WiFi",
        "Bluetooth", "Bluetooth", "Bluetooth",
        "Zigbee", "Zigbee", "Zigbee"
    ],
    "Speed": [1, 5, 10, 1, 5, 10, 1, 5, 10],
    "Throughput": [5.5, 4.2, 2.8,
                   0.9, 0.7, 0.4,
                   0.22, 0.18, 0.10],
    "Delay": [5, 8, 15, 10, 14, 22, 20, 28, 40]
}

df = pd.DataFrame(data)

# ---------------------------------
# 4. LOAD CSV IF PROVIDED
# Usage: python graphs.py data.csv
# ---------------------------------
if len(sys.argv) > 1:
    filename = sys.argv[1]

    if os.path.exists(filename):
        df = pd.read_csv(filename)
        print("Loaded CSV:", filename)
    else:
        print("File not found. Using sample data.")

print("\nAvailable columns:", list(df.columns))
print(df.head())

# ---------------------------------
# 5. DRAW THE SELECTED GRAPH
# ---------------------------------
plt.figure(figsize=(8, 5))

if graph_type == "bar":
    # Average value for each category
    df.groupby(x_column)[y_column].mean().plot(
        kind="bar"
    )
    plt.xlabel(x_column)
    plt.ylabel("Average " + y_column)

elif graph_type == "histogram":
    # Distribution of numerical values
    plt.hist(df[y_column].dropna(), bins=5,
             edgecolor="black")
    plt.xlabel(y_column)
    plt.ylabel("Frequency")

elif graph_type == "boxplot":
    # Distribution of values for each category
    df.boxplot(column=y_column, by=x_column)
    plt.suptitle("")
    plt.xlabel(x_column)
    plt.ylabel(y_column)

elif graph_type == "line":
    # Values across a numerical X axis
    df_sorted = df.sort_values(x_column)
    plt.plot(
        df_sorted[x_column],
        df_sorted[y_column],
        marker="o"
    )
    plt.xlabel(x_column)
    plt.ylabel(y_column)

elif graph_type == "scatter":
    # Relationship between two numerical columns
    plt.scatter(df[x_column], df[y_column])
    plt.xlabel(x_column)
    plt.ylabel(y_column)

else:
    print("Invalid graph type!")
    sys.exit(1)

plt.title(graph_type.capitalize() + " Graph")
plt.grid(True, alpha=0.3)
plt.tight_layout()
plt.show()
