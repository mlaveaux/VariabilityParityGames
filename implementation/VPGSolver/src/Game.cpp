/***********************************************************************************************************************
 * In this project only algorithms and datastructures are implemented properly.
 *
 * The code in this file is not optimized and not up to standards; it is sufficient only for experimental applications
 * but not for any real application.
 **********************************************************************************************************************/
//
// Created by sjef on 5-6-19.
//

#include <algorithm>
#include <chrono>
#include <cstring>
#include <fstream>
#include <iostream>
#include <random>
#include <sstream>
#include <string>
#include <unordered_set>

#include "Game.h"

#define PARSER_LINE_SIZE 16777216

Game::Game(const std::string& filename, const char* specificconf, bool is_parity_game)
  : is_parity_game(is_parity_game)
{
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
  buildInEdges();
}

void configurations_explicit_rec(ConfSet s, 
  std::vector<std::pair<ConfSet, std::string>>& result, 
  std::vector<char>& buffer,
  int var, 
  int max_var,
  const std::vector<ConfSet>& vars)
{
  if (s == emptyset) {
    return;
  }

  if (var == max_var) {
    buffer[var] = '\0';
    result.push_back(std::make_pair(s, std::string( buffer.data())));
  }
  else {
    ConfSet tmp = s;
    tmp &= vars[var];
    buffer[var] = '1';
    configurations_explicit_rec(tmp, result, buffer, var + 1, max_var, vars);

    tmp = s;
    tmp -= vars[var];
    buffer[var] = '0';
    configurations_explicit_rec(tmp, result, buffer, var + 1, max_var, vars);
  }
}

std::vector<std::pair<ConfSet, std::string>> Game::configurations_explicit() const
{
  std::vector<std::pair<ConfSet, std::string>> result;

  std::vector<char> characters(bm_n_vars);
  configurations_explicit_rec(configurations(), result, characters, 0, bm_n_vars, bm_vars);
  return result;
}

void Game::parseConfs(char const* line)
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

  bdd_init(2000000, 2);
  bdd_setvarnum(bm_n_vars);
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
    bm_vars[order[i]] = bdd_ithvar(i);
  }

  parseConfSet(line, 6, bigC);
}

void Game::parseInitialiser(char* line)
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
  in_edges = std::vector<std::vector<std::tuple<int, int>>>(n_nodes);
  m_priority.resize(n_nodes);
  m_owner.resize(n_nodes);
  declared.resize(n_nodes);
}

int Game::parseConfSet(const char* line, int i, ConfSet& result)
{
  if (is_parity_game) {
    result = fullset;
    return i + 1;
  } else {
    bool inverse = false;
    if (line[i] == '!') {
      inverse = true;
      i++;
    }

    result = emptyset;
    ConfSet entry = fullset;
    int var = 0;
    char c;
    do {
      c = line[i++];
      if (c == 'F') {
        if (var != 0) {
          throw std::string("Unexpected F");
        }
        entry = emptyset;
      }
      else if (c == '0') {
        if (var > bm_n_vars) {
          throw std::string("Too many bits");
        }
        entry -= bm_vars[var];
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
        entry = fullset;
        var = 0;
      }
    }
    while (c == '0' || c == '1' || c == '-' || c == '+' || c == 'F');

    // Apply the last result.
    result |= entry;

    if (inverse) {
      // Compute the complement set.
      ConfSet a = result;
      result = fullset;
      result -= a;
    }

    return i;
  }
}

void Game::parseVertex(char* line)
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
  if (p + 1 > priorityI.size()) {
    priorityI.resize(p + 1);
  }

  priorityI[p].insert(index);
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
    if (!(edge_guards[guardindex] == emptyset)) {
      int outindex = out_edges[index].size();
      out_edges[index].resize(outindex + 1);
      out_edges[index][outindex] = std::make_tuple(target, guardindex);
    }
    line += i - 1;
  }

  declared[index] = true;
}

int Game::readUntil(const char* line, char delim)
{
  int i = 0;
  while (*(line + i) != delim && *(line + i) != '\0') {
    i++;
  }
  return i;
}

void Game::write(std::ostream& output, std::optional<ConfSet> conf)
{
  output << "parity " << n_nodes << ';';
  for (int v = 0; v < n_nodes; v++) {
    output << std::endl << v << ' ' << m_priority[v] << ' ' << m_owner[v];
    char separator = ' ';
    for (const auto& e : out_edges[v]) {
      bool is_enabled = true;
      if (conf) {
        ConfSet tmp = conf.value();
        tmp &= edge_guards[edge_index(e)];

        is_enabled = (tmp != emptyset);
      } 
      
      if (is_enabled) {                 
        output << separator << target(e);
        separator = ',';
      }
    }
    output << ';';
  }
}

void Game::buildInEdges()
{
  for (int i = 0; i < n_nodes; i++) {
    for (const auto& e : out_edges[i]) {
      int t = target(e);
      int index = in_edges[t].size();
      in_edges[t].resize(index + 1);
      in_edges[t][index] = std::make_tuple(i, edge_index(e));
    }
  }
}