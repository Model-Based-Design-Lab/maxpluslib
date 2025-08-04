/*
 *  Eindhoven University of Technology
 *  Eindhoven, The Netherlands
 *  Dept. of Electrical Engineering
 *  Electronics Systems Group
 *  Model Based Design Lab (https://computationalmodeling.info/)
 *
 *  Name            :   doubleweightedgraph.h
 *
 *  Author          :   Bram van der Sanden (b.v.d.sanden@tue.nl)
 *
 *  Date            :   June 24, 2017
 *
 *  Function        :   double weighted graph interface
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

#ifndef MAXPLUS_GRAPH_DOUBLEWEIGHTEDGRAPH_H
#define MAXPLUS_GRAPH_DOUBLEWEIGHTEDGRAPH_H

#include "maxplus/algebra/mptype.h"
#include "maxplus/base/fsm/fsm.h"

namespace MaxPlus {

using namespace ::MaxPlus::FSM::Labeled;

/**
 * Double weighted graph.
 * @tparam V state label type
 * @tparam E edge label type
 */
template <typename SL, typename EL>
class DoubleWeightedGraph : virtual public ::FSM::Labeled::FiniteStateMachine<SL, EL> {
public:
    virtual ~DoubleWeightedGraph() = default;

    virtual MPTime getWeight1(const Edge<SL, EL> *e) const = 0;

    virtual MPTime getWeight2(const Edge<SL, EL> *e) const = 0;
};

} // namespace MaxPlus

#endif // MAXPLUS_GRAPH_DOUBLEWEIGHTEDGRAPH_H