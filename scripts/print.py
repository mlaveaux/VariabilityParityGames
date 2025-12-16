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
        print("\\begin{tabular}{r|r|r|r|r|r}")
        print("model & property & family (max) & family-left-optimised (max) & product (max, total) \\\\ \\hline")

        old_experiment = None
        for experiment, properties in timings.items():

            for prop, values in properties.items():
                # Reachable family variant
                family_time = 0.0
                family_recursive_calls = 0
                family_left_optimised_time = 0.0
                family_left_optimised_recursive_calls = 0

                # Product variant
                product_max_recursive_calls = 0
                product_recursive_calls = 0
                reachable_time = 0.0

                for game, timings in values.items():
                    if "reachable.pg" in game:
                        reachable_time += average(timings['solver'])
                        product_max_recursive_calls = max(product_max_recursive_calls, timings['solver'][0].get('recursive_calls', 0))
                        product_recursive_calls = product_recursive_calls + timings['solver'][0].get('recursive_calls', 0)
                    else:
                        family_time = average(timings['algorithm0'])
                        family_recursive_calls = timings['algorithm0'][0].get('recursive_calls', 0)

                        if "algorithm2" in timings:
                            family_left_optimised_time = average(timings['algorithm2'])
                            family_left_optimised_recursive_calls = timings['algorithm2'][0].get('recursive_calls', 0)

                print(f"{experiment if experiment != old_experiment else ''} & {prop} \
                    & {family_time:.1f} ({family_recursive_calls}) & {family_left_optimised_time:.1f} ({family_left_optimised_recursive_calls}) & {reachable_time:.1f} ({product_max_recursive_calls}, {product_recursive_calls}) \\\\")
                old_experiment = experiment

        print("\\end{tabular}")
        print("\\end{table}")

if __name__ == "__main__":
    main()
