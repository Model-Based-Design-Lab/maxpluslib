/*
 *  Eindhoven University of Technology
 *  Eindhoven, The Netherlands
 *  Dept. of Electrical Engineering
 *  Electronics Systems Group
 *  Model Based Design Lab (https://computationalmodeling.info/)
 *
 *  Name            :   mcmyto.cc
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   November 8, 2005
 *
 *  Function        :   Compute the MCM for an HSDF graph using Young-
 *                      Tarjan-Orlin's algorithm.
 *
 *  Acknowledgement :   This code is based the 'mmcycle' program which can be
 *                      found at 'http:// elib.zib.de/pub/Packages/mathprog
 *                      /netopt/mmc-info'. The original code is written by
 *                      Georg Skorobohatyj (bzfskoro@zib.de) and is free
 *                      for redistribution.
 *
 *  History         :
 *      08-11-05    :   Initial version.
 *      05-04-23    :   Refactoring
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

#include "base/analysis/mcm/mcmyto.h"
#include "base/analysis/mcm/mcmgraph.h"
#include <algorithm>
#include <cassert>
#include <cfloat>
#include <cmath>
#include <cstdint>

namespace MaxPlus::Graphs {

class AlgYTO {
public:
    explicit AlgYTO(graph &g) : gr(&g) {}

private:
    static constexpr node *NILN = nullptr;
    static constexpr arc *NILA = nullptr;

    graph *gr;

    std::int32_t update_level = 0;
    node *upd_nodes = nullptr;

    static constexpr std::int32_t dh = 4L;

    /*
     * d_heap implementation
     * ---------------------
     * See R.E. Tarjan: Data Structures and Network
     * Algorithms (Society for Industrial and Applied
     * Mathematics)", Philadelphia, PA, 1983) for a
     * description.
     *
     * Indices are numbered from 0 to hz-1 , hz =
     * maximum heap size, therefore the parent of
     * node x is int((x-1)/dh), dh = d_heap parameter
     * = number of children per node, and the children
     * of node x are the nodes in the interval
     *
     * [dh*x+1, dh*x+2, ..., min (dh *(x+1), hz-1)].
     */

    using item = arc *;

    using d_heap = struct D_heap {
        std::int32_t max_size = 0;
        std::int32_t size = 0;
        std::vector<item> items;
    };

    d_heap h;

        // void printHeap()
        // {
        //     cerr << "Heap: ";
        //     for (std::int32_t i = 0; i < h.size; i++)
        //         cerr << h.items[i]->key << ", ";
        //     cerr << endl;
        // }

    [[nodiscard]] std::int32_t minChild(std::int32_t x) {
        std::int32_t k_min = -1;
        if (x != h.size - 1) {
            CDouble min = DBL_MAX;
            std::int32_t upb = 0;
            if (h.size - 1 < dh * (x + 1)) {
                upb = h.size - 1;
            } else {
                upb = dh * (x + 1);
            }

            for (std::int32_t k = (dh * x) + 1; k <= upb; k++) {
                if (h.items[k]->key < min) {
                    min = h.items[k]->key;
                    k_min = k;
                }
            }
        }

        return (k_min);
    }

    [[nodiscard]] std::int32_t minChildRobust(std::int32_t x, CDouble epsilon) {
        std::int32_t k_min = -1;
        if (x != h.size - 1) {
            CDouble min = DBL_MAX;
            std::int32_t upb = 0;
            if (h.size - 1 < dh * (x + 1)) {
                upb = h.size - 1;
            } else {
                upb = dh * (x + 1);
            }

            for (std::int32_t k = (dh * x) + 1; k <= upb; k++) {
                if (min - h.items[k]->key > epsilon) {
                    min = h.items[k]->key;
                    k_min = k;
                }
            }
        }

        return (k_min);
    }

    void SiftDown(item i, std::int32_t x) {
        std::int32_t c = minChild(x);
        while (c >= 0 && h.items[c]->key < i->key) {
            std::int32_t cc = minChild(c);
            h.items[x] = h.items[c];
            h.items[x]->h_pos = x;
            x = c;
            c = cc;
        }
        h.items[x] = i;
        i->h_pos = x;
    }

    void SiftDownRobust(item i, std::int32_t x, CDouble epsilon) {
        std::int32_t c = minChildRobust(x, epsilon);
        while (c >= 0 && i->key - h.items[c]->key > epsilon) {
            std::int32_t cc = minChildRobust(c, epsilon);
            h.items[x] = h.items[c];
            h.items[x]->h_pos = x;
            x = c;
            c = cc;
        }
        h.items[x] = i;
        i->h_pos = x;
    }

    void SiftUp(item i, std::int32_t x) {
        std::int32_t p = (x == 0) ? -1 : (x - 1) / dh;
        while (p >= 0 && h.items[p]->key > i->key) {
            h.items[x] = h.items[p];
            (h.items[x])->h_pos = x;
            x = p;
            p = (p == 0) ? -1 : (p - 1) / dh;
        }
        h.items[x] = i;
        i->h_pos = x;
    }

    void SiftUpRobust(item i, std::int32_t x, CDouble epsilon) {
        std::int32_t p = (x == 0) ? -1 : (x - 1) / dh;
        while (p >= 0 && h.items[p]->key - i->key > epsilon) {
            h.items[x] = h.items[p];
            (h.items[x])->h_pos = x;
            x = p;
            p = (p == 0) ? -1 : (p - 1) / dh;
        }
        h.items[x] = i;
        i->h_pos = x;
    }

    void Insert(item i) {
        SiftUp(i, h.size);
        ++(h.size);
    }

    void InsertRobust(item i, CDouble epsilon) {
        SiftUpRobust(i, h.size, epsilon);
        ++(h.size);
    }

    void Delete(item i) {
        item j = h.items[h.size - 1];
        --(h.size);
        if (i != j) {
            if (j->key <= i->key) {
                SiftUp(j, i->h_pos);
            } else {
                SiftDown(j, i->h_pos);
            }
        }
    }

    void DeleteRobust(item i, CDouble epsilon) {
        item j = h.items[h.size - 1];
        --(h.size);
        if (i != j) {
            if (!(j->key - i->key > epsilon)) {
                SiftUpRobust(j, i->h_pos, epsilon);
            } else {
                SiftDownRobust(j, i->h_pos, epsilon);
            }
        }
    }

    item DeleteMin() {
        if (h.size == 0) {
            return (NILA);
        }
        item i = h.items[0];
        Delete(i);
        return (i);
    }

    [[nodiscard]] item GetMin() const {
        if (h.size == 0) {
            return (NILA);
        }
        return (h.items[0]);
    }

    void AllocHeap(std::int32_t k) {
        h.items.resize(k);
        h.max_size = k;
        h.size = 0;
    }

    /**
     * update_subtree ()
     * recursive subtree traversal function, produces a one-way liked list of nodes
     * contained in subtree updates node levels and costs of paths along sub-tree to
     * nodes contained in it.
     */
    void update_subtree(node *root) { // NOLINT(*no-recursion)

        root->level = update_level;
        root->link = upd_nodes;
        upd_nodes = root;
        root->in_list = true;

        if (root->first_child != NILN) {
            ++update_level;
            node *vptr = root->first_child;
            do { // NOLINT(*avoid-do-while)
                vptr->cost_t = root->cost_t + vptr->parent_in->cost;
                vptr->transit_time_t = root->transit_time_t + vptr->parent_in->transit_time;
                update_subtree(vptr);
                vptr = vptr->right_sibling;
            } while (vptr != root->first_child);
            --update_level;
        }
    }

public:
    /**
     * mmcycle ()
     *
     * Determines maximum ratio cycle in a directed graph
     * G = (V, E, c), c: E->IR a "cost" function on the
     * edges, alternatively called "length" or "weight".
     *
     * Note that it is assumed that every cycle has transit time  > 0 !
     *
     * Note that it has been observed that the algorithm can behave non-deterministically
     * across compilers, when the graph has multiple equally critical cycles
     * Due to the order in which floating point calculations are scheduled, the
     * algorithm's control flow may follow different paths and lead to different
     * critical cycles as the output.
     * The function mmcycle_robust implements a version of the algorithm that is
     * deterministic across compilers. It may be a little slower.
     *
     * Input parameter:
     * ---------------
     * gr     - graph structure with incidence lists of
     *          incoming and outgoing edges for each node
     *
     * Output parameters:
     * -----------------
     * lambda - maximum cycle ratio
     *
     * cycle  - pointers to arcs on maximum ratio cycle,
     *          ordered in array from top to bottom with
     *          respect to subsequent arcs on cycle
     *
     * len    - number of elements of "cycle"
     *
     * If cycle or len is a nullptr-pointer, then these parameters
     * are not assigned a value.
     *
     * Reference
     * ---------
     * N.E. Young, R. E. Tarjan, J. B. Orlin: "Faster Parametric
     * Shortest Path and Minimum-Balance Algorithms", Networks
     * 21 (1991), 205-221
     *
     *
     * Sketch of algorithm:
     * -------------------
     *
     * 1) Introduce an extra node s and an emanating arc with
     * cost zero to each node of the graph, let V' and E'
     * be the extended node and edge sets respectively and
     * G' = (V',E',c).
     *
     * 2) Let lambda = lambda_ini = sum (abs(c(e)) + 1.0 over
     * all edges e of E';
     *
     * 3) Introduce edge costs c_lambda(e) = c(e) - lambda for
     * all edges e of E', let G'_lambda = (V', E', c_lambda),
     * (all edge costs c_lambda(e) are positive);
     *
     * 4) Set up tree T_lambda of shortest paths from s to all
     * other nodes in G'_lambda, i.e. with respect to edge
     * costs c_lambda, this tree consists of all edges
     * emanating from node s, for each node v retain cost ct(v)
     * of tree path in G' and the number of edges on the path
     * from s to v, i.e. the tree level of v;
     *
     * 5) For all edges e = (u,v) in G - T_lambda compute edge
     * (pivot) key pk as
     *
     * pk(e) = (ct(u) + c(u,v) - ct(v))/(lev(u) + 1 - lev(v))
     *
     * if lev(u) + 1 > lev(v),
     * otherwise set pk(e) = -infinite
     *
     * For each node v select an incoming edge whose key is
     * maximum among all incoming edges and assign it to v
     * as vertex key.
     *
     * 6) Determine an edge e_min = (u',v') such that pk(u',v')
     * is maximum among all edge keys by way of vertex keys -
     * such a key always exists at this point - let lambda =
     * pk(u', v');
     *
     * 7) If v' is an ancestor of u' in the tree, inserting edge
     * (u',v') into it creates a cycle with mean value lambda,
     * this is a maximum mean cycle, return cycle and lambda;
     *
     * 8) Delete edge (u'',v') from the tree and insert edge (u',v')
     * instead, for all nodes v of subtree rooted at v', update
     * lev (v) and ct(v) by adding lev(u') + 1 - lev (v') and
     * ct(u') + c(u',v') - ct(v') respectively, compute edge key
     * for edge (u'',v'), remove edge key of (u',v') and update
     * edge keys for all edges (u,v) with exactly one of its
     * incident nodes u or v in subtree rooted at v' (such edges
     * are the final ones on new shortest paths to destination
     * node v), determine new vertex key for each vertex that
     * has an incoming edge whose key has been changed;
     *
     * 9) goto (6);
     */
    void mmcycle(graph &gr, CDouble *lambda, std::vector<const arc *> *cycle) { // NOLINT(*cognitive-complexity)

        // set up initial tree
        node *s_ptr = gr.vs;
        s_ptr->in_list = false;
        s_ptr->cost_t = 0.0L;
        s_ptr->transit_time_t = 0.0L;
        s_ptr->level = 0L;
        s_ptr->parent_in = NILA;
        s_ptr->left_sibling = s_ptr;
        s_ptr->right_sibling = s_ptr;
        arc *a_ptr = s_ptr->first_arc_out;
        node *v_ptr = a_ptr->head;
        s_ptr->first_child = v_ptr;
        while (a_ptr != NILA) {
            node *w_ptr = a_ptr->head;
            w_ptr->cost_t = 0.0L;
            w_ptr->transit_time_t = 0.0L;
            w_ptr->level = 1L;
            w_ptr->parent_in = a_ptr;
            w_ptr->first_child = NILN;
            w_ptr->left_sibling = v_ptr;
            v_ptr->right_sibling = w_ptr;
            w_ptr->in_list = false;
            a_ptr->in_tree = true;
            a_ptr->h_pos = -1; // arc does not go into heap
            a_ptr->key = DBL_MAX;
            v_ptr = w_ptr;
            a_ptr = a_ptr->next_out;
        }
        s_ptr->first_child->left_sibling = v_ptr;
        v_ptr->right_sibling = s_ptr->first_child;

        // determine upper bound on lambda, can be used as 'infinity'
        // adds up all costs, and divide by smallest non-zero transit time
        // requires that there are no cycles with zero transit time!
        // Also, determine epsilon value on transit times, cost and cost/time ratios
        // as a constant fraction of the smallest observed values
        CDouble total_cost_plus_one = 1.0;
        CDouble min_transit_time = DBL_MAX;
        for (auto &a : gr.arcs) {
            // add costs to total cost
            total_cost_plus_one += fabs(a.cost);
            // keep min of transit times
            if (a.transit_time > 0.0L) {
                min_transit_time = (std::min)(a.transit_time, min_transit_time);
            }
        }
        CDouble infty = total_cost_plus_one / min_transit_time;

        // initial keys of non tree edges are equal to arc costs
        for (auto &a : gr.arcs) {
            if (a.transit_time > 0) {
                a.key = a.cost / a.transit_time;
            } else {
                a.key = infty;
            }
            a.in_tree = false;
        }

        // d-heap used for maintenance of vertex keys
        AllocHeap(gr.n_nodes);

        // compute initial vertex keys
        for (auto &v : gr.nodes) {
            CDouble min = DBL_MAX;
            arc *vmin_a_ptr = NILA;
            a_ptr = v.first_arc_in;
            while (a_ptr != NILA) {
                if (!a_ptr->in_tree && a_ptr->key < min) {
                    min = a_ptr->key;
                    vmin_a_ptr = a_ptr;
                }
                a_ptr = a_ptr->next_in;
            }
            v.v_key = vmin_a_ptr;
            if (vmin_a_ptr != NILA) {
                Insert(vmin_a_ptr);
            }
        }
        gr.vs->v_key = NILA;
        arc *min_a_ptr = nullptr;

        while (true) {
            min_a_ptr = GetMin();
            // should be element on heap
            assert((min_a_ptr != NILA));

            *lambda = min_a_ptr->key;
            if (*lambda >= infty) {
                min_a_ptr = NILA;
                break; // input graph is acyclic in this case
            }

            node *uptr = min_a_ptr->tail;
            v_ptr = min_a_ptr->head;

            /* check if *vptr is an ancestor of *uptr in tree */

            bool foundCycle = false;
            arc *par_a_ptr = uptr->parent_in;

            // MG: below is a fix, not in the original algorithm, since the original algorithm
            // does not seem to anticipate the possibility of self-edges in the graph.
            if (uptr == v_ptr) {
                // statement below was added to make the calculation of the critical cycle work
                // correctly for critical self-edges.
                uptr->parent_in = min_a_ptr;
                break;
            }

            while (par_a_ptr != NILA) {
                if (par_a_ptr->tail == v_ptr) {
                    foundCycle = true;
                    break;
                }
                par_a_ptr = par_a_ptr->tail->parent_in;
            }
            if (foundCycle) {
                break;
            }

            // it is not, remove edge (parent(v),v) from tree and make edge (u,v) a
            // tree edge instead
            par_a_ptr = v_ptr->parent_in;
            par_a_ptr->in_tree = false;
            min_a_ptr->in_tree = true;

            v_ptr->cost_t = uptr->cost_t + min_a_ptr->cost;
            v_ptr->transit_time_t = uptr->transit_time_t + min_a_ptr->transit_time;
            node *w_ptr = par_a_ptr->tail;

            // delete link (w_ptr,v_ptr) from tree
            if (v_ptr->right_sibling == v_ptr) {
                w_ptr->first_child = NILN;
            } else {
                v_ptr->right_sibling->left_sibling = v_ptr->left_sibling;
                v_ptr->left_sibling->right_sibling = v_ptr->right_sibling;
                if (w_ptr->first_child == v_ptr) {
                    w_ptr->first_child = v_ptr->right_sibling;
                }
            }

            // insert link (uptr,vptr) into tree
            v_ptr->parent_in = min_a_ptr;
            if (uptr->first_child == NILN) {
                uptr->first_child = v_ptr;
                v_ptr->right_sibling = v_ptr;
                v_ptr->left_sibling = v_ptr;
            } else {
                v_ptr->right_sibling = uptr->first_child->right_sibling;
                uptr->first_child->right_sibling->left_sibling = v_ptr;
                v_ptr->left_sibling = uptr->first_child;
                uptr->first_child->right_sibling = v_ptr;
            }

            // subtree rooted at v has u as parent node now, update level and cost
            // entries of its nodes accordingly and produce list of nodes contained
            // in subtree
            upd_nodes = NILN;
            update_level = uptr->level + 1;

            update_subtree(v_ptr);
            // now compute new keys of arcs into nodes that have acquired a new
            // shortest path, such arcs have head or tail in the subtree rooted at
            // "vptr", update vertex keys at the same time, nodes to be checked are
            // those contained in the subtree and the ones pointed to by arcs
            // emanating from nodes in the subtree
            v_ptr = upd_nodes;
            while (v_ptr != NILN) {
                if (v_ptr->v_key != NILA) {
                    Delete(v_ptr->v_key);
                }
                CDouble min = DBL_MAX;
                arc *vmin_a_ptr = NILA;
                a_ptr = v_ptr->first_arc_in;
                while (a_ptr != NILA) {
                    if (!a_ptr->in_tree) {
                        uptr = a_ptr->tail;
                        a_ptr->key =
                                uptr->transit_time_t + a_ptr->transit_time > v_ptr->transit_time_t
                                        ? (uptr->cost_t + a_ptr->cost
                                                               - v_ptr->cost_t)
                                                  / (uptr->transit_time_t
                                                                         + a_ptr->transit_time
                                                                         - v_ptr->transit_time_t)
                                        : infty;

                        if (a_ptr->key < min) {
                            min = a_ptr->key;
                            vmin_a_ptr = a_ptr;
                        }
                    }
                    a_ptr = a_ptr->next_in;
                }
                if (vmin_a_ptr != NILA) {
                    Insert(vmin_a_ptr);
                }
                v_ptr->v_key = vmin_a_ptr;

                v_ptr = v_ptr->link;
            }

            min_a_ptr->key = DBL_MAX;

            // now update keys of arcs from nodes in subtree to nodes not contained
            // in subtree and update vertex keys for the latter if necessary
            v_ptr = upd_nodes;
            while (v_ptr != NILN) {
                a_ptr = v_ptr->first_arc_out;
                while (a_ptr != NILA) {
                    if (!a_ptr->in_tree && !a_ptr->head->in_list) {
                        w_ptr = a_ptr->head;
                        CDouble a_key =
                                v_ptr->transit_time_t + a_ptr->transit_time > w_ptr->transit_time_t
                                        ? (v_ptr->cost_t + a_ptr->cost
                                                               - w_ptr->cost_t)
                                                  / (v_ptr->transit_time_t
                                                                         + a_ptr->transit_time
                                                                         - w_ptr->transit_time_t)
                                        : infty;

                        if (a_key < w_ptr->v_key->key) {
                            Delete(w_ptr->v_key);
                            a_ptr->key = a_key;
                            Insert(a_ptr);
                            w_ptr->v_key = a_ptr;
                        } else {
                            a_ptr->key = a_key;
                        }
                    }
                    a_ptr = a_ptr->next_out;
                }
                v_ptr = v_ptr->link;
            }

            v_ptr = upd_nodes;
            while (v_ptr != NILN) {
                v_ptr->in_list = false;
                v_ptr = v_ptr->link;
            }
        }

        if (cycle != nullptr) {
            // *cycle = std::make_shared<std::vector<const arc *>>();
            if (min_a_ptr != NILA) {
                (*cycle).push_back(min_a_ptr);
                a_ptr = min_a_ptr->tail->parent_in;
                while (a_ptr->head != min_a_ptr->head) {
                    (*cycle).push_back(a_ptr);
                    a_ptr = a_ptr->tail->parent_in;
                }
            }
        }
    }

    /**
     * mmcycle_robust ()
     *
     * Determines maximum ratio cycle in a directed graph in a robust way
     * G = (V, E, c), c: E->IR a "cost" function on the
     * edges, alternatively called "length" or "weight".
     * It has been observed that the mmcycle algorithm can behave non - deterministically
     * across compilers, when the graph has multiple equally critical cycles
     * Due to the order in which floating point calculations are scheduled, the
     * algorithm's control flow may follow different paths and lead to different
     * critical cycles as the output.
     * This function mmcycle_robust implements a version of the algorithm that is
     * deterministic across compilers. It may be a little slower.
     * For more info about the algorithm, see the mmcycle function.
     *
     * TODO: see if the algorithms can be unified to remove duplicate code
     **/

    void mmcycle_robust(graph &gr, CDouble *lambda, std::vector<const arc *> *cycle) { // NOLINT(*cognitive-complexity)
        const CDouble MCR_EPSILON_RATIO = 1.0e-8L;

        // set up initial tree
        node *s_ptr = gr.vs;
        s_ptr->in_list = false;
        s_ptr->cost_t = 0.0L;
        s_ptr->transit_time_t = 0.0L;
        s_ptr->level = 0L;
        s_ptr->parent_in = NILA;
        s_ptr->left_sibling = s_ptr;
        s_ptr->right_sibling = s_ptr;
        arc *a_ptr = s_ptr->first_arc_out;
        node *v_ptr = a_ptr->head;
        s_ptr->first_child = v_ptr;
        while (a_ptr != NILA) {
            node *w_ptr = a_ptr->head;
            w_ptr->cost_t = 0.0L;
            w_ptr->transit_time_t = 0.0L;
            w_ptr->level = 1L;
            w_ptr->parent_in = a_ptr;
            w_ptr->first_child = NILN;
            w_ptr->left_sibling = v_ptr;
            v_ptr->right_sibling = w_ptr;
            w_ptr->in_list = false;
            a_ptr->in_tree = true;
            a_ptr->h_pos = -1; // arc does not go into heap
            a_ptr->key = DBL_MAX;
            v_ptr = w_ptr;
            a_ptr = a_ptr->next_out;
        }
        s_ptr->first_child->left_sibling = v_ptr;
        v_ptr->right_sibling = s_ptr->first_child;

        // determine upper bound on lambda, can be used as 'infinity'
        // adds up all costs, and divide by smallest non-zero transit time
        // requires that there are no cycles with zero transit time!
        // Also, determine epsilon value on transit times, cost and cost/time ratios
        // as a constant fraction of the smallest observed values
        CDouble total_cost_plus_one = 1.0;
        CDouble min_transit_time = DBL_MAX;
        CDouble min_cost = DBL_MAX;
        for (const auto &a : gr.arcs) {
            // add costs to total cost
            total_cost_plus_one += fabs(a.cost);
            // keep min of transit times
            if (a.transit_time > 0.0L) {
                min_transit_time = (std::min)(a.transit_time, min_transit_time);
            }
            // keep min of costs
            if (a.cost > 0.0L) {
                min_cost = (std::min)(a.cost, min_cost);
            }
        }
        CDouble infty = total_cost_plus_one / min_transit_time;
        CDouble epsilon_transit_time = MCR_EPSILON_RATIO * min_transit_time;
        CDouble epsilon_cost_time_ratio = MCR_EPSILON_RATIO * (min_cost / min_transit_time);

        // initial keys of non tree edges are equal to arc costs
        for (auto &a : gr.arcs) {
            if (a.transit_time > epsilon_transit_time) {
                a.key = a.cost / a.transit_time;
            } else {
                a.key = infty;
            }
            a.in_tree = false;
        }

        // d-heap used for maintenance of vertex keys
        // d_heap h;
        AllocHeap(gr.n_nodes);

        // compute initial vertex keys
        for (auto &v : gr.nodes) {
            CDouble min = DBL_MAX;
            arc *vmin_a_ptr = NILA;
            a_ptr = v.first_arc_in;
            while (a_ptr != NILA) {
                if (!a_ptr->in_tree && (min - a_ptr->key > epsilon_cost_time_ratio)) {
                    min = a_ptr->key;
                    vmin_a_ptr = a_ptr;
                }
                a_ptr = a_ptr->next_in;
            }
            v.v_key = vmin_a_ptr;
            if (vmin_a_ptr != NILA) {
                InsertRobust(vmin_a_ptr, epsilon_cost_time_ratio);
            }
        }
        gr.vs->v_key = NILA;

        arc *min_a_ptr = nullptr;

        while (true) {
            min_a_ptr = GetMin();
            // there must be an element on the heap
            assert(min_a_ptr != NILA);

            *lambda = min_a_ptr->key;
            if (*lambda >= infty) {
                min_a_ptr = NILA;
                break; // input graph is acyclic in this case
            }

            node *uptr = min_a_ptr->tail;
            v_ptr = min_a_ptr->head;

            /* check if *vptr is an ancestor of *uptr in tree */

            bool foundCycle = false;
            arc *par_a_ptr = uptr->parent_in;

            // MG: below is a fix, not in the original algorithm, since the original algorithm
            // does not seem to anticipate the possibility of self-edges in the graph.
            if (uptr == v_ptr) {
                // statement below was added to make the calculation of the critical cycle work
                // correctly for critical self-edges.
                uptr->parent_in = min_a_ptr;
                break;
            }

            while (par_a_ptr != NILA) {
                if (par_a_ptr->tail == v_ptr) {
                    foundCycle = true;
                    break;
                }
                par_a_ptr = par_a_ptr->tail->parent_in;
            }
            if (foundCycle) {
                break;
            }

            // it is not, remove edge (parent(v),v) from tree and make edge (u,v) a
            // tree edge instead
            par_a_ptr = v_ptr->parent_in;
            par_a_ptr->in_tree = false;
            min_a_ptr->in_tree = true;

            v_ptr->cost_t = uptr->cost_t + min_a_ptr->cost;
            v_ptr->transit_time_t = uptr->transit_time_t + min_a_ptr->transit_time;
            node *w_ptr = par_a_ptr->tail;

            // delete link (w_ptr,v_ptr) from tree
            if (v_ptr->right_sibling == v_ptr) {
                w_ptr->first_child = NILN;
            } else {
                v_ptr->right_sibling->left_sibling = v_ptr->left_sibling;
                v_ptr->left_sibling->right_sibling = v_ptr->right_sibling;
                if (w_ptr->first_child == v_ptr) {
                    w_ptr->first_child = v_ptr->right_sibling;
                }
            }

            // insert link (uptr,vptr) into tree
            v_ptr->parent_in = min_a_ptr;
            if (uptr->first_child == NILN) {
                uptr->first_child = v_ptr;
                v_ptr->right_sibling = v_ptr;
                v_ptr->left_sibling = v_ptr;
            } else {
                v_ptr->right_sibling = uptr->first_child->right_sibling;
                uptr->first_child->right_sibling->left_sibling = v_ptr;
                v_ptr->left_sibling = uptr->first_child;
                uptr->first_child->right_sibling = v_ptr;
            }

            // subtree rooted at v has u as parent node now, update level and cost
            // entries of its nodes accordingly and produce list of nodes contained
            // in subtree
            upd_nodes = NILN;
            update_level = uptr->level + 1;

            update_subtree(v_ptr);
            // now compute new keys of arcs into nodes that have acquired a new
            // shortest path, such arcs have head or tail in the subtree rooted at
            // "vptr", update vertex keys at the same time, nodes to be checked are
            // those contained in the subtree and the ones pointed to by arcs
            // emanating from nodes in the subtree
            v_ptr = upd_nodes;
            while (v_ptr != NILN) {
                if (v_ptr->v_key != NILA) {
                    DeleteRobust(v_ptr->v_key, epsilon_cost_time_ratio);
                }
                CDouble min = DBL_MAX;
                arc *vmin_a_ptr = NILA;
                a_ptr = v_ptr->first_arc_in;
                while (a_ptr != NILA) {
                    if (!a_ptr->in_tree) {
                        uptr = a_ptr->tail;
                        if (uptr->transit_time_t + a_ptr->transit_time - v_ptr->transit_time_t
                            > epsilon_transit_time) {
                            a_ptr->key = (uptr->cost_t + a_ptr->cost - v_ptr->cost_t)
                                         / (uptr->transit_time_t + a_ptr->transit_time
                                            - v_ptr->transit_time_t);
                        } else {
                            a_ptr->key = infty;
                        }

                        if (min - a_ptr->key > epsilon_cost_time_ratio) {
                            min = a_ptr->key;
                            vmin_a_ptr = a_ptr;
                        }
                    }
                    a_ptr = a_ptr->next_in;
                }
                if (vmin_a_ptr != NILA) {
                    InsertRobust(vmin_a_ptr, epsilon_cost_time_ratio);
                }
                v_ptr->v_key = vmin_a_ptr;
                v_ptr = v_ptr->link;
            }

            min_a_ptr->key = DBL_MAX;

            // now update keys of arcs from nodes in subtree to nodes not contained
            // in subtree and update vertex keys for the latter if necessary
            v_ptr = upd_nodes;
            while (v_ptr != NILN) {
                a_ptr = v_ptr->first_arc_out;
                while (a_ptr != NILA) {
                    if (!a_ptr->in_tree && !a_ptr->head->in_list) {
                        w_ptr = a_ptr->head;
                        CDouble a_key = 0;
                        if (v_ptr->transit_time_t + a_ptr->transit_time - w_ptr->transit_time_t
                            > epsilon_transit_time) {
                            a_key = (v_ptr->cost_t + a_ptr->cost - w_ptr->cost_t)
                                    / (v_ptr->transit_time_t + a_ptr->transit_time
                                       - w_ptr->transit_time_t);
                        } else {
                            a_key = infty;
                        }
                        if (w_ptr->v_key->key - a_key > epsilon_cost_time_ratio) {
                            DeleteRobust(w_ptr->v_key, epsilon_cost_time_ratio);
                            a_ptr->key = a_key;
                            InsertRobust(a_ptr, epsilon_cost_time_ratio);
                            w_ptr->v_key = a_ptr;
                        } else {
                            a_ptr->key = a_key;
                        }
                    }
                    a_ptr = a_ptr->next_out;
                }
                v_ptr = v_ptr->link;
            }

            v_ptr = upd_nodes;
            while (v_ptr != NILN) {
                v_ptr->in_list = false;
                v_ptr = v_ptr->link;
            }
        }

        if (cycle != nullptr) {
            *cycle = std::vector<const arc *>();
            if (min_a_ptr != NILA) {
                cycle->push_back(min_a_ptr);
                a_ptr = min_a_ptr->tail->parent_in;
                while (a_ptr->head != min_a_ptr->head) {
                    cycle->push_back(a_ptr);
                    a_ptr = a_ptr->tail->parent_in;
                }
            }
        }
    }
};

/**
 * convertMCMgraphToYTOgraph ()
 * The function converts a weighted directed graph used in the MCM algorithms
 * to graph input for Young-Tarjan-Orlin's algorithm.
 * It assumes that the id's of the nodes are 0 <= id < number of nodes
 */

void convertMCMgraphToYTOgraph(MCMgraph &g,
                               graph &gr,
                               CDouble (*costFunction)(const MCMedge &e),
                               CDouble (*transit_timeFunction)(const MCMedge &e)) {

    gr.n_nodes = static_cast<int>(g.nrVisibleNodes());
    gr.n_arcs = static_cast<int>(g.nrVisibleEdges());
    // allocate space for the nodes, plus one for the exta source node that will be added
    gr.nodes.resize(gr.n_nodes + 1);
    gr.arcs.resize(gr.n_arcs + gr.n_nodes);

    // create nodes
    // keep an index of node id's
    std::map<CId, CId> nodeIndex;
    uint ind = 0;
    for (const auto &n : g.getNodes()) {
        nodeIndex[n.id] = ind;
        node &x = (gr.nodes)[ind];
        x.id = static_cast<int>(n.id + 1);
        x.first_arc_out = nullptr;
        x.first_arc_in = nullptr;

        // Next
        ind++;
    }

    // create arcs
    int aidx = 0;
    for (const auto &e : g.getEdges()) {
        MCMnode *u = e.src;
        MCMnode *v = e.dst;

        arc &a = gr.arcs[aidx];

        a.tail = &(gr.nodes[nodeIndex[u->id]]);
        a.head = &(gr.nodes[nodeIndex[v->id]]);
        a.cost = (*costFunction)(e);
        a.transit_time = (*transit_timeFunction)(e);
        a.next_out = a.tail->first_arc_out;
        a.tail->first_arc_out = &a;
        a.next_in = a.head->first_arc_in;
        a.head->first_arc_in = &a;
        a.mcmEdge = &e;
        // Next
        aidx++;
    }

    // Create a source node which has an edge to all nodes
    gr.vs = &(gr.nodes[gr.n_nodes]);
    gr.vs->id = 0;
    gr.vs->first_arc_out = nullptr;
    gr.vs->first_arc_in = nullptr;
    for (int i = 0; i < gr.n_nodes; i++) {
        arc &a = gr.arcs[aidx];
        a.cost = 0;
        a.transit_time = 0.0;
        a.tail = gr.vs;
        a.head = &(gr.nodes[i]);
        a.next_out = gr.vs->first_arc_out;
        gr.vs->first_arc_out = &a;
        a.next_in = a.head->first_arc_in;
        a.head->first_arc_in = &a;

        // Next
        aidx++;
    }

        // // Print the MCM graph
        // cerr << "#nodes: " << g->nodes.size() << endl;
        // cerr << "#edges: " << g->edges.size() << endl;
        // cerr << "edge: (u, v, w, d)" << endl;
        // for (auto iter = g->edges.begin();
        //      iter != g->edges.end(); iter++)
        // {
        //     MCMedge *e = *iter;

        //     if (!e->visible)
        //         continue;

        //     cerr << "(" << e->src->id;
        //     cerr << ", " << e->dst->id;
        //     cerr << ", " << e->w;
        //     cerr << ", " << e->d;
        //     cerr << ")" << endl;
        // }
        // cerr << endl;

        // // Print the graph used in the YTO algorithm
        // x = gr.nodes;
        // for (int i = 0; i <= gr.n_nodes; i++)
        // {
        //     cerr << "vertex: " << x->id << endl;

        //     for (arc *a = x->first_arc_in; a != nullptr; a = a->next_in)
        //     {
        //         cerr << "   edge from: " << a->tail->id;
        //         cerr << " (weight: " << a->cost << ")" << endl;
        //     }
        //     for (arc *a = x->first_arc_out; a != nullptr; a = a->next_out)
        //     {
        //         cerr << "   edge to:   " << a->head->id;
        //         cerr << " (weight: " << a->cost << ")" << endl;
        //     }

        //     x++;
        // }

    }

namespace {

/**
 * constOne ()
 * The function returns the unit cost associated with an edge.
 */
CDouble constOne(const MCMedge & /*e*/) { return 1.0; }

} // namespace

/**
 * getWeight ()
 * The function returns the weight associated with an edge.
 */
CDouble getWeight(const MCMedge &e) { return e.w; }

/**
 * getDelay ()
 * The function returns the delay associated with an edge.
 */
CDouble getDelay(const MCMedge &e) { return e.d; }

/**
 * maxCycleMeanAndCriticalCycleYoungTarjanOrlin ()
 * The function computes the maximum cycle mean of edge weight of
 * an MCMgraph using Young-Tarjan-Orlin's algorithm.
 * It returns both the MCM and a critical cycle
 * The critical cycle is only returned if cycle is not nullptr. Then *cycle points
 * to an array of *MCMEdges of the critical cycle.
 *
 * Note that the following assumed are made about the MCMgraph
 * 1. it is assumed that all nodes in the graph are 'visible'
 * 2. it is assumed that the node have id's ranging from 0 up to the number of nodes.
 * 3. it is assumed that cycles have a weight > 0 !
 */
CDouble maxCycleMeanAndCriticalCycleYoungTarjanOrlin(MCMgraph &mcmGraph,
                                                     std::vector<const MCMedge *> *cycle) {
    graph ytoGraph;

    // Convert the graph to an input graph for the YTO algorithm
    convertMCMgraphToYTOgraph(mcmGraph, ytoGraph, constOne, getWeight);

    AlgYTO alg(ytoGraph);

    CDouble min_cr = 0;
    if (cycle != nullptr) {
        std::vector<const arc *> ytoCycle;

        // Find maximum cycle mean
        std::int32_t ytoCycLen = 0;
        alg.mmcycle_robust(ytoGraph, &min_cr, &ytoCycle);

        size_t len = ytoCycle.size();

        *cycle = std::vector<const MCMedge *>(len);
        for (uint i = 0; i < len; i++) {
            (*cycle).at(i) = ytoCycle.at(i)->mcmEdge;
        }

    } else {
        // Find maximum cycle mean without cycle
        alg.mmcycle_robust(ytoGraph, &min_cr, nullptr);
    }

    // Cleanup

    return 1.0 / min_cr;
}

/**
 * mcmYoungTarjanOrlin ()
 * The function computes the maximum cycle mean of edge weight per edge of
 * an MCMgraph using Young-Tarjan-Orlin's algorithm.
 */

CDouble maxCycleMeanYoungTarjanOrlin(MCMgraph &mcmGraph) {
    return maxCycleMeanAndCriticalCycleYoungTarjanOrlin(mcmGraph, nullptr);
}

/**
 * maxCycleRatioAndCriticalCycleYoungTarjanOrlin ()
 * The function computes the maximum cycle ratio of edge weight over delay of
 * an MCMgraph using Young-Tarjan-Orlin's algorithm.
 * It returns both the MCR and a critical cycle
 * The critical cycle is only returned if cycle is not nullptr. Then *cycle points
 * to a vector of MCMEdges of the critical cycle.
 */

CDouble maxCycleRatioAndCriticalCycleYoungTarjanOrlin(MCMgraph &mcmGraph,
                                                      std::vector<const MCMedge *> *cycle) {
    graph ytoGraph;

    // catch special case when the graph has no edges
    if (mcmGraph.nrVisibleEdges() == 0) {
        return 0.0;
    }

    // Convert the graph to an input graph for the YTO algorithm
    convertMCMgraphToYTOgraph(mcmGraph, ytoGraph, getDelay, getWeight);

    AlgYTO alg(ytoGraph);

    CDouble min_cr = 0;
    if (cycle != nullptr) {

        std::vector<const arc *> ytoCycle;

        // Find maximum cycle ratio
        std::int32_t ytoCycLen = 0;
        alg.mmcycle_robust(ytoGraph, &min_cr, &ytoCycle);

        *cycle = std::vector<const MCMedge *>(ytoCycle.size());

        // note that mmcycle returns the critical cycle following edges backwards
        // therefore reverse the order of the edges.
        size_t len = ytoCycle.size();
        for (uint i = 0; i < len; i++) {
            cycle->at(i) = ytoCycle[len - 1 - i]->mcmEdge;
        }

    } else {
        // Find maximum cycle ratio without cycle
        alg.mmcycle_robust(ytoGraph, &min_cr, nullptr);
    }

    return 1.0 / min_cr;
}

/**
 * maxCycleRatioYoungTarjanOrlin ()
 * The function computes the maximum cycle ratio of edge weight over delay of
 * an MCMgraph using Young-Tarjan-Orlin's algorithm.
 */

CDouble maxCycleRatioYoungTarjanOrlin(MCMgraph &mcmGraph) {
    return maxCycleRatioAndCriticalCycleYoungTarjanOrlin(mcmGraph, nullptr);
}

/**
 * minCycleRatioAndCriticalCycleYoungTarjanOrlin ()
 * The function computes the minimum cycle ratio of edge weight over delay of
 * an MCMgraph using Young-Tarjan-Orlin's algorithm.
 * It returns both the MCR and a critical cycle
 * The critical cycle is only returned if cycle is not nullptr. Then *cycle points
 * to an array of MCMEdges of the critical cycle.
 */

CDouble minCycleRatioAndCriticalCycleYoungTarjanOrlin(MCMgraph &mcmGraph,
                                                      std::vector<const MCMedge *> *cycle) {
    graph ytoGraph;

    // Convert the graph to an input graph for the YTO algorithm
    convertMCMgraphToYTOgraph(mcmGraph, ytoGraph, getWeight, getDelay);

    AlgYTO alg(ytoGraph);

    CDouble min_cr = 0;
    if (cycle != nullptr) {

        std::vector<const arc *> ytoCycle;

        // Find minimum cycle ratio
        std::int32_t ytoCycLen = 0;
        alg.mmcycle_robust(ytoGraph, &min_cr, &ytoCycle);

        size_t len = ytoCycle.size();
        *cycle = std::vector<const MCMedge *>(len);

        for (uint i = 0; i < len; i++) {
            cycle->at(i) = ytoCycle[i]->mcmEdge;
        }

    } else {
        // Find minimum cycle ratio without cycle
        alg.mmcycle_robust(ytoGraph, &min_cr, nullptr);
    }

    return min_cr;
}

/**
 * minCycleRatioYoungTarjanOrlin ()
 * The function computes the minimum cycle ratio of edge weight over delay of
 * an MCMgraph using Young-Tarjan-Orlin's algorithm.
 */

CDouble minCycleRatioYoungTarjanOrlin(MCMgraph &mcmGraph) {
    return minCycleRatioAndCriticalCycleYoungTarjanOrlin(mcmGraph, nullptr);
}

} // namespace MaxPlus::Graphs
