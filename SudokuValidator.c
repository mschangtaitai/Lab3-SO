#include <sys/types.h>
#include <sys/stat.h>
#include <sys/shm.h>
#include <sys/mman.h>
#include <sys/syscall.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>

int x,y,z;
char sudoku[9][9];
_Bool sudokuStatus = 1;
int ver[9] = {0,0,0,0,0,0,0,0,0};

//Row Verification
void rowVer(){

	omp_set_nested(1);
	omp_set_num_threads(9);

	#pragma omp parallel for private(x) schedule(dynamic)
	for(int i=0; i<9; i++){
		for (int j=0;j<9;j++){
			ver[j] = 0;
		}

		for(int j=0; j<9; j++){
			ver[((int)(sudoku[i][j])-49)] = 1;
		}
		int verTotal = 0;
		for (int j=0;j<9;j++){
			verTotal += ver[j];
		}
		if(verTotal == 9){
			printf("Row %d is good running on process %d", i, syscall(SYS_gettid));
		}
		else{
			printf("Row %d is bad", i);
			sudokuStatus = 0;
		}
		printf("\n");
	}
}

//Column Verification
void *colVer(void *vargp){
	omp_set_nested(1);
	omp_set_num_threads(9);

	#pragma omp parallel for private(y) schedule(dynamic)
	for(int j=0; j<9; j++){
		for (int i=0;i<9;i++){
			ver[i] = 0;
		}

		for(int i=0; i<9; i++){
			ver[((int)(sudoku[i][j])-49)] = 1;
		}
		int verTotal = 0;
		for (int i=0;i<9;i++){
			verTotal += ver[i];
		}
		if(verTotal == 9){
			printf("Column %d is good running on process %d", j, syscall(SYS_gettid));
		}
		else{
			printf("Column %d is bad", j);
			sudokuStatus = 0;
		}
		printf("\n");
	}
}


int main(int argc, char *argv[]){
	omp_set_num_threads(1);

	FILE *fptr;
	char ch;

	if ((fptr = fopen(argv[1], "r")) == NULL){
		printf("Error opening file!");
		exit(1);
	}

	int row = 0;
	int col = 0;

	while((ch = fgetc(fptr)) != EOF && row < 9){
		sudoku[row][col] = ch;
		col++;
		if (col >= 9){
			row++;
			col = 0;
		}
	}

	for (int i=0; i<9; i++){
		for (int j=0; j<9; j++){
			printf("%d", ((int)(sudoku[i][j])-48));		
		}	
		printf("\n");
	}
	printf("\n");
	

	//Square Verification
	int rowPos=0;
	int colPos=0;
	int num = 0;

	omp_set_nested(1);
	omp_set_num_threads(9);
	#pragma omp parallel for private(z) schedule(dynamic)
	for(int square=0; square<9; square++){	
		
		for (int i=0;i<9;i++){
			ver[i] = 0;
		}

		for (int i=0+rowPos; i<3+rowPos; i++){
			for (int j=0+colPos; j<3+colPos; j++){
				ver[((int)(sudoku[i][j])-49)] = 1;
				printf("%d", (int)(sudoku[i][j])-48);		
			}	
			printf("\n");
				
		}
		int verTotal = 0;
		for (int i=0;i<9;i++){
			verTotal += ver[i];
		}
		if(verTotal == 9){
			printf("Square %d is good, running on process %d", square, syscall(SYS_gettid));
		}
		else{
			printf("Square %d is bad", square);
			sudokuStatus = 0;
		}
		colPos += 3;
		if (colPos >= 9){
			colPos = 0;
			rowPos += 3;		
		}
		printf("\n\n");
	}



	//Fork creations
	if (fork() == 0){
		int pid = getppid();
		char strppid[50];
		sprintf(strppid, "%d", pid);

		//execlp("ps", "ps", "-p", strppid, "lLf", NULL);
	}
	else{
		pthread_t thread_id;
		pthread_create(&thread_id, NULL, colVer, NULL);
		pthread_join(thread_id, NULL);
		
		printf("process id: %d \n", syscall(SYS_gettid));

		rowVer();
		
		if(sudokuStatus){
			printf("The sudoku is correct!\n");
		}
		else{
			printf("The sudoku is incorrect!\n");
		}

	}

	return 0;
}
