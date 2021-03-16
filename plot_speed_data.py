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
    "double",
    "drake",
    "cppad",
]


def plot_case(libs, case):
    sizes = libs["cppad"][case][0]["size"]

    for lib in LIBS:
        plt.plot(sizes, libs[lib][case]["mean"])
    plt.title(f"Speed test case: {case}")
    plt.xlabel("Problem size")
    plt.ylabel("Elapsed time (sec)")
    plt.show()

def main():
    with shelve.open("speed_data") as db:
        libs = db["speed_data"]

    for case in CASES:
        plot_case(libs, case)
        plt.figure()
if __name__ == "__main__":
    main()
