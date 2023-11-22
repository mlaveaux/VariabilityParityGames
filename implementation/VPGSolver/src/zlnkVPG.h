//
// Created by sjef on 5-6-19.
//

#ifndef VPGSOLVER_ZLNKVPG_H
#define VPGSOLVER_ZLNKVPG_H

#include "Game.h"
#include "Restriction.h"

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

  /// Initialize solver for variability parity games
  zlnkVPG(const Game& game, bool debug);

  /// Solve the parity game
  std::pair<Restriction, Restriction> solve() const;

  /// Solve the parity game with the optimised recursion
  std::pair<Restriction, Restriction> solve_optimised() const;

protected:  
  /// \brief Implementation of SOLVE(rho)
  std::array<Restriction, 2> solve_rec(Restriction&& rho) const;

  /// \brief Implementation of SOLVE_OPTIMISED(rho)
  std::array<Restriction, 2> solve_optimised_rec(Restriction&& rho) const;

  /// Attract restriction to rho for player alpha towards A, adds these to A as well.
  void attr(int alpha, const Restriction& rho, Restriction& A) const;
  
  /// \returns max { p(v) | v in V && g(v) \neq \emptyset } 
  std::pair<std::size_t, std::size_t> get_highest_lowest_prio(const Restriction& rho) const;
  
  /// Enable more extensive logging.
  bool m_debug = false;

  /// \brief Counts the number of recursive calls performed.
  mutable std::size_t m_recursive_calls = 0;

  /// Measure time spent in the attractor set calculation
  mutable long attracting = 0;

  /// \brief A shared dynamic bitset used to speed up internal computations.
  mutable boost::dynamic_bitset<> m_vertices;

  /// The variability parity game.
  const Game& game;
};

#endif // VPGSOLVER_ZLNKVPG_H
