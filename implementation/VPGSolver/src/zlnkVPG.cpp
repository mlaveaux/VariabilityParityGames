//
// Created by sjef on 5-6-19.
//

#include "zlnkVPG.h"
#include <chrono>
#include <iostream>
#include <map>
#include <queue>

zlnkVPG::zlnkVPG(const Game& game, bool debug)
  : game(game),
    m_debug(debug),
    m_vertices(game.number_of_vertices())
{}

std::pair<Restriction, Restriction> zlnkVPG::solve() const
{
  // Initially all vertices belong to all configurations
  Restriction rho(game.number_of_vertices(), game.configurations());

  auto result = solve_rec(std::move(rho));
  return std::make_pair(result[0], result[1]);
}


std::array<Restriction, 2> zlnkVPG::solve_rec(Restriction&& rho) const {
  // 1. if rho == lambda v in V. \emptyset then
  if (rho.is_empty()) {
    if (m_debug) { std::cerr << "empty subgame" << std::endl; }
    return std::array<Restriction, 2>({rho, rho});
  } else {
    // m := max { p(v) | v in V && rho(v) \neq \emptyset }
    auto [m, l] = get_highest_lowest_prio(rho);

    // 6. \alpha := m mod 2
    int alpha = m % 2;
    int not_alpha = 1 - alpha;

    // Optimisation, terminate early when whole game has a single priority. Then A == U == rho.
    // if (m == l) {
    //   if (alpha == 0) {
    //     if (m_debug) { std::cerr << "single priority won by player 0" << std::endl; }
    //     return std::array<Restriction, 2>({rho, Restriction(game.number_of_vertices())});
    //   } else {        
    //     if (m_debug) { std::cerr << "single priority won by player 1" << std::endl; }
    //     return std::array<Restriction, 2>({Restriction(game.number_of_vertices()), rho});
    //   }
    // }

    // 7. U := lambda v in V. { \rho(v) | p(v) = m }
    Restriction U(game.number_of_vertices());
    for (const auto& v : game.priority_vertices(m)) {
      U[v] = (bdd)rho[v];
    }
    
    if (m_debug) { std::cerr << "solve_rec(rho) |rho| = " 
      << rho.size() << ", m = " 
      << m << ", l = " 
      << l << " and |U| = " << U.size() << std::endl; }

    // 8. A := attr_alpha(U), we update U.
    attr(alpha, rho, U);
    const Restriction& A = U;

    // 9. (W'_0, W'_1) := solve(rho \ A)
    Restriction rho_minus = rho;
    rho_minus -= A;

    if (m_debug) { std::cerr << "begin solve_rec(rho-A)" << std::endl; }
    std::array<Restriction, 2> W_prime = solve_rec(std::move(rho_minus));
    if (m_debug) { std::cerr << "end solve_rec(rho-A)" << std::endl; }
    if (m_debug) { std::cerr << "|W0| = " << W_prime[0].size() << " and |W1| = " << W_prime[1].size() << std::endl; }

    // 10.
    if (W_prime[not_alpha].is_empty()) {
      // W_prime[alpha] not used after this so can be changed.
      // 11. W_alpha := W'_alpha \cup A
      // 20. return (W_0, W_1) 
      if (m_debug) { std::cerr << "return (W'_0, W'_1) " << std::endl; }
      W_prime[alpha] |= A;
      return W_prime;
    } else {
      // B := attr_notalpha(W'_notalpha)
      // W_prime[not_alpha] not used after this so can be changed.
      attr(not_alpha, rho, W_prime[not_alpha]);
      const Restriction& B = W_prime[not_alpha];

      // rho not used after this so can be changed.
      // 15. (W''_0, W''_1) := solve(rho \ B)
      rho -= B;
      if (m_debug) { std::cerr << "begin solve_rec(rho-B)" << std::endl; }
      std::array<Restriction, 2> W_doubleprime = solve_rec(std::move(rho));
      if (m_debug) { std::cerr << "end solve_rec(rho-B)" << std::endl; }

      // 16. W_alpha := W'_notalpha \cup B
      // 20. return (W_0, W_1) 
      W_doubleprime[not_alpha] |= B;
      if (m_debug) { std::cerr << "return (W''_0, W''_1) " << std::endl; }
      return W_doubleprime;
    }
  }
}

std::array<Restriction, 2> zlnkVPG::solve_optimised_rec(Restriction&& rho) const {
  // 1. if rho == lambda v in V. \emptyset then
  if (rho.is_empty()) {
    return std::array<Restriction, 2>({rho, rho});
  } else {
    // 5. m := max { p(v) | v in V && rho(v) \neq \emptyset }
    auto [m, l] = get_highest_lowest_prio(rho);

    // 6. \alpha := m mod 2
    int alpha = (m % 2);
    int not_alpha = 1 - alpha;

    // 7. U := lambda v in V. { \rho(v) | p(v) = m }
    Restriction U(game.number_of_vertices());
    for (const auto& v : game.priority_vertices(m)) {
      U[v] = (bdd)rho[v];
    }

    // 8. A := attr_alpha(U), we update U.
    attr(alpha, rho, U);
    const Restriction& A = U;

    // 9. (W'_0, W'_1) := solve(rho \ A)
    Restriction rho_minus = rho;
    rho_minus -= A;

    std::array<Restriction, 2> W_prime = solve_optimised_rec(std::move(rho_minus));

    // 10.
    if (W_prime[not_alpha].is_empty()) {
      // W_prime[alpha] not used after this so can be changed.
      // 11. W_alpha := W'_alpha \cup A
      // 20. return (W_0, W_1) 
      W_prime[alpha] |= A;
      return W_prime;
    } else {
      // TODO: Finish this case
      // B := attr_notalpha(W'_notalpha)
      // W_prime[not_alpha] not used after this so can be changed.
      attr(not_alpha, rho, W_prime[not_alpha]);
      const Restriction& B = W_prime[not_alpha];

      // rho not used after this so can be changed.
      // 15. (W''_0, W''_1) := solve(rho \ B)
      rho -= B;
      std::array<Restriction, 2> W_doubleprime = solve_optimised_rec(std::move(rho));

      // 16. W_alpha := W'_notalpha \cup B
      // 20. return (W_0, W_1) 
      W_doubleprime[not_alpha] |= B;
      return W_doubleprime;
    }
  }
}

void zlnkVPG::attr(int alpha, const Restriction& rho, Restriction& U) const
{
  auto start = std::chrono::high_resolution_clock::now();

  // We use a dynamic bitset to tag vertices are being part of Q to speed up checks for inclusion in Q.
  //m_vertices.reset();

  // 2. Queue Q := {v \in V | U(v) != \emptset }
  std::queue<int> Q;
  for (int v = 0; v < game.number_of_vertices(); v++) {
    if ((bdd)U[v] != emptyset) {
      Q.push(v);
      m_vertices[v] = true;
    }
  }

  int initial_size = Q.size();

  // 3. A := U, we mutate U directly.
  Restriction& A = U;

  // 4. while Q is not empty do
  while (!Q.empty()) {
    // 5. w := Q.pop()
    int w = Q.front();
    Q.pop();
    m_vertices[w] = false;

    // 6. For every v \in Ew such that rho(v) \intersect \theta(v, w) \intersect A(w) != \emptyset do
    // Our theta is represented by a edge_guard for a given edge index.
    for (const auto& [v, edge] : game.predecessors(w)) {
      ConfSet a = rho[v];
      a &= A[w];
      a &= game.edge_guard(edge);

      if (a != emptyset) {
        // 7. if v in V_\alpha
        if (game.owner(v) == alpha) {
          // 8. a := rho(v) \intersect \theta(v, w) \intersect A(w)
          // This assignment has already been computed above.
        }
        else {
          // 10. a := rho(v)
          a = rho[v];
          // 11. for w' \in vE such that rho(v) && theta(v, w') && \rho(w') != \emptyset do
          for (const auto& [w_prime, edge_succ] : game.successors(v)) {
            ConfSet tmp = rho[v];
            tmp &= game.edge_guard(edge_succ);
            tmp &= rho[w_prime];
            if (tmp != emptyset) {
              // 12. a := a && (C \ (theta(v, w') && \rho(w'))) \cup A(w')
              ConfSet tmp = game.edge_guard(edge_succ);
              tmp &= rho[w_prime];
              tmp = (game.configurations() - tmp) | A[w_prime];
              a &= tmp;              
            }
          }
        }

        // 15. a \ A(v) != \emptyset
        if ((a - A[v]) != emptyset) {
          // 16. A(v) := A(v) \cup a
          A[v] |= a;

          // 17. If v not in Q then Q.push(v)
          if (!m_vertices[v]) {
            Q.push(v);
            m_vertices[v] = true;
          }
        }
      }
    }
  }

  auto end = std::chrono::high_resolution_clock::now();
  auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

  if (m_debug) { std::cerr << "attracted " << m_vertices.count() << " verticed towards " << initial_size << " vertices" << std::endl; }

  attracting += elapsed.count();
}

std::pair<std::size_t, std::size_t> zlnkVPG::get_highest_lowest_prio(const Restriction& rho) const
{
  std::size_t highest = 0;
  std::size_t lowest = std::numeric_limits<std::size_t >::max();

  for (std::size_t v = 0; v < game.number_of_vertices(); v++) {
    if (rho[v] != emptyset) {
      highest = std::max(highest, game.priority(v));
      lowest = std::min(lowest, game.priority(v));
    }
  }

  return std::make_pair(highest, lowest);
}