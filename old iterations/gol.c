#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#define SLEEP usleep(30000)
#define CLS printf("\033[H\033[J")


#define ROWS 20
#define COLS 50



int life;
int fertility;
char world1[COLS][ROWS];
char world2[COLS][ROWS];
char menu;

void seed(){
	for (int j = 0; j < ROWS; j++){
		for (int i = 0; i < COLS; i++){
			life = (rand() % 10);
			if (life < fertility) world1[i][j]='#';
			else world1[i][j]=' ';
		}
	}
}

void printWorld(char world[COLS][ROWS]){
	for (int j = 0; j < ROWS; j++){
		for(int i = 0 ;i < COLS; i++){
			printf("%c", world[i][j]);
		}
		printf("\n"); 
	}
}

void evaluate(int b, int c, char world_a[COLS][ROWS], char world_b[COLS][ROWS]){
	int neighbors = 0;
	for (int i = -1; i <= 1; i++) {
		for (int j = -1; j <= 1; j++) {
			if ((i == 0 && j == 0)				
				|| b + i >= COLS				
				|| b + i < 0
				|| c + j >= ROWS
				|| c + j < 0) 
			{
				continue;						
			}
			if (world_a[b + i][c + j] == '#') neighbors++;
		}
	}
	
	if (neighbors == 3 || (neighbors == 2 && world_a[b][c] == '#')) {
		world_b[b][c] = '#';
	} 
	else {
		world_b[b][c] = ' ';
	}
}

void step(char world_a[COLS][ROWS], char world_b[COLS][ROWS]){ // Iterates through every tile.
	for (int j = 0; j < ROWS; j++){
		for (int i = 0; i < COLS; i++){
			evaluate(i, j, world_a, world_b);
		}
	}
}

int main(){
	// srand(time(NULL));
	fertility = 5;

	seed();
	printWorld(world1);
	
	printf("\n\n Generated World.\n");
	printf("\n\n Type 'Y' to play this world.\n Type 'S' to perform a step \n Type any other value to exit.\n");
	
	scanf("\n%c", &menu);
	CLS;

	int cycle = 0;
	int generation;
	
	do {
	switch (menu){
	case ('Y'): case ('y'):
		for(generation = 0; generation < 300; generation++) {
			SLEEP;
			CLS;
			if (cycle == 0){
				step(world1, world2);
				printWorld(world2);
				cycle = 1;
			} else if (cycle == 1){
				step(world2, world1);
				printWorld(world1);
				cycle = 0;
			}
			SLEEP;
		}
		break;
		
	case ('S'): case ('s'):
		CLS;
		if(cycle == 0){
			printWorld(world1);
			step(world1, world2);
			cycle = 1;
		}
		else if(cycle == 1){
			printWorld(world2);
			step(world2, world1);
			cycle = 0;
		}
		SLEEP;
		break;
	}
	printf("\n\n Type 'Y' to play this world.\n Type 'S' to perform a step \n Type any other value to exit.\n");
	
	scanf("\n%c", &menu);
	} while (menu =='S' || menu =='s' || menu =='Y' || menu == 'y');
}