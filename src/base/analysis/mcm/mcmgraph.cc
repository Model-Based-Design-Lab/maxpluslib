/*
 *  Eindhoven University of Technology
 *  Eindhoven, The Netherlands
 *  Dept. of Electrical Engineering
 *  Electronics Systems Group
 *  Model Based Design Lab (https://computationalmodeling.info/)
 *
 *  Name            :   mcmgraph.cc
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   November 7, 2005
 *
 *  Function        :   Convert HSDF graph to weighted directed graph for MCM
 *                      calculation.
 *
 *  History         :
 *      07-11-05    :   Initial version.
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
#include "base/analysis/mcm/mcm.h"
#include "base/analysis/mcm/mcmyto.h"
#include <cassert>
#include <map>
#include <memory>
#include <set>
#include <vector>

namespace Graphs {
/**
 * ~MCMgraph
 * Constructor.
 */
MCMgraph::MCMgraph() = default;

/**
 * ~MCMgraph
 * Destructor.
 */
MCMgraph::~MCMgraph() = default;

// Copy Constructor
MCMgraph::MCMgraph(const MCMgraph &g) {
    for (const auto &n : g.nodes) {
        this->addNode(n.id, n.visible);
    }
    for (const auto &e : g.edges) {
        this->addEdge(e.id, *this->getNode(e.src->id), *this->getNode(e.dst->id), e.w, e.d);
    }
}

MCMnodeRefs MCMgraph::getNodeRefs() {
    MCMnodeRefs result;
    for (auto &n : this->nodes) {
        result.push_back(&n);
    }
    return result;
}

MCMedgeRefs MCMgraph::getEdgeRefs() {
    MCMedgeRefs result;
    for (auto &e : this->edges) {
        result.push_back(&e);
    }
    return result;
}

/**
 * MCMedge
 * Constructor.
 */
MCMedge::MCMedge(CId eId, MCMnode &src, MCMnode &dst, CDouble w, CDouble d, bool eVisible) :
    id(eId), visible(eVisible), src(&src), dst(&dst), w(w), d(d) {}

/**
 * MCMnode
 * Constructor.
 */
MCMnode::MCMnode(CId nId, bool nVisible) : id(nId), visible(nVisible) {}

/**
 * splitMCMedgeToSequence ()
 * The function converts an MCM edge with more than one delay
 * into a sequence of edges with one delay (uses recursive call
 * to itself).
 */
static void splitMCMedgeToSequence(MCMgraph &g, MCMedge &e) {
    // Create dummy node n;
    MCMnode *n = g.addNode(static_cast<CId>(g.getNodes().size()));

    // Create a new edge between the src node of e and a new
    // dummy node.
    MCMedge *eN = g.addEdge(static_cast<CId>(g.getEdges().size()), *e.src, *n, 0, e.d - 1);

    // Remove e from the set of edges its source node is connected to
    // and add eN to this list
    for (auto iter = e.src->out.begin(); iter != e.src->out.end(); iter++) {
        if ((*iter)->id == e.id) {
            e.src->out.erase(iter);
            break;
        }
    }
    e.src->out.push_back(eN);

    // Connect e to node n
    e.src = n;
    n->out.push_back(&e);
    n->in.push_back(eN);

    // One delay left on e
    e.d = 1;

    // More than one delay on the new edge?
    if (eN->d > 1) {
        splitMCMedgeToSequence(g, *eN);
    }
}

/**
 * addLongestDelayEdgeForNode ()
 * The function add an additional edge between the node n and any node
 * reachable over e which only uses edges with no initial delays (except
 * for the initial edge e). The weight of the new edge is the longest
 * path from the node n to the destination node.
 * The function uses an adapted version of Dijkstra's shortest path
 * algorithm.
 * Note: algorithm assumes that edge weights are integer values !
 */
static void addLongestDelayEdgeForNode(MCMgraph &g, MCMnode &n, MCMedge &e) {
    std::vector<int> d(g.getNodes().size());
    std::vector<const MCMnode *> pi(g.getNodes().size());
    MCMnodeRefs S;
    MCMnodeRefs Q = g.getNodeRefs();

    // Initialize single source
    for (uint v = 0; v < g.getNodes().size(); v++) {
        d[v] = -1;
        pi[v] = nullptr;
    }
    d[n.id] = 0;

    // Initialize the node connected via the edge e to n
    d[e.dst->id] = static_cast<int>(e.w);

    // Remove node n from Q and add it to S (only when edge e is no
    // self-edge)
    if (e.dst->id != n.id) {
        for (auto iter = Q.begin(); iter != Q.end(); iter++) {
            if ((*iter)->id == n.id) {
                // Is edge e
                Q.erase(iter);
                break;
            }
        }
        S.push_back(&n);
    }

    // Find all longest paths till Q is empty or all reachable paths seen
    while (!Q.empty()) {
        MCMnode *u = nullptr;

        // Find node u in Q with largest distance
        int dMax = -1;
        for (auto &iter : Q) {
            if (d[iter->id] > dMax) {
                u = iter;
                dMax = d[u->id];
            }
        }

        // Found no node with a non-negative distance ?
        if (dMax < 0) {
            break;
        }

        // Remove node u from Q and add it to S
        for (auto iter = Q.begin(); iter != Q.end(); iter++) {
            if ((*iter)->id == u->id) {
                Q.erase(iter);
                break;
            }
        }
        S.push_back(u);

        // Relax all nodes v adjacent to u (connected via edges with no tokens)
        for (auto iter = u->out.begin(); iter != u->out.end(); iter++) {
            const MCMedge *e = *iter;
            MCMnode *v = e->dst;

            if (e->d == 0 && d[v->id] < d[u->id] + e->w) {
                if (d[v->id] != -1) {
                    // Found a longer path to v, add it to list of
                    // nodes to be checked (again).
                    Q.push_back(v);
                }

                d[v->id] = d[u->id] + static_cast<int>(e->w);
                pi[v->id] = u;
            }
        }
    }

    // Add an edge between the node n and any node m reachable from n
    // with a weight equal to the longest path from n to m
    for (auto &m : S) {
        // Node m reachable from n and not connected directly to n via e?
        if (d[m->id] > 0 && e.dst->id != m->id) {
            // Create an edge between n and m
            MCMedge *eN = g.addEdge(static_cast<CId>(g.getEdges().size()),
                                    n,
                                    *m,
                                    d[m->id],
                                    e.d,
                                    false); // e.d should always be 1
            n.out.push_back(eN);
            m->in.push_back(eN);
        }
    }
}

/**
 * addLongestDelayEdgesToMCMgraph ()
 * The function adds additional edges to the graph which express the
 * longest path between two nodes crossing one edge with a delay. Edges
 * with no delay are removed and edges with more than one delay element
 * are converted into a sequence of edges with one delay element.
 */
void addLongestDelayEdgesToMCMgraph(MCMgraph &g) {

    // Find longest path between a node n and a node m
    // over all sequences of edges in which only the
    // first edge may contain a delay
    for (auto &e : g.getEdgeRefs()) {
        // More than one delay on the edge?
        if (e->d > 1) {
            splitMCMedgeToSequence(g, *e);
        }
    }

    // Find longest path between a node n and a node m
    // over all sequences of edges in which only the
    // first edge may contain a delay
    for (auto &e : g.getEdgeRefs()) {
        // Initial tokens on edge?
        if (e->d != 0 && e->visible) {
            // Find the longest path from n to any node m
            // and add an edge with the path wait to the graph
            addLongestDelayEdgeForNode(g, *e->src, *e);

            // Seen this edge (set it to invisible)
            e->visible = false;
        }
    }

    // Hide all edges which do not contain a delay
    for (auto &e : g.getEdges()) {
        // No initial tokens on edge?
        e.visible = e.d != 0;
    }
}

namespace {

/**
 * getAdjacentNodes ()
 * The function returns a list with nodes directly reachable from
 * node a (in case transpose if false). If transpose is 'true', the
 * graph is transposed and the function returns a list with nodes
 * which are directly reachable from a in the transposed graph.
 */
MCMnodeRefs getAdjacentNodes(const MCMnode &a, bool transpose) {
    MCMnodeRefs node_refs;

    if (!transpose) {
        for (const auto &e : a.out) {
            if (e->visible) {
                node_refs.push_back(e->dst);
            }
        }
    } else {
        for (const auto &e : a.in) {
            if (e->visible) {
                node_refs.push_back(e->src);
            }
        }
    }

    return node_refs;
}

/**
 * getNextNode ()
 * The function returns the node from the list of nodes with the highest
 * order. Its order is set to -1. If all nodes have order -1, a nullptr pointer is
 * returned.
 */
const MCMnode *getNextNode(MCMnodes &nodes, v_int &order) {
    const MCMnode *a = nullptr;
    int orderA = -1;

    // Find actor with largest order
    for (const auto &b : nodes) {
        if (orderA < order[b.id]) {
            a = &b;
            orderA = order[b.id];
        }
    }

    // All actors have order -1?
    if (orderA == -1) {
        return nullptr;
    }

    return a;
}

/**
 * dfsVisit ()
 * The visitor function of the DFS algorithm.
 */
void dfsVisit(const MCMnode &u,
              int &time,
              v_int &color,
              v_int &d,
              v_int &f,
              std::vector<const MCMnode *> &pi,
              bool transpose) {
    // color[u] <- gray
    color[u.id] = 1;

    // d[u] <- time <- time + 1
    time++;
    d[u.id] = time;

    // for each v in Adj(e)
    MCMnodeRefs adj = getAdjacentNodes(u, transpose);
    for (auto &v : adj) {
        // do if color[v] = white
        if (color[v->id] == 0) {
            pi[v->id] = &u;
            dfsVisit(*v, time, color, d, f, pi, transpose);
        }
    }

    // color[u] <- black
    color[u.id] = 2;

    // f[u] <- time <- time + 1
    time++;
    f[u.id] = time;
}

} // namespace

/**
 * dfsMCMgraph ()
 * The function performs a depth first search on the graph. The discover
 * time (d), finish time (f) and predecessor tree (pi) are passed back.
 * The graph is transposed if the argument 't' is 'true'. Vertices are
 * considered in decreasing order of f[u].
 *
 * The function derives an 'order' vector from the vector 'f'. Actors are
 * considered in decreasing order according to this vector. When an actor
 * is visited, its order is set to -1. At the end, all actors must have
 * order -1 (i.e. all actors are visited).
 */
void dfsMCMgraph(
        MCMgraph &g, v_int &d, v_int &f, std::vector<const MCMnode *> &pi, bool transpose) {

    // for each u in G do order[u] <- f[u]
    v_int order(f);

    // for each u in G do color[u] <- white
    v_int color(g.nrVisibleNodes(), 0);

    // time <- 0
    int time = 0;

    // for each u in G do pi[u] <- NIL
    for (uint u = 0; u < g.nrVisibleNodes(); u++) {
        pi[u] = nullptr;
    }

    // for each u in G (visit in order given by order)
    for (const auto *a = getNextNode(g.getNodes(), order); a != nullptr;
         a = getNextNode(g.getNodes(), order)) {
        // Mark node as visited
        order[a->id] = -1;

        // if color[u] = white
        if (color[a->id] == 0) {
            dfsVisit(*a, time, color, d, f, pi, transpose);
        }
    }
}

/**
 * addNodeToComponent ()
 * The function adds a new copy of a node to the component and it
 * also creates copies for all edges between this node and all nodes
 * already in the component.
 */
static void addNodeToComponent(const MCMnode &n, MCMgraph &comp) {

    // Create a copy of n and add it to the component
    MCMnode *m = comp.addNode(n.id);

    // Check all edges of n for inclusion in the component, first the outgoing edges...
    for (const auto &e : n.out) {
        if (!e->visible) {
            continue;
        }

        // Is destination node in the component?
        for (auto &nn : comp.getNodes()) {
            if (e->dst->id == nn.id) {
                // Add a copy of the edge to the component
                comp.addEdge(e->id, *m, nn, e->w, e->d, e->visible);
                break;
            }
        }
    }
    // ... and now the incoming edges. We must now skip the self edges since they have been included
    // in the previous loop already
    for (const auto &e : n.in) {
        // Is source node in the component?
        for (auto &iterN : comp.getNodes()) {
            // if the source node is in the component and it is not a self-edge
            if ((e->src->id == iterN.id) && (e->src->id != e->dst->id)) {
                // Add a copy of the edge to the component
                comp.addEdge(e->id, iterN, *m, e->w, e->d, e->visible);
                break;
            }
        }
    }
}

/**
 * treeVisitChildren ()
 * The function visits all children of the actor 'u'. The parent-child
 * relation is given via the vector 'pi'.
 */
static bool
treeVisitChildren(MCMgraph &g, std::vector<const MCMnode *> &pi, MCMnode *u, MCMgraph &comp) {
    bool children = false;

    for (uint i = 0; i < g.getNodes().size(); i++) {
        // Node v points to node u (i.e. v is a child of u)?
        if (pi[i] != nullptr && pi[i]->id == u->id) {
            MCMnode *v = nullptr;

            for (auto &n : g.getNodes()) {
                if (n.id == i) {
                    v = &n;
                    break;
                }
            }
            // There must always be a node v.
            assert((v != nullptr));

            // Add node v to the component
            v->visible = true;
            addNodeToComponent(*v, comp);
            children = true;

            // Find all children of v in the tree
            treeVisitChildren(g, pi, v, comp);
        }
    }

    return children;
}

/**
 * findComponentsInMCMgraph ()
 * The function determines the strongly connected components in the graph. To do
 * this, it performs depth-first walk on the forest given by 'pi'.
 */
static void findComponentsInMCMgraph(MCMgraph &g,
                                     std::vector<const MCMnode *> &pi,
                                     MCMgraphs &components,
                                     bool includeComponentsWithoutEdges = false) {
    std::shared_ptr<MCMgraph> comp;

    // Set all node as invisible
    for (auto &n : g.getNodes()) {
        n.visible = false;
    }

    // Find the strongly connected component and make all of its nodes visible
    for (auto &n : g.getNodes()) {

        if (pi[n.id] == nullptr) {
            // Create a new graph for the component
            comp = std::make_shared<MCMgraph>();

            // Find all children of n in the tree
            if (treeVisitChildren(g, pi, &n, *comp)) {
                // Node n has children, so it belongs to the strongly
                // connected component
                n.visible = true;
                addNodeToComponent(n, *comp);
            } else {
                if (!includeComponentsWithoutEdges) {
                    // Node n may have a self-loop making it a strongly
                    // connected component
                    for (auto &e : n.in) {
                        // Is edge a self-loop?
                        if (e->src->id == n.id) {
                            n.visible = true;
                            addNodeToComponent(n, *comp);
                        }
                    }
                } else {
                    n.visible = true;
                    addNodeToComponent(n, *comp);
                }
            }

            // Found a strongly connected component (at least one edge)?
            if (includeComponentsWithoutEdges || comp->nrVisibleEdges() > 0) {
                components.push_back(comp);
            }
        }
    }

    // Make all edges to invisible nodes also invisible
    for (auto &e : g.getEdges()) {
        if (e.visible) {
            if (!e.src->visible || !e.dst->visible) {
                e.visible = false;
            }
        }
    }
}

/**
 * Extract the strongly connected components from the graph. These components
 * are returned as a set of MCM graphs. All nodes which belong to at least
 * one of the strongly connected components are set to visible in the graph g,
 * all other nodes are made invisible. Also edges between two nodes in (possibly
 * different) strongly connected components are made visible and all others
 * invisible. The graph g consists in the end of only nodes which are part of
 * a strongly connected component and all the edges between these nodes. Some
 * MCM algorithms work also on this graph (which reduces the execution time
 * needed in some of the conversion algorithms).
 */

void stronglyConnectedMCMgraph(MCMgraph &g,
                               MCMgraphs &components,
                               bool includeComponentsWithoutEdges) {
    // Initialize
    v_int d(g.nrVisibleNodes());
    v_int f(g.nrVisibleNodes(), 0);
    std::vector<const MCMnode *> pi(g.nrVisibleNodes());

    // Call dfs(g) to compute f[u] for each u
    dfsMCMgraph(g, d, f, pi, false);

    // Call dfs(gT), consider the vertices in order of decreasing f[u]
    dfsMCMgraph(g, d, f, pi, true);

    // Component is given by the vertices of the tree in the depth-first forest
    findComponentsInMCMgraph(g, pi, components, includeComponentsWithoutEdges);

    //// Print the MCM graph in dot-format
    // cerr << "digraph g {" << endl;
    // cerr << "    size=\"7,10\";" << endl;
    // for (auto iter = g->edges.begin(); iter != g->edges.end(); iter++)
    //{
    //     MCMedge *e = *iter;

    //    if (e->visible)
    //    {
    //        cerr << "    " << e->src->id << " -> " << e->dst->id;
    //        cerr << " [ label=" << e->w << " ];" << endl;
    //    }
    //}
    // cerr << "}" << endl;
}

/**
 * relabelMCMgraph ()
 * The function removes all hidden nodes and edges from the graph. All visible
 * edges are assigned a new id starting in the range [0,nrNodes()).
 */
void relabelMCMgraph(MCMgraph &g) {

    std::set<MCMedge *> edgesToRemove;
    std::set<MCMnode *> nodesToRemove;

    // find nodes to remove
    for (auto &n : g.getNodes()) {
        if (!n.visible) {
            nodesToRemove.insert(&n);
        }
    }

    // find edges to remove
    for (auto &e : g.getEdges()) {
        if (!e.visible) {
            edgesToRemove.insert(&e);
        }
    }

    // remove edges
    for (auto *e : edgesToRemove) {
        g.removeEdge(*e);
    }

    // remove nodes
    for (auto *n : nodesToRemove) {
        g.removeNode(*n);
    }

    // Relabel nodes
    uint nodeId = 0;
    for (auto &n : g.getNodes()) {
        if (n.visible) {
            n.id = nodeId;
            nodeId++;
        }
    }

    // Relabel edges
    uint edgeId = 0;
    for (auto &e : g.getEdges()) {
        if (e.visible) {
            e.id = edgeId;
            edgeId++;
        }
    }
}

// Prune the edges in the MCMgraph to maintain only Pareto maximal combinations
// Of the weight and number of tokens pair pair of nodes
// Generates a new graph
// Note this algorithm does currently not distinguish visible and invisible edges!
std::shared_ptr<MCMgraph> MCMgraph::pruneEdges() {

    class _local {
    public:
        std::map<MCMnode *, MCMedgeRefs, MCMNodeLess> paretoEdges;
        void insert(MCMedge *e) {
            MCMedgeRefs &edges = paretoEdges[e->dst];
            // Simple Cull
            auto i = edges.begin();
            bool cont = i != edges.end();
            bool add_e = true;
            while (cont) {
                MCMedge *f = *i;
                // if e is worse than f
                if ((e->d <= f->d) && (e->w >= f->w)) {
                    // remove f from edges;
                    i = edges.erase(i);
                } else {
                    // if f is worse than e
                    if ((f->d <= e->d) && (f->w >= e->w)) {
                        // forget e and stop iterating;
                        add_e = false;
                        cont = false;
                    } else {
                        // incomparable, go on
                        i++;
                    }
                }
                cont = cont && (i != edges.end());
            }
            if (add_e) {
                edges.push_back(e);
            }
        }
    };

    _local local;

    // create new graph
    std::shared_ptr<MCMgraph> result = std::make_shared<MCMgraph>();

    // create all nodes.
    std::map<MCMnode *, MCMnode *> newNodeMap;
    for (auto &u : this->nodes) {
        auto *n = result->addNode(u.id, u.visible);
        newNodeMap[&u] = n;
    }

    // for every node
    for (auto &u : this->nodes) {

        local.paretoEdges.clear();

        // for every outgoing edges to a simple cull Pareto filtering
        for (auto &i : u.out) {
            local.insert(i);
        }

        // add Pareto Edges to new Graph.
        // for every dst node
        for (auto &j : local.paretoEdges) {
            const MCMedgeRefs &edges = j.second;
            // for every edge
            for (const auto &e : edges) {
                result->addEdge(e->id, *newNodeMap[&u], *newNodeMap[e->dst], e->w, e->d, true);
            }
        }
    }

    return result;
}

CDouble MCMgraph::calculateMaximumCycleMeanKarp() { return maximumCycleMeanKarp(*this); }

CDouble MCMgraph::calculateMaximumCycleMeanKarpDouble(const MCMnode **criticalNode) {
    return maximumCycleMeanKarpDouble(*this, criticalNode);
}

CDouble MCMgraph::calculateMaximumCycleRatioAndCriticalCycleYoungTarjanOrlin(
        std::vector<const MCMedge *> *cycle) {
    return maxCycleRatioAndCriticalCycleYoungTarjanOrlin(*this, cycle);
}

void MCMgraph::relabelNodeIds(std::map<CId, CId> *nodeIdMap) {
    int k = 0;
    for (auto &i : this->nodes) {
        MCMnode &n = i;
        if (nodeIdMap != nullptr) {
            (*nodeIdMap)[k] = static_cast<int>(n.id);
        }
        n.id = k;
        k++;
    }
}

std::shared_ptr<MCMgraph> MCMgraph::normalize(CDouble mu) const {
    std::shared_ptr<MCMgraph> result = std::make_shared<MCMgraph>(*this);
    for (auto &e : result->getEdges()) {
        e.w -= mu;
    }
    return result;
}

std::shared_ptr<MCMgraph> MCMgraph::normalize(const std::map<CId, CDouble> &mu) const {
    std::shared_ptr<MCMgraph> result = std::make_shared<MCMgraph>(*this);
    for (auto &e : result->getEdges()) {
        CDouble nc = mu.at(e.src->id);
        if (nc != -DBL_MAX) {
            e.w -= nc;
        }
    }
    return result;
}

// longest path computation, may not be implemented optimally.
std::map<CId, CDouble> MCMgraph::longestPaths(const CId rootNodeId) const {
    std::map<CId, CDouble> result;
    for (const auto &n : this->nodes) {
        result[n.id] = (n.id == rootNodeId) ? 0.0 : -DBL_MAX;
    }
    bool changed = true;
    while (changed) {
        changed = false;
        for (const auto &e : this->edges) {
            if (result[e.src->id] != -DBL_MAX) {
                if (result[e.src->id] + e.w > result[e.dst->id]) {
                    changed = true;
                    result[e.dst->id] = result[e.src->id] + e.w;
                }
            }
        }
    }
    return result;
}

std::map<CId, CDouble> MCMgraph::normalizedLongestPaths(const CId rootNodeId,
                                                        const CDouble mu) const {
    std::shared_ptr<MCMgraph> normalizedGraph = this->normalize(mu);
    std::map<CId, CDouble> result = normalizedGraph->longestPaths(rootNodeId);
    return result;
}

std::map<CId, CDouble> MCMgraph::normalizedLongestPaths(const CId rootNodeId,
                                                        const std::map<CId, CDouble> &mu) const {
    std::shared_ptr<MCMgraph> normalizedGraph = this->normalize(mu);
    std::map<CId, CDouble> result = normalizedGraph->longestPaths(rootNodeId);
    return result;
}

} // namespace Graphs
