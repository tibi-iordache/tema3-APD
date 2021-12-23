#include "mpi.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <iterator>
#include <sstream>

using namespace std;

#define min(a, b) a < b ? a : b

#define MASTER 0
#define CLUSTERS_COUNT 3

vector<int> read_file(int rank) {
    int workers_no;
    vector<int> workers;

    stringstream file_name_stream;
    // TODO: remove the absolute path
    // file_name_stream << "../tests/test1/cluster" << rank << ".txt";
    file_name_stream << "cluster" << rank << ".txt";

    ifstream input_file(file_name_stream.str());

    if (!input_file.is_open()) {
        // error
        return workers;
    }

    input_file >> workers_no;


    for (int i = 0; i < workers_no; ++i) {
        int new_worker;
        input_file >> new_worker;

        workers.push_back(new_worker);
    }

    return workers;
}

void print_topology(int rank, map<int, vector<int>> topology) {
    map<int, vector<int>>::iterator itr;

    cout << rank << " -> ";

    for (itr = topology.begin(); itr != topology.end(); ++itr) {
        cout << itr->first << ":";
        
        vector<int> values = itr->second;

        for (int i = 0; i < values.size(); ++i) {
            cout << values.at(i);

            if (i != values.size() - 1) cout << ",";
        }

        cout << " ";
    }

    cout << endl;
}

int main (int argc, char *argv[])
{   
    int num_tasks, workers_count, rank;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &num_tasks);
    MPI_Comm_rank(MPI_COMM_WORLD,&rank);

    workers_count = num_tasks - CLUSTERS_COUNT;
    
    vector<int> workers;

    // read input from files
    if (rank < 3) {
        workers = read_file(rank);

        if (workers.empty()) {
            exit(EXIT_FAILURE);
        }
    }

    // establishing the topology
    if (rank < 3) {
        // send your rank to the workers
        for (int i = 0; i < workers.size(); ++i) {
            MPI_Send(&rank, 1, MPI_INT, workers.at(i), 0, MPI_COMM_WORLD);
            // TODO: uncomment later
            cout << "M(" << rank << "," << workers.at(i) << ")" << endl;
        }
        
        // send your workers to your neighbours
        int prev_rank = rank == 0 ? 2 : rank - 1;
        int next_rank = (rank + 1) % 3;
        int workers_size = workers.size();

        int prev_coordonator_workers_size, next_coordonator_workers_size;
        vector<int> prev_coordonator_workers;
        vector<int> next_coordonator_workers;

        // send the workers size
        MPI_Send(&workers_size, 1, MPI_INT, next_rank, 0, MPI_COMM_WORLD);
        // TODO: uncomment later
        cout << "M(" << rank << "," << next_rank << ")" << endl;

        MPI_Recv(&prev_coordonator_workers_size, 1, MPI_INT, prev_rank, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        // send the workers
        for (int i = 0; i < workers_size; ++i) {
            MPI_Send(&workers.at(i), 1, MPI_INT, next_rank, 0, MPI_COMM_WORLD);

            // TODO: uncomment later
            cout << "M(" << rank << "," << next_rank << ")" << endl;
        }

        // recv the workers
        for (int i = 0; i < prev_coordonator_workers_size; ++i) {
            int worker_rank;
            
            MPI_Recv(&worker_rank, 1, MPI_INT, prev_rank, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            prev_coordonator_workers.push_back(worker_rank);
        }

        // send the workers size
        MPI_Send(&workers_size, 1, MPI_INT, prev_rank, 0, MPI_COMM_WORLD);
        // TODO: uncomment later
        cout << "M(" << rank << "," << prev_rank << ")" << endl;

        MPI_Recv(&next_coordonator_workers_size, 1, MPI_INT, next_rank, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        

        // send the workers
        for (int i = 0; i < workers_size; ++i) {
            MPI_Send(&workers.at(i), 1, MPI_INT, prev_rank, 0, MPI_COMM_WORLD);

            // TODO: uncomment later
            cout << "M(" << rank << "," << prev_rank << ")" << endl;
        }
        
        // recv the workers
        for (int i = 0; i < next_coordonator_workers_size; ++i) {
            int worker_rank;
            MPI_Recv(&worker_rank, 1, MPI_INT, next_rank, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

            next_coordonator_workers.push_back(worker_rank);
        }


        // send topology to the workers
        for (int i = 0; i < workers_size; ++i) {
            // send prev
            MPI_Send(&prev_rank, 1, MPI_INT, workers.at(i), 0, MPI_COMM_WORLD);
            // TODO: uncomment later
            cout << "M(" << rank << "," << workers.at(i) << ")" << endl;  

            MPI_Send(&prev_coordonator_workers_size, 1, MPI_INT, workers.at(i), 0, MPI_COMM_WORLD);
            // TODO: uncomment later
            cout << "M(" << rank << "," << workers.at(i) << ")" << endl;    

            for (int j = 0; j < prev_coordonator_workers_size; ++j) {
                MPI_Send(&prev_coordonator_workers.at(j), 1, MPI_INT, workers.at(i), 0, MPI_COMM_WORLD);
                // TODO: uncomment later
                cout << "M(" << rank << "," << workers.at(i) << ")" << endl; 
            }

            // send yours
            MPI_Send(&rank, 1, MPI_INT, workers.at(i), 0, MPI_COMM_WORLD);
            // TODO: uncomment later
            cout << "M(" << rank << "," << workers.at(i) << ")" << endl;  

            MPI_Send(&workers_size, 1, MPI_INT, workers.at(i), 0, MPI_COMM_WORLD);
            // TODO: uncomment later
            cout << "M(" << rank << "," << workers.at(i) << ")" << endl;    

            for (int j = 0; j < workers_size; ++j) {
                MPI_Send(&workers.at(j), 1, MPI_INT, workers.at(i), 0, MPI_COMM_WORLD);
                // TODO: uncomment later
                cout << "M(" << rank << "," << workers.at(i) << ")" << endl; 
            }

            // send next
            MPI_Send(&next_rank, 1, MPI_INT, workers.at(i), 0, MPI_COMM_WORLD);
            // TODO: uncomment later
            cout << "M(" << rank << "," << workers.at(i) << ")" << endl;  

            MPI_Send(&next_coordonator_workers_size, 1, MPI_INT, workers.at(i), 0, MPI_COMM_WORLD);
            // TODO: uncomment later
            cout << "M(" << rank << "," << workers.at(i) << ")" << endl;    

            for (int j = 0; j < next_coordonator_workers_size; ++j) {
                MPI_Send(&next_coordonator_workers.at(j), 1, MPI_INT, workers.at(i), 0, MPI_COMM_WORLD);
                // TODO: uncomment later
                cout << "M(" << rank << "," << workers.at(i) << ")" << endl; 
            }
        }
        
        
        // TODO: print topology
        map<int, vector<int>> topology;
        
        topology[prev_rank] = prev_coordonator_workers;
        topology[rank] = workers;
        topology[next_rank] = next_coordonator_workers;
        
        print_topology(rank, topology);

        // --------------------------------------------------------------------
        if (rank == MASTER) {
            int N = atoi(argv[1]);

            vector<int> V;

            for (int i = 0; i < N; ++i) {
                V.push_back(i);
            }

            int total_workers_count = prev_coordonator_workers_size +
                                        workers_size + 
                                        next_coordonator_workers_size;

            // send first batch of the vector to its workers
            for (int i = 0; i < workers_size; ++i) {
                int start = i * (double)N / total_workers_count;
                int end = min((i + 1) * (double)N / total_workers_count, N);

                int elements_count = end - start;

                MPI_Send(&elements_count, 1, MPI_INT, workers.at(i), 0, MPI_COMM_WORLD);
                // TODO: uncomment later
                cout << "M(" << rank << "," << workers.at(i) << ")" << endl; 

                for (int j = start; j < end; ++j) {
                    MPI_Send(&V[j], 1, MPI_INT, workers.at(i), 0, MPI_COMM_WORLD);
                    // TODO: uncomment later
                    cout << "M(" << rank << "," << workers.at(i) << ")" << endl; 
                }
            }

            // send the rest of the vector to other coordonators
            
            // coord 1
            int start = (N / total_workers_count) * workers_size;
            int end = (N / total_workers_count) * (workers_size + next_coordonator_workers_size);

            int elements_count = end - start;

            MPI_Send(&elements_count, 1, MPI_INT, next_rank, 0, MPI_COMM_WORLD);
            // TODO: uncomment later
            cout << "M(" << rank << "," << next_rank << ")" << endl; 

            for (int j = start; j < end; ++j) {
                MPI_Send(&V[j], 1, MPI_INT, next_rank, 0, MPI_COMM_WORLD);
                // TODO: uncomment later
                cout << "M(" << rank << "," << next_rank << ")" << endl; 
            }

            // coord 2
            start = (N / total_workers_count) * (workers_size + next_coordonator_workers_size);
            end = N;

            elements_count = end - start;

            MPI_Send(&elements_count, 1, MPI_INT, prev_rank, 0, MPI_COMM_WORLD);
            // TODO: uncomment later
            cout << "M(" << rank << "," << prev_rank << ")" << endl; 

            for (int j = start; j < end; ++j) {
                MPI_Send(&V[j], 1, MPI_INT, prev_rank, 0, MPI_COMM_WORLD);
                // TODO: uncomment later
                cout << "M(" << rank << "," << prev_rank << ")" << endl; 
            }

            // recv computed values from workers
            vector<int> final_vector;
            
            for (int i = 0; i < workers_size; ++i) {
                int elements_count;
                MPI_Recv(&elements_count, 1, MPI_INT, workers.at(i), 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

                for (int j = 0; j < elements_count; ++j) {
                    int num;
                    MPI_Recv(&num, 1, MPI_INT, workers.at(i), 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

                    final_vector.push_back(num);
                }
            }

            // recv computed values from coord

            // coord 1
            MPI_Recv(&elements_count, 1, MPI_INT, next_rank, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

            for (int j = 0; j < elements_count; ++j) {
                int num;
                MPI_Recv(&num, 1, MPI_INT, next_rank, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

                final_vector.push_back(num);
            }

            // coord 2
            MPI_Recv(&elements_count, 1, MPI_INT, prev_rank, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

            for (int j = 0; j < elements_count; ++j) {
                int num;
                MPI_Recv(&num, 1, MPI_INT, prev_rank, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

                final_vector.push_back(num);
            }

            // print result
            cout << "Rezultat: ";

            for (int i = 0; i < final_vector.size(); ++i) {
                cout << final_vector.at(i) << " ";
            }

            cout << endl;
        } else {
            // TODO: recv vector values from master
            int vector_size;
            vector<int> V;

            MPI_Recv(&vector_size, 1, MPI_INT, MASTER, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

            for (int i = 0; i < vector_size; ++i) {
                int num;
                MPI_Recv(&num, 1, MPI_INT, MASTER, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

                V.push_back(num);
            }

            // send to workers
            for (int i = 0; i < workers_size; ++i) {
                int start = i * (double)vector_size / workers_size;
                int end = min((i + 1) * (double)vector_size / workers_size, vector_size);

                int elements_count = end - start;

                MPI_Send(&elements_count, 1, MPI_INT, workers.at(i), 0, MPI_COMM_WORLD);
                // TODO: uncomment later
                cout << "M(" << rank << "," << workers.at(i) << ")" << endl; 

                for (int j = start; j < end; ++j) {
                    MPI_Send(&V[j], 1, MPI_INT, workers.at(i), 0, MPI_COMM_WORLD);
                    // TODO: uncomment later
                    cout << "M(" << rank << "," << workers.at(i) << ")" << endl; 
                }
            }
            
            // recv computed values
            vector<int> computed_values;

            for (int i = 0; i < workers_size; ++i) {
                int elements_count;
                MPI_Recv(&elements_count, 1, MPI_INT, workers.at(i), 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

                for (int j = 0; j < elements_count; ++j) {
                    int num;
                    MPI_Recv(&num, 1, MPI_INT, workers.at(i), 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

                    computed_values.push_back(num);
                }
            }

            // send to master computed values
            int computed_values_size = computed_values.size();

            MPI_Send(&computed_values_size, 1, MPI_INT, MASTER, 0, MPI_COMM_WORLD);
            // TODO: uncomment later
            cout << "M(" << rank << "," << MASTER << ")" << endl; 

            for (int i = 0; i < computed_values_size; ++i) {
                MPI_Send(&computed_values[i], 1, MPI_INT, MASTER, 0, MPI_COMM_WORLD);
                // TODO: uncomment later
                cout << "M(" << rank << "," << MASTER << ")" << endl; 
            }
        }

    } else {
        int coordonator_rank;

        // receive your coordonator rank
        MPI_Recv(&coordonator_rank, 1, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        
        // receive the topology
        map<int, vector<int>> topology;

        for (int i = 0; i < CLUSTERS_COUNT; ++i) {
            int current_rank;
            MPI_Recv(&current_rank, 1, MPI_INT, coordonator_rank, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

            int cluster_size;
            
            MPI_Recv(&cluster_size, 1, MPI_INT, coordonator_rank, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

            vector<int> values;

            for (int j = 0; j < cluster_size; ++j) {
                int num;
                MPI_Recv(&num, 1, MPI_INT, coordonator_rank, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

                values.push_back(num);
            }

            topology[current_rank] = values;
        }

        print_topology(rank, topology);

        // --------------------------------------------------------------------

        // recv vector values from coord
        int elements_count;
        vector<int> computed_values;

        MPI_Recv(&elements_count, 1, MPI_INT, coordonator_rank, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        for (int i = 0; i < elements_count; ++i) {
            int num;
            MPI_Recv(&num, 1, MPI_INT, coordonator_rank, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

            num *= 2;

            computed_values.push_back(num);
        }

        int computed_values_size = computed_values.size();

        MPI_Send(&computed_values_size, 1, MPI_INT, coordonator_rank, 0, MPI_COMM_WORLD);
        // TODO: uncomment later
        cout << "M(" << rank << "," << coordonator_rank << ")" << endl; 

        // send values to coord
        for (int i = 0; i < computed_values_size; ++i) {
            MPI_Send(&computed_values[i], 1, MPI_INT, coordonator_rank, 0, MPI_COMM_WORLD);
            // TODO: uncomment later
            cout << "M(" << rank << "," << coordonator_rank << ")" << endl; 
        }
    }

    MPI_Finalize();
    return 0;
}
