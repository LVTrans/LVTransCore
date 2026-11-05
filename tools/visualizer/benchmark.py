"""Plot one benchmark run or compare two runs against simulation time."""

import argparse
import csv
import math
from pathlib import Path
from statistics import mean, median

import matplotlib.pyplot as plt
from matplotlib.ticker import FuncFormatter, MaxNLocator


def format_time(seconds, _position=None):
    hours, remainder = divmod(max(0, round(seconds)), 3600)
    minutes, seconds = divmod(remainder, 60)
    return f"{hours:02d}:{minutes:02d}:{seconds:02d}"


def read_benchmark(path):
    with path.open(newline="", encoding="utf-8-sig") as file:
        reader = csv.DictReader(file, skipinitialspace=True)
        if reader.fieldnames:
            reader.fieldnames = [name.strip() for name in reader.fieldnames]
        rows = list(reader)
    if not rows:
        raise ValueError("CSV contains no data rows")
    columns = tuple(
        [float(row[name]) for row in rows]
        for name in ("SimTime [s]", "IterationsPerSecond")
    )
    if any(not math.isfinite(value) for column in columns for value in column):
        raise ValueError("CSV contains nonfinite values")
    return columns


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("csv_file", type=Path, help="First benchmark CSV")
    parser.add_argument(
        "comparison_csv", type=Path, nargs="?", help="Optional second benchmark CSV"
    )
    parser.add_argument(
        "--save", type=Path, help="Save the plot instead of opening a window"
    )
    args = parser.parse_args()
    paths = [args.csv_file]
    if args.comparison_csv is not None:
        paths.append(args.comparison_csv)
    runs = []
    for path in paths:
        try:
            runs.append(read_benchmark(path))
        except (
            OSError,
            UnicodeError,
            csv.Error,
            KeyError,
            TypeError,
            ValueError,
        ) as error:
            parser.error(f"Could not read benchmark CSV '{path}': {error}")

    comparing = len(runs) == 2
    labels = [path.stem for path in paths]
    if comparing and labels[0] == labels[1]:
        labels = [f"Run {i + 1}: {path}" for i, path in enumerate(paths)]

    # Compare the first N samples from each run; statistics use the same subset.
    row_count = min(len(run[0]) for run in runs)
    runs = [tuple(column[:row_count] for column in run) for run in runs]
    fig, axes = plt.subplots(
        1,
        len(runs),
        squeeze=False,
        figsize=(16 if comparing else 11, 5),
        layout="constrained",
    )
    for index, (label, (times, rates)) in enumerate(zip(labels, runs)):
        ax = axes[0, index]
        ax.plot(
            times,
            rates,
            color="C0",
            linewidth=1.5,
            marker=".",
            markersize=4,
            label="Throughput (iterations/s)",
        )
        ax.axhline(
            mean(rates),
            color="C1",
            linestyle="--",
            label=f"Average: {mean(rates):,.1f} iterations/s",
        )
        ax.axhline(
            median(rates),
            color="C2",
            linestyle=":",
            label=f"Median: {median(rates):,.1f} iterations/s",
        )
        ax.legend(fontsize="small")
        ax.set_title(f"{label} - Throughput" if comparing else "Throughput")
        ax.set_xlabel("Simulation time (hh:mm:ss)")
        ax.set_ylabel("Iterations per second")
        ax.xaxis.set_major_locator(MaxNLocator(nbins=4))
        ax.xaxis.set_major_formatter(FuncFormatter(format_time))
        ax.yaxis.set_major_formatter(FuncFormatter(lambda value, _: f"{value:,.0f}"))
        ax.set_xlim(left=0, right=max(times) or 1)
        ax.grid(True, alpha=0.3)
    title = " vs ".join(labels)
    if comparing:
        title += f" (first {row_count}s)"
    fig.suptitle(title)

    if args.save:
        fig.savefig(args.save, dpi=160)
    else:
        plt.show()


if __name__ == "__main__":
    main()
