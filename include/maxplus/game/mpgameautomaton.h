/*
 *  Eindhoven University of Technology
 *  Eindhoven, The Netherlands
 *  Dept. of Electrical Engineering
 *  Electronics Systems Group
 *  Model Based Design Lab (https://computationalmodeling.info/)
 *
 *  Name            :   mpgameautomaton.h
 *
 *  Author          :   Bram van der Sanden (b.v.d.sanden@tue.nl)
 *
 *  Date            :   June 24, 2017
 *
 *  Function        :   max plus automaton where states are partitioned between two players
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

#ifndef MAXPLUS_GRAPH_MPSTATESPACE_H
#define MAXPLUS_GRAPH_MPSTATESPACE_H

#include "maxplus/algebra/mpmatrix.h"
#include "maxplus/base/fsm/fsm.h"
#include "maxplus/graph/mpautomaton.h"
#include "ratiogame.h"

namespace MaxPlus {

/**
 * A max-plus automaton with rewards. In addition to the usual max-plus automaton,
 * its edges are labeled with rewards; a quantified amount of 'progress'.
 * Furthermore, the set of vertices is partitioned in player-0 and player-1 vertices.
 */
class MaxPlusGameAutomatonWithRewards : public MaxPlusAutomatonWithRewards,
                                        public RatioGame<MPAStateLabel, MPAREdgeLabel> {
public:
    MaxPlusGameAutomatonWithRewards() = default;

    ~MaxPlusGameAutomatonWithRewards() override = default;
    ;

    std::set<MPARState *> &getV0() override { return this->setV0; }

    std::set<MPARState *> &getV1() override { return this->setV1; }

    void addV0(MPARState *s) { this->setV0.insert(s); }

    void addV1(MPARState *s) { this->setV1.insert(s); }

    MPTime getWeight1(const MPAREdge *e) const override { return MPTime(e->getLabel().reward); }

    MPTime getWeight2(const MPAREdge *e) const override { return e->getLabel().delay; }

private:
    std::set<MPARState *> setV0;
    std::set<MPARState *> setV1;
};
} // namespace MaxPlus

#endif // MAXPLUS_GRAPH_MPSTATESPACE_H
