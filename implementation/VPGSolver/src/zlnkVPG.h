//
// Created by sjef on 5-6-19.
//

#ifndef VPGSOLVER_ZLNKVPG_H
#define VPGSOLVER_ZLNKVPG_H

#include "Game.h"

#include <boost/dynamic_bitset.hpp>

#include <array>
#include <vector>

class RestrictionProxy {

public:
  RestrictionProxy(ConfSet& entry, std::size_t& nonempty_count)
    : m_nonempty_count(nonempty_count),
      m_entry(entry)
  {}

  RestrictionProxy& operator=(const ConfSet& other) {
    if (m_entry == emptyset && other != emptyset) {
      m_nonempty_count += 1;
    } else if (m_entry != emptyset && other == emptyset) {
      m_nonempty_count -= 1;
    }

    m_entry = other;
    return *this;
  }

  RestrictionProxy& operator|=(const ConfSet& other) {
    if (m_entry == emptyset && other != emptyset) {
      m_nonempty_count += 1;
    }

    m_entry |= other;
    return *this;
  }
  
  RestrictionProxy& operator-=(const ConfSet& other) {
    bool was_empty = (m_entry == emptyset);
    m_entry -= other;

    if (!was_empty && m_entry == emptyset) {
      m_nonempty_count -= 1;
    }
    return *this;
  }

  operator ConfSet() const {
    return m_entry;
  }

private:
  ConfSet& m_entry;
  std::size_t& m_nonempty_count;
};

/// \brief A mapping from vertices to configurations.
class Restriction {

public:
  Restriction(std::size_t number_of_vertices, ConfSet initial = emptyset)
    : m_mapping(number_of_vertices, initial)
  {
    m_nonempty_count = initial == emptyset ? 0 : number_of_vertices;
  }

  /// \returns The size of the restriction. 
  std::size_t size() const {
    return m_mapping.size();
  }
  
  /// \returns The size of the restriction. 
  std::size_t count() const {
    std::size_t count = 0;
    for (const auto& entry : m_mapping) {
      count += (entry != emptyset);
    }
    assert(m_nonempty_count == count);

    return m_nonempty_count;
  }

  /// \returns True iff the given confset is equal to lambda x in V. \emptyset
  bool is_empty() const {
    return m_nonempty_count == 0;
  }

  ConfSet operator[](std::size_t index) const {
    return m_mapping[index];
  }

  RestrictionProxy operator[](std::size_t index) {
    return RestrictionProxy(m_mapping[index], m_nonempty_count);
  }

  Restriction& operator|=(const Restriction& other) {
    assert(m_mapping.size() == other.m_mapping.size());

    for (std::size_t i = 0; i < m_mapping.size(); ++i) {
      (*this)[i] |= other[i];
    }
    
    return *this;
  }
  
  Restriction& operator-=(const Restriction& other) {    
    assert(m_mapping.size() == other.m_mapping.size());
    assert(size() >= 0);

    for (std::size_t i = 0; i < m_mapping.size(); ++i) {
      (*this)[i] -= other[i];
    }

    assert(size() >= 0);
    return *this;
  }

private:
  friend RestrictionProxy;

  std::vector<ConfSet> m_mapping;

  std::size_t m_nonempty_count = 0; // Invariant: counts the number of empty positions in the mapping.
};

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

  /// Measure time spent in the attractor set calculation
  mutable long attracting = 0;

  /// \brief A shared dynamic bitset used to speed up internal computations.
  mutable boost::dynamic_bitset<> m_vertices;

  /// The variability parity game.
  const Game& game;
};

#endif // VPGSOLVER_ZLNKVPG_H
