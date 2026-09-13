import argparse
import csv
from pathlib import Path

import matplotlib.pyplot as plt


def add_linked_markers(fig, axes, times, values, fields):
    markers = []
    for ax, name in zip(axes, fields):
        vertical = ax.axvline(times[0], color="0.4", linestyle="--", visible=False)
        horizontal = ax.axhline(
            values[name][0], color="0.4", linestyle="--", visible=False
        )
        label = ax.text(
            0.98, 0.95, "", transform=ax.transAxes, ha="right", va="top",
            bbox=dict(facecolor="white", edgecolor="none", alpha=0.85),
            visible=False,
        )
        markers.append((vertical, horizontal, label))

    def on_move(event):
        if event.inaxes not in axes or event.xdata is None:
            hide_markers(event)
            return

        # Snap every plot to the same recorded time sample.
        index = min(range(len(times)), key=lambda i: abs(times[i] - event.xdata))
        time = times[index]
        for name, (vertical, horizontal, label) in zip(fields, markers):
            value = values[name][index]
            vertical.set_xdata([time, time])
            horizontal.set_ydata([value, value])
            label.set_text(f"t = {time:g}   {name} = {value:g}")
            for artist in (vertical, horizontal, label):
                artist.set_visible(True)
        fig.canvas.draw_idle()

    def hide_markers(event):
        if any(vertical.get_visible() for vertical, _, _ in markers):
            for group in markers:
                for artist in group:
                    artist.set_visible(False)
            fig.canvas.draw_idle()

    fig.canvas.mpl_connect("motion_notify_event", on_move)
    fig.canvas.mpl_connect("figure_leave_event", hide_markers)


def main():
    parser = argparse.ArgumentParser(description="Plot CSV columns against time (t).")
    parser.add_argument("csv_file", type=Path, help="CSV file with a numeric t column")
    args = parser.parse_args()

    try:
        with args.csv_file.open(newline="", encoding="utf-8-sig") as file:
            reader = csv.DictReader(file)
            if not reader.fieldnames or "t" not in reader.fieldnames:
                parser.error("CSV must contain a 't' column")

            fields = [name for name in reader.fieldnames if name != "t"]
            if not fields:
                parser.error("CSV must contain at least one column besides 't'")

            times = []
            values = {name: [] for name in fields}
            for row in reader:
                times.append(float(row["t"]))
                for name in fields:
                    values[name].append(float(row[name]))

    except (OSError, UnicodeError, csv.Error, ValueError, TypeError) as error:
        parser.error(f"Could not read numeric CSV data: {error}")

    if not times:
        parser.error("CSV contains no data rows")

    fig, axes = plt.subplots(
        len(fields), 1, sharex=True, squeeze=False,
        figsize=(9, 2.5 * len(fields)),
    )
    for index, name in enumerate(fields):
        ax = axes[index, 0]
        ax.plot(times, values[name], color=f"C{index % 10}")
        ax.set_ylabel(name)
        ax.grid(True, alpha=0.3)

    axes[-1, 0].set_xlabel("Time (t)")
    fig.suptitle(args.csv_file.name)
    fig.tight_layout()
    add_linked_markers(fig, list(axes[:, 0]), times, values, fields)
    plt.show()


if __name__ == "__main__":
    main()
