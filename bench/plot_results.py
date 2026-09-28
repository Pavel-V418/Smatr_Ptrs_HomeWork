#!/usr/bin/env python3
"""Строит графики time/op и памяти по bench_results.csv (см. ТЗ:
результаты нужно представить и в табличном, и в графическом виде).

Запуск:
    python3 plot_results.py path/to/bench_results.csv [outdir]

Требует matplotlib и pandas (pip install matplotlib pandas).
"""
import sys
from pathlib import Path

import pandas as pd
import matplotlib.pyplot as plt


def main():
    if len(sys.argv) < 2:
        print("Использование: plot_results.py bench_results.csv [outdir]")
        sys.exit(1)

    csv_path = Path(sys.argv[1])
    outdir = Path(sys.argv[2]) if len(sys.argv) > 2 else csv_path.parent
    outdir.mkdir(parents=True, exist_ok=True)

    df = pd.read_csv(csv_path)

    for group, sub in df.groupby("group"):
        fig, (ax_time, ax_mem) = plt.subplots(1, 2, figsize=(11, 4.5))

        for variant, vsub in sub.groupby("variant"):
            vsub = vsub.sort_values("n")
            ax_time.plot(vsub["n"], vsub["ns_per_op"], marker="o", label=variant)
            ax_mem.plot(vsub["n"], vsub["bytes_total"], marker="o", label=variant)

        for ax, title, ylabel, col in (
                (ax_time, f"{group}: время на операцию", "нс/операцию", "ns_per_op"),
                (ax_mem, f"{group}: суммарно выделено памяти", "байт", "bytes_total"),
        ):
            ax.set_xscale("log")
            # Лог-шкала по Y падает с предупреждением, если все значения нулевые
            # (например, у сценария без единой аллокации на обеих сторонах) —
            # в этом случае просто рисуем линейно.
            if (sub[col] > 0).any():
                ax.set_yscale("log")
            ax.set_xlabel("N")
            ax.set_ylabel(ylabel)
            ax.set_title(title)
            ax.legend()
            ax.grid(True, which="both", alpha=0.3)

        fig.tight_layout()
        out_file = outdir / f"{group}.png"
        fig.savefig(out_file, dpi=150)
        print(f"saved {out_file}")


if __name__ == "__main__":
    main()