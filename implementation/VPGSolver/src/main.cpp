#include <chrono>
#include <fstream>
#include <iostream>

#include "zlnkPG.h"
#include "zlnkVPG.h"
#include "Game.h"

void print_set(const Restriction& W, const std::vector<std::pair<ConfSet, std::string>>& configurations, bool full_solution)
{
  for (const auto& product : configurations) {    
    std::cout << "For product " << product.second << " the following vertices are in: ";
    for (std::size_t v = 0; v < W.number_of_vertices(); v++) {
      ConfSet tmp = W[v];
      tmp &= product.first;

      if (tmp != emptyset) {
        std::cout << v << ',';
      }

      if (!full_solution) {
        // Only show first vertex otherwise.
        break;
      }
    }
    std::cout << "\n";
  }
}

void print_set(const boost::dynamic_bitset<>& V, bool full_solution)
{
  for (int v = 0; v < V.size(); v++) {
    if (V[v]) {
      std::cout << v << ',';
    }

    if (!full_solution) {
      // Only show first vertex otherwise.
      break;
    }
  }
}

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

  // Projection mode, for every configuration generates the flatted parity game.
  bool projectmode = false;
  char* projectname;

  for (int i = 2; i < argc; i++) {
    std::string argument(argv[i]);
    if (argument.compare("--project") == 0) {
      projectmode = true;
      projectname = argv[i+1];
      i++;
    } else if (argument.compare("--parity-game") == 0) {
      is_parity_game = true;
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
  Game g(argv[1], specificconf, is_parity_game);

  if (projectmode) {
    // Determine all configurations
    std::vector<std::pair<ConfSet, std::string>> allconfs = g.configurations_explicit();

    // Write the projected VPG
    for (const auto& conf : allconfs) {
      std::string output_name = std::string(projectname);
      output_name += conf.second + ".pg";

      // Write the projection.
      std::ofstream output(output_name);
      g.write(output, conf.first);
      std::cout << "Projected to " << output_name << std::endl;
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
    std::cout << "Solving time: " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << " ms\n";

    std::cout << "W0: ";
    print_set(W0, print_solution);
    std::cout << "\n";
    std::cout << "W1: ";
    print_set(W1, print_solution);
    std::cout << "\n";
  } else {
    assert(g.configurations() != emptyset);

    zlnkVPG z(g, debug);
    const auto [W0, W1] = z.solve();

    auto end = std::chrono::high_resolution_clock::now();
    std::cout << "Solving time: " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << " ms\n";

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