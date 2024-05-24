/*
 *  Eindhoven University of Technology
 *  Eindhoven, The Netherlands
 *  Dept. of Electrical Engineering
 *  Electronics Systems Group
 *  Model Based Design Lab (https://computationalmodeling.info/)
 *
 *  Name            :   mpstatespace.h
 *
 *  Author          :   Marc Geilen (m.c.w.geilen@tue.nl)
 *                  :   Bram van der Sanden (b.v.d.sanden@tue.nl)
 *
 *  Date            :   April 26, 2010
 *
 *  Function        :   Max plus state space.
 *
 *  History         :
 *      26-04-10    :   Initial version.
 *      24-06-17    :   Max-plus library.
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

namespace MaxPlus {

using ELSState = ::FSM::Labeled::State<CId, MPString>;
using ELSEdge = ::FSM::Labeled::Edge<CId, MPString>;
using ELSEdgeRef = ::FSM::Labeled::EdgeRef<CId, MPString>;
using ELSSetOfStates = ::FSM::Labeled::SetOfStates<CId, MPString>;
using ELSSetOfEdges = ::FSM::Labeled::SetOfEdges<CId, MPString>;

// /**
//  * Edge labeled scenario FSM.
//  */
// class EdgeLabeledScenarioFSM : public ::FSM::Labeled::FiniteStateMachine<CId, MPString> {
// public:
//     virtual ~EdgeLabeledScenarioFSM() {};
// };

using MLSEdgeLabel = struct MLSEdgeLabel {
    Matrix *mat;
    CDouble rew;
};

using MLSState = ::FSM::Labeled::State<CId, MLSEdgeLabel>;
using MLSEdge = ::FSM::Labeled::Edge<CId, MLSEdgeLabel>;
using MLSSetOfStates = ::FSM::Labeled::SetOfStates<CId, MLSEdgeLabel>;
using MLSSetOfEdges = ::FSM::Abstract::SetOfEdges;

/**
 * Matrix and reward labeled scenario FSM.
 */
class MatrixLabeledScenarioFSM : public ::FSM::Labeled::FiniteStateMachine<CId, MLSEdgeLabel> {};

} // namespace MaxPlus

#endif // MAXPLUS_GRAPH_MPSTATESPACE_H
