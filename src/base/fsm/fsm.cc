/*
 *  Eindhoven University of Technology
 *  Eindhoven, The Netherlands
 *  Dept. of Electrical Engineering
 *  Electronics Systems Group
 *  Model Based Design Lab (https://computationalmodeling.info/)
 *
 *  Name            :   fsm.cc
 *
 *  Author          :   Marc Geilen (m.c.w.geilen@tue.nl)
 *
 *  Date            :   March 23, 2009
 *
 *  Function        :   finite state machines
 *
 *  History         :
 *      23-03-09    :   Initial version.
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

#include "base/fsm/fsm.h"
#include <memory>

using namespace MaxPlus;

namespace MaxPlus::FSM {

CId FSM::Abstract::WithUniqueID::nextID = 0;

namespace StateStringLabeled {

void FiniteStateMachine::addStateLabeled(const MPString &sl) { this->addState(sl); }

void FiniteStateMachine::addEdgeLabeled(const MPString &src, const MPString &dst) {
    const Labeled::StateRef<MPString, char> s_src = this->getStateLabeled(src);
    const Labeled::StateRef<MPString, char> s_dst = this->getStateLabeled(dst);
    Labeled::FiniteStateMachine<MPString, char>::addEdge(*s_src, 'X', *s_dst);
}

std::unique_ptr<Abstract::SetOfStateRefs> FiniteStateMachine::reachableStates() {
    return std::move(
            Labeled::FiniteStateMachine<MPString, char>::reachableStates());
}

void FiniteStateMachine::setInitialStateLabeled(const MPString &sl) { this->setInitialState(sl); }

} // namespace StateStringLabeled

namespace Product {

const Abstract::SetOfEdges &State::getOutgoingEdges() { // NOLINT(*no-recursion)
    if (!outgoingEdgesDone) {
        // compute outgoing edges
        const Abstract::SetOfEdgeRefs &oea = this->sa->getOutgoingEdges();
        const Abstract::SetOfEdgeRefs &oeb = this->sb->getOutgoingEdges();
        auto i = oea.begin();
        auto j = oeb.begin();
        while (i != oea.end()) {
            while (j != oeb.end()) {
                if (this->fsm->matchEdges(*(*i), *(*j))) {
                    Abstract::Edge* e = this->fsm->ensureEdge(*(*i), *(*j));
                    this->insertOutgoingEdge(*e);
                }
                j++;
            }
            i++;
        }
        outgoingEdgesDone = true;
    }
    return this->getOutgoingEdges();
}
} // namespace Product

} // namespace MaxPlus::FSM