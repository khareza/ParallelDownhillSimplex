// DownhillSimplex.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"
#include "DownhillSimplex.h"
#include <iostream>
#include "mpi.h""

#define master 0

using namespace std;

// Example using amoeba
//
// Function to be minimized is
// x^2 - 4x + y^2 - y - xy
//
// The function argument is a vector whose size is 2.
// z[0] corresponds to x and z[1] corresponds to y

Doub func(VecDoub_I & z)
{
    return SQR(z[0]) - 4.0*z[0] + SQR(z[1]) - z[1] - z[0] * z[1];
}

int main(int argc, char* argv[])
{
	int rank;
	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Status status;

    const Doub FTOL = 1.0e-16;

    const Int mp = 3;
    const Int np = 2;

    Doub p_array[mp*np] = {
        0.0, 0.0,
        1.2, 0.0,
        0.0, 0.8
    };

    MatDoub p(mp, np, p_array);
	DownhillSimplex a(FTOL);

	if (rank == master)
	{
		cout << fixed << setprecision(6);

		cout << "Vertices of initial simplex:" << endl << endl;
		cout << setw(3) << "i" << setw(10) << "x[i]" << setw(12) << "y[i]" << endl;
		cout << " --------------------------" << endl;
		for (Int i = 0; i < mp; i++) {
			cout << setw(3) << i;
			for (Int j = 0; j < np; j++) {
				cout << setw(12) << p[i][j];
			}
			cout << endl;
		}
		cout << endl;

		MPI_Barrier(MPI_COMM_WORLD);

		cout << "Number of function evaluations: " << a.nfunc << endl << endl;
		cout << "Vertices of final simplex and function" << endl;
		cout << "values at the vertices:" << endl << endl;
		cout << setw(3) << "i" << setw(10) << "x[i]" << setw(12) << "y[i]";
		cout << setw(14) << "function" << endl;
		cout << " --------------------------------------" << endl;
		
		double resultVertices[mp];
		double funcValue = 0;
		for (Int i = 1; i < mp; i++) 
		{

			MPI_Recv(&resultVertices, np, MPI_DOUBLE, i, 0, MPI_COMM_WORLD, &status);
			cout << setw(3) << i;
			for (Int j = 0; j < np; j++) {
				cout << setw(12) << resultVertices[j];
			}
			MPI_Recv(&funcValue, 1, MPI_DOUBLE, i, 0, MPI_COMM_WORLD, &status);
			cout << setw(12) << funcValue << endl;
		}
		cout << endl;

	}
	else
	{
		Doub singleVerticeArray[np];
		for (int i = 0; i < mp; i++)
		{
			int index = (((rank - 1)*np) + i);
			singleVerticeArray[i] = p_array[index];
		}
		MatDoub pp(mp, np, singleVerticeArray);

		a.minimize(p, func);

		double* buf[np];
		const void* buf2 = &a.y[0];
		for (int i = 0; i < np; i++)
		{
			buf[i] = &a.p[0][i];
		}

		MPI_Send(*buf, np, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD);
		MPI_Send(buf2, 1, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD);
		MPI_Barrier(MPI_COMM_WORLD);
	}


	MPI_Finalized(&rank);
	system("pause");
	return 0;
}