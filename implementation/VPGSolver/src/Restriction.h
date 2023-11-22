
#pragma once

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
