/*
 *  Eindhoven University of Technology
 *  Eindhoven, The Netherlands
 *  Dept. of Electrical Engineering
 *  Electronics Systems Group
 *  Model Based Design Lab (https://computationalmodeling.info/)
 *
 *  Name            :   strategyvector.h
 *
 *  Author          :   Bram van der Sanden (b.v.d.sanden@tue.nl)
 *
 *  Date            :   June 24, 2017
 *
 *  Function        :   strategy vector that can store the current strategy of the players
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

#ifndef MAXPLUS_GAME_STRATEGYVECTOR_H
#define MAXPLUS_GAME_STRATEGYVECTOR_H

#include "maxplus/base/fsm/fsm.h"
#include "ratiogame.h"
#include <utility>

namespace MaxPlus {

using namespace ::FSM::Labeled;

/**
 * Vector that can store the strategy for both players in the graph. For any
 * vertex in the graph this map contains the unique successor given the current
 * strategy. Note that some vertices might be unreachable given a strategy.
 *
 * @tparam SL state label type
 * @tparam EL edge label type
 * @author Bram van der Sanden
 */
template <typename SL, typename EL> class StrategyVector {
public:
    StrategyVector() = default;
    ;

    explicit StrategyVector(StrategyVector *vec) {
        std::map<const State<SL, EL> *, const State<SL, EL> *> copy(vec->strategyVector);
        this->strategyVector = copy;
    }

    /**
     * Given the graph, initialize a random strategy. For each vertex in the
     * graph we choose one of the successors as the successor given the
     * strategy.
     *
     * @param graph game graph on which a random strategy is initialized
     */
    void initializeRandomStrategy(RatioGame<SL, EL> &graph) {
        for (auto &it : graph.getStates()) {
            auto &si = *(it.second);
            // Source vertex.
            auto &src = dynamic_cast<State<SL, EL> &>(si);
            const auto &es =
                    dynamic_cast<const FSM::Abstract::SetOfEdgeRefs &>(src.getOutgoingEdges());

            // Find the first outgoing edge, and get the target.
            auto *e = dynamic_cast<Edge<SL, EL> *>(*es.begin());
            auto &dest = dynamic_cast<const State<SL, EL> &>(e->getDestination());

            strategyVector[&src] = &dest;
        }
    }

    /**
     * Update the strategy by changing the unique successor of the given vertex.
     *
     * @param v         vertex of which a new successor is set
     * @param successor successor of the given vertex in the strategy
     */
    void setSuccessor(const State<SL, EL> *v, const State<SL, EL> *successor) {
        strategyVector[v] = successor;
    }

    /**
     * Return the unique successor given the current strategy.
     *
     * @param v vertex of which the unique successor is returned.
     * @return the successor of {@code v} given the current strategy.
     */
    const State<SL, EL> *getSuccessor(const State<SL, EL> *v) {
        return strategyVector.find(v)->second;
    }

    /**
     * Return the current strategy.
     *
     * @return current strategy with (vertex, successor(vertex)) pairs.
     */
    std::map<State<SL, EL> *, State<SL, EL> *> *getMap() { return &strategyVector; }

private:
    std::map<const State<SL, EL> *, const State<SL, EL> *> strategyVector;
};

} // namespace MaxPlus

#endif // MAXPLUS_GAME_STRATEGYVECTOR_H