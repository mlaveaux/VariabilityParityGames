
#pragma once

#include "Game.h"

#include <boost/dynamic_bitset.hpp>

#include <array>
#include <vector>

inline
void print_set(const Game& game, ConfSet set)
{
  std::cout << "{ ";
  for (const auto& product : game.configurations_explicit(set)) {
    std::cout << product.second << ", ";    
  }

  std::cout << "}" << std::endl;  
}

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
    assert((m_nonempty_count == 0) == (count() == 0));
    return m_nonempty_count == 0;
  }

  void clear() {
    for (std::size_t i = 0; i < m_mapping.size(); ++i) {
      m_mapping[i] = emptyset;
    }

    m_nonempty_count = 0;
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

  Restriction& operator-=(const ConfSet& C) {    
    assert(m_mapping.size() == other.m_mapping.size());
    assert(size() >= 0);

    for (std::size_t i = 0; i < m_mapping.size(); ++i) {
      (*this)[i] -= C;
    }

    assert(size() >= 0);
    return *this;
  }

  bool operator==(const Restriction& other) {
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
  
  bool operator!=(const Restriction& other) {
    return !(*this == other);
  }

  void print(const Game& game) {

    for (int i = 0; i < m_mapping.size(); ++i) {
      if (m_mapping[i] != emptyset) {
        std::cout << i << ": ";
        print_set(game, m_mapping[i]);
      }
    }
  }

private:
  friend RestrictionProxy;

  std::vector<ConfSet> m_mapping;

  const Game* m_game; ///< Only for debugging purposes.

  std::size_t m_nonempty_count = 0; // Invariant: counts the number of empty positions in the mapping.
};

inline
void print_set(const Restriction& W, const std::vector<std::pair<ConfSet, std::string>>& configurations, bool full_solution)
{
  for (const auto& product : configurations) {    
    std::cout << "For product " << product.second << " the following vertices are in: ";
    for (std::size_t v = 0; v < W.size(); v++) {
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
