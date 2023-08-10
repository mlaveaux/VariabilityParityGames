//
// Created by sjef on 19-8-19.
//

#ifndef VPGSOLVER_UNORDEREDVERTEXSET_H
#define VPGSOLVER_UNORDEREDVERTEXSET_H

#include "bool_reference.h"
#include <unordered_set>

using namespace std;

/**
 * Extend the unordered set class with an [] operator such that this[i] == true iff i is in this
 */
class UnorderedVertexSet : public unordered_set<int>
{
public:
  bool_reference operator[](int __n) { return bool_reference(this, __n); }
  UnorderedVertexSet();
  explicit UnorderedVertexSet(int size);
};

#endif // VPGSOLVER_UNORDEREDVERTEXSET_H
