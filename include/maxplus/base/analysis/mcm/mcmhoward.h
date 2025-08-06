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

#ifndef MAXPLUS_BASE_ANALYSIS_MCM_MCMHOWARD_H_INCLUDED
#define MAXPLUS_BASE_ANALYSIS_MCM_MCMHOWARD_H_INCLUDED

#include "maxplus/base/analysis/mcm/mcmgraph.h"
#include <memory>
namespace MaxPlus::Graphs {
/**
 * convertMCMgraphToMatrix ()
 * The function converts a weighted directed graph used in the MCM algorithms
 * to a sparse matrix input for Howard's algorithm.
 */
void convertMCMgraphToMatrix(MCMgraph &g,
                             std::unique_ptr<std::vector<int>> *ij,
                             std::unique_ptr<std::vector<CDouble>> *A);

/**
 * Howard ()
 * Howard Policy Iteration Algorithm for Max Plus Matrices.
 *
 * INPUT of Howard Algorithm:
 *      ij,A,nnodes,narcs : sparse description of a matrix.
 *
 * OUTPUT:
 *      chi cycle time vector
 *      v bias
 *      pi optimal policy
 *      NIterations: Number of iterations of the algorithm
 *      NComponents: Number of connected components of the optimal policy
 *
 * ASSUMPTIONS
 *      The graph has a non-zero number of nodes
 *      The graph has a non-zero number of edges
 *      Every node in the graph has outgoing directed edges
 */

void Howard(const std::vector<int> &ij,
            const std::vector<CDouble> &A,
            int nr_nodes,
            int nr_arcs,
            std::unique_ptr<std::vector<CDouble>> *chi,
            std::unique_ptr<std::vector<CDouble>> *v,
            std::unique_ptr<std::vector<int>> *policy,
            int *nr_iterations,
            int *nr_components);

/**
 * maximumCycleMeanHoward ()
 * Howard Policy Iteration Algorithm for Max Plus Matrices.
 *
 * INPUT MCMgraph which must have outgoing edges from every node
 *
 * OUTPUT:
 *      maximum cycle mean
 *      a node on the cycle with maximum cycle mean
 *
 * ASSUMPTIONS
 *      The graph has a non-zero number of nodes
 *      The graph has a non-zero number of edges
 *      Every node in the graph has outgoing directed edges
 *
 */
CDouble maximumCycleMeanHoward(MCMgraph &g, MCMnode **criticalNode);

/**
 * maximumCycleMeanHowardGeneral ()
 * Howard Policy Iteration Algorithm for Max Plus Matrices.
 *
 * INPUT MCMgraph  which can be arbitrary
 *
 * OUTPUT:
 *      maximum cycle mean
 *      a node on the cycle with maximum cycle mean
 *
 */
CDouble maximumCycleMeanHowardGeneral(MCMgraph &g, MCMnode **criticalNode);

} // namespace MaxPlus::Graphs
#endif