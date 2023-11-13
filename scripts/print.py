import argparse
import logging
import json

formatter = logging.Formatter("%(message)s")
logging.basicConfig(level=logging.DEBUG)

def average(timings):
    total = 0.0

    for result in timings:
        total += result['solving']

    return total / 5 * 1000.0


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

        for experiment, properties in timings.items():
            family_time = 0.0
            family_optimised_time = 0.0
            product_time = 0.0
            reachable_time = 0.0

            for prop, values in properties.items():
                for game, timings in values.items():
                    if "project" in game and "reachable" not in game:
                        product_time += average(timings['default'])
                    elif "reachable" in game:
                        reachable_time += average(timings['default'])
                    else:
                        print(game)
                        family_time = average(timings['default'])
                        family_optimised_time = average(timings['optimised'])

                print(f"Experiment {experiment} and property {prop} timing family {family_time:.2f} optimised {family_optimised_time:.2f} and product time {product_time:.2f}, reachable {reachable_time:.2f}")

if __name__ == "__main__":
    main()
