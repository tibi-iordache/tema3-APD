// Iordache Tiberiu-Mihai 336CA

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

vector<int> read_file(int rank)
{
	int workers_no;
	vector<int> workers;

	stringstream file_name_stream;
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

void print_log_message(int sender, int receiver)
{
	// using stringstream to avoid overlapping prints
	stringstream log_message;

	log_message << "M(" << sender << "," << receiver << ")" << endl; 

	cout << log_message.str();
}

void print_topology(int rank, map<int, vector<int>> topology)
{
	map<int, vector<int>>::iterator itr;
	stringstream message;

	message << rank << " -> ";

	for (itr = topology.begin(); itr != topology.end(); ++itr) {
		message << itr->first << ":";
		
		vector<int> values = itr->second;

		for (int i = 0; i < values.size(); ++i) {
			message << values[i];

			if (i != values.size() - 1) message << ",";
		}

		message << " ";
	}

	message << endl;

	cout << message.str();
}

void print_result_vector(vector<int> v)
{
	stringstream message;
	message << "Rezultat: ";

	for (int i = 0; i < v.size(); ++i) {
		message << v[i] << " ";
	}

	message << endl;
	cout << message.str();
}

// sends each vector elem one by one
void send_vector_elems(vector<int> v,
                       int size,
                       int start,
                       int end,
                       int sender,
                       int receiver)
{
	int elements_count = end - start;

	MPI_Send(&size, 1, MPI_INT, receiver, 0, MPI_COMM_WORLD);
	print_log_message(sender, receiver);

	for (int i = start; i < end; ++i) {
		MPI_Send(&v[i], 1, MPI_INT, receiver, 0, MPI_COMM_WORLD);
		print_log_message(sender, receiver);
	}
}

// sends the whole vector 
void send_vector_data(vector<int> v, int size, int sender, int receiver)
{
	// send the size first
	MPI_Send(&size, 1, MPI_INT, receiver, 0, MPI_COMM_WORLD);
	print_log_message(sender, receiver);

	MPI_Send(v.data(), size, MPI_INT, receiver, 0, MPI_COMM_WORLD);
	print_log_message(sender, receiver);
}

// recv each vector elem one by one 
vector<int> recv_vector_elems(int sender)
{
	int size;
	MPI_Recv(&size, 1, MPI_INT, sender, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

	vector<int> vector_data;

	for (int i = 0; i < size; ++i) {
		int num;
		MPI_Recv(&num, 1, MPI_INT, sender, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

		vector_data.push_back(num);
	}

	return vector_data;
}

// recv the whole vector
vector<int> recv_vector_data(int *size, int sender)
{
	// recv the vector size first
	MPI_Recv(&(*size), 1, MPI_INT, sender, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

	vector<int> vector_data((*size));

	MPI_Recv(vector_data.data(), (*size), MPI_INT, sender, 0,
			 MPI_COMM_WORLD, MPI_STATUS_IGNORE);

	return vector_data;
}

// returns the topology
map<int, vector<int>>  establish_topology_coord(int rank, 
												vector<int> workers) {
	// send your rank to the workers
	for (int i = 0; i < workers.size(); ++i) {
		MPI_Send(&rank, 1, MPI_INT, workers[i], 0, MPI_COMM_WORLD);
		print_log_message(rank, workers[i]);
	}
	
	// send your workers ranks to your neighbours
	int prev_rank = rank == 0 ? 2 : rank - 1;
	int next_rank = (rank + 1) % 3;
	int workers_size = workers.size();

	int prev_coordonator_workers_size, next_coordonator_workers_size;

	// send to the next neighbour and then recv from the prev one
	send_vector_data(workers, workers.size(), rank, next_rank);

	vector<int> prev_coordonator_workers = 
				recv_vector_data(&prev_coordonator_workers_size, prev_rank);

	// send to the prev neighbour and then recv from the next one
	send_vector_data(workers, workers.size(), rank, prev_rank);

	vector<int> next_coordonator_workers =
				recv_vector_data(&next_coordonator_workers_size, next_rank);

	// send topology to the workers
	for (int i = 0; i < workers_size; ++i) {
		// send the rank first
		MPI_Send(&prev_rank, 1, MPI_INT, workers[i], 0, MPI_COMM_WORLD);
		print_log_message(rank, workers[i]);

		// send prev coordinator workers ranks
		send_vector_data(prev_coordonator_workers,
							prev_coordonator_workers.size(),
							rank, 
							workers[i]);
		
		MPI_Send(&rank, 1, MPI_INT, workers[i], 0, MPI_COMM_WORLD);
		print_log_message(rank, workers[i]);

		// send your workers ranks
		send_vector_data(workers,
							workers.size(),
							rank, 
							workers[i]);
		
		MPI_Send(&next_rank, 1, MPI_INT, workers[i], 0, MPI_COMM_WORLD);
		print_log_message(rank, workers[i]);

		// send next coordinator workers ranks
		send_vector_data(next_coordonator_workers,
							next_coordonator_workers.size(),
							rank, 
							workers[i]);
	}
	
	// print topology
	map<int, vector<int>> topology;
	
	topology[prev_rank] = prev_coordonator_workers;
	topology[rank] = workers;
	topology[next_rank] = next_coordonator_workers;
	
	print_topology(rank, topology);

	return topology;
}

// returns the coord rank
int establish_topology_worker(int rank) {
	int coordinator_rank;

	// receive your coordinator rank
	MPI_Recv(&coordinator_rank,
				1,
				MPI_INT,
				MPI_ANY_SOURCE,
				0,
				MPI_COMM_WORLD,
				MPI_STATUS_IGNORE);
	
	// receive the topology
	map<int, vector<int>> topology;

	for (int i = 0; i < CLUSTERS_COUNT; ++i) {
		int current_rank;
		MPI_Recv(&current_rank,
					1,
					MPI_INT,
					coordinator_rank,
					0,
					MPI_COMM_WORLD,
					MPI_STATUS_IGNORE);
		
		int cluster_size;
		vector<int> values = recv_vector_data(&cluster_size, coordinator_rank);

		topology[current_rank] = values;            
	}

	print_topology(rank, topology);

	return coordinator_rank;
}

void compute_vector_values_coord(int rank,
								 map<int, vector<int>> topology, 
								 int N) {
	if (rank == MASTER) {
		int prev_rank = rank == 0 ? 2 : rank - 1;
		int next_rank = (rank + 1) % 3;

		vector<int> prev_coord_workers = topology[prev_rank];
		vector<int> workers = topology[rank];
		vector<int> next_coord_workers = topology[next_rank];
		
		// generate the inital values of the vector
		vector<int> V;

		for (int i = 0; i < N; ++i) {
			V.push_back(i);
		}

		int total_workers_count = prev_coord_workers.size() +
									workers.size() + 
									next_coord_workers.size();

		// send first batch of the vector to this coordinator workers
		for (int i = 0; i < workers.size(); ++i) {
			// divide equally between workers
			int start = i * (double)N / total_workers_count;
			int end = min((i + 1) * (double)N / total_workers_count, N);

			int elements_count = end - start;

			send_vector_elems(V, elements_count, start, end, rank, workers[i]);
		}

		// send the rest of the vector to other coordinators
		// coord 1
		int start = (N / total_workers_count) * workers.size();
		int end = (N / total_workers_count) 
				* (workers.size() + next_coord_workers.size());

		int elements_count = end - start;

		send_vector_elems(V, elements_count, start, end, rank, next_rank);

		// coord 2
		start = (N / total_workers_count) 
				* (workers.size() + next_coord_workers.size());
		end = N;

		elements_count = end - start;

		send_vector_elems(V, elements_count, start, end, rank, prev_rank);

		// recv computed values from workers
		vector<int> final_vector;
		
		for (int i = 0; i < workers.size(); ++i) {
			vector<int> recv_elems = recv_vector_elems(workers.at(i));

			final_vector.insert(final_vector.end(), 
								recv_elems.begin(),
								recv_elems.end());
		}

		// recv computed values from coordinators
		// coord 1
		vector<int> coord1_elems = recv_vector_elems(next_rank);
		final_vector.insert(final_vector.end(),
							coord1_elems.begin(),
							coord1_elems.end());

		// coord 2
		vector<int> coord2_elems = recv_vector_elems(prev_rank);
		final_vector.insert(final_vector.end(),
							coord2_elems.begin(),
							coord2_elems.end());

		// print the computed values
		print_result_vector(final_vector);
	} else {
		vector<int> workers = topology[rank];

		// recv vector values from master
		vector<int> V = recv_vector_elems(MASTER);

		// send to workers
		for (int i = 0; i < workers.size(); ++i) {
			// divide equally between workers
			int start = i * (double)V.size() / workers.size();
			int end = min((i + 1) * (double)V.size() / workers.size(), V.size());

			int elements_count = end - start;

			send_vector_elems(V, elements_count, start, end, rank, workers[i]);
		}
		
		// recv computed values
		vector<int> computed_values;

		for (int i = 0; i < workers.size(); ++i) {
			vector<int> computed_values_from_worker =
										recv_vector_elems(workers[i]);

			computed_values.insert(computed_values.end(),
									computed_values_from_worker.begin(),
									computed_values_from_worker.end());
		}

		// send to master computed values
		int computed_values_size = computed_values.size();

		send_vector_elems(computed_values,
							computed_values_size,
							0,
							computed_values_size,
							rank,
							MASTER);
	}
}

void compute_vector_values_worker(int rank, int coordonator_rank) {
	// receive vector values from coordinator
	int elements_count;
	vector<int> computed_values;

	MPI_Recv(&elements_count,
				1,
				MPI_INT,
				coordonator_rank,
				0,
				MPI_COMM_WORLD,
				MPI_STATUS_IGNORE);

	for (int i = 0; i < elements_count; ++i) {
		int num;
		MPI_Recv(&num,
					1,
					MPI_INT,
					coordonator_rank,
					0,
					MPI_COMM_WORLD,
					MPI_STATUS_IGNORE);

		// multiply each value before storing it
		num *= 2;

		computed_values.push_back(num);
	}

	// send the computed values back to the coordinator
	int computed_values_size = computed_values.size();

	send_vector_elems(computed_values,
						computed_values_size,
						0,
						computed_values_size,
						rank,
						coordonator_rank);
}


int main (int argc, char *argv[])
{   
	int rank;

	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD,&rank);

	vector<int> workers;

	// read input from files (only the coordinators)
	if (rank < CLUSTERS_COUNT) {
		workers = read_file(rank);

		if (workers.empty()) {
			exit(EXIT_FAILURE);
		}
	}

	// solve the tasks
	if (rank < CLUSTERS_COUNT) {
		map<int, vector<int>> topology = establish_topology_coord(rank, workers);
		
		int N = atoi(argv[1]);

		compute_vector_values_coord(rank, topology, N);
	}
	else {
		int coord_rank = establish_topology_worker(rank);

		compute_vector_values_worker(rank, coord_rank);
	}

	MPI_Finalize();

	return 0;
}
