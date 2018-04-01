// This code belongs to Saksham Mittal
#include <iostream>
#include <thread>
#include <ctime>
#include <stdlib.h>
#include <chrono>
#include <mutex>
using namespace std;

struct node {
    int color;      // color of vertice
    int parent;     // parent of vertice
    int v_no;       // vertice number
};

struct priority_queue {
    int v1;
    int v2;
    int weight;
};

int N = 9;
int t = 6;

bool working[6];    // Change size to t

// int **E, **outputE;
int E[9][9] = { { -1,  4, -1, -1, -1, -1, -1,  8, -1},
		    	{  4, -1,  8, -1, -1, -1, -1, 11, -1},
		    	{ -1,  8, -1,  7, -1,  4, -1, -1,  2},
		    	{ -1, -1,  7, -1,  9, 14, -1, -1, -1},
		    	{ -1, -1, -1,  9, -1, 10, -1, -1, -1},
                { -1, -1,  4, 14, 10, -1,  2, -1, -1},
                { -1, -1, -1, -1, -1,  2, -1,  1,  6},
                {  8, 11, -1, -1, -1, -1,  1, -1,  7},
                { -1, -1,  2, -1, -1, -1,  6,  7, -1},
		    	};;
int outputE[9][9];
struct node *vertices;
// struct priority_queue **n;
struct priority_queue n[10][6];      // Change second parameter to t, first parameter to n + 1

mutex mtx, mtx_merge;

void printMST();
// Make t threads and call compute function on all
thread *th;

void mergeTree(int index1, int index2) {
    // This function merges two threads (index2 into index1 adn index1 < index2)

    // printf("Priority queue of thread %d\n", index1);
    // for(int i=0; i<n[N][index1].weight; i++) {
    //     printf("Edge: %d %d %d\n", n[i][index1].v1, n[i][index1].v2, n[i][index1].weight);
    // }
    //
    // printf("Priority queue of thread %d\n", index2);
    // for(int i=0; i<n[N][index2].weight; i++) {
    //     printf("Edge: %d %d %d\n", n[i][index2].v1, n[i][index2].v2, n[i][index2].weight);
    // }

    // Modify the priority queue of thread index1 to include edges of thread index2
    for(int i=0; i<n[N][index2].weight; i++) {
        bool found = false;
        if(vertices[n[i][index2].v2].color != index1) {
            // Now check if that v2 is in the priority queue of thread index1
            // If so, update if necessary
            // Else add that edge to priority queue
            for(int j=0; j<n[N][index1].weight; j++) {
                if(n[j][index1].v2 == n[i][index2].v2) {
                    if(n[i][index2].weight < n[j][index1].weight) {
                        n[j][index1].weight = n[i][index2].weight;
                    }
                    found = true;
                    break;
                }
            }
            if(!found) {
                n[n[N][index1].weight][index1].v1 = n[i][index2].v1;
                n[n[N][index1].weight][index1].v2 = n[i][index2].v2;
                n[n[N][index1].weight][index1].weight = n[i][index2].weight;
            }
        }
    }

    // printf("-------------------------------------------\nPriority queue of thread %d\n", index1);
    // for(int i=0; i<n[N][index1].weight; i++) {
    //     printf("Edge: %d %d %d\n", n[i][index1].v1, n[i][index1].v2, n[i][index1].weight);
    // }

    // This changes the color of nodes of MST of thread index2 to color of index1
    for(int i=0; i<N; i++) {
        if(vertices[i].color == index2) {
            for(int j=0; j<N; j++) {
                if(vertices[j].color == index2) {
                    vertices[j].color = index1;
                }
            }
            vertices[i].color = index1;
        }
    }

}

void compute(int th_id) {
    // This function computes MST for each thread concurrently
    n[N][th_id].weight = 0;    // Initialising size of priority queue
    int idx;
    // priority queue for neighbours of the current MST

    while(true && working[th_id]) {
        bool found1 = false, found2 = false, found3 = false, join = false;
        struct node r;
        // Find an uncolored node randomly
        // Traverse vertices array to find the first uncolored node
        found1 = false;
        for(int i=0; i<N; i++) {
            if(vertices[i].color == -1) {
                r = vertices[i];
                found1 = true;
                break;
            }
        }

        if(!found1) {
            return;
        }

        // printf("Vertice found by thread %d is %d\n", th_id, r.v_no);
        vertices[r.v_no].color = th_id;

        // printf("Priority queue decrease key called by thread %d\n", th_id);

        for(int i=0; i<N; i++) {
            found2 = false;
            if(E[i][r.v_no] != -1) {
                for(int j=0; j<n[N][th_id].weight; j++) {
                    if(n[j][th_id].v1 == i || n[j][th_id].v2 == i) {
                        found2 = true;
                        if(E[r.v_no][i] < n[j][th_id].weight) {
                            if(n[j][th_id].v1 == i) {
                                n[j][th_id].v2 = r.v_no;
                                n[j][th_id].weight = E[i][r.v_no];
                            } else {
                                n[j][th_id].v1 = r.v_no;
                                n[j][th_id].weight = E[i][r.v_no];
                            }
                        }
                    }
                }
                if(!found2) {
                    n[n[N][th_id].weight][th_id].v1 = r.v_no;
                    n[n[N][th_id].weight][th_id].v2 = i;
                    n[n[N][th_id].weight][th_id].weight = E[i][r.v_no];
                    n[N][th_id].weight++;
                }
            }
        }
        // mtx.lock();
        // printf("Priority queue of thread %d\n", th_id);
        // for(int i=0; i<n[N][th_id].weight; i++) {
        //     printf("Edge: %d %d %d\n", n[i][th_id].v1, n[i][th_id].v2, n[i][th_id].weight);
        // }
        // mtx.unlock();

        while(n[N][th_id].weight && working[th_id]) {
            // minimum of the priority queue 'n'
            struct priority_queue minnode;
            minnode.weight = INT_MAX;
            // TO DO: use min heap later
            for(int i=0; i<n[N][th_id].weight; i++) {
                if(n[i][th_id].weight < minnode.weight) {
                    minnode = n[i][th_id];
                    idx = i;
                }
            }

            for(int i=idx + 1; i<n[N][th_id].weight; i++) {
                n[i - 1][th_id] = n[i][th_id];
            }
            n[N][th_id].weight--;

            // printf("minnode selected is: %d %d %d\n", minnode.v1, minnode.v2, minnode.weight);

            if(!working[th_id]) {
                return;
            }

            mtx.lock();
            // printMST();
            if(!working[th_id]) {
                join = true;
            }
            if(vertices[minnode.v2].color == -1 && working[th_id]) {
                // printf("minnode is uncolored\n");
                // include the node in the MST
                vertices[minnode.v2].color = th_id;
                // printf("Color of minnode now: %d\n", vertices[minnode.v2].color);

                // Add the edge to MST
                outputE[minnode.v1][minnode.v2] = minnode.weight;
                outputE[minnode.v2][minnode.v1] = minnode.weight;

                // printf("Edge added by thread %d: %d %d %d\n", th_id, minnode.v1, minnode.v2, minnode.weight);
                //
                // printf("priority queue before:\n");
                // for(int i=0; i<n[N][th_id].weight; i++) {
                //     printf("Edge: %d %d %d\n", n[i][th_id].v1, n[i][th_id].v2, n[i][th_id].weight);
                // }

                for(int i=0; i<N; i++) {
                    found3 = false;
                    if(E[minnode.v2][i] != -1) {
                        for(int j=0; j<n[N][th_id].weight; j++) {
                            if(n[j][th_id].v2 == i) {
                                // printf("i: %d, j: %d\n", i, j);
                                found3 = true;
                                if(E[minnode.v2][i] < n[j][th_id].weight) {
                                    n[j][th_id].v1 = minnode.v2;
                                    n[j][th_id].weight = E[i][minnode.v2];
                                }
                            }
                        }
                        // printf("Color of vertice to be added: %d\n", vertices[i].color);
                        // printf("Found: %d\n", found3);
                        if(!found3 && vertices[i].color != th_id) {
                            n[n[N][th_id].weight][th_id].v1 = minnode.v2;
                            n[n[N][th_id].weight][th_id].v2 = i;
                            n[n[N][th_id].weight][th_id].weight = E[i][minnode.v2];
                            n[N][th_id].weight++;
                        }
                    }
                }

                // printf("priority queue after:\n");
                // for(int i=0; i<n[N][th_id].weight; i++) {
                //     printf("Edge: %d %d %d\n", n[i][th_id].v1, n[i][th_id].v2, n[i][th_id].weight);
                // }

            } else if(vertices[minnode.v2].color != th_id  && working[th_id]) {
                int j = vertices[minnode.v2].color;
                if(th_id < j) {
                    mtx_merge.lock();
                    mergeTree(th_id, j);
                    // Kill j thread
                    working[j] = false;
                    mtx_merge.unlock();
                } else if(j < th_id) {
                    mtx_merge.lock();
                    mergeTree(j, th_id);
                    // Cannot kill th_id thread
                    join = true;
                    working[th_id] = false;
                    mtx_merge.unlock();
                }
            }
            mtx.unlock();
            if(join) {
                // Kill th_id thread
                return;
            }
        }

    }

}

void MSTParallel() {
    th = new thread[t];

    for(int i=0; i<t; i++) {
        working[i] = true;
    }
    for(int i=0; i<t; i++) {
        // printf("I am thread %d\n", i);
        th[i] = thread(compute, i);
    }

    // main thread waits for all threads to compute and finish (and later joins them)
    // th[0].join();
    for(int i=0; i<t; i++) {
        th[i].join();
    }
}

// For debugging reasons
void printMST() {
    for(int i=0; i<N; i++) {
        for(int j=0; j<N; j++) {
            if(outputE[i][j] == -1) {
                cout << "0 ";
            } else {
                cout /*<< vertices[i].color << " "*/ << outputE[i][j] << " " /*<< vertices[j].color << " | "*/;
            }
        }
        cout << endl;
    }
}

int main(int argc, char const *argv[]) {
    // cin >> N;
    // E = new int[N][N];
    // outputE = new int[N][N];

    vertices = new struct node[N];

    // n = new struct priority_queue[7][t];
    // Nth index is size of the priority queue

    // Initialising the adjacency matrix
    for(int i=0; i<N; i++) {
        for(int j=0; j<N; j++)
            // -1 means no edge b/w i and j
            // E[i][j] = -1;
            outputE[i][j] = -1;
    }

    for(int i=0; i<N; i++) {
        // '-1' color means uncolored
        vertices[i].color = -1;
        // intially parent is same as node
        vertices[i].parent = i;
        vertices[i].v_no = i;
    }

    // Take input as an edge and store it in adjacency matrix
    // int a, v1, v2, edge_w;
    // while(true) {
    //     cout << "Press 1 to enter edge and 0 to exit: " << endl;
    //     cin >> a;
    //     if(a == 1) {
    //         cin >> v1 >> v2 >> edge_w;
    //         // The tree is a undirected one
    //         E[v1 - 1][v2 - 1] = edge_w;
    //         E[v2 - 1][v1 - 1] = edge_w;
    //     } else {
    //         break;
    //     }
    //
    // }

    cout << "The input Graph is: " << endl;
    for(int i=0; i<N; i++) {
        for(int j=0; j<N; j++) {
            if(E[i][j] != -1) {
                cout << E[i][j] << " ";
            } else {
                cout << "0 ";
            }
        }
        cout << endl;
    }

    MSTParallel();

    cout << "The final MST is: " << endl;
    printMST();
  return 0;
}
