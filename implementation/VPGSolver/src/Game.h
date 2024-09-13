
#ifndef VPGSOLVER_GAME_H
#define VPGSOLVER_GAME_H

#include <iostream>
#include <optional>
#include <functional>
#include <map>
#include <tuple>
#include <unordered_set>
#include <vector>


#if ENABLE_BUDDY
#include "bdd.h"
#define BDD_MANAGER bool
#define BDD bdd
#define BDD_IS_EMPTY(manager, set) (set == bddfalse)
#define BDD_MINUS(manager, a, b) (a - b)
#define BDD_EMPTYSET(manager) bddfalse
#define BDD_UNIVERSE(manager) bddtrue
#else
#include "oxidd/zbdd.hpp"
#define BDD_MANAGER oxidd::zbdd_manager
#define BDD oxidd::zbdd_function
#define BDD_IS_EMPTY(manager, set) (set == manager.f())
#define BDD_MINUS(manager, a, b) (b).imp_strict(a)
#define BDD_EMPTYSET(manager) manager.f()
#define BDD_UNIVERSE(manager) manager.t()
#endif


#define SMH_TARGET(a) std::get<0>(a)
#define edge_index(a) std::get<1>(a)

/// Represent VPG using a double edge relation. Every edge has an edge index.
/// Every edge index is mapped to a set of configurations guarding the edge.
class Game
{
public:

  Game(BDD_MANAGER& manager,
    std::vector<BDD> bm_vars, 
    BDD bigC,
    std::vector<int> priority, 
    std::vector<int> owner, 
    std::vector<std::vector<std::tuple<int, int>>> out_edges,
    std::vector<BDD> edge_guards);


  // Write the parity game to the output stream
  void write(std::ostream& output, std::optional<BDD> conf = std::optional<BDD>({}));

  /// \returns A game only containing the vertices reachable from the initial state (0).
  std::pair<Game, std::vector<int>> compute_reachable() const;

  /// \returns Returns the total number of vertices. A game can potentially have vertices for which no outgoing edge is defined.
  int number_of_vertices() const { return m_owner.size(); }

  /// \returns Returns the full set of configurations.
  BDD configurations() const { return bigC; }

  /// \returns Returns the full set of configurations as pair of bdd representation and explicit representation.
  std::vector<std::pair<BDD, std::string>> configurations_explicit() const;

  /// \returns Returns the full set of configurations as pair of bdd representation and explicit representation.
  std::vector<std::pair<BDD, std::string>> configurations_explicit(BDD set) const;

  /// \returns The owner of the vertex.
  int owner(int vertex) const { return m_owner[vertex]; }

  /// \returns The owner of the vertex.
  std::size_t priority(int vertex) const { return m_priority[vertex]; }

  /// \returns The vertices for the given priority
  const std::unordered_set<int>& priority_vertices(int prio) const { return priorityI[prio]; }

  /// \returns Returns the list of predecessors for a given vertex, given by a pair (vertex, edge_index).
  /// The edge index can be used to obtain the guard. 
  const std::vector<std::tuple<int, int>>& predecessors(int vertex) const { return in_edges[vertex]; }

  /// \returns Returns the list of successors for a given vertex, given by a pair (vertex, edge_index).
  /// The edge index can be used to obtain the guard. 
  const std::vector<std::tuple<int, int>>& successors(int vertex) const { return out_edges[vertex]; }

  /// \returns The configuration set for a given edge index. 
  const BDD& edge_guard(int edge_index) const { return edge_guards[edge_index]; }

private:
  BDD_MANAGER& m_manager;
  
  std::vector<BDD> bm_vars;

  BDD bigC;
  std::vector<int> m_priority;
  std::vector<int> m_owner;
  
  std::vector<std::vector<std::tuple<int, int>>> out_edges;
  std::vector<BDD> edge_guards;

  // This is derived from the input
  std::vector<std::vector<std::tuple<int, int>>> in_edges;  
  std::vector<std::unordered_set<int>> priorityI;
};

///
/// The variability parity game format is an extension of the pgsolve format
/// The first line starts with "confs <BDD>;". A <BDD> is essentially a boolean formula encoded as follows.
/// (<entry>+?)*;, where each <entry> is a string (0|1|-|F)* indicating the value of the expession
class GameParser
{
public:
  GameParser(BDD_MANAGER& manager) : m_manager(manager) {}

  /// Parses a variability parity game from the given file, optionally a concrete configuration can be given to be projected on.
  /// is_parity_game means the game is a standard parity game without configurations
  Game parse(const std::string& filename, const char* specificconf, bool is_parity_game);

private:
  /// \brief Parse the configurations from the given line. 
  void parseConfs(char const* line);
  void parseInitialiser(char* line);
  int parseConfSet(const char* line, int i, BDD& result);
  void parseVertex(char* line);
  int readUntil(const char* line, char delim);

  BDD_MANAGER& m_manager;
  
  // Intermediate store for the output.
  std::vector<BDD> bm_vars;

  BDD bigC;
  std::vector<int> m_priority;
  std::vector<int> m_owner;
  
  std::vector<std::vector<std::tuple<int, int>>> out_edges;
  std::vector<BDD> edge_guards;

  // This is all parsing related.
  bool is_parity_game = false;
  bool specificvarlast = false;
  int specificvar;
  int bm_n_vars;
  int n_nodes;

  std::vector<bool> declared;
  std::map<std::string, BDD> parseCache;
};

#endif // VPGSOLVER_GAME_H
