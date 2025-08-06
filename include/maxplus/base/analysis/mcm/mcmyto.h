/*
 *  Eindhoven University of Technology
 *  Eindhoven, The Netherlands
 *  Dept. of Electrical Engineering
 *  Electronics Systems Group
 *  Model Based Design Lab (https://computationalmodeling.info/)
 *
 *  Name            :   mcmyto.h
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   March 28, 2011
 *
 *  Function        :   Compute the MCM for an HSDF graph using Young-
 *                      Tarjan-Orlin's algorithm.
 *
 *  Acknowledgement :   This code is based the 'mmcycle' program which can be
 *                      found at 'http://elib.zib.de/pub/Packages/mathprog
 *                      /netopt/mmc-info'. The original code is written by
 *                      Georg Skorobohatyj (bzfskoro@zib.de) and is free
 *                      for redistribution.
 *
 *  History         :
 *      28-03-11    :   Initial version.
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

#ifndef MAXPLUS_BASE_ANALYSIS_MCM_MCMYTO_H_INCLUDED
#define MAXPLUS_BASE_ANALYSIS_MCM_MCMYTO_H_INCLUDED

#include "maxplus/base/analysis/mcm/mcmgraph.h"
#include <cstdint>
#include <vector>

namespace MaxPlus::Graphs {

using node = struct Node {
    // index into nodes array of graph structure is id - 1
    std::int32_t id;
    struct Arc *first_arc_out;
    struct Arc *first_arc_in;
    struct Node *link; // for one way linked list
    bool in_list;

    // tree links
    struct Arc *parent_in;
    struct Node *first_child;
    struct Node *left_sibling;
    struct Node *right_sibling;

    // next entries for maintenance of edge keys
    std::int32_t level;
    CDouble cost_t;         // sum of costs on edges of tree path to node with
                            // respect to original edge costs
    CDouble transit_time_t; // sum of transit time on edges of tree path
                            // to node
    struct Arc *v_key;      // pointer to incoming arc with minimum key
};

using arc = struct Arc {
    node *tail;
    node *head;
    struct Arc *next_out; // in incidence list of tail node
    struct Arc *next_in;  // in incidence list of head node
    bool in_tree;
    CDouble cost;
    CDouble transit_time;
    CDouble key;
    std::int32_t h_pos;     // of arc in heap
    const MCMedge *mcmEdge; // MCMedge from which the arc was constructed in the conversion
};

using graph = struct Graph {
    std::int32_t n_nodes;
    std::vector<node> nodes;
    std::int32_t n_arcs;
    std::vector<arc> arcs;
    node *vs; // additional node with zero cost outgoing edges
};

/**
 * mcmYoungTarjanOrlin ()
 * The function computes the maximum cycle mean of edge weight per edge of
 * an MCMgraph using Young-Tarjan-Orlin's algorithm.
 */
CDouble maxCycleMeanYoungTarjanOrlin(MCMgraph &mcmGraph);

/// The function computes the maximum cycle mean of edge weight of
/// an MCMgraph using Young-Tarjan-Orlin's algorithm.
/// It returns both
/// The critical cycle is only returned if cycle is not NULL. Then *cycle points
/// to an array of *MCMEdges of the critical cycle.
/// @param mcmGraph The graph to analyze
/// @param cycle Pointer to the critical cycle
/// @return the MCM and a critical cycle
CDouble maxCycleMeanAndCriticalCycleYoungTarjanOrlin(MCMgraph &mcmGraph,
                                                     std::vector<const MCMedge *> *cycle);

/**
 * maxCycleRatioYoungTarjanOrlin ()
 * The function computes the maximum cycle ratio of edge weight over delay of
 * an MCMgraph using Young-Tarjan-Orlin's algorithm.
 */
CDouble maxCycleRatioYoungTarjanOrlin(MCMgraph &mcmGraph);

/**
 * maxCycleRatioAndCriticalCycleYoungTarjanOrlin ()
 * The function computes the maximum cycle ratio of edge weight over delay of
 * an MCMgraph using Young-Tarjan-Orlin's algorithm.
 * It returns both the MCR and a critical cycle
 * The critical cycle is only returned if cycle is not NULL. Then *cycle points
 * to an array of *MCMEdges of the critical cycle/
 */
CDouble maxCycleRatioAndCriticalCycleYoungTarjanOrlin(MCMgraph &mcmGraph,
                                                      std::vector<const MCMedge *> *cycle);

/**
 * minCycleRatioYoungTarjanOrlin ()
 * The function computes the minimum cycle ratio of edge weight over delay of
 * an MCMgraph using Young-Tarjan-Orlin's algorithm.
 */
CDouble minCycleRatioYoungTarjanOrlin(MCMgraph &mcmGraph);

/**
 * minCycleRatioAndCriticalCycleYoungTarjanOrlin ()
 * The function computes the minimum cycle ratio of edge weight over delay of
 * an MCMgraph using Young-Tarjan-Orlin's algorithm.
 * It returns both the MCR and a critical cycle
 * The critical cycle is only returned if cycle is not NULL. Then *cycle points
 * to an array of *MCMEdges of the critical cycle.
 */
CDouble minCycleRatioAndCriticalCycleYoungTarjanOrlin(MCMgraph &mcmGraph,
                                                      std::vector<const MCMedge *> *cycle);

/**
 * getDelay ()
 * The function returns the delay associated with an edge.
 */
CDouble getDelay(const MCMedge &e);

/**
 * getWeight ()
 * The function returns the weight associated with an edge.
 */
CDouble getWeight(const MCMedge &e);

/**
 * convertMCMgraphToYTOgraph ()
 * The function converts a weighted directed graph used in the MCM algorithms
 * to graph input for Young-Tarjan-Orlin's algorithm.
 * It assumes that the id's of the nodes are 0 <= id < number of nodes
 */
void convertMCMgraphToYTOgraph(MCMgraph &g,
                               graph &gr,
                               CDouble (*costFunction)(const MCMedge &e),
                               CDouble (*transit_timeFunction)(const MCMedge &e));

/**
 * mmcycle ()
 *
 * Determines maximum mean cycle in a directed graph
 * G = (V, E, c), c: E->IR a "cost" function on the
 * edges, alternatively called "length" or "weight".
 */

void mmcycle(graph &gr, CDouble *lambda, std::vector<const arc *> *cycle);

} // namespace MaxPlus::Graphs

#endif
