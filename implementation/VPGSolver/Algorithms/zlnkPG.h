//
// Created by sjef on 26-10-19.
//

#ifndef VPGSOLVER_ZLNKPG_H
#define VPGSOLVER_ZLNKPG_H

#include "../Game.h"

/// Implementation of Zielonka's recursive algorithm.
class zlnkPG
{
public:

  /// Initialize a solver for the complete game.
  zlnkPG(const Game& game, bool metrics);

  //// Solve the parity game.
  std::pair<boost::dynamic_bitset<>, boost::dynamic_bitset<>> solve() const;

private:

  /// Attract vertices towards A for player alpha, puts them in A. Restricted game to vertices in V.
  void attr(int alpha, const boost::dynamic_bitset<>& V, boost::dynamic_bitset<>& A) const;

  /// Recurve solving call.
  std::array<boost::dynamic_bitset<>,2> solve_rec(boost::dynamic_bitset<>&& V) const;

  /// \returns max{p(v) | v \in V}
  int get_highest_prio(const boost::dynamic_bitset<>& V) const;
  
  /// The parity game
  const Game& game;
    
  /// Flag to enable metrics.
  bool conf_metricoutput;

  /// Measure time spent in the attractor set calculation
  mutable long attracting = 0;

  mutable boost::dynamic_bitset<> m_vertices;
};

#endif // VPGSOLVER_ZLNKPG_H
