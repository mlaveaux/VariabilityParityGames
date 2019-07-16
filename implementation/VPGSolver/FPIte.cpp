//
// Created by sjef on 11-7-19.
//

#include "FPIte.h"
#include <algorithm>
#include <iostream>

#define targetIsIn(t) ZZ[game->reindexedNew[t]]
FPIte::FPIte(Game *game, VertexSet *P0, VertexSet *VP1, vector<bool> *edgeenabled, VertexSet * W0) {
    this->game = game;
    this->P0 = P0;
    this->VP1 = VP1;
    this->edgeenabled = edgeenabled;
    this->W0 = W0;
}

FPIte::FPIte(Game *game) {
    this->game = game;
    this->P0 =  new VertexSet();
    this->P0->resize(game->n_nodes);
    this->VP1 =  new VertexSet();
    this->VP1->resize(game->n_nodes);
    fill(this->VP1->begin(), this->VP1->end(), true);
    this->edgeenabled = new vector<bool>(game->edge_guards.size());
    fill(this->edgeenabled->begin(), this->edgeenabled->end(), true);
    this->W0 = new VertexSet();
    this->W0->resize(game->n_nodes);
}

void FPIte::init(int i, int ie) {
    if(i >= d || ie >= d || i < 0 || ie < 0)
        return;
    if(i % 2 == 0)
        copyWithPrio(&ZZ, VP1, i, ie);
    else
        copyWithPrio(&ZZ, P0, i, ie);
}

void FPIte::diamondbox(VertexSet *Z, int maxprio) {
    for(int p = 0;p <= maxprio;p++) {
        for(auto & v : game->priorityI[p]){
            if(targetIsIn(v) != targetWasIn[v]){
                for (auto &edge : game->in_edges[v]) {
                    if (!(*edgeenabled)[guard_index(edge)])
                        continue;
                    int t = target(edge);
                    // Vertices in P0 are always in and vertices in P1 are always out, so we don't need to reconsider them
                    if ((*P0)[game->reindexedNew[t]] || !(*VP1)[game->reindexedNew[t]])
                        continue;

                    if (game->owner[t] == 0) {
                        if (targetIsIn(v))
                            edgecount[t]++;
                        else
                            edgecount[t]--;
                        (*Z)[game->reindexedNew[t]] = edgecount[t] > 0;
                    } else {
                        if (targetIsIn(v))
                            edgecount[t]--;
                        else
                            edgecount[t]++;
                        (*Z)[game->reindexedNew[t]] = edgecount[t] == 0;
                    }
                }
                targetWasIn[v] = targetIsIn(v);
            }
        }
    }
}

void FPIte::diamondbox(VertexSet *Z) {
    edgecount.resize(game->n_nodes);
    targetWasIn.resize(game->n_nodes);
    for(int i = 0;i<game->n_nodes;i++){
        bool in;
//        if((*P0)[game->reindexedNew[i]]) {
//            in = true;
//            targetWasIn[i] = true;
//        } else if(!(*VP1)[game->reindexedNew[i]])
//            in = false;
//        else {
        targetWasIn[i] = targetIsIn(i);
        if (game->owner[i] == 0) {
            in = false;
            for (int j = 0; j < game->out_edges[i].size(); j++) {
                auto edge = game->out_edges[i][j];
                if (!(*edgeenabled)[guard_index(edge)])
                    continue;
                int t = target(edge);
                if (targetIsIn(t)) {
                    in = true;
                    edgecount[i]++;
                }
            }
        } else {
            in = true;
            for (int j = 0; j < game->out_edges[i].size(); j++) {
                auto edge = game->out_edges[i][j];
                if (!(*edgeenabled)[guard_index(edge)])
                    continue;
                int t = target(edge);
                if (!targetIsIn(t)) {
                    in = false;
                    edgecount[i]++;
                }
            }
        }
//        }
        (*Z)[game->reindexedNew[i]] = in;
    }
}

void FPIte::solve() {
    d = game->priorityI.size();
    ZZ.resize(game->n_nodes);

    if((d-1) % 2 == 0){
        init(0,d - 1);
        init(1,d - 2);
    } else {
        init(1,d - 1);
        init(0,d - 2);
    }

    int i;
    bool equal;
    bool first = true;
    do{
        if(first) {
            diamondbox(W0);
            first = false;
        } else {
            diamondbox(W0, i - 1);
        }
        i = 1;
        equal = false;
        if(!compareWithPrio(&ZZ, W0,0))
            copyWithPrio(&ZZ,W0,0);
        else {
            do {
                equal = compareWithPrio(&ZZ, W0, i);
                copyWithPrio(&ZZ, W0, i);
                i++;
            } while (equal && i < d);
            if(i % 2 == 0){
                init(0,i-2);
            } else {
                init(1,i-2);
            }
        }
    }while(!(i >= d && equal));
//    int i;
//    do {
//        copyWithPrio(&ZZa[0],&ZZ[0],0);
//        diamondbox(W0);
//        copyWithPrio(&ZZ[0],W0,0);
//        i = 0;
//        while(compareWithPrio(&ZZ[i], &ZZa[i],i) && i < d -1){
//            i++;
//            copyWithPrio(&ZZa[i],&ZZ[i],i);
//            copyWithPrio(&ZZ[i], W0,i);
//            init(i-1);
//        }
//    } while (!(i == d-1 && compareWithPrio(&ZZ[d-1],&ZZa[d-1],d-1)));
//    do {
//        ZZa[0] = ZZ[0];
//        diamondbox(W0);
//        ZZ[0] = *W0;
//        i = 0;
//        while(ZZ[i] == ZZa[i] && i < d - 1){
//            i++;
//            ZZa[i] = ZZ[i];
//            ZZ[i] = ZZ[i-1];
//            init(i-1);
//        }
//    } while (!(i == d-1 && ZZ[d-1] == ZZa[d-1]));
}

FPIte::~FPIte() {
}

void FPIte::copyWithPrio(VertexSet *Z, VertexSet *ZP, int p) {
//    for(int v : game->priorityI[p]) {
//        v = game->reindexedNew[v];
//        (*Z)[v] = (*ZP)[v];
//    }
    int start = game->reindexPCutoff[p];
    copy_n(ZP->begin() + start,
                    game->reindexPCutoff[p+2] - start,
                    Z->begin() + start);
}

bool FPIte::copyAndCompareWithPrio(VertexSet *Z, VertexSet *ZP, int p) {
    bool equal = true;
    for(int v : game->priorityI[p]) {
        v = game->reindexedNew[v];
        if((*Z)[v] != (*ZP)[v]) {
            equal = false;
            (*Z)[v] = (*ZP)[v];
        }
    }
    return equal;
}

bool FPIte::compareWithPrio(VertexSet *Z, VertexSet *ZP, int p) {
//    bool equal = true;
//    for(int v : game->priorityI[p]) {
//        v = game->reindexedNew[v];
//        if ((*Z)[v] != (*ZP)[v])
//            equal = false;
//    }
//    return equal;
    int start = game->reindexPCutoff[p];
    return equal(ZP->begin() + start,
           ZP->begin() + game->reindexPCutoff[p+2],
           Z->begin() + start);
}

void FPIte::copyWithPrio(VertexSet *Z, VertexSet *ZP, int sp, int ep) {
    int start = game->reindexPCutoff[sp];
    copy_n(ZP->begin() + start,
           game->reindexPCutoff[ep+2] - start,
           Z->begin() + start);
}
