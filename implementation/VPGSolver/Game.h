
#ifndef VPGSOLVER_GAME_H
#define VPGSOLVER_GAME_H


#include <iostream>
#include <optional>
#include <map>
#include <tuple>
#include <unordered_set>
#include <vector>

#include "conf.h"

#define target(a) std::get<0>(a)
#define edge_index(a) std::get<1>(a)

/// Represent VPG using a double edge relation. Every edge has an edge index.
/// Every edge index is mapped to a set of configurations guarding the edge.
///
/// The variability parity game format is an extension of the pgsolve format
/// The first line starts with "confs <confset>;". A <confset> is essentially a boolean formula encoded as follows.
/// (<entry>+?)*;, where each <entry> is a string (0|1|-|F)* indicating the value of the expession
class Game
{
public:

  /// Parses a variability parity game from the given file, optionally a concrete configuration can be given to be projected on.
  /// is_parity_game means the game is a standard parity game without configurations
  Game(const std::string& filename, const char* specificconf, bool is_parity_game);

  // Write the parity game to the output stream
  void write(std::ostream& output, std::optional<ConfSet> conf);

  /// \returns Returns the total number of vertices. A game can potentially have vertices for which no outgoing edge is defined.
  int number_of_vertices() const { return n_nodes; }

  /// \returns Returns the full set of configurations.
  ConfSet configurations() const { return bigC; }

  /// \returns Returns the full set of configurations as pair of bdd representation and explicit representation.
  std::vector<std::pair<ConfSet, std::string>> configurations_explicit() const;

  /// \returns The owner of the vertex.
  int owner(int vertex) const { return m_owner[vertex]; }

  /// \returns The owner of the vertex.
  int priority(int vertex) const { return m_priority[vertex]; }

  /// \returns The vertices for the given priority
  const std::unordered_set<int>& priority_vertices(int prio) const { return priorityI[prio]; }

  /// \returns Returns the list of predecessors for a given vertex, given by a pair (vertex, edge_index).
  /// The edge index can be used to obtain the guard. 
  const std::vector<std::tuple<int, int>>& predecessors(int vertex) const { return in_edges[vertex]; }

  /// \returns Returns the list of successors for a given vertex, given by a pair (vertex, edge_index).
  /// The edge index can be used to obtain the guard. 
  const std::vector<std::tuple<int, int>>& successors(int vertex) const { return out_edges[vertex]; }

  /// \returns The configuration set for a given edge index. 
  const ConfSet& edge_guard(int edge_index) const { return edge_guards[edge_index]; }

private:
  /// \brief Parse the configurations from the given line. 
  void parseConfs(char const* line);
  void parseInitialiser(char* line);
  int parseConfSet(const char* line, int i, ConfSet& result);
  void parseVertex(char* line);
  int readUntil(const char* line, char delim);

  /// \brief Compute the predecessor relation from the outgoing edges.
  void buildInEdges();
  
  std::vector<ConfSet> bm_vars;
  int bm_n_vars;
  ConfSet bigC;
  int n_nodes;

  std::vector<int> m_priority;
  std::vector<int> m_owner;
  std::vector<bool> declared;
  
  std::vector<std::vector<std::tuple<int, int>>> out_edges;
  std::vector<std::vector<std::tuple<int, int>>> in_edges;
  std::vector<ConfSet> edge_guards;
  
  std::vector<std::unordered_set<int>> priorityI;

  bool is_parity_game = false;
  bool specificvarlast = false;
  int specificvar;

  std::map<std::string, ConfSet> parseCache;
};

#endif // VPGSOLVER_GAME_H
