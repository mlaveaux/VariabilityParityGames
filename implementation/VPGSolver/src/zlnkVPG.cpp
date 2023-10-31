//
// Created by sjef on 5-6-19.
//

#include "zlnkVPG.h"
#include <chrono>
#include <iostream>
#include <map>
#include <queue>

zlnkVPG::zlnkVPG(const Game& game, bool metrics)
  : game(game),
    conf_metricoutput(metrics),
    m_vertices(game.number_of_vertices())
{}

std::pair<std::vector<ConfSet>, std::vector<ConfSet>> zlnkVPG::solve() const
{
  // Initially all vertices belong to all configurations
  std::vector<ConfSet> rho(game.number_of_vertices());
  for (ConfSet& conf : rho) {
    conf = game.configurations();
  }

  auto result = solve_rec(std::move(rho));
  return std::make_pair(result[0], result[1]);
}

/// \returns True iff the given confset is equal to lambda x in V. \emptyset
/// TODO: This could be sped up by introducing a bitvector to indicate that a configuration is empty.
bool is_confset_empty(const std::vector<ConfSet>& rho) {
  for (const ConfSet& conf : rho) {
    if (conf != emptyset) {
      return false;
    }
  }

  return true;
}

/// \returns lambda x in V . rhs(x) cup lhs(x), but mutating lhs instead.
void confset_union(const std::vector<ConfSet>& rhs, std::vector<ConfSet>& lhs) {
  for (std::size_t i = 0; i < lhs.size(); ++i) {
    lhs[i] |= rhs[i];
  }
}

void confset_minus(const std::vector<ConfSet>& rhs, std::vector<ConfSet>& lhs) {
  for (std::size_t i = 0; i < lhs.size(); ++i) {
    lhs[i] -= rhs[i];
  }
}

std::array<std::vector<ConfSet>, 2> zlnkVPG::solve_rec(std::vector<ConfSet>&& rho) const {
  // 1. if rho == lambda v in V. \emptyset then
  if (is_confset_empty(rho)) {
    return std::array<std::vector<ConfSet>, 2>({rho, rho});
  } else {
    // m := max { p(v) | v in V && rho(v) \neq \emptyset }
    int m = get_highest_prio(rho);

    // 6. \alpha := m mod 2
    int alpha = (m % 2);
    int not_alpha = 1 - alpha;

    // 7. U := lambda v in V. { \rho(v) | p(v) = m }
    std::vector<ConfSet> U(game.number_of_vertices());
    for (const auto& v : game.priority_vertices(m)) {
      U[v] = rho[v];
    }

    // 8. A := attr_alpha(U), we update U.
    attr(alpha, rho, U);
    const std::vector<ConfSet>& A = U;

    // 9. (W'_0, W'_1) := solve(rho \ A)
    std::vector<ConfSet> rho_minus = rho;
    confset_minus(A, rho_minus);

    std::array<std::vector<ConfSet>, 2> W_prime = solve_rec(std::move(rho_minus));

    // 10.
    if (is_confset_empty(W_prime[not_alpha])) {
      // W_prime[alpha] not used after this so can be changed.
      // 11. W_alpha := W'_alpha \cup A
      // 20. return (W_0, W_1) 
      confset_union(A, W_prime[alpha]);
      return W_prime;
    } else {
      // B := attr_notalpha(W'_notalpha)
      // W_prime[not_alpha] not used after this so can be changed.
      attr(not_alpha, rho, W_prime[not_alpha]);
      const std::vector<ConfSet>& B = W_prime[not_alpha];

      // rho not used after this so can be changed.
      // 15. (W''_0, W''_1) := solve(rho \ B)
      confset_minus(B, rho);
      std::array<std::vector<ConfSet>, 2> W_doubleprime = solve_rec(std::move(rho));

      // 16. W_alpha := W'_notalpha \cup B
      // 20. return (W_0, W_1) 
      confset_union(B, W_doubleprime[not_alpha]);
      return W_doubleprime;
    }
  }
}

std::array<std::vector<ConfSet>, 2> zlnkVPG::solve_optimised_rec(std::vector<ConfSet>&& rho) const {
  // 1. if rho == lambda v in V. \emptyset then
  if (is_confset_empty(rho)) {
    return std::array<std::vector<ConfSet>, 2>({rho, rho});
  } else {
    // 5. m := max { p(v) | v in V && rho(v) \neq \emptyset }
    int m = get_highest_prio(rho);

    // 6. \alpha := m mod 2
    int alpha = (m % 2);
    int not_alpha = 1 - alpha;

    // 7. U := lambda v in V. { \rho(v) | p(v) = m }
    std::vector<ConfSet> U(game.number_of_vertices());
    for (const auto& v : game.priority_vertices(m)) {
      U[v] = rho[v];
    }

    // 8. A := attr_alpha(U), we update U.
    attr(alpha, rho, U);
    const std::vector<ConfSet>& A = U;

    // 9. (W'_0, W'_1) := solve(rho \ A)
    std::vector<ConfSet> rho_minus = rho;
    confset_minus(A, rho_minus);

    std::array<std::vector<ConfSet>, 2> W_prime = solve_optimised_rec(std::move(rho_minus));

    // 10.
    if (is_confset_empty(W_prime[not_alpha])) {
      // W_prime[alpha] not used after this so can be changed.
      // 11. W_alpha := W'_alpha \cup A
      // 20. return (W_0, W_1) 
      confset_union(A, W_prime[alpha]);
      return W_prime;
    } else {
      // TODO: Finish this case
      // B := attr_notalpha(W'_notalpha)
      // W_prime[not_alpha] not used after this so can be changed.
      attr(not_alpha, rho, W_prime[not_alpha]);
      const std::vector<ConfSet>& B = W_prime[not_alpha];

      // rho not used after this so can be changed.
      // 15. (W''_0, W''_1) := solve(rho \ B)
      confset_minus(B, rho);
      std::array<std::vector<ConfSet>, 2> W_doubleprime = solve_optimised_rec(std::move(rho));

      // 16. W_alpha := W'_notalpha \cup B
      // 20. return (W_0, W_1) 
      confset_union(B, W_doubleprime[not_alpha]);
      return W_doubleprime;
    }
  }
}

void zlnkVPG::attr(int alpha, const std::vector<ConfSet>& rho, std::vector<ConfSet>& U) const
{
  auto start = std::chrono::high_resolution_clock::now();

  // We use a dynamic bitset to tag vertices are being part of Q to speed up checks for inclusion in Q.
  m_vertices.reset();

  // 2. Queue Q := {v \in V | U(v) != \emptset }
  std::queue<int> Q;
  for (int v = 0; v < game.number_of_vertices(); v++) {
    if (U[v] != emptyset) {
      Q.push(v);
      m_vertices[v] = true;
    }
  }

  // 3. A := U, we mutate U directly.
  std::vector<ConfSet>& A = U;

  // 4. while Q is not empty do
  while (!Q.empty()) {
    // 5. w := Q.pop()
    int w = Q.front();
    Q.pop();

    // 6. For every v \in Ew such that rho(v) \intersect \theta(v, w) \intersect A(w) != \emptyset do
    // Our theta is represented by a edge_guard for a given edge index.
    for (const auto& [v, edge] : game.predecessors(w)) {
      ConfSet a;
      a = rho[v];
      a &= A[w];
      a &= game.edge_guard(edge);

      if (a != emptyset) {
        // 7. if v in V_\alpha
        if (game.owner(v) == alpha) {
          // 8. a := rho(v) \intersect \theta(v, w) \intersect A(w) != \emptyset
          // This assignment has already been computed above.
        } 
        // 9. Else
        else {
          // 10. a := rho(v)
          a = rho[v];
          // 11. for w' \in vE such that rho(v) && theta(v, w') && \rho(w') != \emptyset do
          for (const auto& [w_prime, edge] : game.successors(v)) {
            ConfSet tmp = rho[v];
            tmp &= game.edge_guard(edge);
            tmp &= rho[w_prime];
            if (tmp != emptyset) {
              // 12. a := a && (C \ (theta(v, w') && \rho(w'))) \cup A(w')
              ConfSet tmp = game.edge_guard(edge);
              tmp &= rho[w_prime];
              tmp = (game.configurations() - tmp) | A[w];
              a &= tmp;              
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
    attracting += elapsed.count();
  }
}

int zlnkVPG::get_highest_prio(const std::vector<ConfSet>& rho) const
{
  int highest = 0;
  for (int v = 0; v < rho.size(); v++) {
    if (rho[v] != emptyset) {
      highest = std::max(highest, game.priority(v));
    }
  }

  return highest;
}