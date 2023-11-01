//
// Created by sjef on 5-6-19.
//

#ifndef VPGSOLVER_ZLNKVPG_H
#define VPGSOLVER_ZLNKVPG_H

#include "Game.h"

#include <boost/dynamic_bitset.hpp>

#include <array>
#include <vector>
/// Implementation of the recursive algorithm for VPGs.
///
/// A restriction \rho: V -> 2^C is represented by a vector of ConfSet for every vertex and a bitvector indicating
/// whether \rho(v) is defined for a vertex v. This bitvector is technically redundant but can be more efficiently
/// check for emptiness, i.e. \lambda v \in V. \emptyset.
class zlnkVPG
{
public:

  /// Initialize solver for full variability parity game
  zlnkVPG(const Game& game, bool metrics);

  /// Solve the parity game
  std::pair<std::vector<ConfSet>, std::vector<ConfSet>> solve() const;

protected:  
  /// \brief Implementation of SOLVE(rho)
  std::array<std::vector<ConfSet>, 2> solve_rec(std::vector<ConfSet>&& rho) const;

  /// \brief Implementation of SOLVE_OPTIMISED(rho)
  std::array<std::vector<ConfSet>, 2> solve_optimised_rec(std::vector<ConfSet>&& rho) const;

  /// Attract restriction to rho for player alpha towards A, adds these to A as well.
  void attr(int alpha, const std::vector<ConfSet>& rho, std::vector<ConfSet>& A) const;
  
  /// \returns max { p(v) | v in V && g(v) \neq \emptyset } 
  int get_highest_prio(const std::vector<ConfSet>& rho) const;
  
  /// Flag to enable metrics. If set to true the algorithm will output the number of configurations we were able to attract simultaneously.
  bool conf_metricoutput;

  /// Measure time spent in the attractor set calculation
  mutable long attracting = 0;

  /// \brief A shared dynamic bitset used to speed up internal computations.
  mutable boost::dynamic_bitset<> m_vertices;

  /// The variability parity game.
  const Game& game;
};

#endif // VPGSOLVER_ZLNKVPG_H
