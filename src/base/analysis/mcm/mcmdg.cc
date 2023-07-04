/*
 *  Eindhoven University of Technology
 *  Eindhoven, The Netherlands
 *  Dept. of Electrical Engineering
 *  Electronics Systems Group
 *  Model Based Design Lab (https://computationalmodeling.info/)
 *
 *  Name            :   mcmdg.cc
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   November 8, 2005
 *
 *  Function        :   Compute the MCM for an HSDF graph using Dasdan-Gupta's
 *                      algorithm.
 *
 *  @article{Dasdan1998FasterMA,
 *    title={Faster maximum and minimum mean cycle algorithms for system-performance analysis},
 *    author={Ali Dasdan and Rajesh K. Gupta},
 *    journal={IEEE Trans. Comput. Aided Des. Integr. Circuits Syst.},
 *    year={1998},
 *    volume={17},
 *    pages={889-899}
 * }
 *
 *  History         :
 *      08-11-05    :   Initial version.
 *
 *
 *  Copyright 2023 Eindhoven University of Technology
 *
 *  Permission is hereby granted, free of charge, to any person obtaining
 *  a copy of this software and associated documentation files (the “Software”),
 *  to deal in the Software without restriction, including without limitation
 *  the rights to use, copy, modify, merge, publish, distribute, sublicense,
 *  and/or sell copies of the Software, and to permit persons to whom the
 *  Software is furnished to do so, subject to the following conditions:
 *
 *  The above copyright notice and this permission notice shall be included
 *  in all copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 *  SOFTWARE.
 */

#include "base/analysis/mcm/mcmgraph.h"
#include "base/math/cmath.h"
#include <climits>
#include <cmath>
#include <memory>

namespace Graphs {

// assumes teh graph is strongly connected and assumes the node ids are 0...N-1,
// where N is the number of nodes
CDouble mcmDG_SCC(MCMgraph &mcmGraph) {

    // Allocate memory
    const int n = static_cast<int>(mcmGraph.nrVisibleNodes());
    std::vector<int> level(n);
    std::vector<std::vector<int>> pi(n + 1, std::vector<int>(n));
    std::vector<std::vector<int>> d(n + 1, std::vector<int>(n));

    // Initialize
    for (int i = 0; i < n; i++) {
        level[i] = -1;
    }
    d[0][0] = 0;
    pi[0][0] = -1;
    level[0] = 0;
    std::list<int> Q_k;
    Q_k.push_back(0);
    std::list<MCMnode *> Q_u;
    Q_u.push_back(&(mcmGraph.getNodes().front()));

    // Compute the distances
    int k = Q_k.front();
    Q_k.pop_front();
    MCMnode *u = Q_u.front();
    Q_u.pop_front();
    do {
        for (auto &e : u->out) {
            MCMnode *v = e->dst;

            if (level[v->id] < static_cast<int>(k + 1)) {
                Q_k.push_back(k + 1);
                Q_u.push_back(v);
                pi[k + 1][v->id] = level[v->id];
                level[v->id] = k + 1;
                d[k + 1][v->id] = -INT_MAX;
            }
            d[k + 1][v->id] = MAX(d[k + 1][v->id], d[k][u->id] + ((int)e->w));
        }
        k = Q_k.front();
        Q_k.pop_front();
        u = Q_u.front();
        Q_u.pop_front();
    } while (k < n);

    // Compute lambda using Karp's theorem
    CDouble l = -INT_MAX;
    for (const auto &u : mcmGraph.getNodes()) {

        if (level[u.id] == n) {
            CDouble ld = INT_MAX;
            k = pi[n][u.id];
            while (k > -1) {
                ld = MIN(ld, (CDouble)(d[n][u.id] - d[k][u.id]) / (CDouble)(n - k));
                k = pi[k][u.id];
            }
            l = MAX(l, ld);
        }
    }

    return l;
}

/**
 * mcmDG ()
 * The function computes the maximum cycle mean of an MCM graph using
 * Dasdan-Gupta's algorithm.
 * Note: this algorithm assumes that edge weights are integer valued !
 * @todo
 * check if algorithm can be generalized to float edge weights
 */
CDouble mcmDG(MCMgraph &mcmGraph) {

    MCMgraphs sccs;
    stronglyConnectedMCMgraph(mcmGraph, sccs, false);

    auto mcm = static_cast<CDouble>(-INFINITY);
    for (auto &scc : sccs) {
        scc->relabelNodeIds();
        CDouble cmcm = mcmDG_SCC(*scc);
        if (cmcm > mcm) {
            mcm = cmcm;
        }
    }

    return mcm;
}

} // namespace Graphs