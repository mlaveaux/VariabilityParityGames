import argparse
import logging
import json

formatter = logging.Formatter("%(message)s")
logging.basicConfig(level=logging.DEBUG)

def average(timings):
    total = 0.0

    for result in timings:
        total += result['solving']

    return total / len(timings) * 1000.0


def main():
    parser = argparse.ArgumentParser(
        prog="print.py",
        description="Print JSON output of run.py as a LaTeX table",
        epilog="",
    )

    parser.add_argument(
        "input", action="store", type=str
    )

    args = parser.parse_args()

    # Print a nice table
    with open(args.input, "r", encoding="utf-8") as json_file:
        timings = json.load(json_file)

        print("\\begin{table}[h]")
        print("\\begin{tabular}{r|r|r|r|r}")
        print("model & property & family & family-optimised & product \\\\ \\hline")

        old_experiment = None
        for experiment, properties in timings.items():

            for prop, values in properties.items():
                family_time = 0.0
                family_recursive_calls = 0
                family_optimised_time = 0.0
                family_optimised_recursive_calls = 0
                product_time = 0.0
                product_recursive_calls = 0
                reachable_time = 0.0

                for game, timings in values.items():
                    if "project" in game and "reachable" not in game:
                        product_time += average(timings['default'])
                    elif "reachable" in game:
                        reachable_time += average(timings['default'])
                        product_recursive_calls = max(product_recursive_calls, timings['default'][0].get('recursive_calls', 0))
                    else:
                        family_time = average(timings['default'])
                        family_recursive_calls = timings['default'][0].get('recursive_calls', 0)

                        if "optimised" in timings:
                            family_optimised_time = average(timings['optimised'])
                            family_optimised_recursive_calls = timings['optimised'][0].get('recursive_calls', 0)

                print(f"{experiment if experiment != old_experiment else ''} & {prop} \
                    & {family_time:.1f} ({family_recursive_calls}) & {family_optimised_time:.1f} ({family_optimised_recursive_calls}) & {reachable_time:.1f} ({product_recursive_calls}) \\\\")
                old_experiment = experiment

        print("\\end{tabular}")
        print("\\end{table}")

if __name__ == "__main__":
    main()
