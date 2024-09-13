
#pragma once

#include "Game.h"

#include <boost/dynamic_bitset.hpp>

#include <array>
#include <vector>

inline
void print_set(const Game& game, BDD set)
{
  std::cout << "{ ";
  for (const auto& product : game.configurations_explicit(set)) {
    std::cout << product.second << ", ";    
  }

  std::cout << "}" << std::endl;  
}

class SubmapProxy {

public:
  SubmapProxy(BDD_MANAGER& manager, BDD& entry, std::size_t& nonempty_count)
    : m_nonempty_count(nonempty_count),
      m_manager(manager),
      m_entry(entry)
  {}

  SubmapProxy& operator=(const BDD& other) {
    if (BDD_IS_EMPTY(m_manager, m_entry) && !(BDD_IS_EMPTY(m_manager, other))) {
      m_nonempty_count += 1;
    } else if (!BDD_IS_EMPTY(m_manager, m_entry) && BDD_IS_EMPTY(m_manager, other)) {
      m_nonempty_count -= 1;
    }

    m_entry = other;
    return *this;
  }

  SubmapProxy& operator|=(const BDD& other) {
    if (BDD_IS_EMPTY(m_manager, m_entry) && !BDD_IS_EMPTY(m_manager, other)) {
      m_nonempty_count += 1;
    }

    m_entry |= other;
    return *this;
  }
  
  SubmapProxy& operator-=(const BDD& other) {
    bool was_empty = (BDD_IS_EMPTY(m_manager, m_entry));
    m_entry = BDD_MINUS(manager, m_entry, other);

    if (!was_empty && BDD_IS_EMPTY(m_manager, m_entry)) {
      m_nonempty_count -= 1;
    }
    return *this;
  }
  
  SubmapProxy& operator&=(const BDD& other) {
    bool was_empty = (BDD_IS_EMPTY(m_manager, m_entry));
    m_entry &= other;

    if (!was_empty && BDD_IS_EMPTY(m_manager, m_entry)) {
      m_nonempty_count -= 1;
    }
    return *this;
  }

  operator BDD() const {
    return m_entry;
  }

private:
  BDD& m_entry;
  std::size_t& m_nonempty_count;
  BDD_MANAGER& m_manager;
};

/// \brief A mapping from vertices to configurations.
class Submap {

public:
  Submap(const Game& game, BDD_MANAGER& manager, BDD initial)
    : m_game(&game), m_mapping(game.number_of_vertices(), initial),
      m_manager(manager)
  {
    m_nonempty_count = BDD_IS_EMPTY(m_manager, initial) ? 0 : game.number_of_vertices();
  }

  Submap& operator=(const Submap& other)
  {
    // Nice boilerplate
    m_mapping = other.m_mapping;
    m_game = other.m_game;
    m_manager = other.m_manager;
    m_nonempty_count = other.m_nonempty_count;
    return *this;
  }

  /// \returns The size of the restriction. 
  std::size_t size() const {
    return m_mapping.size();
  }
  
  /// \returns The size of the restriction. 
  std::size_t count() const {
    std::size_t count = 0;
    for (const auto& entry : m_mapping) {
      count += !BDD_IS_EMPTY(m_manager, entry);
    }
    assert(m_nonempty_count == count);

    return m_nonempty_count;
  }

  /// \returns True iff the given BDD is equal to lambda x in V. \emptyset
  bool is_empty() const {
    assert((m_nonempty_count == 0) == (count() == 0));
    return m_nonempty_count == 0;
  }

  void clear() {
    for (std::size_t i = 0; i < m_mapping.size(); ++i) {
      m_mapping[i] = BDD_EMPTYSET(m_manager);
    }

    m_nonempty_count = 0;
  }

  BDD operator[](std::size_t index) const {
    return m_mapping[index];
  }

  SubmapProxy operator[](std::size_t index) {
    return SubmapProxy(m_manager, m_mapping[index], m_nonempty_count);
  }

  Submap& operator|=(const Submap& other) {
    assert(m_mapping.size() == other.m_mapping.size());

    for (std::size_t i = 0; i < m_mapping.size(); ++i) {
      (*this)[i] |= other[i];
    }
    
    return *this;
  }
  
  Submap& operator-=(const Submap& other) {    
    assert(m_mapping.size() == other.m_mapping.size());
    assert(size() >= 0);

    for (std::size_t i = 0; i < m_mapping.size(); ++i) {
      (*this)[i] -= other[i];
    }

    assert(size() >= 0);
    return *this;
  }
  
  Submap& operator&=(const Submap& other) {    
    assert(m_mapping.size() == other.m_mapping.size());
    assert(size() >= 0);

    for (std::size_t i = 0; i < m_mapping.size(); ++i) {
      (*this)[i] &= other[i];
    }

    assert(size() >= 0);
    return *this;
  }

  Submap& operator-=(const BDD& C) {    
    assert(size() >= 0);

    for (std::size_t i = 0; i < m_mapping.size(); ++i) {
      (*this)[i] -= C;
    }

    assert(size() >= 0);
    return *this;
  }

  bool operator==(const Submap& other) {
    for (std::size_t i = 0; i < m_mapping.size(); ++i) {
      if (m_mapping[i] != other.m_mapping[i]) {
        std::cerr << "Mismatch on vertex " << i << std::endl;
        print_set(*m_game, m_mapping[i]);
        print_set(*m_game, other.m_mapping[i]);

        return false;
      }
    }

    return true;
  }
  
  bool operator!=(const Submap& other) {
    return !(*this == other);
  }

  void print(const Game& game) {

    for (int i = 0; i < m_mapping.size(); ++i) {
      if (!BDD_IS_EMPTY(m_manager, m_mapping[i])) {
        std::cout << i << ": ";
        print_set(game, m_mapping[i]);
      }
    }
  }

private:
  friend SubmapProxy;

  std::vector<BDD> m_mapping;

  const Game* m_game; ///< Only for debugging purposes.
  BDD_MANAGER& m_manager;

  std::size_t m_nonempty_count = 0; // Invariant: counts the number of empty positions in the mapping.
};

inline
void print_set(BDD_MANAGER& manager, const Submap& W, const std::vector<std::pair<BDD, std::string>>& configurations, bool full_solution)
{
  for (const auto& product : configurations) {    
    std::cout << "For product " << product.second << " the following vertices are in: ";
    for (std::size_t v = 0; v < W.size(); v++) {
      BDD tmp = W[v];
      tmp &= product.first;

      if (!BDD_IS_EMPTY(manager, tmp)) {
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

inline
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
