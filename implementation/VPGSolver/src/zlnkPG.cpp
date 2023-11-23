//
// Created by sjef on 26-10-19.
//

#include "zlnkPG.h"
#include <chrono>
#include <iostream>
#include <map>
#include <queue>


zlnkPG::zlnkPG(const Game& game, bool debug)
  : game(game),
    m_debug(debug),
    m_vertices(game.number_of_vertices())
{}

std::pair<boost::dynamic_bitset<>, boost::dynamic_bitset<>> zlnkPG::solve() const {

  // All vertices are in the initial game.
  boost::dynamic_bitset<> V(game.number_of_vertices());
  V.set();

  std::array<boost::dynamic_bitset<>,2> result = solve_rec(std::move(V));
  std::cout << "Performed " << m_recursive_calls << " recursive calls" << std::endl;
  
  return std::make_pair(result[0], result[1]);
}

std::array<boost::dynamic_bitset<>,2> zlnkPG::solve_rec(boost::dynamic_bitset<>&& V) const
{
  m_recursive_calls += 1;

  if (!V.any()) {
    return std::array<boost::dynamic_bitset<>,2>({V, V});
  } else {
    auto [m, l] = get_highest_lowest_prio(V);

    int alpha = m % 2;
    int not_alpha = 1 - alpha;

    // 7.
    boost::dynamic_bitset<> U(game.number_of_vertices());
    for (const auto& v : game.priority_vertices(m)) {
      if (V[v]) {
        U[v] = true;
      }      
    }

    if (m_debug) { std::cerr << "solve_rec(V) |V| = " 
      << V.count() << ", m = " 
      << m << ", l = " 
      << l << " and |U| = " << U.count() << std::endl; }

    attr(alpha, V, U);
    const boost::dynamic_bitset<>& A = U; // U has been mutated to A.

    // TODO: W_prime[alpha] is unnecessarily created when W_prime[not_alpha] is empty.
    if (m_debug) { std::cerr << "begin solve_rec(V-A)" << std::endl; }
    std::array<boost::dynamic_bitset<>,2> W_prime = solve_rec(V - A);
    if (m_debug) { std::cerr << "end solve_rec(V-A)" << std::endl; }
    if (!W_prime[not_alpha].any()) {
      // W_prime[alpha] not used after this so can be changed.
      W_prime[alpha] |= A;
      return W_prime;
    } else {
      // W_prime[not_alpha] not used after this so can be changed.
      attr(not_alpha, V, W_prime[not_alpha]);
      const boost::dynamic_bitset<>& B = W_prime[not_alpha];

      // V not used after this so can be changed.
      V -= B;
      if (m_debug) { std::cerr << "begin solve_rec(V-B)" << std::endl; }
      std::array<boost::dynamic_bitset<>,2> W_doubleprime = solve_rec(std::move(V));
      if (m_debug) { std::cerr << "end solve_rec(V-B)" << std::endl; }

      W_doubleprime[not_alpha] |= B;
      return W_doubleprime;
    }
  }
}

void zlnkPG::attr(int alpha, const boost::dynamic_bitset<>& V, boost::dynamic_bitset<>& A) const
{
  auto start = std::chrono::high_resolution_clock::now();

  // We use this vector to tag vertices as belonging to Q for quick inclusion checks.
  m_vertices.reset();

  // 2. Q = {v \in A}
  std::queue<int> Q;
  for (int v = 0; v < A.size(); v++) {
    if (A[v]) {
      Q.push(v);
      m_vertices[v] = true;
    }
  }

  int initial_size = A.count();

  // 4. While Q is not empty do
  while (!Q.empty()) {
    // 5. w <- Q.pop()
    int w = Q.front();
    Q.pop();

    // For every v \in Ew do
    for (auto& [v, edge] : game.predecessors(w)) {
      if (V[v]) { 
        bool attracted = true; 
        if (game.owner(v) == alpha) {
          // v \in V and v in V_\alpha
          attracted = true;
        } else {
          // v \in V and v notin V_\alpha
          for (auto& [w_prime, edge] : game.successors(v)) {
            if (V[w_prime] && !A[w_prime]) {
              attracted = false;
            }
          }
        }

        if (attracted) {
          if (!m_vertices[v]) {
            m_vertices[v] = true;
            A[v] = true;
            Q.push(v);
          }
        } 
      }
    }
  }

  auto end = std::chrono::high_resolution_clock::now();
  auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
  attracting += elapsed.count();

  if (m_debug) { std::cerr << "attracted " << A.count() << " verticed towards " << initial_size << " vertices" << std::endl; }
}


std::pair<std::size_t, std::size_t> zlnkPG::get_highest_lowest_prio(const boost::dynamic_bitset<>& V) const
{
  std::size_t highest = 0;  
  std::size_t lowest = std::numeric_limits<std::size_t >::max();

  for (std::size_t v = 0; v < game.number_of_vertices(); v++) {
    if (V[v]) {
      highest = std::max(highest, game.priority(v));
      lowest = std::min(lowest, game.priority(v));
    }
  }

  return std::make_pair(highest, lowest);
}