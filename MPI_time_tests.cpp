#include "mpi.h"
#include "stdc++.h"
#include <iostream>
#include <cstdlib>
#include <unistd.h>

using namespace std;

int main(int argc, char* argv[]) {

    MPI::Init(argc, argv);

    auto mpisize = MPI::COMM_WORLD.Get_size();
    auto mpirank = MPI::COMM_WORLD.Get_rank();
    auto mpiroot = 0;
    double t1, t2, individualTime;
    double sendarray[mpisize];
    double totalTime;

    for (int i = 0; i < mpisize; i++){

        if (mpirank == i){
            t1 = MPI::Wtime();
            usleep(1000000);
            t2 = MPI::Wtime();
            individualTime = t2 - t1;
        }
    }

    MPI_Gather(&individualTime, 1, MPI::DOUBLE, sendarray, 1, MPI::DOUBLE, mpiroot, MPI::COMM_WORLD); 
    MPI_Reduce(&individualTime, &totalTime, 1, MPI::DOUBLE, MPI::SUM, 0, MPI::COMM_WORLD);

    if (mpirank == 0){

        for (int i = 0; i < mpisize; i++){
            cout << "Process " << i << ": " << sendarray[i] << " seconds" << endl << "---------------" << endl;
        }
        cout << "Total time: " << totalTime << " seconds" << endl;
    }

    MPI::Finalize();
    return 0;
}