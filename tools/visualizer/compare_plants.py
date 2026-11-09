"""Save all plant benchmark plots and a paired C++/LabVIEW median report.

Run from any directory: python3 tools/visualizer/compare_plants.py
Uses benchmark.py for reading and plotting the original run samples.
"""

import argparse
import os
from pathlib import Path
from statistics import median

os.environ.setdefault("MPLCONFIGDIR", "/tmp/lvtranscore-matplotlib")

import matplotlib

matplotlib.use("Agg")
import matplotlib.pyplot as plt
from matplotlib.ticker import FuncFormatter

from benchmark import plot_benchmarks, read_benchmark


ROOT = Path(__file__).resolve().parents[2]
DT_VALUES = ("0.1", "0.01", "0.001", "0.0001", "0.00001")
COLORS = ("#003f5c", "#bb4e99")


def throughput(value):
    if value >= 1_000_000:
        return f"{value / 1_000_000:.3f}M"
    return f"{value / 1_000:.3f}k"


def save_runs(paths, runs, labels, title, destination):
    fig = plot_benchmarks(paths, runs, separate=len(runs) > 1)
    fig.suptitle(title + "\nAll recorded samples; median excludes the first sample")
    for ax, label, (times, rates) in zip(fig.axes, labels, runs):
        ax.lines[0].set_label(label)
        ax.lines[0].set_color(COLORS[label == "LabVIEW"])
        rate = median(rates[1:])
        ax.lines[1].set_ydata([rate, rate])
        ax.lines[1].set_label(f"Median: {rate:,.1f} iterations/s")
        ax.set_title(label)
        if max(times) < 60:
            ax.xaxis.set_major_formatter(FuncFormatter(lambda value, _: f"{value:g}"))
            ax.set_xlabel("Simulation time (s)")
        ax.legend(fontsize="small")
    fig.savefig(destination, dpi=160)
    plt.close(fig)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--data-dir", type=Path, default=ROOT / "benchmarks/benchmark_data"
    )
    parser.add_argument(
        "--output-dir", type=Path, default=ROOT / "benchmarks/plots/plant_comparison"
    )
    args = parser.parse_args()

    # Discover and validate all pairs before writing any output.
    pairs = []
    for plant in "ABC":
        directory = args.data_dir / f"plant{plant}"
        files = sorted(p for p in directory.iterdir() if p.suffix in {".csv", ".txt"})
        used = set()
        for dt in DT_VALUES:
            paths, runs = [], []
            for labview in (False, True):
                matches = [
                    p
                    for p in files
                    if p.stem.endswith(f"_dt_{dt}") and ("LabVIEW" in p.stem) == labview
                ]
                if len(matches) != 1:
                    parser.error(
                        f"Expected one {plant=} {dt=} {labview=} file, found {matches}"
                    )
                path = matches[0]
                times, rates = read_benchmark(path)
                if len(rates) < 2 or any(rate <= 0 for rate in rates[1:]):
                    parser.error(
                        f"{path}: expected positive throughput after the first sample"
                    )
                if any(right < left for left, right in zip(times, times[1:])):
                    parser.error(f"{path}: simulation time decreases")
                paths.append(path)
                runs.append((times, rates))
                used.add(path)
            pairs.append((plant, dt, paths, runs))
        if used != set(files):
            parser.error(f"Unmatched benchmark files: {set(files) - used}")

    output = args.output_dir
    output.mkdir(parents=True, exist_ok=True)
    report = [
        "# C++ and LabVIEW plant benchmarks",
        "",
        "Throughput is the median of `IterationsPerSecond`",
        "![Median throughput and speedup](summary.png)",
        "",
        "| Plant | dt (s) | C++ (iterations/s) | LabVIEW (iterations/s) | C++ speedup | Samples used (C++ / LV) | Paired plot |",
        "|---|---:|---:|---:|---:|---:|---|",
    ]
    sources = ["", "## Individual runs and source data", ""]
    summaries = {plant: [] for plant in "ABC"}
    for plant, dt, paths, runs in pairs:
        folder = output / f"plant{plant}"
        folder.mkdir(exist_ok=True)
        medians = [median(rates[1:]) for _, rates in runs]
        speedup = medians[0] / medians[1]
        summaries[plant].append((*medians, speedup))
        pair_name = f"plant{plant}_dt_{dt}_comparison.png"
        save_runs(
            paths,
            runs,
            ["C++", "LabVIEW"],
            f"Plant {plant}, dt = {dt} s",
            folder / pair_name,
        )
        report.append(
            f"| {plant} | {dt} | {throughput(medians[0])} | {throughput(medians[1])} | "
            f"{speedup:.2f}× | {len(runs[0][1]) - 1} / {len(runs[1][1]) - 1} | "
            f"[Plot](plant{plant}/{pair_name}) |"
        )
        for path, run, label in zip(paths, runs, ("C++", "LabVIEW")):
            image_name = path.stem + ".png"
            save_runs(
                [path],
                [run],
                [label],
                f"Plant {plant}, dt = {dt} s",
                folder / image_name,
            )
            source = Path(os.path.relpath(path, output)).as_posix()
            sources.append(
                f"- Plant {plant}, dt = {dt}, {label}: "
                f"[plot](plant{plant}/{image_name}), [source]({source})"
            )

    fig, axes = plt.subplots(3, 1, figsize=(12, 11), layout="constrained")
    for ax, plant in zip(axes, "ABC"):
        values = summaries[plant]
        for index, label in enumerate(("C++", "LabVIEW")):
            ax.bar(
                [i + (index - 0.5) * 0.36 for i in range(5)],
                [row[index] for row in values],
                width=0.36,
                color=COLORS[index],
                label=label,
            )
        ax.set_yscale("log")
        ax.set_ylim(1_000, 100_000_000)
        ax.yaxis.set_major_formatter(FuncFormatter(lambda value, _: throughput(value)))
        ax.set_xticks(range(5), DT_VALUES)
        ax.set_xlabel("dt (s)")
        ax.set_ylabel("Median iterations/s")
        ax.set_title(f"Plant {plant}", loc="left", weight="bold")
        ax.grid(axis="y", alpha=0.2)
        ax.set_axisbelow(True)
        ax.legend(loc="upper right")
        for index, (cpp, labview, speedup) in enumerate(values):
            ax.text(
                index,
                max(cpp, labview) * 1.25,
                f"{speedup:.2f}×",
                ha="center",
                fontsize=10,
            )
    fig.suptitle(
        "C++ and LabVIEW benchmark throughput\n",
        fontsize=15,
    )
    fig.savefig(output / "summary.png", dpi=160)
    plt.close(fig)
    report.extend(sources)
    report.extend(
        [
            "",
            "## Regenerate",
            "",
            "```sh",
            "python3 tools/visualizer/compare_plants.py",
            "```",
            "",
        ]
    )
    (output / "README.md").write_text("\n".join(report), encoding="utf-8")
    print("\n".join(report[6:24]))
    print(f"Saved {len(pairs) * 3 + 1} PNGs and README.md to {output}")


if __name__ == "__main__":
    main()
