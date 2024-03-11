#include "zlnkPG.h"
#include "zlnkVPG.h"
#include "Game.h"

#include <chrono>
#include <fstream>
#include <iostream>
#include <optional>

int run(int argc, char** argv)
{
  if (argc < 2) {
    std::cerr << "Incorrect number of params";
    return 2;
  }

  char const* specificconf = "";

  bool debug = false;

  /// 
  bool print_solution = false;
  bool metricoutput = false;

  // Parse the input as a regular parity game and use the respective solver.
  bool is_parity_game = false;

  // Projection mode, for every configuration generates the parity game with the enabled edges.
  std::optional<std::string> output_projection;

  // Compute the reachable part of the parity game.
  std::optional<std::string> output_reachable;

  // Solve family, optimised, right or left.
  int algorithm = 0;

  for (int i = 2; i < argc; i++) {
    std::string argument(argv[i]);
    if (argument.compare("--project") == 0) {
      output_projection = argv[i+1];
      i++;
    } else if (argument.compare("--reachable") == 0) {
      output_reachable = argv[i+1];
      i++;
    } else if (argument.compare("--parity-game") == 0) {
      is_parity_game = true;
    } else if (argument.compare("--algorithm") == 0) {
      algorithm = std::stoi(argv[i+1]);
      i++;
    } else if (argument.compare("--print-solution") == 0) {
      print_solution = true;
    } else if (argument.compare("--debug") == 0) {
      debug = true;
    } else {
      std::cerr << "Unknown parameter: " << argv[i];
      return -1;
    }
  }

  // Read the input game
  Game g = GameParser().parse(argv[1], specificconf, is_parity_game);

  if (output_projection) {
    // Determine all configurations
    std::vector<std::pair<ConfSet, std::string>> allconfs = g.configurations_explicit();

    // Write the projected VPG
    for (const auto& conf : allconfs) {
      std::string output_name = output_projection.value();
      output_name += conf.second + ".pg";

      // Write the projection.
      std::ofstream output(output_name);
      g.write(output, conf.first);
      std::cout << "Projected to " << output_name << std::endl;
    }

    return 0;
  }

  if (output_reachable) {
    if (!is_parity_game) {
      std::cerr << "reachability can only be used with parity games, not vpgs" << std::endl;
    }

    auto [reachable, mapping] = g.compute_reachable();
    std::ofstream output(output_reachable.value());
    reachable.write(output);

    // Check whether reachable is correct.
    if(true) {
      zlnkPG z(g, debug);
      const auto [W0, W1] = z.solve();

      zlnkPG z2(reachable, debug);
      const auto [W0_reachable, W1_reachable] = z2.solve();

      for (std::size_t v = 0; v < g.number_of_vertices(); ++v) {
        if (W0[v]) {
          assert(mapping[v] == -1 || W0_reachable[mapping[v]]);
        } else {
          assert(W1[v]);
          assert(mapping[v] == -1 || W1_reachable[mapping[v]]);
        }
      }
    }

    return 0;
  }
  
  // enable cache after parsing
  bdd_gbc();
  bdd_setcacheratio(200);
  bdd_gbc();

  auto start = std::chrono::high_resolution_clock::now();

  if (is_parity_game) {

    zlnkPG z(g, debug);
    const auto [W0, W1] = z.solve();

    auto end = std::chrono::high_resolution_clock::now();
    std::cout << "Solving time: " << std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count() / 1000000.0 << " ms\n";

    std::cout << "W0: ";
    print_set(W0, print_solution);
    std::cout << "\n";
    std::cout << "W1: ";
    print_set(W1, print_solution);
    std::cout << "\n";
  } else {
    assert(g.configurations() != emptyset);

    zlnkVPG z(g, debug);

    Restriction W0(g);
    Restriction W1(g);
    if (algorithm == 0) {
      std::tie(W0, W1) = z.solve();
    } else if (algorithm == 1) {
      std::tie(W0, W1) = z.solve_optimised();
    } else if (algorithm == 2) {
      std::tie(W0, W1) = z.solve_optimised_left();
    } else {
      std::abort();
    }

    auto end = std::chrono::high_resolution_clock::now();
    std::cout << "Solving time: " << std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count() / 1000000.0 << " ms\n";

    std::cout << "W0: \n";
    print_set(W0, g.configurations_explicit(), print_solution);
    std::cout << "W1: \n";
    print_set(W1, g.configurations_explicit(), print_solution);
  }

  return 0;
}

int main(int argc, char** argv)
{
  try {
    run(argc, argv);
  }
  catch(const std::exception& ex) {
    std::cerr << ex.what();
    return 1;
  }

  return 0;
}