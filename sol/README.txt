Iordache Tiberiu-Mihai 336CA

	Tema 3 APD

	TODO: remove makefile run_default rule





	// TODO: remove the absolute path
	// file_name_stream << "../tests/test1/cluster" << rank << ".txt";

	run_default_test:
	mpirun --oversubscribe -np 9 ./tema3 12 0 