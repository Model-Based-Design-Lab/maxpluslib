/*
 *  Eindhoven University of Technology
 *  Eindhoven, The Netherlands
 *  Dept. of Electrical Engineering
 *  Electronics Systems Group
 *  Model Based Design Lab (https://computationalmodeling.info/)
 *
 *  Name            :   mcmdg.cc
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   November 8, 2005
 *
 *  Function        :   Compute the MCM for a graph using Dasdan-Gupta's
 *                      algorithm.
 *
 *  @article{Dasdan1998FasterMA,
 *    title={Faster maximum and minimum mean cycle algorithms for system-performance analysis},
 *    author={Ali Dasdan and Rajesh K. Gupta},
 *    journal={IEEE Trans. Comput. Aided Des. Integr. Circuits Syst.},
 *    year={1998},
 *    volume={17},
 *    pages={889-899}
 * }
 *
 *  History         :
 *      08-11-05    :   Initial version.
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

#ifndef MAXPLUS_BASE_ANALYSIS_MCM_MCMDG_H_INCLUDED
#define MAXPLUS_BASE_ANALYSIS_MCM_MCMDG_H_INCLUDED

#include "maxplus/base/analysis/mcm/mcmgraph.h"

namespace Graphs {

/**
 * mcmDG ()
 * The function computes the maximum cycle mean of an MCM graph using
 * Dasdan-Gupta's algorithm.
 * Note: this algorithm assumes that edge weights are integer valued !
 *
 * @todo
 * check if algorithm can be generalized to float edge weights
 */
CDouble mcmDG(MCMgraph &mcmGraph);

} // namespace Graphs

#endif