#include "mpi.h"
#include "stdc++.h"

using namespace std;

int main(int argc, char* argv[]) {


    MPI::Init(argc, argv);

    auto mpisize = MPI::COMM_WORLD.Get_size();
    auto mpirank = MPI::COMM_WORLD.Get_rank();
    auto mpiroot = 0;
    const int living = 1;
    const int dead = 0;
    int numRows = atoi(argv[1]);
    int numColumns = atoi(argv[2]);
    int numIterations = atoi(argv[3]);
    


    if (argc != 4){
        cout << "4 process minimum" << endl;
        exit(1);
    }

    if (numRows < 1 or numColumns < 1 or numIterations < 1){
        cout << "Invalid command line arguments" << endl;
        exit(1);
    }
    

    //numRowsLocal is the amount of rows held be each process. If we have 5 rows, and are performing 5 processes, then each proc will be handling 1 row
    auto numRowsLocal = numRows / mpisize;
    if (mpirank == mpisize - 1){
        numRowsLocal += numRows % mpisize; //put remaining rows on the last rank
    }
    // numRowsWithBorder adds in the two extra rows needed for the board to "wrap" -- same applies for numColumnsWithBorder
    auto numRowsWithBorder = numRowsLocal + 2;
    auto numColumnsWithBorder = numColumns + 2;
    
    /* 
    world is a 2d-vector of ints
    the x-axis is the number of rows with our "ghost rows" 
    the y-axis is the number of columns with border 
    newWorld is identical to the initial world, but will be used for updating 
    */
    vector<vector<int> > world(numRowsWithBorder, vector<int>(numColumnsWithBorder, 0));
    vector<vector<int> > newWorld(numRowsWithBorder, vector<int>(numColumnsWithBorder, 0));
    
  


    /* 
    the outer loop start at the second row in order to skip the border row
    continue the loop as long as the row we are on is less than or equal to the number of rows in this specific process
    nested loop starts at the first column in order to skip the border column -- we are not accessing individual cells
    for each cell generate a random number that is either 0 or 1 and assign it to a cell 
    */
    for (auto ith_row = 1; ith_row <= numRowsLocal; ith_row++){
        for (auto ith_column = 1; ith_column <= numColumnsWithBorder; ith_column++){
            world[ith_row][ith_column] = rand() % 2; // 1 = living -- 0 = dead
        }
    }

    /* 
    these are created to make our sends and recieves easy as we can determine neighboring processes with these two lines
    if the current process is process 0 (the top of the board), then our border row above will be recieved from the last process
    otherwise our bordering row above is recieved from the process before the current process
    inverse logic applies to determining the bordering row below
    */
    auto neighborAbove = (mpirank == 0) ? mpisize - 1 : mpirank - 1;
    auto neighborBelow = (mpirank == mpisize - 1) ? 0 : mpirank + 1;
    
    // main loop where life cycle is performed
    for (auto ith_iteration = 0; ith_iteration < numIterations; ith_iteration++) {
        
        //ghost rows
        //send top row to rank above
        MPI::COMM_WORLD.Send(&world[1][0], numColumnsWithBorder, MPI::INT, neighborAbove, 0);

        //send bottom row to rank bellow
        MPI::COMM_WORLD.Send(&world[numRowsLocal][0], numColumnsWithBorder, MPI::INT, neighborBelow, 0);
        
        //recieve bottom row from bellow
        MPI::COMM_WORLD.Recv(&world[numRowsLocal + 1][0], numColumnsWithBorder, MPI::INT, neighborBelow, 0);

        // recieve top row from above
        MPI::COMM_WORLD.Recv(&world[0][0], numColumnsWithBorder, MPI::INT, neighborAbove, 0);


        /* 
        does not require communication because we are seperating the world by row
        the border column for each side is simply the last column on the opposite side of the processes
        so here we simply set our border columns equal to the column on the opposite side of board
        */
        for (auto ith_row = 0; ith_row < numRowsWithBorder; ith_row++) {
            world[ith_row][0] = world[ith_row][numColumns];
            world[ith_row][numColumns + 1] = world[ith_row][1];
        }

        //display current world
        // all ranks send their data to the root for printing as to avoid the board printing in random order
        if (mpirank != mpiroot){
            for (int ith_row = 1; ith_row <= numRowsLocal; ith_row++){
                MPI::COMM_WORLD.Send(&world[ith_row][1], numColumns, MPI::INT, mpiroot, 0);
            }
        }
        
        //start the printing process by showing which cycle we are printing
        if (mpirank == mpiroot){
            cout << "\n\n" << "iteration: " << ith_iteration + 1 << endl;

            bool stayedTheSame = true;
            //this prints out only the data assigned to the root rank
            for (auto ith_row = 1; ith_row <= numRowsLocal; ith_row++){
                for(auto ith_column = 1; ith_column <= numColumns; ith_column++){
                    cout << world[ith_row][ith_column] << " ";
                }
                cout << endl;
            }

            
            // loop that recieves all of the data sent to the root process
            for (auto fromRank = 1; fromRank < mpisize; fromRank++) {
                auto numRecieved = numRows / mpisize; //the number of recieves coming in

                //factors for our last rank having a greater number of rows
                if (fromRank == mpisize - 1){
                    numRecieved += numRows % mpisize;
                }
                // recvDataHolder is a vector that will store all of the data being recieved from other ranks
                vector<int> recvDataHolder(numColumns, 0);
                for (auto ith_recv = 0; ith_recv < numRecieved; ith_recv++){
                    MPI::COMM_WORLD.Recv(&recvDataHolder[0], numColumns, MPI::INT, fromRank, 0);

                    // print each number recieved and held inside of recvDataHolder 
                    for (auto i : recvDataHolder){
                        cout << i << " ";
                    }
                    cout << endl;
                }
            }
        }

        //we start at index 1 so the we are not checking any of the border cells
        for (auto ith_row = 1; ith_row <= numRowsLocal; ith_row++){
            for (auto ith_column = 1; ith_column <= numColumns; ith_column++){
                auto numLivingNeighbors = 0; //counter for living neighbors

                // goes through each of the 8 surrounding neighbors, and if they are living, we add one to numLivingNeighbors
                for (auto jth_row = ith_row - 1; jth_row <= ith_row + 1; jth_row++){
                    for (auto jth_column = ith_column - 1; jth_column <= ith_column + 1; jth_column++){
                        if ((jth_row != ith_row or jth_column != ith_column) and world[jth_row][jth_column] == living){
                            numLivingNeighbors++;
                        }
                    } 
                }


                /* 
                after counting up the total number of living neighbors we perform the main logic of the game of life:
                1. Any live cell with fewer than two live neighbours dies, as if by underpopulation.
                2. Any live cell with two or three live neighbours lives on to the next generation.
                3. Any live cell with more than three live neighbours dies, as if by overpopulation.
                4. Any dead cell with exactly three live neighbours becomes a live cell, as if by reproduction.
                */
                if (numLivingNeighbors < 2){
                    newWorld[ith_row][ith_column] = dead;
                }
                else if (world[ith_row][ith_column] == living and (numLivingNeighbors == 2 or numLivingNeighbors == 3)){
                    newWorld[ith_row][ith_column] = living;
                }
                else if (numLivingNeighbors > 3) {
                    newWorld[ith_row][ith_column] = dead;
                }
                else if (world[ith_row][ith_column] == dead and numLivingNeighbors == 3){
                    newWorld[ith_row][ith_column] = living;
                }

            }
        }

        MPI_Barrier(MPI::COMM_WORLD);

        int changeGlobal;
        int changeLocal;

        if (ith_iteration > 1 and world == newWorld){
            changeLocal = 0;
        }
        else {
            changeLocal = 1;
        }

        MPI_Reduce(&changeLocal, &changeGlobal, 1, MPI::INT, MPI::SUM, 0, MPI::COMM_WORLD);

        if (changeGlobal >= 1){
            for (auto ith_row = 1; ith_row <= numRowsLocal; ith_row++) {
                for (auto ith_column = 1; ith_column <= numColumns; ith_column++){
                    world[ith_row][ith_column] = newWorld[ith_row][ith_column];
            }
        }
        }
        else{
            MPI_Abort(MPI::COMM_WORLD, 1);
        }
    
    } //end of main loop

    //mpi barrier
    //mpi gather -- gather to process 0 -- mpi allgather -- optimization
    //mpi_time 
    MPI::Finalize();
    return 0;
}

//to compile: mpic++ -std=c++14  mpi_gol.cpp -o life
//to run: mpirun -np 4 ./life 3 5 100  (command line arguments are in the following order: rows, columns, number of life cycles)