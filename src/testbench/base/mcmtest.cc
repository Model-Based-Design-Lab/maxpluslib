#include <algorithm>

#include "algebra/mptype.h"
#include "base/analysis/mcm/mcm.h"
#include "base/analysis/mcm/mcmdg.h"
#include "base/analysis/mcm/mcmgraph.h"
#include <base/analysis/mcm/mcmhoward.h>
#include <base/analysis/mcm/mcmyto.h>
#include "mcmtest.h"
#include "testing.h"
#include <random>
#include <array>
#include <type_traits>


using namespace MaxPlus;
using namespace Graphs;

void MCMTest::Run() {
    this->test_dg();
    this->test_howard();
    this->test_karp();
    this->test_yto();
    this->test_prune();
};

MCMgraph makeGraph1() {
    MCMgraph g;

    MCMnode &n0 = *g.addNode(0);
    MCMnode &n1 = *g.addNode(1);
    MCMnode &n2 = *g.addNode(2);
    MCMnode &n3 = *g.addNode(3);
    MCMnode &n4 = *g.addNode(4);
    g.addEdge(0, n0, n1, 1.0, 1.0);
    g.addEdge(1, n1, n2, 2.0, 4.0);
    g.addEdge(2, n2, n3, 3.0, 0.0);
    g.addEdge(3, n3, n0, 4.0, 1.0);
    g.addEdge(4, n3, n3, 1.0, 0.3);
    g.addEdge(5, n3, n4, 4.0, 0.0);
    g.addEdge(6, n4, n4, 1.0, 1.0);
    return g;
}

// A corner case graph
MCMgraph makeGraph2() {
    MCMgraph g;

    MCMnode &n0 = *g.addNode(0);
    MCMnode &n1 = *g.addNode(1);
    g.addEdge(0, n0, n1, 1.0, 1.0);
    //g.addEdge(1, n1, n1, 1.0, 1.0);
    return g;
}

MCMgraph makeRandomGraph(unsigned int numberOfNodes, unsigned int numberOfEdges, unsigned int seed) {
    MCMgraph g;

    std::mt19937 rng(seed);
    unsigned int actualNumberOfNodes = (numberOfNodes / 2) + (rng() % (numberOfNodes / 2));

    std::vector<MCMnode *> nodes;
    for (unsigned int i = 0; i < actualNumberOfNodes; i++) {
        nodes.push_back(g.addNode(i));
    }

    // cap the number of edges
    unsigned int actualNumberOfEdges = (numberOfEdges / 2) + (rng() % (numberOfEdges / 2));
    if (actualNumberOfEdges > (actualNumberOfNodes - 1) * actualNumberOfNodes) {
        actualNumberOfEdges = (actualNumberOfNodes - 1) * actualNumberOfNodes;
    }

    // random edges
    std::set<std::pair<CId, CId>> existingEdges;
    unsigned int i = 0;
    for (; i < actualNumberOfEdges; i++) {
        CId src = 0;
        CId dst = 0;
        do {
            src = rng() % actualNumberOfNodes;
            dst = rng() % actualNumberOfNodes;
        } while (existingEdges.find(std::make_pair(src, dst))!=existingEdges.end());
        existingEdges.insert(std::make_pair(src, dst));
        CDouble w = static_cast<CDouble>(rng() % 100000) / 1000.0;
        CDouble d = static_cast<CDouble>(rng() % 100000) / 1000.0;
        g.addEdge(i, *(nodes[src]), *(nodes[dst]), w, d);
    }

    // ensure all nodes have an outgoing edge
    for (auto &n : nodes) {
        if (n->out.size() == 0) {
            CId dst = rng() % actualNumberOfNodes;
            CDouble w = static_cast<CDouble>(rng() % 100000) / 1000.0;
            CDouble d = static_cast<CDouble>(rng() % 100000) / 1000.0;
            g.addEdge(i++, *n, *(nodes[dst]), w, d);
        }
    }

    return g;
}



// Test mcmdg.
void MCMTest::test_dg() {
    std::cout << "Running test: MCM-dg" << std::endl;

    MCMgraph g1 = makeGraph1();
    CDouble result =  mcmDG(g1);
    ASSERT_APPROX_EQUAL(2.5, result, 1e-5);

    MCMgraph g2 = makeGraph2();
    result = mcmDG(g2);
    ASSERT_EQUAL(-INFINITY, result);
}

/// Test MCM Howard.
void MCMTest::test_howard() {
    std::cout << "Running test: MCM-Howard" << std::endl;
    MCMgraph g1 = makeGraph1();
    
    std::shared_ptr<std::vector<int>> ij = nullptr;
    std::shared_ptr<std::vector<CDouble>> A = nullptr;
    std::shared_ptr<std::vector<CDouble>> chi = nullptr;
    std::shared_ptr<std::vector<CDouble>> v = nullptr;
    std::shared_ptr<std::vector<int>> policy = nullptr;
    int nr_iterations = 0;
    int nr_components = 0;
    
    convertMCMgraphToMatrix(g1, &ij, &A);

   Howard(*ij,
           *A,
           static_cast<int>(g1.getNodes().size()),
           static_cast<int>(g1.getEdges().size()),
           &chi,
           &v,
           &policy,
           &nr_iterations,
           &nr_components);
    

    ASSERT_APPROX_EQUAL(2.5, chi->at(0), 1e-5);
    ASSERT_APPROX_EQUAL(2.5, chi->at(1), 1e-5);
    ASSERT_APPROX_EQUAL(2.5, chi->at(2), 1e-5);
    ASSERT_APPROX_EQUAL(2.5, chi->at(3), 1e-5);
    ASSERT_APPROX_EQUAL(1.0, chi->at(4), 1e-5);
    ASSERT_EQUAL(1, policy->at(0));
    ASSERT_EQUAL(2, policy->at(1));
    ASSERT_EQUAL(3, policy->at(2));
    ASSERT_EQUAL(0, policy->at(3));
    ASSERT_EQUAL(4, policy->at(4));

    MCMnode *criticalNode;
    CDouble result = maximumCycleMeanHowardGeneral(g1, &criticalNode);
    ASSERT_APPROX_EQUAL(2.5, result, 1e-5);
    ASSERT_THROW(criticalNode->id == 0 || criticalNode->id == 1 || criticalNode->id == 2
                 || criticalNode->id == 3);

    MCMgraph g2 = makeGraph2();
    result = maximumCycleMeanHowardGeneral(g2, &criticalNode);
    ASSERT_EQUAL(-INFINITY, result);
}

/// Test MCM Karp.
void MCMTest::test_karp() {
    std::cout << "Running test: MCM-Karp" << std::endl;

    MCMgraph g1 = makeGraph1();

    CDouble result = maximumCycleMeanKarp(g1);
    ASSERT_APPROX_EQUAL(2.5, result, 1e-5);

    const MCMnode *criticalNode;
    result = maximumCycleMeanKarpDouble(g1, &criticalNode);
    ASSERT_APPROX_EQUAL(2.5, result, 1e-5);
    ASSERT_THROW(criticalNode->id == 0 || criticalNode->id == 1 || criticalNode->id == 2
                 || criticalNode->id == 3);

    MCMgraph g2 = makeGraph2();

    result = maximumCycleMeanKarpDoubleGeneral(g2);
    ASSERT_EQUAL(-INFINITY, result);

}



/// Test MCM YTO.
void MCMTest::test_yto() {
    std::cout << "Running test: MCM-YTO" << std::endl;

    MCMgraph g1 = makeGraph1();
    CDouble result = maxCycleMeanYoungTarjanOrlin(g1);
    ASSERT_APPROX_EQUAL(2.5, result, 1e-5);

    std::shared_ptr<std::vector<const MCMedge *>> cycle;
    result = maxCycleMeanAndCriticalCycleYoungTarjanOrlin(g1, &cycle);
    ASSERT_APPROX_EQUAL(2.5, result, 1e-5);
    ASSERT_THROW(cycle->size() == 4);
    int eid = cycle->at(0)->id;
    ASSERT_THROW(eid == 0 || eid == 1 || eid == 2 || eid == 3);

    result = maxCycleRatioYoungTarjanOrlin(g1);
    ASSERT_APPROX_EQUAL(10.0 / 3.0, result, 1e-5);

    result = maxCycleRatioAndCriticalCycleYoungTarjanOrlin(g1, &cycle);
    ASSERT_APPROX_EQUAL(10.0 / 3.0, result, 1e-5);
    eid = cycle->at(0)->id;
    ASSERT_THROW(eid == 4);

    result = minCycleRatioYoungTarjanOrlin(g1);
    ASSERT_APPROX_EQUAL(1.0, result, 1e-5);

    result = minCycleRatioAndCriticalCycleYoungTarjanOrlin(g1, &cycle);
    ASSERT_APPROX_EQUAL(1.0, result, 1e-5);
    eid = cycle->at(0)->id;
    ASSERT_THROW(eid == 6);

    // TODO: check of the cycle ratio are identical to old SDF3
    std::array<CDouble,250> expectedMinCycleRatios = {
            0.526141,  0.124523,  0.0389826, 0.289127,   0.11451,    0.723313,   1.07027,
            0.956067,  0.570066,  0.746341,  0.33622,    0.475978,   0.902663,   1.02951,
            0.088711,  0.269878,  0.213763,  0.468927,   2.13632,    0.564086,   0.656227,
            0.621099,  1.20636,   0.77875,   5.06401,    0.216956,   0.308638,   0.35851,
            0.847727,  0.262273,  0.78317,   0.711161,   0.0598175,  1.35369,    0.713742,
            0.687969,  1.12182,   0.357931,  0.209504,   0.265194,   1.02591,    0.329403,
            0.753796,  0.356683,  0.462371,  0.0856026,  0.481905,   0.437436,   0.462771,
            0.604891,  0.0723836, 1.37879,   0.208156,   1.06655,    0.839532,   0.441806,
            0.718773,  0.944303,  0.687192,  0.652867,   0.229211,   0.405059,   0.238781,
            0.980149,  0.994927,  0.288884,  0.765275,   0.0598507,  0.506098,   0.193252,
            0.357463,  0.790442,  0.926149,  0.933226,   0.862442,   0.22172,    0.648843,
            0.487559,  0.569503,  1.58893,   0.451955,   2.30444,    0.132518,   0.31322,
            0.716398,  1.23326,   0.196893,  1.18315,    0.772721,   1.15355,    0.153664,
            1.38356,   0.190047,  4.67662,   0.656561,   0.461505,   0.246274,   0.513893,
            0.832467,  0.452531,  0.6412,    0.36582,    0.61421,    0.145586,   0.0500161,
            0.215703,  0.446588,  1.23612,   0.731997,   0.00290626, 0.59127,    0.470511,
            0.924516,  0.0461689, 0.421068,  0.176232,   0.0565415,  0.183527,   0.216565,
            0.0051396, 0.0814156, 0.199074,  0.397271,   0.952548,   0.428215,   0.553574,
            0.227344,  0.97122,   0.330989,  0.262043,   1.01896,    0.0408578,  1.09593,
            0.296915,  0.43137,   0.585671,  0.723156,   1.21793,    0.00657344, 0.514615,
            0.612248,  0.330343,  0.6682,    0.259912,   1.07794,    1.66545,    0.479252,
            0.454742,  0.987109,  0.121702,  0.0453588,  0.542963,   0.583882,   1.02169,
            0.8876,    0.34417,   0.624893,  0.94663,    0.507723,   0.797985,   1.10759,
            1.00966,   0.133526,  0.367633,  0.389806,   0.513096,   0.334658,   0.193679,
            0.97823,   0.497859,  0.650175,  0.429531,   1.26854,    5.049,      0.605791,
            0.770483,  0.840804,  0.386462,  0.616477,   1.77663,    0.905697,   0.508591,
            0.159205,  0.972549,  0.296205,  0.776466,   0.582595,   1.83776,    0.329751,
            0.83243,   0.161612,  0.800077,  0.739183,   0.510201,   0.934052,   0.538563,
            0.434412,  0.636206,  1.46169,   0.0165175,  0.798292,   1.05746,    0.734096,
            0.417915,  0.586467,  0.76847,   0.702786,   1.17993,    0.0524402,  0.381511,
            0.0990757, 0.0298227, 0.166558,  1.17975,    1.49291,    1.43093,    0.285337,
            0.170363,  0.566864,  0.623089,  2.16305,    0.358837,   0.695627,   0.00636429,
            0.905727,  0.112028,  0.432291,  0.827206,   0.670839,   0.106227,   0.911849,
            0.192765,  2.22425,   0.0746305, 0.333403,   0.0338224,  0.61903,    0.773725,
            1.07322,   0.0764852, 0.011864,  0.00518094, 0.178574,   0.140178,   0.96676,
            0.136082,  0.194365,  0.425315,  0.471688,   0.923484};

    // go through a bunch of (deterministic) small pseudo-random graphs
    for (int k = 0; k < 250; k++){
        MCMgraph gr = makeRandomGraph(10, 10, k);
        result = minCycleRatioAndCriticalCycleYoungTarjanOrlin(gr, &cycle);
        ASSERT_APPROX_EQUAL(expectedMinCycleRatios[k], result, 1e-4);
        //std::cout << "MCM: " << result << "cycle length: " << cycle->size() << std::endl;
    }

    std::array<CDouble, 25> expectedMaxCycleRatios = {
            304.647, 122.759, 104.209, 126.85,  1341.83, 135.611, 186.801, 151.854, 341.077,
            216.21,  148.74,  117.382, 127.863, 246.385, 92.0357, 72.9353, 124.893, 202.074,
            281.92,  481.408, 253.295, 849.381, 123.668, 64.0955, 219.92};
    // go through a bunch of (deterministic) big pseudo-random graphs
    for (int k = 0; k < expectedMaxCycleRatios.size(); k++) {
        MCMgraph gr = makeRandomGraph(1000, 100000, k);
        result = maxCycleRatioAndCriticalCycleYoungTarjanOrlin(gr, &cycle);
        ASSERT_APPROX_EQUAL(expectedMaxCycleRatios[k], result, 1e-2);
        //std::cout << "MCM: " << result << "cycle length: " << cycle->size() << std::endl;
    }


}

void MCMTest::test_prune() {

    MCMgraph mcmGraph;
    auto& n36 = *mcmGraph.addNode(36);
    auto& n37 = *mcmGraph.addNode(37);
    auto& n38 = *mcmGraph.addNode(38);
    auto& n39 = *mcmGraph.addNode(39);
    auto& n40 = *mcmGraph.addNode(40);


    mcmGraph.addEdge(0, n36, n36, 5.83e+06, 1);
    mcmGraph.addEdge(1, n36, n37, 1.52647e+07, 1);
    mcmGraph.addEdge(2, n37, n37, 5.83e+06, 1);
    mcmGraph.addEdge(3, n38, n37, 9.43472e+06, 1);
    mcmGraph.addEdge(4, n39, n37, 8.72299e+06, 1);
    mcmGraph.addEdge(5, n40, n37, 8.72299e+06, 1);
    mcmGraph.addEdge(6, n36, n38, 7.01739e+06, 1);
    mcmGraph.addEdge(7, n38, n38, 1.18739e+06, 1);
    mcmGraph.addEdge(8, n36, n39, 1.22848e+07, 1);
    mcmGraph.addEdge(9, n38, n39, 6.45482e+06, 1);
    mcmGraph.addEdge(10, n39, n39, 5.74309e+06, 1);
    mcmGraph.addEdge(11, n36, n40, 1.23277e+07, 1);
    mcmGraph.addEdge(12, n38, n40, 6.49772e+06, 1);
    mcmGraph.addEdge(13, n39, n40, 5.78599e+06, 1);
    mcmGraph.addEdge(14, n40, n40, 5.78599e+06, 1);
    mcmGraph.addEdge(15, n36, n36, 5.83e+06, 1);
    mcmGraph.addEdge(16, n36, n37, 1.32964e+07, 1);
    mcmGraph.addEdge(17, n37, n37, 4.64e+06, 1);
    mcmGraph.addEdge(18, n38, n37, 7.46636e+06, 1);
    mcmGraph.addEdge(19, n39, n37, 6.93511e+06, 1);
    mcmGraph.addEdge(20, n40, n37, 6.93511e+06, 1);
    mcmGraph.addEdge(21, n36, n38, 6.65645e+06, 1);
    mcmGraph.addEdge(22, n38, n38, 826446, 1);
    mcmGraph.addEdge(23, n36, n39, 1.09086e+07, 1);
    mcmGraph.addEdge(24, n38, n39, 5.07857e+06, 1);
    mcmGraph.addEdge(25, n39, n39, 4.54732e+06, 1);
    mcmGraph.addEdge(26, n36, n40, 1.09515e+07, 1);
    mcmGraph.addEdge(27, n38, n40, 5.12147e+06, 1);
    mcmGraph.addEdge(28, n39, n40, 4.59021e+06, 1);
    mcmGraph.addEdge(29, n40, n40, 4.59021e+06, 1);
    mcmGraph.addEdge(30, n36, n36, 5.83e+06, 1);
    mcmGraph.addEdge(31, n36, n37, 1.49199e+07, 1);
    mcmGraph.addEdge(32, n37, n37, 5.76e+06, 1);
    mcmGraph.addEdge(33, n38, n37, 9.08995e+06, 1);
    mcmGraph.addEdge(34, n39, n37, 8.61563e+06, 1);
    mcmGraph.addEdge(35, n40, n37, 8.61563e+06, 1);
    mcmGraph.addEdge(36, n36, n38, 6.54258e+06, 1);
    mcmGraph.addEdge(37, n38, n38, 712576, 1);
    mcmGraph.addEdge(38, n36, n39, 1.19727e+07, 1);
    mcmGraph.addEdge(39, n38, n39, 6.14268e+06, 1);
    mcmGraph.addEdge(40, n39, n39, 5.66836e+06, 1);
    mcmGraph.addEdge(41, n36, n40, 1.20156e+07, 1);
    mcmGraph.addEdge(42, n38, n40, 6.18558e+06, 1);
    mcmGraph.addEdge(43, n39, n40, 5.71125e+06, 1);
    mcmGraph.addEdge(44, n40, n40, 5.71125e+06, 1);
    mcmGraph.addEdge(45, n36, n36, 5.83e+06, 1);
    mcmGraph.addEdge(46, n36, n37, 1.51086e+07, 1);
    mcmGraph.addEdge(47, n37, n37, 5.83e+06, 1);
    mcmGraph.addEdge(48, n38, n37, 9.27858e+06, 1);
    mcmGraph.addEdge(49, n39, n37, 8.12511e+06, 1);
    mcmGraph.addEdge(50, n40, n37, 8.72299e+06, 1);
    mcmGraph.addEdge(51, n36, n38, 6.7051e+06, 1);
    mcmGraph.addEdge(52, n38, n38, 875098, 1);
    mcmGraph.addEdge(53, n36, n39, 1.09329e+07, 1);
    mcmGraph.addEdge(54, n38, n39, 5.1029e+06, 1);
    mcmGraph.addEdge(55, n39, n39, 4.54732e+06, 1);
    mcmGraph.addEdge(56, n36, n40, 1.21716e+07, 1);
    mcmGraph.addEdge(57, n38, n40, 6.34157e+06, 1);
    mcmGraph.addEdge(58, n39, n40, 5.1881e+06, 1);
    mcmGraph.addEdge(59, n40, n40, 5.78599e+06, 1);
    mcmGraph.addEdge(60, n36, n36, 5.83e+06, 1);
    mcmGraph.addEdge(61, n36, n37, 1.44916e+07, 1);
    mcmGraph.addEdge(62, n37, n37, 4.64e+06, 1);
    mcmGraph.addEdge(63, n38, n37, 8.66157e+06, 1);
    mcmGraph.addEdge(64, n39, n37, 8.10599e+06, 1);
    mcmGraph.addEdge(65, n40, n37, 7.5081e+06, 1);
    mcmGraph.addEdge(66, n36, n38, 6.7051e+06, 1);
    mcmGraph.addEdge(67, n38, n38, 875098, 1);
    mcmGraph.addEdge(68, n36, n39, 1.21716e+07, 1);
    mcmGraph.addEdge(69, n38, n39, 6.34157e+06, 1);
    mcmGraph.addEdge(70, n39, n39, 5.78599e+06, 1);
    mcmGraph.addEdge(71, n40, n39, 5.1881e+06, 1);
    mcmGraph.addEdge(72, n36, n40, 1.09329e+07, 1);
    mcmGraph.addEdge(73, n38, n40, 5.1029e+06, 1);
    mcmGraph.addEdge(74, n40, n40, 4.54732e+06, 1);

    std::shared_ptr<MCMgraph> result =  mcmGraph.pruneEdges(); 


    CDouble mcr1 = maxCycleRatioAndCriticalCycleYoungTarjanOrlin(mcmGraph, nullptr);
    ASSERT_APPROX_EQUAL(mcr1, 5.83e+06, 1e3);

    CDouble mcr2 = maxCycleRatioAndCriticalCycleYoungTarjanOrlin(*result, nullptr);   

    ASSERT_APPROX_EQUAL(mcr1, mcr2, 1e3);

}
