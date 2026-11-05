"""Plot benchmark throughput against elapsed simulation time."""

import argparse
import csv
from pathlib import Path
from statistics import mean, median

import matplotlib.pyplot as plt
from matplotlib.ticker import FuncFormatter, MaxNLocator


def format_time(seconds, _position=None):
    hours, remainder = divmod(max(0, round(seconds)), 3600)
    minutes, seconds = divmod(remainder, 60)
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
            reader = csv.DictReader(file, skipinitialspace=True)
            rows = list(reader)
        times = [float(row["SimTime [s]"]) for row in rows]
        rates = [float(row["IterationsPerSecond"]) for row in rows]
        real_time = [float(row["RealTime [s]"]) for row in rows]
    except (OSError, UnicodeError, csv.Error, KeyError, TypeError, ValueError) as error:
        parser.error(f"Could not read benchmark CSV: {error}")
    if not times:
        parser.error("CSV contains no data rows")

    fig, (ax, realtime_ax) = plt.subplots(
        2, 1, sharex=True, figsize=(11, 8), layout="constrained"
    )
    ax.plot(times, rates, color="C0", linewidth=1.5, marker=".",
            markersize=4, label="Throughput (iterations/s)")
    realtime_ax.plot(times, real_time, color="C3", linewidth=1.5,
                     marker=".", markersize=4, label="Real time")
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
    fig.suptitle(args.csv_file.stem)
    ax.set_title("Throughput")
    realtime_ax.set_title("Real time")
    realtime_ax.set_xlabel("Simulation time (hh:mm:ss)")
    ax.set_ylabel("Iterations per second")
    realtime_ax.set_ylabel("Real time (hh:mm:ss)")
    realtime_ax.yaxis.set_major_formatter(FuncFormatter(format_time))
    realtime_ax.xaxis.set_major_locator(MaxNLocator(nbins=5))
    realtime_ax.xaxis.set_major_formatter(FuncFormatter(format_time))
    ax.yaxis.set_major_formatter(FuncFormatter(lambda value, _: f"{value:,.0f}"))
    ax.set_xlim(left=0, right=max(times) or 1)
    ax.grid(True, alpha=0.3)
    realtime_ax.grid(True, alpha=0.3)

    if args.save:
        fig.savefig(args.save, dpi=160)
    else:
        plt.show()


if __name__ == "__main__":
    main()
