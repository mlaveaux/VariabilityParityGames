/***********************************************************************************************************************
 * In this project only algorithms and datastructures are implemented properly.
 *
 * The code in this file is not optimized and not up to standards; it is sufficient only for experimental applications
 * but not for any real application.
 **********************************************************************************************************************/
//
// Created by sjef on 5-6-19.
//

#include "Game.h"

#include <algorithm>
#include <chrono>
#include <cstring>
#include <fstream>
#include <iostream>
#include <random>
#include <sstream>
#include <string>
#include <unordered_set>
#include <queue>

#include <boost/dynamic_bitset.hpp>

#define PARSER_LINE_SIZE 16777216


Game GameParser::parse(const std::string& filename, const char* specificconf, bool is_parity_game)
{
  this->is_parity_game = is_parity_game;

  int c = 0;
  if (is_parity_game) {
    c++;
    parseConfs("confs 1;");
  }

  std::ifstream infile(filename);
  char* s = new char[PARSER_LINE_SIZE];
  while (infile.good()) {
    infile.getline(s, PARSER_LINE_SIZE, ';');
    if (c == 0) {
      // create bigC
      std::cout << "Found confs: " << s << '\n';
      parseConfs(s);
      if (strlen(specificconf) > 0) {
        parseConfSet(specificconf, 0, bigC);
      }
      c++;
    }
    else if (c == 1) {
      std::cout << "Found game: " << s << '\n';
      // init with parity
      parseInitialiser(s);
      c++;
    }
    else {
      // add vertex
      if (strlen(s) > 0) {
        parseVertex(s);
      }
    }
  }
  delete[] s;
  if (!infile.eof()) {
    throw std::string("could not open file");
  }

  return Game(m_manager, bm_vars, bigC, m_priority, m_owner, out_edges, edge_guards);
}

void GameParser::parseConfs(char const* line)
{
  while (*line == '\n' || *line == '\t' || *line == ' ') {
    line++;
  }

  if (strncmp(line, "confs ", 6) != 0) {
    throw std::string("Expected confs");
  }

  int i = 6;
  char c;
  do {
    c = line[i++];
  }
  while (c != '\0' && c != '+' && c != ';');
  bm_n_vars = i - 7;
  bm_vars.resize(bm_n_vars);

#ifdef ENABLE_BUDDY
  bdd_init(2000000, 2);
  bdd_setvarnum(bm_n_vars);
  bdd_varblockall();
#else

#endif

  std::vector<int> order;
  order.resize(bm_n_vars);

  for (i = 0; i < bm_n_vars; i++) {
    order[i] = i;
  }

  if (specificvarlast) {
    order[bm_n_vars - 1] = specificvar;
    order[specificvar] = bm_n_vars - 1;
  }

  std::cout << "Bdd order: ";
  for (i = 0; i < bm_n_vars; i++) {
    std::cout << '[' << i << "]=" << order[i] << ", ";
  }

  std::cout << "\n";
  for (i = 0; i < bm_n_vars; i++) {
#ifdef ENABLE_BUDDY
    bm_vars[order[i]] = bdd_ithvar(i);
#else
    bm_vars[order[i]] = m_manager.new_var();
#endif
  }

  parseConfSet(line, 6, bigC);
}

void GameParser::parseInitialiser(char* line)
{
  while (*line == '\n' || *line == '\t' || *line == ' ') {
    line++;
  }

  if (strncmp(line, "parity ", 7) != 0) {
    throw std::string("Expected parity");
  }

  int parity = atoi(line + 7);
  
  n_nodes = parity;
  out_edges = std::vector<std::vector<std::tuple<int, int>>>(n_nodes);
  m_priority.resize(n_nodes);
  m_owner.resize(n_nodes);
  declared.resize(n_nodes);
}

int GameParser::parseConfSet(const char* line, int i, BDD& result)
{
  if (is_parity_game) {
    result = BDD_UNIVERSE(m_manager);
    return i + 1;
  } else {
    bool inverse = false;
    if (line[i] == '!') {
      inverse = true;
      i++;
    }

    result = BDD_EMPTYSET(m_manager);
    BDD entry = BDD_UNIVERSE(m_manager);
    int var = 0;
    char c;
    do {
      c = line[i++];
      if (c == 'F') {
        if (var != 0) {
          throw std::string("Unexpected F");
        }
        entry = BDD_EMPTYSET(m_manager);
      }
      else if (c == '0') {
        if (var > bm_n_vars) {
          throw std::string("Too many bits");
        }
        entry = BDD_MINUS(m_manager, entry, bm_vars[var]);
        var++;
      }
      else if (c == '1') {
        if (var > bm_n_vars) {
          throw std::string("Too many bits");
        }
        entry &= bm_vars[var];
        var++;
      }
      else if (c == '-') {
        if (var > bm_n_vars) {
          throw std::string("Too many bits");
        }
        var++;
      }
      else if (c == '+') {
        result |= entry;
        entry = BDD_UNIVERSE(m_manager);
        var = 0;
      }
    }
    while (c == '0' || c == '1' || c == '-' || c == '+' || c == 'F');

    // Apply the last result.
    result |= entry;

    if (inverse) {
      // Compute the complement set.
      BDD a = result;
      result = BDD_UNIVERSE(m_manager);
      result = BDD_MINUS(m_manager, result, a);
    }

    return i;
  }
}

void GameParser::parseVertex(char* line)
{
  while (*line == '\n' || *line == '\t' || *line == ' ') {
    line++;
  }

  int index;
  int i;
  i = readUntil(line, ' ');
  index = atoi(line);
  if (declared[index]) {
    throw std::string("Already declared vertex " + std::to_string(index));
  }

  line += i + 1;
  i = readUntil(line, ' ');
  int p = atoi(line);
  m_priority[index] = p;
  line += i + 1;
  i = readUntil(line, ' ');
  m_owner[index] = atoi(line);
  line += i + 1;

  while (*line != '\0') {
    if (*line == ',') {
      line++;
    }
    int target;
    if (is_parity_game) {
      i = readUntil(line, ',');
      target = atoi(line);
      line += i;
    }
    else {
      i = readUntil(line, '|');
      target = atoi(line);
      line += i + 1;
    }

    int guardindex = edge_guards.size();
    edge_guards.resize(guardindex + 1);
    i = parseConfSet(line, 0, edge_guards[guardindex]);
    edge_guards[guardindex] &= bigC;
    if (!BDD_IS_EMPTY(m_manager, edge_guards[guardindex])) {
      int outindex = out_edges[index].size();
      out_edges[index].resize(outindex + 1);
      out_edges[index][outindex] = std::make_tuple(target, guardindex);
    }
    line += i - 1;
  }

  declared[index] = true;
}

int GameParser::readUntil(const char* line, char delim)
{
  int i = 0;
  while (*(line + i) != delim && *(line + i) != '\0') {
    i++;
  }
  return i;
}

Game::Game(BDD_MANAGER& manager,
  std::vector<BDD> bm_vars, 
  BDD bigC,
  std::vector<int> priority, 
  std::vector<int> owner, 
  std::vector<std::vector<std::tuple<int, int>>> out_edges,
  std::vector<BDD> edge_guards) :
  m_manager(manager),
  bm_vars(bm_vars),
  bigC(bigC),
  m_priority(priority),
  m_owner(owner),
  out_edges(out_edges),
  edge_guards(edge_guards)
{
  // Compute the inverse mapping of priority
  for(std::size_t v = 0; v < m_priority.size(); ++v) {
    int p = m_priority[v];
    if (p + 1 > priorityI.size()) {
      priorityI.resize(p + 1);
    }

    priorityI[p].insert(v);
  }

  in_edges.resize(out_edges.size());
  
  // Compute the input edges.
  for (int i = 0; i < owner.size(); i++) {
    for (const auto& e : out_edges[i]) {
      int t = SMH_TARGET(e);
      int index = in_edges[t].size();
      in_edges[t].resize(index + 1);
      in_edges[t][index] = std::make_tuple(i, edge_index(e));
    }
  }
}

void configurations_explicit_rec(BDD_MANAGER& manager,
  BDD s, 
  std::vector<std::pair<BDD, std::string>>& result, 
  std::vector<char>& buffer,
  int var, 
  int max_var,
  const std::vector<BDD>& vars)
{
  if (BDD_IS_EMPTY(manager, s)) {
    return;
  }

  if (var == max_var) {
    buffer[var] = '\0';
    result.push_back(std::make_pair(s, std::string( buffer.data())));
  }
  else {
    BDD tmp = s;
    tmp &= vars[var];
    buffer[var] = '1';
    configurations_explicit_rec(manager, tmp, result, buffer, var + 1, max_var, vars);

    tmp = s;
    tmp = BDD_MINUS(manager, tmp, vars[var]);
    buffer[var] = '0';
    configurations_explicit_rec(manager, tmp, result, buffer, var + 1, max_var, vars);
  }
}

std::vector<std::pair<BDD, std::string>> Game::configurations_explicit() const
{
  std::vector<std::pair<BDD, std::string>> result;

  std::vector<char> characters(bm_vars.size());
  configurations_explicit_rec(m_manager, configurations(), result, characters, 0, bm_vars.size(), bm_vars);
  return result;
}

std::vector<std::pair<BDD, std::string>> Game::configurations_explicit(BDD set) const
{
  std::vector<std::pair<BDD, std::string>> result;

  std::vector<char> characters(bm_vars.size());
  configurations_explicit_rec(m_manager, set, result, characters, 0, bm_vars.size(), bm_vars);
  return result;
}

void Game::write(std::ostream& output, std::optional<BDD> conf)
{
  output << "parity " << m_owner.size() << ';';
  for (int v = 0; v < m_owner.size(); v++) {
    output << std::endl << v << ' ' << m_priority[v] << ' ' << m_owner[v];
    char separator = ' ';
    for (const auto& e : out_edges[v]) {
      bool is_enabled = true;
      if (conf) {
        BDD tmp = conf.value();
        tmp &= edge_guards[edge_index(e)];

        is_enabled = !BDD_IS_EMPTY(m_manager, tmp);
      } 
      
      if (is_enabled) {                 
        output << separator << SMH_TARGET(e);
        separator = ',';
      }
    }
    output << ';';
  }
}


std::pair<Game, std::vector<int>> Game::compute_reachable() const {

  // The new graph.
  std::vector<std::vector<std::tuple<int, int>>> new_out_edges;
  std::vector<BDD> new_edge_guards;
  std::vector<int> priority;
  std::vector<int> owner;

  // A mapping from old vertex indices to new vertices.
  std::vector<int> mapping(m_owner.size(), -1);
  boost::dynamic_bitset<> visited(m_owner.size());

  auto add_vertex = [&](std::size_t v) {
    if (mapping[v] != -1) {
      return mapping[v];
    }
    
    // Add a new vertex for this one
    int new_v = priority.size();
    priority.push_back(m_priority[v]);
    owner.push_back( m_owner[v]);
    new_out_edges.resize(out_edges.size() + 1);

    // Update mapping.
    mapping[v] = new_v;
    return new_v;
  };

  std::queue<int> Q;
  Q.push(0);
  visited[0] = true;

  while(!Q.empty()) {
    auto v = Q.front();
    Q.pop();

    std::size_t new_v = add_vertex(v);

    // For every outgoing edge, visit the resulting state.
    for (const auto& [w_prime, edge_succ] : successors(v)) {
      std::size_t new_edge = edge_guards.size();
      std::size_t new_w = add_vertex(w_prime);
      new_edge_guards.push_back(edge_guards[edge_succ]);
      new_out_edges[new_v].push_back(std::make_tuple(new_w, new_edge));

      if (!visited[w_prime]) {
        visited[w_prime] = true;
        Q.push(w_prime);
      }
    }
  }

  return std::make_pair(Game(m_manager, bm_vars, bigC, priority, owner, new_out_edges, new_edge_guards), mapping);
}