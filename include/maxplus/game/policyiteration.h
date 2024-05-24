/*
 *  Eindhoven University of Technology
 *  Eindhoven, The Netherlands
 *  Dept. of Electrical Engineering
 *  Electronics Systems Group
 *  Model Based Design Lab (https://computationalmodeling.info/)
 *
 *  Name            :   policyiteration.h
 *
 *  Author          :   Bram van der Sanden (b.v.d.sanden@tue.nl)
 *
 *  Date            :   June 24, 2017
 *
 *  Function        :   policy iteration algorithm to solve ratio games
 *
 *  History         :
 *      24-06-17    :   Initial version.
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

#ifndef BASE_MAXPLUS_GAME_POLICYITERATION_H
#define BASE_MAXPLUS_GAME_POLICYITERATION_H

#include "maxplus/base/fsm/fsm.h"
#include "ratiogame.h"
#include "strategyvector.h"
#include <algorithm> // std::max
#include <cmath>     // -Infinity
#include <stack>
#include <utility> // std::pair

namespace MaxPlus {

using namespace ::FSM::Labeled;

/**
 * Policy Iteration Algorithm to solve ratio games.
 * <p>
 * The algorithm start with random strategies for both players, and then starts
 * improving the strategies until convergence. When the algorithm terminates,
 * both strategies are optimal.
 * <p>
 * The algorithm has been adapted from the MDG algorithm by Chaloupka, see also
 * Chaloupka, 2011, Algorithms for Mean-Payoff and Energy Games (PhD thesis).
 *
 * @author Bram van der Sanden
 */
template <typename SL, typename EL> class PolicyIteration {
    const CDouble EPSILON = 10e-5;

public:
    ~PolicyIteration() = default;
    ;

    struct PolicyIterationResult {
        std::map<const State<SL, EL> *, CDouble> values;
        StrategyVector<SL, EL> strategy;
    };

    /**
     * Find the values of the vertices and an optimal strategy for the given
     * ratio game.
     *
     * @tparam SL state label type
     * @tparam EL edge label type
     * @param game game graph
     * @return
     */
    PolicyIterationResult solve(RatioGame<SL, EL> &graph) { return solve(graph, EPSILON); }

    /**
     * Find the values of the vertices and an optimal strategy for the given
     * ratio game.
     *
     * @tparam SL state label type
     * @tparam EL edge label type
     * @param game game graph
     * @param epsilon maximum absolute error between two values when they are considered equal
     * @return
     */
    PolicyIterationResult solve(RatioGame<SL, EL> &game, CDouble epsilon) {
        if (!checkEachStateHasSuccessor(game)) {
            throw std::runtime_error(
                    "Input game graph is not valid. Some states have no successor.");
        }
        // Initialize arbitrary positional strategies.
        std::shared_ptr<StrategyVector<SL, EL>> initialStrategy =
                std::make_shared<StrategyVector<SL, EL>>();
        initialStrategy->initializeRandomStrategy(game);

        return policyIteration(game, initialStrategy, epsilon);
    }

private:
    /**
     * Policy iteration algorithm to find the value of each state and the strategy.
     */
    PolicyIterationResult policyIteration(RatioGame<SL, EL> &game,
                                          std::shared_ptr<StrategyVector<SL, EL>> initialStrategy,
                                          CDouble epsilon) {
        const SetOfStates<SL, EL> &states = game.getStates();

        bool improvement = true;

        // Initialize distance vector.
        std::map<const State<SL, EL> *, CDouble> distVector =
                initializeVector(states, static_cast<CDouble>(INFINITY));
        std::map<const State<SL, EL> *, CDouble> dw2vector =
                initializeVector(states, static_cast<CDouble>(INFINITY));

        // Initial ratio vector.
        std::map<const State<SL, EL> *, CDouble> ratioVector =
                initializeVector(states, static_cast<CDouble>(-INFINITY));

        // Initialize arbitrary positional strategies.
        StrategyVector<SL, EL> currentStrategy = *initialStrategy;

        // Initialize state ids.
        std::map<const State<SL, EL> *, CDouble> stateIds;
        int cid = 0;
        for (auto &it : states) {
            auto &si = *(it.second);
            // Source vertex.
            auto &state = dynamic_cast<State<SL, EL> &>(si);
            stateIds[&state] = cid;
            cid++;
        }

        while (improvement) {
            improvement = false;

            // Improve the strategy of player 1.
            Player1Result result = improveStrategyPlayer1(
                    game, currentStrategy, distVector, ratioVector, dw2vector, stateIds, epsilon);

            distVector = result.d_i_t;
            ratioVector = result.r_i_t;
            currentStrategy = result.s_i_t;
            dw2vector = result.dw2_i_t;

            // Improve the strategy of player 0, just one iteration.
            std::set<StateRef<SL, EL>> &states = game.getV0();
            for (auto &si : states) {
                // Source vertex.
                auto v = dynamic_cast<StateRef<SL, EL>>(si);
                // Outgoing edges.
                auto es = dynamic_cast<const FSM::Abstract::SetOfEdgeRefs &>(v->getOutgoingEdges());
                for (const auto &ei : es) {
                    auto e = dynamic_cast<EdgeRef<SL, EL>>(ei);

                    const auto &u = dynamic_cast<const StateRef<SL, EL>>(e->getDestination());

                    CDouble mw = ratioVector[u];
                    CDouble w1 = static_cast<CDouble>(game.getWeight1(e));
                    CDouble w2 = static_cast<CDouble>(game.getWeight2(e));

                    CDouble reweighted = w1 - mw * w2;

                    CDouble w2sum_v = dw2vector[v];
                    CDouble w2sum_u = dw2vector[u] + w2;

                    CDouble delta = std::max(w2sum_v, w2sum_u) * epsilon;

                    CDouble dv = distVector[v];
                    CDouble du = distVector[u] + reweighted;

                    // Player 0 wants to maximize the ratio.
                    if (lessThan(ratioVector[v], ratioVector[u], epsilon)
                        || (equalTo(ratioVector[v], ratioVector[u], epsilon)
                            && (lessThan(dv, du, 2.0 * delta)))) {
                        currentStrategy.setSuccessor(v, u);
                        improvement = true;
                    }
                }
            }
        }

        PolicyIterationResult result = PolicyIterationResult();
        result.strategy = *initialStrategy;
        result.values = ratioVector;
        return result;
    }

    struct Player1Result {
        std::map<const State<SL, EL> *, CDouble> d_i_t;
        std::map<const State<SL, EL> *, CDouble> r_i_t;
        StrategyVector<SL, EL> s_i_t;
        std::map<const State<SL, EL> *, CDouble> dw2_i_t;
    };

    /**
     * Improve the strategy of player 1 until the strategy is optimal against
     * the current strategy of player 0. This procedure has been adapted from
     * MDG in (Chaloupka, 2011).
     *
     * @param game            current ratio game
     * @param currentStrategy current strategy of both players
     * @param d_prev          previous distance vector
     * @param r_prev          previous ratio vector
     * @param dw2_prev        previous distance w.r.t. weights w2 vector
     * @param stateIds        map with unique id for each state
     * @param epsilon         epsilon value for equality on real numbers
     * @return new distance and ratio vectors, and a new strategy vector with an
     * updated strategy for player 1
     */
    Player1Result improveStrategyPlayer1(RatioGame<SL, EL> &game,
                                         StrategyVector<SL, EL> &currentStrategy,
                                         std::map<const State<SL, EL> *, CDouble> &d_prev,
                                         std::map<const State<SL, EL> *, CDouble> &r_prev,
                                         std::map<const State<SL, EL> *, CDouble> &dw2_prev,
                                         std::map<const State<SL, EL> *, CDouble> &stateIds,
                                         CDouble epsilon) {

        // Start strategy improvement of Player 1.
        bool improvement = true;
        std::map<const State<SL, EL> *, CDouble> d_i_t(d_prev);
        std::map<const State<SL, EL> *, CDouble> r_i_t(r_prev);
        std::map<const State<SL, EL> *, CDouble> dw2_i_t(dw2_prev);
        std::shared_ptr<StrategyVector<SL, EL>> s_i_t =
                std::make_shared<StrategyVector<SL, EL>>(currentStrategy);

        while (improvement) {
            improvement = false;
            // Evaluate the current strategies of both players.
            StrategyEvaluation evalResult =
                    evaluateStrategy(game, s_i_t, d_prev, r_prev, dw2_prev, stateIds, epsilon);

            // Update the current vectors.
            d_i_t = evalResult.d_i_t;
            r_i_t = evalResult.r_i_t;
            dw2_i_t = evalResult.dw2_i_t;

            std::set<StateRef<SL, EL>> &states = game.getV1();
            for (auto &si : states) {
                // Source vertex.
                auto v = dynamic_cast<StateRef<SL, EL>>(si);

                // Outgoing edges.
                const auto &es =
                        dynamic_cast<const FSM::Abstract::SetOfEdgeRefs &>(v->getOutgoingEdges());
                for (const auto &ei : es) {
                    auto e = dynamic_cast<EdgeRef<SL, EL>>(ei);

                    auto u = dynamic_cast<const StateRef<SL, EL>>(e->getDestination());

                    CDouble cycleRatio = r_i_t[u];
                    CDouble w1 = static_cast<CDouble>(game.getWeight1(e));
                    CDouble w2 = static_cast<CDouble>(game.getWeight2(e));

                    CDouble reweighted = w1 - cycleRatio * w2;

                    CDouble w2sum_v = dw2_i_t[v];
                    CDouble w2sum_u = dw2_i_t[u] + w2;

                    CDouble delta = std::max(w2sum_v, w2sum_u) * epsilon;

                    CDouble dv = d_i_t[v];
                    CDouble du = d_i_t[u] + reweighted;

                    if (greaterThan(r_i_t[v], r_i_t[u], epsilon)
                        || (equalTo(r_i_t[v], r_i_t[u], epsilon)
                            && (greaterThan(dv, du, 2.0 * delta)))) {

                        // Improve strategy if either a smaller ratio
                        // can be obtained, or the distance becomes smaller.
                        s_i_t->setSuccessor(v, u);
                        improvement = true;
                    }
                }
            }
        }

        Player1Result result;
        result.d_i_t = d_i_t;
        result.r_i_t = r_i_t;
        result.s_i_t = *s_i_t;
        result.dw2_i_t = dw2_i_t;
        return result;
    }

    struct StrategyEvaluation {
        std::map<const State<SL, EL> *, CDouble> d_i_t;
        std::map<const State<SL, EL> *, CDouble> r_i_t;
        std::map<const State<SL, EL> *, CDouble> dw2_i_t;
    };

    /**
     * Evaluate the given strategies of both players, i.e. calculate both the
     * distance and ratio of each vertex in the out-degree-one graph, and return
     * the vectors.
     *
     * @param game           out-degree-one graph to be evaluated
     * @param strategy       current strategy of both players
     * @param distanceVector old distance vector
     * @param ratioVector    old ratio vector
     * @param dw2_prev        previous distance w.r.t. weights w2 vector
     * @param stateIds        map with unique id for each state
     * @return the new distance and ratio vectors
     */
    StrategyEvaluation evaluateStrategy(RatioGame<SL, EL> &game,
                                        std::shared_ptr<StrategyVector<SL, EL>> currentStrategy,
                                        std::map<const State<SL, EL> *, CDouble> &distanceVector,
                                        std::map<const State<SL, EL> *, CDouble> &ratioVector,
                                        std::map<const State<SL, EL> *, CDouble> &dw2,
                                        std::map<const State<SL, EL> *, CDouble> &stateIds,
                                        CDouble epsilon) {
        // Find the selected vertex in each cycle, and store the ratio value.
        CycleResult cycleResult = findCyclesInRestrictedGraph(game, currentStrategy, stateIds);
        std::map<const State<SL, EL> *, CDouble> r_i_t = cycleResult.valueMap;

        // Calculate the values for both vectors given the selected vertices
        // and the ratio of each cycle.
        DistanceResult dr = computeDistances(game,
                                             currentStrategy,
                                             cycleResult.states,
                                             r_i_t,
                                             distanceVector,
                                             ratioVector,
                                             dw2,
                                             epsilon);

        // Values of the new vectors.
        StrategyEvaluation se;
        se.d_i_t = dr.d_i_t;
        se.r_i_t = dr.r_i_t;
        se.dw2_i_t = dr.dw2;
        return se;
    }

    struct CycleResult {
        std::shared_ptr<FSM::Abstract::SetOfStateRefs> states;
        std::map<const State<SL, EL> *, CDouble> valueMap;
    };

    /**
     * Find all cycles in the out-degree-one graph given the current strategy.
     * Return all selected vertices in those cycles and the ratio of each cycle,
     * stored in the ratio vector at the selected vertex entry.
     *
     * @param game            out-degree-one graph to be evaluated
     * @param currentStrategy current strategy of both players
     * @param stateIds        map with unique id for each state
     * @return
     */
    CycleResult findCyclesInRestrictedGraph(RatioGame<SL, EL> &game,
                                            std::shared_ptr<StrategyVector<SL, EL>> currentStrategy,
                                            std::map<const State<SL, EL> *, CDouble> &stateIds) {
        const SetOfStates<SL, EL> &states = game.getStates();
        const State<SL, EL> *BOTTOM_VERTEX = nullptr;

        std::shared_ptr<FSM::Abstract::SetOfStateRefs> selectedVertices =
                std::make_shared<FSM::Abstract::SetOfStateRefs>();

        // Initially, all vertices are unvisited.
        std::map<const State<SL, EL> *, const State<SL, EL> *> visited =
                initializeVector(states, BOTTOM_VERTEX);

        std::map<const State<SL, EL> *, CDouble> r_i_t;

        for (auto &it : states) {
            // Source vertex.
            auto &si = *(it.second);
            auto v = dynamic_cast<const State<SL, EL> *>(&(si));
            if (visited[v] == BOTTOM_VERTEX) {
                const State<SL, EL> *u = v;
                while (visited[u] == BOTTOM_VERTEX) {
                    visited[u] = v;
                    u = currentStrategy->getSuccessor(u);
                }
                if (visited[u] == v) {
                    const State<SL, EL> *v_s = u;
                    const State<SL, EL> *x = currentStrategy->getSuccessor(u);

                    // Initialize both numerator and denominator.
                    EdgeRef<SL, EL> e = game.getEdge(*u, *(currentStrategy->getSuccessor(u)));
                    auto w1sum = static_cast<CDouble>(game.getWeight1(e));
                    auto w2sum = static_cast<CDouble>(game.getWeight2(e));

                    while (x != u) {
                        // Find the vertex with the lowest id for unique
                        // ordering, to ensure always the same vertex is the
                        // "selected vertex" in the cycle.
                        if (stateIds[x] < stateIds[v_s]) {
                            v_s = x;
                        }
                        EdgeRef<SL, EL> x_succ =
                                game.getEdge(*x, *(currentStrategy->getSuccessor(x)));
                        auto w1 = static_cast<CDouble>(game.getWeight1(x_succ));
                        auto w2 = static_cast<CDouble>(game.getWeight2(x_succ));

                        w1sum += w1;
                        w2sum += w2;
                        x = currentStrategy->getSuccessor(x);
                    }
                    // Store the cycle ratio for the cycle containing v_s.
                    r_i_t[v_s] = w1sum / w2sum;
                    selectedVertices->insert(v_s);
                }
            }
        }

        CycleResult result;
        result.states = selectedVertices;
        result.valueMap = r_i_t;

        return result;
    }

    struct DistanceResult {
        std::map<const State<SL, EL> *, CDouble> d_i_t;
        std::map<const State<SL, EL> *, CDouble> r_i_t;
        std::map<const State<SL, EL> *, CDouble> dw2;
    };

    /**
     * Compute the new distance and ratio vectors.
     *
     * @param game             ratio game
     * @param currentStrategy  current strategy of both players
     * @param selectedVertices selected vertices of all cycles in the graph
     * @param r_i_t            ratio vector, given the new player-1 strategy
     * @param d_prev           previous distance vector
     * @param r_prev           previous ratio vector
     * @param dw2_prev        previous distance w.r.t. weights w2 vector
     * @param epsilon         epsilon value for equality on real numbers
     * @return
     */
    DistanceResult
    computeDistances(RatioGame<SL, EL> &game,
                     std::shared_ptr<StrategyVector<SL, EL>> currentStrategy,
                     const std::shared_ptr<FSM::Abstract::SetOfStateRefs> &selectedStates,
                     std::map<const State<SL, EL> *, CDouble> &r_i_t,
                     std::map<const State<SL, EL> *, CDouble> &d_prev,
                     std::map<const State<SL, EL> *, CDouble> &r_prev,
                     std::map<const State<SL, EL> *, CDouble> &dw2_prev,
                     CDouble epsilon) {
        const SetOfStates<SL, EL> &states = game.getStates();

        std::stack<const State<SL, EL> *> stack;
        // Initialize visited vector.
        std::map<const State<SL, EL> *, bool> visited = initializeVector(states, false);
        // Initialize selected states.
        std::map<const State<SL, EL> *, CDouble> d_i_t;
        std::map<const State<SL, EL> *, CDouble> dw2;

        // For all selected states.
        for (const auto &ssi : *selectedStates) {
            auto u = dynamic_cast<const State<SL, EL> *>(ssi);
            if (equalTo(r_i_t[u], r_prev[u], epsilon)) {
                d_i_t[u] = d_prev[u];
                dw2[u] = dw2_prev[u];
            } else {
                d_i_t[u] = 0.0;
                dw2[u] = 0.0;
            }
            visited[u] = true;
        }

        // For all states.
        for (auto &it : states) {
            auto &si = *(it.second);
            auto state = dynamic_cast<const State<SL, EL> *>(&(si));
            if (!visited[state]) {
                const State<SL, EL> *u = state;
                while (!visited[u]) {
                    visited[u] = true;
                    stack.push(u);
                    u = currentStrategy->getSuccessor(u);
                }
                while (!stack.empty()) {
                    const State<SL, EL> *x = stack.top();
                    stack.pop();
                    EdgeRef<SL, EL> e = game.getEdge(*x, *u);
                    auto w1 = static_cast<CDouble>(game.getWeight1(e));
                    auto w2 = static_cast<CDouble>(game.getWeight2(e));

                    CDouble cycleRatio = r_i_t[u];

                    CDouble reweighted = w1 - cycleRatio * w2;

                    // Update cycle ratio.
                    r_i_t[x] = cycleRatio;
                    // Update distance to cycle.
                    d_i_t[x] = d_i_t[u] + reweighted;
                    dw2[x] = dw2[u] + w2;
                    u = x;
                }
            }
        }

        DistanceResult result;
        result.r_i_t = r_i_t;
        result.d_i_t = d_i_t;
        result.dw2 = dw2;
        return result;
    }

    /**
     * Check whether each state has at least one outgoing edge.
     * @param graph game graph
     * @return true if and only if each state has at least one outgoing edge
     */
    bool checkEachStateHasSuccessor(RatioGame<SL, EL> &graph) {
        for (auto &it : graph.getStates()) {
            auto &si = it.second;
            // For each state fetch its outgoing edges.
            auto &src = dynamic_cast<State<SL, EL> &>(*si);
            const auto &es =
                    dynamic_cast<const FSM::Abstract::SetOfEdgeRefs &>(src.getOutgoingEdges());

            // Check whether there are any outgoing edges.
            if (es.empty()) {
                return false;
            }
        }
        return true;
    }

    /**
     * Initialize a vector, where each vertex is mapped to a default value
     * {@code value}.
     *
     * @tparam <T>      default value type
     * @param states keys
     * @param value    default value for each key
     * @return map where each vertex is mapped to the default value
     */
    template <typename T>
    std::map<const State<SL, EL> *, T> initializeVector(const SetOfStates<SL, EL> &states, T value) {
        std::map<const State<SL, EL> *, T> vector;

        for (auto &it : states) {
            auto &si = *(it.second);
            // Source vertex.
            auto &state = dynamic_cast<State<SL, EL> &>(si);
            vector[&state] = value;
        }
        return vector;
    }

    bool equalTo(CDouble a, CDouble b, CDouble epsilon) {
        return a == b ? true : std::abs(a - b) < epsilon;
    }

    bool greaterThan(CDouble a, CDouble b, CDouble epsilon) { return a - b > epsilon; }

    bool lessThan(CDouble a, CDouble b, CDouble epsilon) { return b - a > epsilon; }
};
} // namespace MaxPlus

#endif // BASE_MAXPLUS_GAME_POLICYITERATION_H
