"""Plot benchmark throughput against elapsed simulation time."""

import argparse
import csv
from pathlib import Path
from statistics import mean, median

import matplotlib.pyplot as plt
from matplotlib.ticker import FuncFormatter, MaxNLocator


def elapsed_seconds(row):
    if "Time (hh:mm:ss)" in row:
        hours, minutes, seconds = map(float, row["Time (hh:mm:ss)"].split(":"))
        return hours * 3600 + minutes * 60 + seconds
    return float(row["Time"])


def format_time(seconds, _position=None):
    hours, remainder = divmod(max(0, round(seconds)), 3600)
    minutes, seconds = divmod(remainder, 60)
    # Elapsed hours can exceed 24; this is not a time of day.
    return f"{hours:02d}:{minutes:02d}:{seconds:02d}"


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("csv_file", type=Path)
    parser.add_argument(
        "--save", type=Path, help="Save the plot instead of opening a window"
    )
    args = parser.parse_args()

    try:
        with args.csv_file.open(newline="", encoding="utf-8-sig") as file:
            rows = list(csv.DictReader(file))
        times = [elapsed_seconds(row) for row in rows]
        rates = [float(row["IterationsPerSecond"]) for row in rows]
    except (OSError, UnicodeError, csv.Error, KeyError, TypeError, ValueError) as error:
        parser.error(f"Could not read benchmark CSV: {error}")
    if not times:
        parser.error("CSV contains no data rows")

    fig, ax = plt.subplots(figsize=(11, 5), layout="constrained")
    ax.plot(times, rates, linewidth=1.5, marker=".", markersize=4)
    average_rate = mean(rates)
    median_rate = median(rates)
    ax.axhline(
        average_rate,
        color="C1",
        linestyle="--",
        label=f"Average: {average_rate:,.1f} iterations/s",
    )
    ax.axhline(
        median_rate,
        color="C2",
        linestyle=":",
        label=f"Median: {median_rate:,.1f} iterations/s",
    )
    ax.legend()
    ax.set_title(args.csv_file.stem)
    ax.set_xlabel("Simulation time (hh:mm:ss)")
    ax.set_ylabel("Iterations per second")
    ax.xaxis.set_major_locator(MaxNLocator(nbins=7))
    ax.xaxis.set_major_formatter(FuncFormatter(format_time))
    ax.yaxis.set_major_formatter(FuncFormatter(lambda value, _: f"{value:,.0f}"))
    ax.set_xlim(left=0, right=max(times) or 1)
    ax.grid(True, alpha=0.3)

    if args.save:
        fig.savefig(args.save, dpi=160)
    else:
        plt.show()


if __name__ == "__main__":
    main()
