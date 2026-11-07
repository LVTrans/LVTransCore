"""Plot benchmark CSVs together against simulation time."""

import argparse
import csv
import math
from fnmatch import fnmatchcase
from pathlib import Path
from statistics import median

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


def plot_benchmarks(paths, runs, separate=False):
    comparing = len(runs) > 1
    labels = [path.stem for path in paths]
    labels = [
        f"Run {i + 1}: {path}" if labels.count(label) > 1 else label
        for i, (path, label) in enumerate(zip(paths, labels))
    ]

    plot_count = len(runs) if separate else 1
    columns = min(plot_count, 2)
    rows = math.ceil(plot_count / columns)
    fig, axes = plt.subplots(
        rows,
        columns,
        squeeze=False,
        figsize=(14 if columns > 1 else 11, 4 * rows if rows > 1 else 5),
        layout="constrained",
    )
    axes = axes.ravel()
    for ax in axes[plot_count:]:
        fig.delaxes(ax)
    axes = axes[:plot_count]
    for index, (label, (times, rates)) in enumerate(zip(labels, runs)):
        ax = axes[index if separate else 0]
        ax.plot(
            times,
            rates,
            color=f"C{index % 10}" if not separate else "C0",
            linewidth=1.5,
            marker=".",
            markersize=4,
            label=label,
        )
        if separate or not comparing:
            ax.axhline(
                median(rates),
                color="C2",
                linestyle=":",
                label=f"Median: {median(rates):,.1f} iterations/s",
            )
        if separate:
            ax.set_title(label)
            ax.set_xlim(left=0, right=max(times) or 1)

    if not separate:
        axes[0].set_xlim(left=0, right=max(max(times) for times, _ in runs) or 1)
    for ax in axes:
        ax.legend(fontsize="small")
        ax.set_xlabel("Simulation time (hh:mm:ss)")
        ax.set_ylabel("Iterations per second")
        ax.xaxis.set_major_locator(MaxNLocator(nbins=4))
        ax.xaxis.set_major_formatter(FuncFormatter(format_time))
        ax.yaxis.set_major_formatter(FuncFormatter(lambda value, _: f"{value:,.0f}"))
        ax.grid(True, alpha=0.3)
    fig.suptitle("Benchmark throughput" if comparing else labels[0])
    return fig


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "csv_files", type=Path, nargs="+", help="One or more benchmark CSV files"
    )
    parser.add_argument(
        "--separate", action="store_true",
        help="Show each run in its own subplot, in a grid with at most two columns",
    )
    parser.add_argument(
        "--save", type=Path, help="Save the plot instead of opening a window"
    )
    parser.add_argument(
        "--exclude", action="append", default=[], metavar="PATTERN",
        help="Skip matching filenames or supplied paths (repeatable; quote wildcards)",
    )
    args = parser.parse_args()
    paths = [
        path for path in args.csv_files
        if not any(
            fnmatchcase(path.name, pattern) or fnmatchcase(str(path), pattern)
            for pattern in args.exclude
        )
    ]
    if not paths:
        parser.error("No benchmark CSV files remain after exclusions")
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

    fig = plot_benchmarks(paths, runs, separate=args.separate)

    if args.save:
        fig.savefig(args.save, dpi=160)
        plt.close(fig)
    else:
        plt.show()


if __name__ == "__main__":
    main()
