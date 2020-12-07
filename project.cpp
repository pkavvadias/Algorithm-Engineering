#include <iostream>
#include <stdlib.h>
#include <fstream>
#include <time.h>
#include <limits>
#include "boost/graph/adjacency_list.hpp"
#include "boost/graph/random.hpp"
#include "boost/graph/make_connected.hpp"
#include "boost/graph/graph_utility.hpp"
#include "boost/graph/visitors.hpp"
#include "boost/graph/breadth_first_search.hpp"
#include "boost/property_map/property_map.hpp"

using namespace std;
using namespace boost;

struct EdgeProperties {
    int capacity;                
};

typedef adjacency_list<vecS, vecS, directedS, no_property, EdgeProperties> Graph;
typedef graph_traits<Graph>::vertex_descriptor Vertex;
typedef graph_traits<Graph>::edge_descriptor Edge;
typedef graph_traits<Graph>::vertex_iterator Vertex_iter;
typedef graph_traits<Graph>::edge_iterator Edge_iter;
typedef property_map<Graph, vertex_index_t>::type vertexIndexMap;
typedef vector<int> distVector;
typedef iterator_property_map<distVector::iterator, vertexIndexMap> distMap;
typedef vector<Vertex> predVector;
typedef iterator_property_map<predVector::iterator, vertexIndexMap> predMap;
typedef list<Edge> list_of_edges;
typedef vector<list_of_edges> aVector;
typedef iterator_property_map<aVector::iterator, vertexIndexMap> Map_t;

/*
* BFS visitor class
*/
class bfs_discovery_visitor : public bfs_visitor <> {
};

/*
* Function declaration
*/
Vertex advance(Vertex i, Vertex j, Edge e, predMap& pred);
Vertex retreat(Vertex i, Vertex s, Map_t& A_Map, predMap& pred, distMap& dist, Graph& g, int n);
void augment(Graph& g, Vertex s, Vertex t, predMap& pred, int max_capacity, Map_t& A_Map);

int flow = 0; //Flow initialisation

int main(int argc, char **argv) {

    
    int n;                  //Number of nodes
    int max_capacity;
    int max_dist;
    int inEdges, outEdges;
    Vertex s, t, i, j, nil;
    Edge e;
    Edge_iter ei, ei_end;
    Graph g;

    nil = INT_MAX;

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
        file >> j;
        add_edge(i, j, g);
    }

    file.close();

    n = num_vertices(g);

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

    for(tie(ei, ei_end) = edges(g); ei != ei_end; ++ei) {
        g[*ei].capacity = rand() % ((max_capacity - 1) + 1) + 1;
    }

    /*
    * Create distMap for distance
    */
    distVector mDistVector(n, 0);
    vertexIndexMap mVertexIndexMap = get(vertex_index, g);
    distMap dist = make_iterator_property_map(mDistVector.begin(), mVertexIndexMap);

    /*
    * Create predMap for predecessor
    */
    predVector mPredVector(n, nil);
    vertexIndexMap m_VertexIndexMap = get(vertex_index, g);
    predMap pred = make_iterator_property_map(mPredVector.begin(), m_VertexIndexMap);

    /*
    * Create A_Map for A(i) lists
    */
    aVector mAVector(n);
    vertexIndexMap myVertexIndexMap = get(vertex_index, g);
    Map_t A_Map = make_iterator_property_map(mAVector.begin(), myVertexIndexMap);

    /*
    * s = source, t = sink 
    */
    for(pair<Vertex_iter, Vertex_iter> vIter = vertices(g); vIter.first != vIter.second; ++vIter.first) {
        inEdges = 0;
        outEdges = out_degree(*vIter.first, g);

        for(tie(ei, ei_end) = edges(g); ei != ei_end; ++ei) {
            if(*vIter.first == target(*ei, g)) {
                ++inEdges;
            }
        }

        if(inEdges == 0) {
            s = *vIter.first;
        }
        else if(outEdges == 0) {
            t = *vIter.first;
        }
    }

    /* 
    * Start timer 
    */
    timespec timer;
    timer.tv_nsec = 0;
    clock_settime(CLOCK_PROCESS_CPUTIME_ID, &timer);


    /*
    * BFS visitor
    */
    bfs_discovery_visitor vis;
    breadth_first_search(g, s, visitor(vis));

    /*
    * Get distance labels
    */
    max_dist = dist[t];
    for(pair<Vertex_iter, Vertex_iter> vIter = vertices(g); vIter.first != vIter.second; ++vIter.first) {
        dist[*vIter.first] = max_dist - dist[*vIter.first];
    }
    /*
    * Initialise list of A(i) with all arcs from i
    */
    for(pair<Vertex_iter, Vertex_iter> vIter = vertices(g); vIter.first != vIter.second; ++vIter.first) {
        for(tie(ei, ei_end) = edges(g); ei != ei_end; ++ei) {
            if(source(*ei, g) == *vIter.first) {
                A_Map[*vIter.first].push_back(*ei);
            }
        }
    }

    i = s; // Initialise, at first vertex i = source

    while(dist[s] < n) {

        outEdges = 0;
        for(tie(ei, ei_end) = edges(g); ei != ei_end; ++ei) {

            if((source(*ei, g) == i) && (g[*ei].capacity > 0)) {

                j = target(*ei, g);
                ++outEdges;

                if(dist[j] < dist[i]) {
                    break;
                }
            }
        }

        /*
        * If i has admissible arc
        */
        if((outEdges > 0) && (dist[j] < dist[i])) {
            i = advance(i, j, *ei, pred);

            if(i == t) {
                augment(g, s, t, pred, max_capacity, A_Map);
                i = s;
            }
        }
        else {
            i = retreat(i, s, A_Map, pred, dist, g, n);
        }
    }

    /*
    * Get time
    */
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &timer);
    cout << "Time spent: " << timer.tv_nsec * 0.000000001 << " seconds"<< endl;

    cout << "The maximum flow is: " << flow << endl;

    return 0;
}


/*
* Advance
*/
Vertex advance(Vertex i, Vertex j, Edge e, predMap& pred) {
    pred[j] = i;
    i = j;

    return i;
}

/*
* Retreat
*/
Vertex retreat(Vertex i, Vertex s, Map_t& A_Map, predMap& pred, distMap& dist, Graph& g, int n) {

    int min;
    Vertex j;

    min = n;

    for(list<Edge>::iterator itr_a = A_Map[i].begin() ; itr_a != A_Map[i].end(); ++itr_a) {
       if((dist[target(*itr_a, g)] < min) && (g[*itr_a].capacity > 0)) {
            min = dist[target(*itr_a, g)];
        }
    }

    dist[i] = min + 1;

    if(i != s) {
        i = pred[i];
    }

    return i;
}


/*
* Augment
*/
void augment(Graph& g, Vertex s, Vertex t, predMap& pred, int max_capacity, Map_t& A_Map) {

    int delta;
    Vertex j;
    Edge e;
    Edge_iter ei, ei_end;

    list<Vertex> P;

    
    /*
    * Identify augmenting path P from s to t using predecessor indices
    */
    j = t;

    P.push_front(t);
    while(pred[j] != s) {
        P.push_front(pred[j]);
        j = pred[j];
    }
    P.push_front(s);
    
    /*
    * delta : = min{r:(i, j)∈ P}
    */
    delta = max_capacity;

    list<Vertex>::iterator itr = P.begin();

    for(tie(ei, ei_end) = edges(g); ei != ei_end; ++ei) {

        if((source(*ei,g) == *itr) && (target(*ei, g) == *next(itr, 1))) {
            if(g[*ei].capacity < delta) {
                delta = g[*ei].capacity;
            }
            ++itr;
        }

        if(itr == P.end()) {
            break;
        }

    }

    flow = flow + delta;

    /*
    * Augment delta units of flow on path P
    */
    itr = P.begin();

    for(tie(ei, ei_end) = edges(g); ei != ei_end; ++ei) {

        if((source(*ei,g) == *itr) && (target(*ei, g) == *next(itr, 1))) {

            g[*ei].capacity = g[*ei].capacity - delta;

            // Create arc of it doesn't exist
            if(!edge(*(next(itr, 1)), *itr, g).second) {
                add_edge(*(next(itr, 1)), *itr, g);
                e = edge(*(next(itr, 1)), *itr, g).first;
                g[e].capacity = delta;

                A_Map[*next(itr, 1)].push_back(e);
            }
            else {
                e = edge(*(next(itr, 1)), *itr, g).first;
                g[e].capacity = g[e].capacity + delta;
            }
            ++itr;
        }

        if(itr == P.end()) {
            break;
        }
    }
}



