import logging
import json

formatter = logging.Formatter("%(message)s")
logging.basicConfig(level=logging.DEBUG)

def average(timings):
    total = 0.0

    for result in timings:
        total += result['VPGSolver_bdd']['solving']

    return total


def main():

    # Print a nice table
    with open("results.json", "r", encoding="utf-8") as json_file:
        timings = json.load(json_file)

        for experiment, properties in timings.items():
            family_time = 0.0
            product_time = 0.0

            for prop, values in properties.items():
                for game, timings in values.items():
                    if "single" in game:
                        product_time += average(timings)
                    else:
                        family_time = average(timings)

                print(f"Experiment {experiment} and property {prop} timing family {family_time:.2f} and product time {product_time:.2f}")

if __name__ == "__main__":
    main()
