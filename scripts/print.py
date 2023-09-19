import logging
import json

formatter = logging.Formatter("%(message)s")
logging.basicConfig(level=logging.DEBUG)

def main():
    # Verify the solutions
    with open("solutions.json", "r", encoding="utf-8") as json_file:
        solutions = json.load(json_file)

        for experiment, properties in solutions.items():
            # Find the family solution
            family_solutions = {}
            for prop, games in properties.items():
                for game, solution in games.items():
                    if "project" not in game:
                        logging.info("Checking property %s and family game %s", prop, game)
                        family_solutions = solution
                        break

                for product, result in family_solutions.items():
                    checked = False
                    for product_prop, solution in games.items():

                        logging.info("Validating product %s for game %s", product, product_prop)
                        if product in product_prop:
                            # The product result must match the family result.
                            logging.info("Checking product %s", product)
                            assert result == product_prop[0]
                            checked = True

                    assert checked

    # Print a nice table
    with open("results.json", "r", encoding="utf-8") as json_file:
        timings = json.load(json_file)


if __name__ == "__main__":
    main()
