/*
 *  Eindhoven University of Technology
 *  Eindhoven, The Netherlands
 *  Dept. of Electrical Engineering
 *  Electronics Systems Group
 *  Model Based Design Lab (https://computationalmodeling.info/)
 *
 *  Name            :   mcmhoward.cc
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   November 8, 2005
 *
 *  Function        :   Compute the MCM for an HSDF graph using Howard's
 *                      algorithm implemented in Max-Plus algebra.
 *
 *  Acknowledgement :   This code is based on 'Howard Policy Iteration Algorithm
 *                      for Max Plus Matrices' written by Stephane Gaubert
 *                      (Stephane.Gaubert@inria.fr). The max-plus version of
 *                      Howard's algorithm is described in the paper:
 *                      'Numerical computation of spectral elements in max-plus
 *                      algebra'. Jean Cochet-Terrasson, Guy Cohen, Stephane
 *                      Gaubert, Michael Mc Gettrick, Jean-Pierre Quadrat
 *                      IFAC Workshop on System Structure and Control,
 *                      Nantes, July 1997.
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
#include "base/exception/exception.h"

#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <memory>

namespace Graphs {
/*
 * Howard terminates with an error if MAX_NIterations occur.
 */
constexpr unsigned int MAX_NIterations = 1000;
constexpr CDouble EPSILON_FACTOR = 0.000000001;

#define EPSILON (-HUGE_VAL)

class AlgHoward {

public:
    AlgHoward(const std::vector<int> &ij,
              const std::vector<CDouble> &A,
              int nr_nodes,
              int nr_arcs,
              std::shared_ptr<std::vector<CDouble>> *chi,
              std::shared_ptr<std::vector<CDouble>> *v,
              std::shared_ptr<std::vector<int>> *policy,
              int *nr_iterations,
              int *nr_components) :
        ij(ij),
        a(A),
        nr_nodes(nr_nodes),
        narcs(nr_arcs),
        chi(chi),
        v(v),
        pi(policy),
        NIterations(nr_iterations),
        NComponents(nr_components) {}

    void Run() {

        bool improved = false;
        *NIterations = 0;

        *pi = std::make_shared<std::vector<int>>(nr_nodes);
        *chi = std::make_shared<std::vector<CDouble>>(nr_nodes);
        *v = std::make_shared<std::vector<CDouble>>(nr_nodes);

        Safety_Check();
        Allocate_Memory();
        Epsilon(&epsilon);
        Initial_Policy();
        New_Build_Inverse();

        do {
            Value();
            Improve(&improved);
            Update_Policy();
            New_Build_Inverse();
            (*NIterations)++;
        } while (improved && *NIterations < MAX_NIterations);

        End_Message();
    }

private:
    const std::vector<int> &ij;
    const std::vector<CDouble> &a;
    int nr_nodes;
    int narcs;
    std::shared_ptr<std::vector<CDouble>> *chi;
    std::shared_ptr<std::vector<CDouble>> *v;
    std::shared_ptr<std::vector<int>> *pi;
    int *NIterations;
    int *NComponents;

    std::shared_ptr<std::vector<int>> new_pi =
            std::make_shared<std::vector<int>>(); /*  new policy */
    /* The inverse policy is coded by a linearly chained list.
     * pi_inv_idx[i]= pointer to the chain of inverses of node i.
     */
    std::vector<int> pi_inv_idx;

    /* pi_inv_idx[j]= pointer to the next inverse */
    std::vector<int> pi_inv_succ;

    /* corresponding node  */
    std::vector<int> pi_inv_elem;

    /* pi_inv_last[i]= last inverse of i */
    std::vector<int> pi_inv_last;

    std::vector<CDouble> c;
    std::vector<CDouble> v_aux;
    std::vector<CDouble> new_c;
    std::vector<CDouble> new_chi;
    std::vector<int> visited;
    std::vector<int> component;
    CDouble lambda = 0;
    CDouble epsilon = 0;
    int color = 1;

    /**
     * Epsilon ()
     * The termination tests are performed up to an epsilon constant, which is fixed
     * heuristically by the following routine.
     */
    void Epsilon(CDouble *epsilon) {
        CDouble MAX = a[0];
        CDouble MIN = a[0];

        for (int i = 1; i < narcs; i++) {
            if (a[i] > MAX) {
                MAX = a[i];
            }
            if (a[i] < MIN) {
                MIN = a[i];
            }
        }
        *epsilon = (MAX - MIN) * EPSILON_FACTOR;
    }

    /**
     * Initial_Policy
     * Build an admissible policy pi and its associated cost vector c from ij and A.
     * Reasonable greedy rule to determine the first policy. pi(node i) = arc with
     * maximal weight starting from i for full random matrices, this choice of
     * initial policy seems to cut the number of iterations by a fv_aux\[(.*?)\]actor 1.5, by
     * comparison with a random initial policy.
     */
    void Initial_Policy() {
        /* we loose a O(nr_nodes) time here ... */
        /* we use the auxiliary variable v_aux to compute the row max of A */
        for (int i = 0; i < nr_nodes; i++) {
            v_aux[i] = EPSILON;
        }

        for (int i = 0; i < narcs; i++) {
            if (v_aux[ij[i * 2]] <= a[i]) {
                (**pi)[ij[i * 2]] = ij[i * 2 + 1];
                c[ij[i * 2]] = a[i];
                v_aux[ij[i * 2]] = a[i];
            }
        }
    }

    void New_Build_Inverse() {
        int ptr = 0;

        for (int i = 0; i < nr_nodes; i++) {
            pi_inv_idx[i] = -1;
            pi_inv_last[i] = -1;
        }

        for (int i = 0; i < nr_nodes; i++) {
            int j = (**pi)[i];
            if (pi_inv_idx[j] == -1) {
                pi_inv_succ[ptr] = -1;
                pi_inv_elem[ptr] = i;
                pi_inv_last[j] = ptr;
                pi_inv_idx[j] = ptr;
                ptr++;
            } else {
                pi_inv_succ[ptr] = -1;
                pi_inv_elem[ptr] = i;
                int locus = pi_inv_last[j];
                pi_inv_succ[locus] = ptr;
                pi_inv_last[j] = ptr;
                ptr++;
            };
        }
    }

    void Init_Depth_First() {
        for (int j = 0; j < nr_nodes; j++) {
            visited[j] = 0;
            component[j] = 0;
        }
    }

    /**
     *
     * Given the value of v at initial point i, we compute v[j] for all predecessor
     * j of i, according to the spectral equation, v[j]+ lambda = A(arc from j to i)
     * v[i] the array visited is changed by side effect.
     */
    void New_Depth_First_Label(int i) {
        int a = pi_inv_idx[i];
        while (a != -1 && visited[pi_inv_elem[a]] == 0) {
            int next_i = pi_inv_elem[a];
            visited[next_i] = 1;
            (**v)[next_i] = -lambda + c[next_i] + (**v)[i];
            component[next_i] = color;
            (**chi)[next_i] = lambda;
            New_Depth_First_Label(next_i);
            a = pi_inv_succ[a];
        }
    }

    void Visit_From(const int initial_point, const int color) {
        auto pir = **pi;

        int index = initial_point;
        component[index] = color;
        int newindex = pir[index];

        while (component[newindex] == 0) {
            component[newindex] = color;
            index = newindex;
            newindex = pir[index];
        }

        /* a cycle has been detected, since newindex is already visited */
        CDouble weight = 0;
        int length = 0;
        int i = index;
        do {
            weight += c[i];
            length++;
            i = pir[i];
        } while (i != index);

        lambda = weight / length;
        (**v)[i] = v_aux[i]; /* keeping the previous value */
        (**chi)[i] = lambda;
        New_Depth_First_Label(index);
    }

    /**
     * Value()
     * Computes the value (v,chi) associated with a policy pi.
     */
    void Value() {
        color = 1;

        Init_Depth_First();
        int initial_point = 0;

        do {
            Visit_From(initial_point, color);
            while ((initial_point < nr_nodes) && (component[initial_point] != 0)) {
                initial_point++;
            }
            color++;
        } while (initial_point < nr_nodes);

        *NComponents = --color;
    }

    void Init_Improve() {
        for (int i = 0; i < nr_nodes; i++) {
            new_chi[i] = (**chi)[i];
            v_aux[i] = (**v)[i];
            (*new_pi)[i] = (**pi)[i];
            new_c[i] = c[i];
        }
    }

    void First_Order_Improvement(bool *improved) {
        for (int i = 0; i < narcs; i++) {
            if ((**chi)[ij[i * 2 + 1]] > new_chi[ij[i * 2]]) {
                *improved = true;
                (*new_pi)[ij[i * 2]] = ij[i * 2 + 1];
                new_chi[ij[i * 2]] = (**chi)[ij[i * 2 + 1]];
                new_c[ij[i * 2]] = a[i];
            }
        }
    }

    void Second_Order_Improvement(bool *improved) {
        auto chir = **chi;
        auto vr = **v;
        if (*NComponents > 1) {
            for (int i = 0; i < narcs; i++) {
                /* arc i is critical */
                if (chir[ij[i * 2 + 1]] == new_chi[ij[i * 2]]) {
                    CDouble w = a[i] + vr[ij[i * 2 + 1]] - chir[ij[i * 2 + 1]];
                    if (w > v_aux[ij[i * 2]] + epsilon) {
                        *improved = true;
                        v_aux[ij[i * 2]] = w;
                        (*new_pi)[ij[i * 2]] = ij[i * 2 + 1];
                        new_c[ij[i * 2]] = a[i];
                    }
                }
            }
        } else {
            /* we know that all the arcs realize the max in the
            first order improvement */
            for (int i = 0; i < narcs; i++) {
                CDouble w = a[i] + vr[ij[i * 2 + 1]] - chir[ij[i * 2 + 1]];
                if (w > v_aux[ij[i * 2]] + epsilon) {
                    *improved = true;
                    v_aux[ij[i * 2]] = w;
                    (*new_pi)[ij[i * 2]] = ij[i * 2 + 1];
                    new_c[ij[i * 2]] = a[i];
                }
            }
        }
    }

    void Improve(bool *improved) {
        *improved = false;
        Init_Improve();

        /* a first order policy improvement may occur */
        if (*NComponents > 1) {
            First_Order_Improvement(improved);
        }

        if (!*improved) {
            Second_Order_Improvement(improved);
        }
    }

    void Allocate_Memory() {
        new_pi->resize(nr_nodes);
        pi_inv_idx.resize(nr_nodes);
        pi_inv_succ.resize(nr_nodes);
        pi_inv_elem.resize(nr_nodes);
        pi_inv_last.resize(nr_nodes);
        visited.resize(nr_nodes);
        component.resize(nr_nodes);
        c.resize(nr_nodes);
        new_c.resize(nr_nodes);
        v_aux.resize(nr_nodes);
        new_chi.resize(nr_nodes);
    }

    void Check_Rows() {
        // check if every node has outgoing edges.
        std::vector<int> u(nr_nodes);

        for (int i = 0; i < narcs; i++) {
            // ij[2*k] is the source node number of edge number k
            u[ij[2 * i]] = 1;
        }

        for (int i = 0; i < nr_nodes; i++) {
            if (u[i] == 0) {
                throw CException("Failed check on rows in Howard's MCM algorithm.");
            }
        }
    }

    void Safety_Check() {
        if (nr_nodes < 1) {
            throw CException("Howard: number of nodes must be a positive integer.");
        }

        if (narcs < 1) {
            throw CException("Howard: number of arcs must be a positive integer.");
        }

        Check_Rows();
    }

    void Update_Policy() {
        for (int i = 0; i < nr_nodes; i++) {
            (**pi)[i] = (*new_pi)[i];
            c[i] = new_c[i];
            v_aux[i] = (**v)[i]; /* Keep a copy of the current value function */
        }
    }

    void End_Message() {
        if (*NIterations == MAX_NIterations) {
            throw CException("Howard: exceeded maximum number of iterations.");
        }
    }
};

/**
 * Howard ()
 * Howard Policy Iteration Algorithm for Max Plus Matrices.
 *
 * INPUT of Howard Algorithm:
 *      ij,A,nr_nodes,narcs : sparse description of a matrix.
 *
 * OUTPUT:
 *      chi cycle time vector
 *      v bias
 *      pi optimal policy
 *      NIterations: Number of iterations of the algorithm
 *      NComponents: Number of connected components of the optimal policy
 *
 * REQUIRES: O(nr_nodes) SPACE
 * One iteration requires: O(narcs+nr_nodes) TIME
 *
 * The following variables should be defined in the environment from which the
 * Howard routine is called.
 *
 * INPUT VARIABLES
 * int nr_nodes;  number of nodes of the graph
 * int nr_arcs;   number of arcs of the graph
 * int *ij;       array of integers of size 2*narcs
 *                for (0 <=k <narcs), the arc numbered k  goes from
 *                IJ[k][0] =(IJ[2k]) to IJ[k][1] (=IJ[2k+1])
 * CDouble *A;     array of CDouble of size narcs
 *                A[k]=weight of the arc numbered k
 *
 * OUTPUT VARIABLES
 * CDouble *v;   array of CDouble of size nr_nodes (the bias vector)
 * CDouble *chi; array of CDouble of size nr_nodes (the cycle time vector)
 * int *POLICY; array of integer of size nr_nodes (an optimal policy)
 * int nr_iterations; the number of iterations of the algorithm
 * int nr_components; the number of connected components of the optimal
 *               policy which is returned.
 */

void Howard(const std::vector<int> &ij,
            const std::vector<CDouble> &A,
            int nr_nodes,
            int nr_arcs,
            std::shared_ptr<std::vector<CDouble>> *chi,
            std::shared_ptr<std::vector<CDouble>> *v,
            std::shared_ptr<std::vector<int>>(*policy),
            int *nr_iterations,
            int *nr_components) {

    bool improved = false;
    *nr_iterations = 0;

    AlgHoward AH(ij, A, nr_nodes, nr_arcs, chi, v, policy, nr_iterations, nr_components);
    AH.Run();
}

/**
 * convertMCMgraphToMatrix ()
 * The function converts a weighted directed graph used in the MCM algorithms
 * to a sparse matrix input for Howard's algorithm.
 */
void convertMCMgraphToMatrix(MCMgraph &g,
                             std::shared_ptr<std::vector<int>> *ij,
                             std::shared_ptr<std::vector<CDouble>> *A) {
    uint i = 0;
    uint j = 0;
    v_uint mapId(g.getNodes().size());

    // Re-map the id of all visible nodes back to the range [0, g->nrNodes())
    for (const auto &n : g.getNodes()) {
        if (n.visible) {
            mapId[i] = j;
            j++;
        }
        i++;
    }

    *ij = std::make_shared<std::vector<int>>(2 * g.getEdges().size());
    auto &ijr = *(*ij);
    *A = std::make_shared<std::vector<CDouble>>(g.getEdges().size());
    auto &Ar = *(*A);

    // Create an entry in the matrices for each edge
    int k = 0;
    for (const auto &e : g.getEdges()) {
        // Is the edge an existing edge in the graph?
        if (e.visible) {
            ijr[2 * k] = mapId[e.src->id];
            ijr[2 * k + 1] = mapId[e.dst->id];
            Ar[k] = e.w;

            // Next edge
            k++;
        }
    }
}

CDouble maximumCycleMeanHoward(MCMgraph &g, MCMnode **criticalNode) {

    if (g.numberOfNodes() == 0) {
        if (criticalNode != nullptr) {
            (*criticalNode) = nullptr;
        }
        return -INFINITY;
    }

    std::shared_ptr<std::vector<int>> ij = nullptr;
    std::shared_ptr<std::vector<CDouble>> A = nullptr;
    std::shared_ptr<std::vector<CDouble>> chi = nullptr;
    std::shared_ptr<std::vector<CDouble>> v = nullptr;
    std::shared_ptr<std::vector<int>> policy = nullptr;
    int nr_iterations = 0;
    int nr_components = 0;

    convertMCMgraphToMatrix(g, &ij, &A);

    Howard(*ij,
           *A,
           g.numberOfNodes(),
           g.numberOfEdges(),
           &chi,
           &v,
           &policy,
           &nr_iterations,
           &nr_components);

    // find maximum cycle mean in chi vector
    MCMnodes &nodes = g.getNodes();
    auto n = nodes.begin();
    CDouble mcm = chi->at(0);
    MCMnode *critNode = &(*n);
    for (auto &val : *chi) {
        if (val > mcm) {
            mcm = val;
            critNode = &(*n);
            n++;
        }
    }

    if (criticalNode != nullptr) {
        (*criticalNode) = &(*n);
    }
    return mcm;
}

CDouble maximumCycleMeanHowardGeneral(MCMgraph &g, MCMnode **criticalNode) {

    MCMgraphs sccs;
    stronglyConnectedMCMgraph(g, sccs, false);

    CDouble mcm = -INFINITY;
    if (criticalNode != nullptr) {
        *criticalNode = nullptr;
    }
    for (auto &scc : sccs) {
        std::map<CId, CId> nodeMap;
        scc->relabelNodeIds(&nodeMap);
        MCMnode *sccCriticalNode = nullptr;
        ;
        CDouble cmcm = maximumCycleMeanHoward(*scc, &sccCriticalNode);
        if (cmcm > mcm) {
            mcm = cmcm;
            if (criticalNode != nullptr) {
                *criticalNode = g.getNode(nodeMap[sccCriticalNode->id]);
            }
        }
    }

    return mcm;
}

} // namespace Graphs