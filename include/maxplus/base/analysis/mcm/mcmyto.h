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

#ifndef BASE_ANALYSIS_MCM_MCMYTO_H_INCLUDED
#define BASE_ANALYSIS_MCM_MCMYTO_H_INCLUDED

#include "base/analysis/mcm/mcmgraph.h"
namespace Graphs {

typedef struct Node {
    // index into nodes array of graph structure is id - 1
    long id;
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
    long level;
    double cost_t;         // sum of costs on edges of tree path to node with
                           // respect to original edge costs
    double transit_time_t; // sum of transit time on edges of tree path
                           // to node
    struct Arc *v_key;     // pointer to incoming arc with minimum key
} node;

using arc = struct Arc {
    node *tail;
    node *head;
    struct Arc *next_out; // in incidence list of tail node
    struct Arc *next_in;  // in incidence list of head node
    bool in_tree;
    double cost;
    double transit_time;
    double key;
    long h_pos;       // of arc in heap
    std::shared_ptr<MCMedge> mcmEdge; // MCMedge from which the arc was constructed in the conversion
};

using graph = struct Graph {
    long n_nodes;
    node *nodes;
    long n_arcs;
    arc *arcs;
    node *vs; // additional node with zero cost outgoing edges
};

/**
 * mcmYoungTarjanOrlin ()
 * The function computes the maximum cycle mean of edge weight per edge of
 * an MCMgraph using Young-Tarjan-Orlin's algorithm.
 */
CDouble maxCycleMeanYoungTarjanOrlin(std::shared_ptr<MCMgraph> mcmGraph);

/**
 * maxCycleMeanAndCriticalCycleYoungTarjanOrlin ()
 * The function computes the maximum cycle mean of edge weight of
 * an MCMgraph using Young-Tarjan-Orlin's algorithm.
 * It returns both the MCM and a critical cycle
 * The critical cycle is only returned if cycle and len are not NULL. Then *cycle points
 * to an array of *MCMEdges of the critical cycle and *len indicates the length of the cycle.
 * *cycle is a freshly allocated array and it is the caller's obligation to deallocate it
 * in due time.
 */
CDouble
maxCycleMeanAndCriticalCycleYoungTarjanOrlin(std::shared_ptr<MCMgraph> mcmGraph, std::shared_ptr<MCMedge> **cycle, uint *len);

/**
 * maxCycleRatioYoungTarjanOrlin ()
 * The function computes the maximum cycle ratio of edge weight over delay of
 * an MCMgraph using Young-Tarjan-Orlin's algorithm.
 */
CDouble maxCycleRatioYoungTarjanOrlin(std::shared_ptr<MCMgraph> mcmGraph);

/**
 * maxCycleRatioAndCriticalCycleYoungTarjanOrlin ()
 * The function computes the maximum cycle ratio of edge weight over delay of
 * an MCMgraph using Young-Tarjan-Orlin's algorithm.
 * It returns both the MCR and a critical cycle
 * The critical cycle is only returned if cycle and len are not NULL. Then *cycle points
 * to an array of *MCMEdges of the critical cycle and *len indicates the length of the cycle.
 * *cycle is a freshly allocated array and it is the caller's obligation to deallocate it
 * in due time.
 */
CDouble
maxCycleRatioAndCriticalCycleYoungTarjanOrlin(const MCMgraph& mcmGraph, std::shared_ptr<MCMedge> **cycle, uint *len);

/**
 * minCycleRatioYoungTarjanOrlin ()
 * The function computes the minimum cycle ratio of edge weight over delay of
 * an MCMgraph using Young-Tarjan-Orlin's algorithm.
 */
CDouble minCycleRatioYoungTarjanOrlin(std::shared_ptr<MCMgraph> mcmGraph);

/**
 * minCycleRatioAndCriticalCycleYoungTarjanOrlin ()
 * The function computes the minimum cycle ratio of edge weight over delay of
 * an MCMgraph using Young-Tarjan-Orlin's algorithm.
 * It returns both the MCR and a critical cycle
 * The critical cycle is only returned if cycle and len are not NULL. Then *cycle points
 * to an array of *MCMEdges of the critical cycle and *len indicates the length of the cycle.
 * *cycle is a freshly allocated array and it is the caller's obligation to deallocate it
 * in due time.
 */
CDouble
minCycleRatioAndCriticalCycleYoungTarjanOrlin(std::shared_ptr<MCMgraph> mcmGraph, std::shared_ptr<MCMedge> **cycle, uint *len);

/**
 * getDelay ()
 * The function returns the delay associated with an edge.
 */
double getDelay(std::shared_ptr<MCMedge> e);

/**
 * getWeight ()
 * The function returns the weight associated with an edge.
 */
double getWeight(std::shared_ptr<MCMedge> e);

/**
 * convertMCMgraphToYTOgraph ()
 * The function converts a weighted directed graph used in the MCM algorithms
 * to graph input for Young-Tarjan-Orlin's algorithm.
 * It assumes that the id's of the nodes are 0 <= id < number of nodes
 */
void convertMCMgraphToYTOgraph(std::shared_ptr<MCMgraph> g,
                               graph *gr,
                               double (*costFunction)(std::shared_ptr<MCMedge> e),
                               double (*transit_timeFunction)(std::shared_ptr<MCMedge> e));

/**
 * mmcycle ()
 *
 * Determines maximum mean cycle in a directed graph
 * G = (V, E, c), c: E->IR a "cost" function on the
 * edges, alternatively called "length" or "weight".
 */

void mmcycle(graph *gr, double *lambda, arc **cycle, long *len);

} // namespace Graphs

#endif
