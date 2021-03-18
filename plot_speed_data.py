#!/usr/bin/python3 -B
"""Plot speed comparisons for further contemplation.
"""

import shelve

import matplotlib.pyplot as plt

CASES = [
    "det_lu",
    "det_minor",
    "mat_mul",
    "ode",
    "poly",
]

LIBS = [
    "cppad",
    "double",
    "drake",
]

MARKERS = {
    "cppad": "x",
    "double": "+",
    "drake": "D",
}


def plot_case(axx, libs, case):
    sizes = libs["cppad"][case][0]["size"]

    for lib in LIBS:
        durations = libs[lib][case]["mean"]
        axx.plot(sizes, durations, label=lib, marker=MARKERS[lib])
        # axx.annotate(lib, (sizes[-1], durations[-1]))
    axx.set_title(f"Speed test case: {case}")
    axx.set_xlabel("Problem size")
    axx.set_ylabel("Elapsed time (sec)")
    axx.legend()

def main():
    with shelve.open("speed_data") as db:
        libs = db["speed_data"]

    fig, axs = plt.subplots(3, 2)

    assert len(axs.flat) >= len(CASES)
    pairs = zip(CASES, axs.flat)

    for case, axx in pairs:
        plot_case(axx, libs, case)
    plt.show()

if __name__ == "__main__":
    main()
