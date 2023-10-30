//
// Created by sjef on 11-6-19.
//

#ifndef VPGSOLVER_CONF_H
#define VPGSOLVER_CONF_H

/**
 * Define subsetbdd to indicate the sets of configurations ought to be expressed as bdd
 * Define subsetexplicit to indicate the sets of configurations ought to be expressed as bdd
 * Define VertexSetZlnkIsBitVector to represent sets of vertices used by the recursive algorithm as bit vectors
 * Define VertexSetZlnkIsHashSet to represent sets of vertices used by the recursive algorithm as unordered sets
 *
 * Define VertexSetFPIte to be the type used to represent sets of vertices in the fixed-point iteration algorithm
 */

#include "boost/dynamic_bitset.hpp"

#include "bdd.h"
#define ConfSet bdd
#define fullset bddtrue
#define emptyset bddfalse

#endif // VPGSOLVER_CONF_H
