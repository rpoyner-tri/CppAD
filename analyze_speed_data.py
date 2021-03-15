#!/usr/bin/python3 -B
"""Organize and present speed data from experiment output files.
"""
import collections
import pprint
import shelve
import statistics

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
            trials[trial]["duration"] = [1 / x for x in scratch[RATE_PATT]]
    return trials


def main():
    # Ingest the data from output files.
    libs = collections.defaultdict(dict)
    for lib in LIBS:
        libs[lib] = collections.defaultdict(dict)
        for case in CASES:
            libs[lib][case] = make_lib_case_trials(lib, case)

    # Enforce invariants.
    trials0 = len(libs[LIBS[0]][CASES[0]])
    sizes0 = sum([libs[LIBS[0]][case][0]["size"] for case in CASES], [])
    for lib in LIBS:
        assert lib in libs
        sizes = sum([libs[lib][case][0]["size"] for case in CASES], [])
        assert sizes == sizes0
        for case in CASES:
            assert case in libs[lib]
            trials = len(libs[lib][case])
            assert trials == trials0, (
                lib, case, trials, trials0,
                [key for key in libs[lib][case].keys()])
            size0 = libs[lib][case][0]["size"]
            for k in range(trials0):
                size = libs[lib][case][k]["size"]
                assert size == size0, (
                    lib, case, size, size0)

    # TODO: min, max, mean, [stddev?]
    for lib in LIBS:
        for case in CASES:
            case_data = libs[lib][case]
            case_data["max"] = []
            case_data["min"] = []
            case_data["mean"] = []
            case_data["stdev"] = []
            for k in range(len(size0)):
                case_data["max"].append(
                    max([case_data[x]["duration"][k] for x in range(trials0)]))
                case_data["min"].append(
                    min([case_data[x]["duration"][k] for x in range(trials0)]))
                case_data["mean"].append(
                    sum([case_data[x]["duration"][k] for x in range(trials0)])
                    / trials0)
                case_data["stdev"].append(
                    statistics.stdev(
                        [case_data[x]["duration"][k] for x in range(trials0)]))
    #pprint.pprint(libs)

    with shelve.open("speed_data") as db:
        db["speed_data"] = libs

    # TODO: comparison [percentages+/-, graphs?]


if __name__ == "__main__":
    main()
