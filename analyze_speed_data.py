#!/usr/bin/python3 -B
"""Organize and present speed data from experiment output files.
"""
import collections
import pprint

CASES = [
    "det_lu",
    "det_minor",
    "mat_mul",
    "ode",
    "poly",
]

LIBS = [
    "drake",
    "cppad",
]


def make_lib_case_trials(lib, case):
    FILE_PATT = f"{lib}.30x.out"
    SIZE_PATT = f"{case}_size"
    RATE_PATT = f"{lib}_{case}_rate"
    trial = 0
    trials = collections.defaultdict(dict)
    scratch = {}
    for line in open(FILE_PATT):
        if SIZE_PATT in line:
            if "size" in trials[trial]:
                trial += 1
            exec(line, globals(), scratch)
            trials[trial]["size"] = scratch[SIZE_PATT]
        elif RATE_PATT in line:
            exec(line, globals(), scratch)
            trials[trial]["rate"] = scratch[RATE_PATT]
    return trials


def main():
    # Ingest the data from output files.
    libs = collections.defaultdict(dict)
    for lib in LIBS:
        cases = collections.defaultdict(dict)
        libs[lib] = cases
        for case in CASES:
            trials = make_lib_case_trials(lib, case)
            cases[case] = trials
    pprint.pprint(libs)

    # Enforce invariants.
    trials0 = len(libs[LIBS[0]][CASES[0]])
    size0 = libs[LIBS[0]][CASES[0]]["size"]
    for lib in LIBS:
        assert lib in libs
        for case in CASES:
            assert case in libs[lib]
            trials = len(libs[lib][case])
            assert trials == trials0
            size = libs[lib][case]["size"]
            assert size == size0

    # TODO: min, max, mean, [stddev?]
    # TODO: comparison [percentages+/-, graphs?]

if __name__ == "__main__":
    main()
