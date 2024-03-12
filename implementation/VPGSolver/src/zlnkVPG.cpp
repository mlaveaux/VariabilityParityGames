//
// Created by sjef on 5-6-19.
//

#include "zlnkVPG.h"
#include "Game.h"
#include "Submap.h"
#include <chrono>
#include <iostream>
#include <map>
#include <queue>

zlnkVPG::zlnkVPG(const Game& game, bool debug)
  : game(game),
    m_verbose(debug),
    m_vertices(game.number_of_vertices())
{}

std::pair<Submap, Submap> zlnkVPG::solve() const
{
  // Initially all vertices belong to all configurations
  Submap gamma(game, game.configurations());

  auto result = solve_rec(std::move(gamma));
  std::cout << "Performed " << m_recursive_calls << " recursive calls" << std::endl;
  
  return std::make_pair(result[0], result[1]);
}

std::pair<Submap, Submap> zlnkVPG::solve_optimised() const
{
  // Initially all vertices belong to all configurations
  Submap gamma(game, game.configurations());

  auto result = solve_optimised_rec(std::move(gamma));
  std::cout << "Performed " << m_recursive_calls << " recursive calls" << std::endl;

  return std::make_pair(result[0], result[1]);
}

std::pair<Submap, Submap> zlnkVPG::solve_optimised_left() const
{
  // Initially all vertices belong to all configurations
  Submap gamma(game, game.configurations());

  auto result = solve_optimised_left_rec(std::move(gamma));
  std::cout << "Performed " << m_recursive_calls << " recursive calls" << std::endl;

  return std::make_pair(result[0], result[1]);
}


std::array<Submap, 2> zlnkVPG::solve_rec(Submap&& gamma) const {
  m_recursive_calls += 1;

  // 1. if \gamma == \epsilon then
  if (gamma.is_empty()) {
    if (m_verbose) { std::cerr << "empty subgame" << std::endl; }
    return std::array<Submap, 2>({gamma, gamma});
  } else {
    // m := max { p(v) | v in V && \gamma(v) \neq \emptyset }
    auto [m, l] = get_highest_lowest_prio(gamma);

    // 6. x := m mod 2
    int x = m % 2;
    int not_x = 1 - x;

    // Optimisation, terminate early when whole game has a single priority. Then A == U == gamma.
    // if (m == l) {
    //   if (alpha == 0) {
    //     if (m_verbose) { std::cerr << "single priority won by player 0" << std::endl; }
    //     return std::array<Submap, 2>({gamma, Submap(game.number_of_vertices())});
    //   } else {        
    //     if (m_verbose) { std::cerr << "single priority won by player 1" << std::endl; }
    //     return std::array<Submap, 2>({Submap(game.number_of_vertices()), gamma});
    //   }
    // }

    // 7. \mu := lambda v in V. { \gamma(v) | p(v) = m }
    Submap mu(game);
    for (const auto& v : game.priority_vertices(m)) {
      mu[v] = (bdd)gamma[v];
    }
    
    if (m_verbose) { std::cerr << "solve_rec(gamma) |gamma| = " 
      << gamma.count() << ", m = " 
      << m << ", l = " 
      << l << " and |mu| = " << mu.count() << std::endl; }

    // 8. \alpha := attr_\alpha(U), we update \mu.
    attr(x, gamma, mu);
    const Submap& alpha = mu;

    // 9. (W'_0, W'_1) := solve(\gamma \ \alpha)
    Submap gamma_minus = gamma;
    gamma_minus -= alpha;

    if (m_verbose) { std::cerr << "begin solve_rec(gamma \\ alpha)" << std::endl; }
    std::array<Submap, 2> omega_prime = solve_rec(std::move(gamma_minus));
    if (m_verbose) { std::cerr << "end solve_rec(gamma \\ alpha)" << std::endl; }
    if (m_verbose) { std::cerr << "|omega'_0| = " << omega_prime[0].count() << " and |omega'_1| = " << omega_prime[1].count() << std::endl; }

    // 10. if omega_prime_x == \epsilon then
    if (omega_prime[not_x].is_empty()) {
      // W_prime[alpha] not used after this so can be changed.
      // 11. omega_x := omega'_x \cup alpha
      // 20. return (omega_0, omega_1) 
      if (m_verbose) { std::cerr << "return (omega'_0, omega'_1) " << std::endl; }
      omega_prime[x] = gamma;
      omega_prime[not_x].clear();
      return omega_prime;
    } else {
      // \beta := attr_notalpha(\omega'_notalpha)
      // \omega'[not_x] not used after this so can be changed.
      attr(not_x, gamma, omega_prime[not_x]);
      const Submap& beta = omega_prime[not_x];

      // gamma not used after this so can be changed.
      // 15. (omega''_0, omega''_1) := solve(gamma \ beta)
      gamma -= beta;
      if (m_verbose) { std::cerr << "begin solve_rec(gamma - beta)" << std::endl; }
      std::array<Submap, 2> omega_doubleprime = solve_rec(std::move(gamma));
      if (m_verbose) { std::cerr << "end solve_rec(gamma - beta)" << std::endl; }

      // 16. W_alpha := W'_notalpha \cup B
      // 20. return (W_0, W_1) 
      omega_doubleprime[not_x] |= beta;
      if (m_verbose) { std::cerr << "return (omega''_0, omega''_1) " << std::endl; }
      return omega_doubleprime;
    }
  }
}

std::array<Submap, 2> zlnkVPG::solve_optimised_rec(Submap&& rho) const {
  m_recursive_calls += 1;

  // 1. if rho == lambda v in V. \emptyset then
  if (rho.is_empty()) {
    return std::array<Submap, 2>({rho, rho});
  } else {
    // 5. m := max { p(v) | v in V && rho(v) \neq \emptyset }
    auto [m, l] = get_highest_lowest_prio(rho);

    // 6. \alpha := m mod 2
    int alpha = (m % 2);
    int not_alpha = 1 - alpha;

    // 7. U := lambda v in V. { \rho(v) | p(v) = m }
    Submap U(game);
    for (const auto& v : game.priority_vertices(m)) {
      U[v] = (bdd)rho[v];
    }

    // 8. A := attr_alpha(U), we update U.
    attr(alpha, rho, U);
    Submap& A = U;

    // 9. (W'_0, W'_1) := solve(rho \ A)
    Submap rho_minus = rho;
    rho_minus -= A;

    std::array<Submap, 2> W_prime = solve_optimised_rec(std::move(rho_minus));

    // 10.
    if (W_prime[not_alpha].is_empty()) {
      // W_prime[alpha] not used after this so can be changed.
      // 11. W_alpha := W'_alpha \cup A
      // 20. return (W_0, W_1) 
      W_prime[alpha] |= A;
      return W_prime;
    } else {
      // 14. B := attr_notalpha(W'_notalpha)
      // W_prime[not_alpha] not used after this so can be changed.
      attr(not_alpha, rho, W_prime[not_alpha]);
      const Submap& B = W_prime[not_alpha];

      // 15. { c \in bigC | \exists v in V: B(v) }
      ConfSet C = emptyset;
      for (std::size_t i = 0; i < B.size(); ++i) {
        C |= B[i];
      }

      // 16. A := ((A cup W_\alpha) \ B) \ lambda v in V. C 
      A |= W_prime[alpha];
      A -= B;
      A -= C;

      // rho not used after this so can be changed.
      // 15. (W''_0, W''_1) := solve(rho \ (A \cup B))
      rho -= A;
      rho -= B;
      std::array<Submap, 2> W_doubleprime = solve_optimised_rec(std::move(rho));

      // 16. W_alpha := W'_notalpha \cup B
      // 20. return (W_0, W_1) 
      W_doubleprime[alpha] |= A;
      W_doubleprime[not_alpha] |= B;
      return W_doubleprime;
    }
  }
}


std::array<Submap, 2> zlnkVPG::solve_optimised_left_rec(Submap&& gamma) const {
  m_recursive_calls += 1;
  Submap gamma_copy = gamma;

  // 1. if \gamma == \epsilon then
  if (gamma.is_empty()) {
    // 2. return (\epsilon, \epsilon)
    if (m_verbose) { std::cerr << "empty subgame" << std::endl; }
    return std::array<Submap, 2>({gamma, gamma});
  } else {
    m_depth += 1;

    // 5. m := max { p(v) | v in V && \gamma(v) \neq \emptyset }
    auto [m, l] = get_highest_lowest_prio(gamma);

    // 6. x := m mod 2
    int x = (m % 2);
    int not_x = 1 - x;

    // 7. C := { c in \bigC | exists v in V : p(v) = m && c in \gamma(v) }
    // 8. \mu := lambda v in V. { \gamma(v) | p(v) = m }
    Submap mu(game);
    ConfSet C = emptyset;
    for (const auto& v : game.priority_vertices(m)) {
      mu[v] = (bdd)gamma[v];
      C |= gamma[v];
    }
    
    if (m_verbose) { std::cerr << "solve_optimised_left_rec(gamma) |gamma| = " 
      << gamma.count() << ", m = " 
      << m << ", l = " 
      << l << " and |mu| = " << mu.count() << std::endl; }

    if (m_debug) {
      std::cerr << "gamma = ";
      gamma.print(game);

      std::cerr << "C = ";
      print_set(game, C);
    }

    // 9. \alph := attr_x(\mu), we update \mu.
    attr(x, gamma, mu);
    Submap& alpha = mu;

    // 10. (\omega'_0, \omega'_1) := solve(gamma \ A)
    Submap gamma_minus = gamma;
    gamma_minus -= alpha;

    if (m_verbose) { std::cerr << m_depth << " - begin solve_optimised_left_rec(gamma \\ alpha)" << std::endl; }
    std::array<Submap, 2> omega_prime = solve_optimised_left_rec(std::move(gamma_minus));
    if (m_verbose) { std::cerr << m_depth << " - end solve_optimised_left_rec(gamma \\ alpha)" << std::endl; }

    // omega_prime[not_x] restricted to C
    ConfSet C_restriction = game.configurations();
    C_restriction -= C;
    Submap omega_prime_not_x_restricted = omega_prime[not_x];
    omega_prime_not_x_restricted -= C_restriction;

    // 10.
    if (omega_prime_not_x_restricted.is_empty()) {
      // \omega'[x] not used after this so can be changed.
      // 11. \omega_x := \omega'_x \cup A
      // 20. return (\omega_0, \omega_1)
      if (m_verbose) { std::cerr << "return (omega'_0, omega'_1) " << std::endl; }
      omega_prime[x] |= alpha;

      // Assert that the winning sets form a partition.
      Submap tmp = omega_prime[0];
      tmp |= omega_prime[1];
      assert(tmp == gamma_copy);
      
      Submap tmp4 = omega_prime[0];
      tmp4 &= omega_prime[1];
      assert(tmp4.is_empty());

      m_depth -= 1;
      return omega_prime;
    } else {
      // 14. C' := { c in C | exists v in V : c \in \omega'_not_x(v) }
      ConfSet C_prime = emptyset;
      for (std::size_t v = 0; v < game.number_of_vertices(); ++v) {
        C_prime |= omega_prime[not_x][v];
      }
      C_prime &= C;
      
      if (m_verbose) {
        std::cerr << "C' = ";
        print_set(game, C_prime);
      }

      ConfSet C_prime_restriction = game.configurations();
      C_prime_restriction -= C_prime;

      // No longer used so can be overwritten
      // omega_prime[not_x] restricted to C'
      Submap& omega_prime_not_x_restricted_prime = omega_prime_not_x_restricted;
      omega_prime_not_x_restricted_prime = omega_prime[not_x];
      omega_prime_not_x_restricted_prime -= C_prime_restriction;

      // 16. B := attr_not_x(\omega'_not_x | C')
      attr(not_x, gamma, omega_prime_not_x_restricted_prime);
      const Submap& alpha_prime = omega_prime_not_x_restricted_prime;

      // gamma not used after this so can be changed.
      // 15. (W''_0, W''_1) := solve(gamma|C' \ \alpha'))
      gamma -= C_prime_restriction;
      gamma -= alpha_prime;

      if (m_verbose) { std::cerr << m_depth << " - begin solve_optimised_left_rec(gamma | C' - alpha')" << std::endl; }
      std::array<Submap, 2> omega_doubleprime = solve_optimised_left_rec(std::move(gamma));
      if (m_verbose) { std::cerr << m_depth << " - end solve_optimised_left_rec(gamma | C' - alpha')" << std::endl; }

      // 16. \omega'_x := \omega'_x|C' \cup \omega''_x
      // 16. \omega_not_x := \omega'_not_x|C' \cup \omega''_x \cup \alpha'
      // 20. return (W_0, W_1)
      omega_prime[x] -= C_prime;
      omega_prime[not_x] -= C_prime;

      alpha -= C_prime;
      omega_doubleprime[x] |= omega_prime[x];
      omega_doubleprime[x] |= alpha;
      omega_doubleprime[not_x] |= omega_prime[not_x];
      omega_doubleprime[not_x] |= alpha_prime;

      if (m_verbose) { std::cerr << "return (omega''_0, omega''_1) " << std::endl; }

      if (m_verbose) { std::cerr << alpha_prime.count() << std::endl; }
      
      // Assert that the winning sets form a partition.
      Submap tmp = omega_doubleprime[0];
      tmp |= omega_doubleprime[1];
      std::cerr << tmp.count() << std::endl;
      std::cerr << gamma_copy.count() << std::endl;
      assert(tmp == gamma_copy);

      Submap tmp4 = omega_doubleprime[0];
      tmp4 &= omega_doubleprime[1];
      assert(tmp4.is_empty());

      m_depth -= 1;
      return omega_doubleprime;
    }
  }
}


void zlnkVPG::attr(int alpha, const Submap& gamma, Submap& U) const
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
  Submap& A = U;

  // 4. while Q is not empty do
  while (!Q.empty()) {
    // 5. w := Q.pop()
    int w = Q.front();
    Q.pop();
    m_vertices[w] = false;

    // 6. For every v \in Ew such that gamma(v) \intersect \theta(v, w) \intersect A(w) != \emptyset do
    // Our theta is represented by a edge_guard for a given edge index.
    for (const auto& [v, edge] : game.predecessors(w)) {
      ConfSet a = gamma[v];
      a &= A[w];
      a &= game.edge_guard(edge);

      if (a != emptyset) {
        // 7. if v in V_\alpha
        if (game.owner(v) == alpha) {
          // 8. a := gamma(v) \intersect \theta(v, w) \intersect A(w)
          // This assignment has already been computed above.
        }
        else {
          // 10. a := gamma(v)
          a = gamma[v];
          // 11. for w' \in vE such that gamma(v) && theta(v, w') && \gamma(w') != \emptyset do
          for (const auto& [w_prime, edge_succ] : game.successors(v)) {
            ConfSet tmp = gamma[v];
            tmp &= game.edge_guard(edge_succ);
            tmp &= gamma[w_prime];
            if (tmp != emptyset) {
              // 12. a := a && (C \ (theta(v, w') && \gamma(w'))) \cup A(w')
              ConfSet tmp = game.edge_guard(edge_succ);
              tmp &= gamma[w_prime];
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

  if (m_verbose) { std::cerr << "attracted " << U.count() << " vertices towards " << initial_size << " vertices" << std::endl; }

  attracting += elapsed.count();
}

std::pair<std::size_t, std::size_t> zlnkVPG::get_highest_lowest_prio(const Submap& gamma) const
{
  std::size_t highest = 0;
  std::size_t lowest = std::numeric_limits<std::size_t >::max();

  for (std::size_t v = 0; v < game.number_of_vertices(); v++) {
    if (gamma[v] != emptyset) {
      highest = std::max(highest, game.priority(v));
      lowest = std::min(lowest, game.priority(v));
    }
  }

  return std::make_pair(highest, lowest);
}