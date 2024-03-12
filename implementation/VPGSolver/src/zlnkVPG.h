//
// Created by sjef on 5-6-19.
//

#ifndef VPGSOLVER_ZLNKVPG_H
#define VPGSOLVER_ZLNKVPG_H

#include "Game.h"
#include "Submap.h"

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
  std::pair<Submap, Submap> solve() const;

  /// Solve the parity game with the optimised recursion by Tim, not in the article.
  std::pair<Submap, Submap> solve_optimised() const;

  /// Solve the parity game with the optimised recursion in Algorithm 2
  std::pair<Submap, Submap> solve_optimised_left() const;

protected:  
  /// \brief Implementation of SOLVE(rho)
  std::array<Submap, 2> solve_rec(Submap&& rho) const;

  /// \brief Implementation of SOLVE_OPTIMISED(rho) in Algorithm 2
  std::array<Submap, 2> solve_optimised_rec(Submap&& rho) const;

  /// \brief Implementation of SOLVE_OPTIMISED(rho) in Algorithm 4
  std::array<Submap, 2> solve_optimised_left_rec(Submap&& rho) const;

  /// Attract restriction to rho for player alpha towards A, adds these to A as well.
  void attr(int alpha, const Submap& rho, Submap& A) const;
  
  /// \returns max { p(v) | v in V && g(v) \neq \emptyset } 
  std::pair<std::size_t, std::size_t> get_highest_lowest_prio(const Submap& rho) const;
  
  /// Enable more extensive logging.
  bool m_debug = false;

  /// Enable certain amount of logging.
  bool m_verbose = false;

  /// \brief Counts the number of recursive calls performed.
  mutable std::size_t m_recursive_calls = 0;

  /// Measure time spent in the attractor set calculation
  mutable long attracting = 0;

  /// \brief A shared dynamic bitset used to speed up internal computations.
  mutable boost::dynamic_bitset<> m_vertices;

  mutable std::size_t m_depth = 0;

  /// The variability parity game.
  const Game& game;
};

#endif // VPGSOLVER_ZLNKVPG_H
