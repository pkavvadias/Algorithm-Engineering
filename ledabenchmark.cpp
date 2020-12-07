#include <iostream>
#include <fstream>
#include <limits>
#include "time.h"
#include "LEDA/graph/graph.h"
#include "LEDA/graph/edge_array.h"
#include "LEDA/graph/node_array.h"
#include "LEDA/graph/max_flow.h"

using namespace std;
using namespace leda;

int main(int argc, char **argv) {

    int i, j;
    int in;
    int max_capacity;
    int flow_value;
    int inEdges, outEdges;
    bool is_maximum_flow;
    node s, t, u, v, w;
    edge e;
    graph G;
    node_array<int> Node(G);

    /*
    * Check if user entered graph file 
    */
    if(argc < 2) {
        cout << "No filename given" << endl;
        exit(1);
    }

    /*
    * Create graph based on the .txt file
    */
    ifstream file;
    file.open (argv[1]);
    if (!file.is_open()) {
        cout << "File not found" << endl;
        exit(1);
    }

    while(file >> i) {

        in = 0;
        forall_edges(e, G) {
            if(G.source(e)->id() == i) {
                u = G.source(e);
                ++in;
            }
            else if(G.target(e)->id() == i) {
                u = G.target(e);
                ++in;
            }
        }

        if(in == 0) {
            u = G.new_node();
            Node.init(G);
            Node[u] = i;
        }

        file >> j;

        in = 0;
        forall_edges(e, G) {
            if(G.source(e)->id() == j) {
                v = G.source(e);
                ++in;
            }
            else if(G.target(e)->id() == j) {
                v = G.target(e);
                ++in;
            }
        }

        if(in == 0) {
            v = G.new_node();
            Node.init(G);
            Node[v] = j;
        }

        e = G.new_edge(u, v);
    }

    file.close();

    cout << endl;

    /*
    * Enter max capacity
    */
    cout << "Enter max capacity ";
    cin >> max_capacity;
    cout << endl;

    /*  
    * Randomly assign capacity on each edge between 1 and max_capacity    
    */
    srand ( (unsigned)time(NULL));

    edge_array<int> cap(G);
    forall_edges(e, G) {
        cap[e] = rand() % ((max_capacity - 1) + 1) + 1;
    }
    /*
    * s = source, t = sink 
    */
    forall_nodes(u, G) {

        inEdges = 0;
        forall_edges(e, G) {
            if(G.target(e) == u) {
                ++inEdges;
                break;
            }
        }

        if(inEdges == 0) {
            s = u;
        }

        outEdges = 0;
        forall_edges(e, G) {
            if(G.source(e) == u) {
                ++outEdges;
                break;
            }
        }

        if(outEdges == 0) {
            t = u;
        }
    }

    edge_array<int> flow(G);

    /* 
    * Start timer 
    */
    timespec timer;
    timer.tv_nsec = 0;
    clock_settime(CLOCK_PROCESS_CPUTIME_ID, &timer);

    /* 
    * LEDA MAX_FLOW() 
    */
    flow_value = MAX_FLOW(G, s, t, cap, flow);

    /* 
    * Get time
    */
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &timer);
    cout << "Time spent: " << timer.tv_nsec * 0.000000001 << " seconds"<< endl;

    cout << "The maximum flow is: " << flow_value << endl;

    return 0;
}