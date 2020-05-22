#define ITERATIONS 10000000
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#define N 16

short int A[N][N];
short int B[N][N];
short int C1[N][N];
short int C2[N][N];
short int C3[N][N];

void getDifferenceC() {
	int i;
	int j;
	int k;

	double start_timeC = clock();
	for (int y = 0; y < ITERATIONS; y++)
	{
		for (j = 0; j < N; j++)
		{
			for (k = 0; k < N; k++)
				C1[j][k] = A[j][k] - B[j][k];
		}

	}
	double end_timeC = clock();
	double search_timeC = end_timeC - start_timeC;
	printf("TimeC= %lf seconds\n",search_timeC/1000);
	printf("the result of difference C \n");
	for (i = 0; i < N; i++)
	{
		for (j = 0; j < N; j++)
			printf("%3d ", C1[i][j]);
		printf("\n");
	}
}

void getDifferenceASM() {
	int i = 0;
	int j = 0;
	
	double start_timeASM = clock();
	_asm finit
	for (int y = 0; y < ITERATIONS; y++)
	{
		_asm {
			pusha
			mov i, 0
			mov eax, i
			loopI :
			mov j, 0
				loopJ :
				mov ecx, i
				shl ecx, 5 
				lea edx, B[ecx]
				mov ecx, j
				movsx edx, [edx + ecx * 2]
				
				jl loopJ
				add i, 1
				cmp i, 10h
				jl loopI

				mov edx, 0
				mov ebx, 0
				mov eax, 0
				mov ecx, 100h
				loop1 :
			movsx eax, A[edx]
				sub ax, word ptr B[edx]
				mov C2[ebx], eax
				add ebx, 2
				add edx, 2
				loop loop1
				popa
		}_asm fwait
	}
	double end_timeASM = clock();
	double search_timeASM = end_timeASM - start_timeASM;
	
	printf("\n\nTimeASM= %lf seconds\n", search_timeASM / 1000);
	printf("\nthe result of difference ASM\n");
	for (i = 0; i < N; i++)
	{
		for (j = 0; j < N; j++)
			printf("%3d ", C2[i][j]);
		printf("\n");
	}

}

void getDifferenceMMX() {

	double start_timeMMX = clock();

	_asm finit
	for (int y = 0; y < ITERATIONS; y++)
	{
		_asm
		{
			pusha
			mov ecx, 64
			xor esi, esi
			xor ebx, ebx

			loop1 :
				movq MM2, B[esi]
				movq MM3, A[esi] //записываем первые 64бита матрицы в регистр
				psubw  MM3, MM2 //добавляем содержимое регистра MM2 к содержимому регистра MM3
				movq C3[esi], MM3 //помещаем содержимое регистра в результирующую матрицу
				add esi, 8

				loop loop1

				emms
				popa
		}_asm fwait
	}

	double end_timeMMX = clock();
	double search_timeMMX = end_timeMMX - start_timeMMX;

	printf("\n\nTimeMMX= %lf seconds\n", search_timeMMX / 1000);
	printf("\nThe result of difference MMX\n");
	for (int i = 0; i < N; i++)
	{
		for (int j = 0; j < N; j++)
			printf("%3d ", C3[i][j]);
		printf("\n");
	}

}

int main()
{
	for (int i = 0; i < N; i++)
	{
		for (int j = 0; j < N; j++)
		{
			A[i][j] = (short int)(rand() * rand() % 100 + rand() % 5 * rand() % 13);
						
		}
					
	}
	for (int i = 0; i < N; i++)
	{
		for (int j = 0; j < N; j++)
		{
			B[i][j] = (short int)(rand() * rand() % 100 + rand() % 5 * rand() % 13);

		}

	}
	getDifferenceC();
	getDifferenceASM();
	getDifferenceMMX();

	getchar();
	return 0;
}
