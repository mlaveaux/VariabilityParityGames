#include <chrono>
#include <fstream>
#include <iostream>

#include "Algorithms/zlnkPG.h"
#include "Algorithms/zlnkVPG.h"
#include "Game.h"

using namespace std;

int main(int argc, char** argv)
{
  if (argc < 2) {
    cerr << "Incorect params";
    return 2;
  }

  Game g;

  char* specificconf;
  bool specificconfenabled = false;

  bool fulloutput = false;
  bool metricoutput = false;
  bool priocompress = false;
  bool SolveFPIte = false;
  bool assistedW0 = false;
  char* assistanceW0;
  bool assistrandom = false;
  int assistn;
  bool solvelocal = false;

  // Parse the input as a regular parity game and use the respective solver.
  bool regulargame = false;

  bool compressvertices = false;
  char* metricdir;
  bool projectmode = false;
  char* projectname;

  for (int i = 2; i < argc; i++) {
    std::string argument(argv[i]);
    if (argument.compare("--project") == 0) {
      projectmode = true;
      projectname = argv[i+1];
      i++;
    } else if (argument.compare("--parity-game") == 0) {
      regulargame = true;
    } else if (argument.compare("--print-solution") == 0) {
      fulloutput = true;
    }
    else {
      switch (*argv[i]) {
      case 'v':
        compressvertices = true;
        break;
      case 'o':
        g.specificvarlast = true;
        g.specificvar = atoi(argv[i] + 1);
        break;
      case 'l':
        solvelocal = true;
        break;
      case 'P':
        assistedW0 = true;
        assistanceW0 = argv[i] + 1;
        break;
      case 'a':
        assistrandom = true;
        assistn = atoi(argv[i] + 1);
        break;
      case 'c':
        specificconfenabled = true;
        specificconf = argv[i] + 1;
        break;
      case 'm':
        metricoutput = true;
        metricdir = argv[i] + 1;
        break;
      case 'p':
        priocompress = true;
        break;
      case 'F':
        SolveFPIte = true;
        priocompress = true;
        break;
      default:
        cerr << "Unknown parameter: " << argv[i];
        break;
      }
    }
  }

  if (regulargame) {
    g.parsePGFromFile(argv[1]);
  }

  else {
    if (specificconfenabled) {
      g.parseVPGFromFile(argv[1], specificconf);
    }
    else {
      g.parseVPGFromFile(argv[1]);
    }
  }

  if (projectmode) {
    // Determine all configurations
    vector<tuple<ConfSet, string>> allconfs;
    g.findAllElements(g.bigC, &allconfs);

    // Write the projected VPG
    for (const auto& conf : allconfs) {
      string output_name = string(projectname);
      output_name += get<1>(conf) + ".pg";

      // Write the projection.
      ofstream output;
      output.open(output_name);
      g.writePG(&output, get<0>(conf));
      std::cout << "Projected to " << output_name << endl;
    }
    return 0;
  }
  
  time_t t = time(0);

  // enable cache after parsing
  bdd_gbc();
  bdd_setcacheratio(200);
  bdd_gbc();

  auto start = std::chrono::high_resolution_clock::now();

  if (compressvertices) {
    g.compressVertices();
  }

  if (priocompress) {
    g.compressPriorities();
  }

  auto* W0BigV = new VertexSetZlnk(g.n_nodes);
  auto* W1BigV = new VertexSetZlnk(g.n_nodes);
  vector<ConfSet>* W0vc = nullptr;
  vector<ConfSet>* W1vc = nullptr;    

  if (regulargame) {
    zlnkPG z(&g, metricoutput);

    if (solvelocal) {
      z.solvelocal = 2;
    }

    z.solve(W0BigV, W1BigV);
  } else {
    zlnkVPG z(&g, metricoutput);

    W0vc = new vector<ConfSet>(g.n_nodes);
    W1vc = new vector<ConfSet>(g.n_nodes);
    for (int i = 0; i < g.n_nodes; i++) {
      (*W0vc)[i] = emptyset;
      (*W1vc)[i] = emptyset;
    }

    if (solvelocal) {
      z.solvelocal = 2;
    }

    z.solve(W0BigV, W0vc, W1BigV, W1vc);
  }

  auto end = std::chrono::high_resolution_clock::now();

  auto elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
  std::cout << "Solving time: " << elapsed.count() << " ns";

  std::cout << "W0: \n";
  g.printCV(W0BigV, W0vc, fulloutput);
  std::cout << "W1: \n";
  g.printCV(W1BigV, W1vc, fulloutput);

  return 0;
}