/*
 *  Eindhoven University of Technology
 *  Eindhoven, The Netherlands
 *  Dept. of Electrical Engineering
 *  Electronics Systems Group
 *  Model Based Design Lab (https://computationalmodeling.info/)
 *
 *  Name            :   mcm.cc
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   July 25, 2005
 *
 *  Function        :   Maximum cycle mean of a HSDF graph
 *
 *  History         :
 *      25-07-05    :   Initial version.
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

#include "base/analysis/mcm/mcm.h"

namespace MaxPlus::Graphs {
// #define __CALC_MCM_PER_CYCLE__

#ifndef __CALC_MCM_PER_CYCLE__

/**
 * mcmGetAdjacentActors ()
 * The function returns a list with actors directly reachable from
 * actor a.
 */

v_uint mcmGetAdjacentActors(uint a,
                            const v_uint &nodeId,
                            const v_uint &actorId,
                            const std::vector<v_uint> &graph,
                            uint nrNodes) {
    v_uint actors;

    uint u = nodeId[a];
    for (uint v = 0; v < nrNodes; v++) {
        if (graph[u][v] == 1) {
            actors.push_back(actorId[v]);
        }
    }

    return actors;
}

#else // __CALC_MCM_PER_CYCLE__

#endif // __CALC_MCM_PER_CYCLE__
} // namespace MaxPlus::Graphs
