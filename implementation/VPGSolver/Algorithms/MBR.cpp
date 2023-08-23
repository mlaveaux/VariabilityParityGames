//
// Created by sjef on 11-7-19.
//

#include "MBR.h"
#include <iostream>
#include <fstream>
#include <algorithm>
#include <queue>
#include <chrono>
#include "FPIte.h"

vector<ConfSet> MBR::winningConf;
vector<VertexSetFPIte> MBR::winningVertices;
bool MBR::metric_output;
string MBR::metric_dir;
bool MBR::fulloutput = false;

MBR::MBR(Game *game, ConfSet *conf,  VertexSetFPIte *P0, VertexSetFPIte *VP1, int feature) {
    this->game = game;
    this->conf = conf;
    this->P0 = P0;
    this->VP1 = VP1;
    this->feature = feature;
}

MBR::MBR(Game *game) {
    this->game = game;
    this->conf = new ConfSet;
    *this->conf = game->bigC;

    this->P0 =  new VertexSetFPIte();
    this->P0->resize(game->n_nodes);
    this->VP1 =  new VertexSetFPIte();
    this->VP1->resize(game->n_nodes);
    fill(this->VP1->begin(), this->VP1->end(), true);
    this->feature =  0;
}

void MBR::solve() {
    if(metric_output){
        // maintain the binary tree representing the recursion
        if(this->measured == nullptr)
            this->measured = new bintree<vector<long>>();
        this->measured->value = new vector<long>(5);
        (*this->measured->value)[4] = confstring;

#ifdef subsetbdd
        (*this->measured->value)[0] = std::count_if(P0->begin(), P0->end(), [](bool b){return b;});
        (*this->measured->value)[1] = game->n_nodes - std::count_if(VP1->begin(), VP1->end(), [](bool b){return b;});
#endif
    }
    if(this->feature == game->bm_n_vars){
        // There is only a single configuration left. The VPG only contains edges admitting this configurations, so it essentially is a parity game which we now solve.
        int i = winningConf.size();
        winningConf.resize(i+1);
        winningVertices.resize(i+1);
        winningConf[i] = *conf;
        winningVertices[i].resize(game->n_nodes);
        FPIte fpite(game, P0, VP1, &(winningVertices[i]));
        fpite.solvelocal = solvelocal;

        if(metric_output)
        {
            auto start = std::chrono::high_resolution_clock::now();
            fpite.solve();
            auto end = std::chrono::high_resolution_clock::now();
            (*this->measured->value)[2] = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
            cout << "Assisted leaf " << (*this->measured->value)[0] + (*this->measured->value)[1] << " with time " << (*this->measured->value)[2] << endl;
        } else {
            fpite.solve();
        }
        if(!MBR::fulloutput){
            bool v0 = winningVertices[i][game->reindexedNew[game->findVertexWinningFor0()]];
            winningVertices[i].resize(1);
            winningVertices[i][0] = v0;
        }
        return;
    }


    // edge relations for the subgame and pessimistic games we are creating
    auto * subgame_out0 = new vector<std::tuple<int, int>>[game->n_nodes];
    auto * subgame_in0  = new vector<std::tuple<int, int>>[game->n_nodes];
    auto * subgame_out1 = new vector<std::tuple<int, int>>[game->n_nodes];
    auto * subgame_in1  = new vector<std::tuple<int, int>>[game->n_nodes];
    // Use P0 and VP1 for one branch and initialize P0b and VP1b for the other branch
    auto * P0b = new VertexSetFPIte;
    auto * VP1b = new VertexSetFPIte;
    // initialize with the current edge relationship
    copyEdges(subgame_out0, subgame_in0);
    copyEdges(subgame_out1, subgame_in1);
    vector<std::tuple<int, int>> * orgin, * orgout;
    orgin = game->in_edges;
    orgout = game->out_edges;

    // create pessimistic games
    createPessimisticGames(subgame_out0, subgame_in0, subgame_out1, subgame_in1);

    // Set game to the pessimistic game 0
    VertexSetFPIte W0(game->n_nodes);
    game->in_edges = subgame_in0;
    game->out_edges = subgame_out0;


    // solve the pessimistic game 0
    auto *fpite0 = new FPIte(game, P0, VP1, &W0);
    if (metric_output) {
        auto start = std::chrono::high_resolution_clock::now();
        fpite0->solve();
        auto end = std::chrono::high_resolution_clock::now();
        (*this->measured->value)[2] = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
//            if(std::count_if(W0.begin(), W0.end(), [](bool b){return b;}) - std::count_if(P0->begin(), P0->end(), [](bool b){return b;}) < 3000)
//        if(this->feature != 0)
//            (*this->measured->value)[2] = 0;
            if(metric_dir.length() > 0){
                ofstream f;
                f.open(metric_dir + '/' + to_string(confstring) + "_0.pg");
                game->writePG(&f);
                f.close();
            }
        cout << "Assisted node " << (*this->measured->value)[0] + (*this->measured->value)[1] << " with time " << (*this->measured->value)[2] << endl;
    } else {
        fpite0->solve();
    }
    // Vertices won by player 0 in this game are won by player 0 for all configurations. Therefore we also now these are never won by player 1 in pessimistic game 1
    *P0 = W0;

    // set game to pessimistic game 1
    game->in_edges = subgame_in1;
    game->out_edges = subgame_out1;
    // solve pessimistic game 1
    auto *fpite1 = new FPIte(game, P0, VP1, &W0);
    if (metric_output) {
        auto start = std::chrono::high_resolution_clock::now();
        fpite1->solve();
        auto end = std::chrono::high_resolution_clock::now();
        (*this->measured->value)[3] = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
//            if(std::count_if(VP1->begin(), VP1->end(), [](bool b){return b;}) - std::count_if(W0.begin(), W0.end(), [](bool b){return b;}) < 3000)
//        if(this->feature != 0)
//            (*this->measured->value)[3] = 0;
        if(metric_dir.length() > 0) {
            ofstream f;
            f.open(metric_dir + '/' + to_string(confstring) + "_1.pg");
            game->writePG(&f);
            f.close();
        }
        cout << "Assisted node " << (*this->measured->value)[0] + (*this->measured->value)[1] << " with time " << (*this->measured->value)[3] << endl;
    } else {
        fpite1->solve();
    }
    // Vertices won by player 1 in this game are won by player 1 for all configurations.
    *VP1 = W0;

    delete fpite0;
    delete fpite1;

    game->in_edges = orgin;
    game->out_edges = orgout;
    delete[] subgame_in1;
    delete[] subgame_out1;

    bool done;
    if(this->solvelocal){
        //Local, we can terminate if vertex 0 is found in P0 or not found in VP1
        done = (*P0)[game->reindexedNew[game->findVertexWinningFor0()]] || !(*VP1)[game->reindexedNew[game->findVertexWinningFor0()]];
    } else {
        //Global, we can terminate if all vertices are pre-solved.
        done = *P0 == *VP1;
    }
    if (done) {
        int i = winningConf.size();
        winningConf.resize(i + 1);
        winningVertices.resize(i + 1);
        winningConf[i] = *conf;
        winningVertices[i] = *P0;

        if(!MBR::fulloutput){
            bool v0 = winningVertices[i][game->reindexedNew[game->findVertexWinningFor0()]];
            winningVertices[i].resize(1);
            winningVertices[i][0] = v0;
        }
        return;
    }

    int confastring = confstring;
    int confbstring = confstring;
    auto * confa = new ConfSet;
    auto * confb = new ConfSet;
    // Keep splitting conf until we find two non-empty sets.
    do {
        *confa = *conf;
        parition(confa, confb);
        // maintain the string representation
        confastring *= 10;
        confastring += 1;
        confbstring *= 10;
        this->feature++;
        if(this->feature >  game->bm_n_vars)
            throw "Empty conf sets found in partitioning";
    } while (*confa == emptyset || *confb == emptyset);

    copyEdges(subgame_out0, subgame_in0);
    //create subgames for the configuration partition
    createSubGames(confa, subgame_out0, subgame_in0);
    createSubGames(confb, orgout, orgin);

    // Recursively solve the subgames
    *P0b = *P0;
    *VP1b = *VP1;
    MBR ma(game, confa, P0, VP1, feature);
    MBR mb(game, confb, P0b, VP1b, feature);
    if(metric_output){
        this->measured->left = new bintree<vector<long>>();
        this->measured->right = new bintree<vector<long>>();
        ma.measured = this->measured->left;
        mb.measured = this->measured->right;
        ma.confstring = confastring;
        mb.confstring = confbstring;
    }
    ma.solvelocal = solvelocal;
    mb.solvelocal = solvelocal;
    game->out_edges = subgame_out0;
    game->in_edges = subgame_in0;
    ma.solve();
    delete[] subgame_out0;
    delete[] subgame_in0;
    game->out_edges = orgout;
    game->in_edges = orgin;
    mb.solve();

    delete P0b;
    delete VP1b;
    delete confa;
    delete confb;
}


void MBR::parition(ConfSet *org, ConfSet *part) {
    *part = *org;
    *org &= game->bm_vars[feature];
    *part -= game->bm_vars[feature];
}

void MBR::createPessimisticGames(vector<std::tuple<int, int>> *pessimistic_out0,
                                 vector<std::tuple<int, int>> *pessimistic_in0,
                                 vector<std::tuple<int, int>> *pessimistic_out1,
                                 vector<std::tuple<int, int>> *pessimistic_in1) {
    // First decide which edges should be enabled and which should be disabled. We calculate this simultaneously
    vector<bool> edgeenabled0(game->edge_guards.size()),edgeenabled1(game->edge_guards.size()), *pess, *opt;
    for(int v = 0;v<game->n_nodes;v++){
        for(const auto & e : game->out_edges[v]) {
            if (game->owner[v] == 0) {
                pess = &edgeenabled0;
                opt = &edgeenabled1;
            } else {
                pess = &edgeenabled1;
                opt = &edgeenabled0;
            }
            // the edge is always in for the optimistic player
            int gi = edge_index(e);
            (*opt)[gi] = true;

            // the edge is only in for the pessimistic player if all configurations are in
            ConfSet g = *conf;
            g -= game->edge_guards[gi];
            (*pess)[gi] = g == emptyset;
        }
    }
    // Create new edge relations
    for(int v = 0;v<game->n_nodes;v++) {
        removeDisabledEdge(pessimistic_out0 + v, &edgeenabled0);
        removeDisabledEdge(pessimistic_in0 + v, &edgeenabled0);
        removeDisabledEdge(pessimistic_out1 + v, &edgeenabled1);
        removeDisabledEdge(pessimistic_in1 + v, &edgeenabled1);
    }
}

void MBR::removeDisabledEdge(vector<std::tuple<int,int>> * edge, vector<bool> * edgeenabled){
    auto it = edge->begin();
    while(it != edge->end()){
        int guard_index = edge_index(*it);
        if(!(*edgeenabled)[guard_index]){
            it = edge->erase(it);
        } else {
            ++it;
        }
    }
}

void MBR::createSubGames(ConfSet *confP, vector<std::tuple<int, int>> *subgame_out,
                         vector<std::tuple<int, int>> *subgame_in) {
    // First decide which edges are in and which are out
    vector<bool> edgeenabled(game->edge_guards.size());
    for(int v = 0;v<game->n_nodes;v++) {
        for (const auto & e : game->out_edges[v]) {
            ConfSet g = *confP;
            int gi = edge_index(e);
            g &= game->edge_guards[gi];
            // When something is left the edge is kept
            edgeenabled[gi] = !(g == emptyset);
        }
    }
    // create new edge relations
    for(int v = 0;v<game->n_nodes;v++) {
        removeDisabledEdge(subgame_out+v, &edgeenabled);
        removeDisabledEdge(subgame_in+v, &edgeenabled);
    }
}

void MBR::copyEdges(vector<std::tuple<int, int>> *edgeout, vector<std::tuple<int, int>> *edgein) {
    copy(game->in_edges, game->in_edges + game->n_nodes, edgein);
    copy(game->out_edges, game->out_edges + game->n_nodes, edgeout);
}


void MBR::printMeasurements(ostream * output) {
    *output << "digraph timetree {" << endl;
    int nodes = printNode(output, this->measured,0);
    *output << '}' << endl;
    cout << "nr of nodes: " << nodes << endl;
    cout << "Solved " << ((nodes+1)*3/2-2) << " games" << endl;
}

int MBR::printNode(ostream *output, bintree<vector<long>> *node, int c) {
    if(node == nullptr)
        return -1;
    *output << 'n' << c++ << " [label=\"";
    *output << "V0: " << (*node->value)[0] << '/' << this->game->n_nodes << endl;
    *output << "V1: " << (*node->value)[1] << '/' << this->game->n_nodes << endl;
    *output << "Solve 1: " << (*node->value)[2] << "ns" << endl;
    *output << "Solve 2: " << (*node->value)[3] << "ns" << endl;
    *output << "Conf: " << (*node->value)[4];
    *output << "\"];" << endl;
    int l = printNode(output, node->left, c);
    if(l == -1) {
        int r = printNode(output, node->right, c);
        if(r != -1){
            *output << 'n' << (c-1) << " -> n" << c << ';' << endl;
            return r;
        }
        return c;
    } else {
        int r = printNode(output, node->right, l);
        *output << 'n' << (c-1) << " -> n" << c << ';' << endl;
        if(r != -1){
            *output << 'n' << (c-1) << " -> n" << l << ';' << endl;
            return r;
        }
        return l;
    }
}
