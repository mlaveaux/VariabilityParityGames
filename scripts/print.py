import logging
import json

formatter = logging.Formatter("%(message)s")
logging.basicConfig(level=logging.DEBUG)

def main():

    # Print a nice table
    with open("results.json", "r", encoding="utf-8") as json_file:
        timings = json.load(json_file)

        for experiment, prop in timings:
            family_time = 0.0
            product_time = 0.0

            for game, timings in prop:
                if "single" in game:
                    product_time = 



if __name__ == "__main__":
    main()
