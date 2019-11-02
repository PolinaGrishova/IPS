﻿#include <stdio.h>
#include <ctime>
#include <cilk/cilk.h>
#include <cilk/cilk_api.h>
#include <cilk/reducer_opadd.h>
#include <chrono>
#include <iostream>

using namespace std::chrono;
using namespace std;

// количество строк в исходной квадратной матрице
const int MATRIX_SIZE = 1500;
duration<double> duration_pf, duration_sf;

/// Функция InitMatrix() заполняет переданную в качестве 
/// параметра квадратную матрицу случайными значениями
/// matrix - исходная матрица СЛАУ
void InitMatrix(double** matrix)
{
	for (int i = 0; i < MATRIX_SIZE; ++i)
	{
		matrix[i] = new double[MATRIX_SIZE + 1];
	}

	for (int i = 0; i < MATRIX_SIZE; ++i)
	{
		for (int j = 0; j <= MATRIX_SIZE; ++j)
		{
			matrix[i][j] = rand() % 2500 + 1;
		}
	}
}

/// Функция SerialGaussMethod() решает СЛАУ методом Гаусса 
/// matrix - исходная матрица коэффиициентов уравнений, входящих в СЛАУ,
/// последний столбей матрицы - значения правых частей уравнений
/// rows - количество строк в исходной матрице
/// result - массив ответов СЛАУ
void SerialGaussMethod(double** matrix, const int rows, double* result)
{
	int k;
	double koef;

	high_resolution_clock::time_point t1 = high_resolution_clock::now();

	// прямой ход метода Гаусса
	for (k = 0; k < rows; ++k)
	{
		//
		for (int i = k + 1; i < rows; ++i)
		{
			koef = -matrix[i][k] / matrix[k][k];

			for (int j = k; j <= rows; ++j)
			{
				matrix[i][j] += koef * matrix[k][j];
			}
		}
	}

	high_resolution_clock::time_point t2 = high_resolution_clock::now();

	duration_sf = (t2 - t1);
	cout << "Duration of forvard run is: " << duration_sf.count() << " seconds" << endl;

	// обратный ход метода Гаусса
	result[rows - 1] = matrix[rows - 1][rows] / matrix[rows - 1][rows - 1];

	for (k = rows - 2; k >= 0; --k)
	{
		result[k] = matrix[k][rows];

		//
		for (int j = k + 1; j < rows; ++j)
		{
			result[k] -= matrix[k][j] * result[j];
		}

		result[k] /= matrix[k][k];
	}
}

void ParallelGaussMethod(double** matrix, const int rows, double* result)
{
	int k;
	double koef;

	high_resolution_clock::time_point t1 = high_resolution_clock::now();

	// прямой ход метода Гаусса
	for (k = 0; k < rows; ++k)
	{
		cilk_for (int i = k + 1; i < rows; ++i)
		{
			koef = -matrix[i][k] / matrix[k][k];

			for (int j = k; j <= rows; ++j)
			{
				matrix[i][j] += koef * matrix[k][j];
			}
		}
	}

	high_resolution_clock::time_point t2 = high_resolution_clock::now();

	duration_pf = (t2 - t1);
	cout << "Duration of parallel forvard run is: " << duration_pf.count() << " seconds" << endl;

	// обратный ход метода Гаусса
	result[rows - 1] = matrix[rows - 1][rows] / matrix[rows - 1][rows - 1];

	for (k = rows - 2; k >= 0; --k)
	{
		//result[k] = matrix[k][rows];
		cilk::reducer_opadd<double> res(matrix[k][rows]);

		cilk_for(int j = k + 1; j < rows; ++j)
		{
			res -= matrix[k][j] * result[j];
		}

		//result[k] /= matrix[k][k];
		result[k] = res->get_value() / matrix[k][k];
	}
}

int test_lab3(bool test, int size)
{
	srand((unsigned)time(0));

	int i;

	// кол-во строк в матрице, приводимой в качестве примера
	int test_matrix_lines;
	if (test)
		test_matrix_lines = 4;
	else
		test_matrix_lines = size;

	double** test_matrix = new double* [test_matrix_lines];

	// цикл по строкам
	if (test)
		for (i = 0; i < test_matrix_lines; ++i)
		{
			// (test_matrix_lines + 1)- количество столбцов в тестовой матрице,
			// последний столбец матрицы отведен под правые части уравнений, входящих в СЛАУ
			test_matrix[i] = new double[test_matrix_lines + 1];
		}
	else
		InitMatrix(test_matrix);

	// массив решений СЛАУ
	double* result = new double[test_matrix_lines];

	// инициализация тестовой матрицы
	if (test) {
		test_matrix[0][0] = 2; test_matrix[0][1] = 5;  test_matrix[0][2] = 4;  test_matrix[0][3] = 1;  test_matrix[0][4] = 20;
		test_matrix[1][0] = 1; test_matrix[1][1] = 3;  test_matrix[1][2] = 2;  test_matrix[1][3] = 1;  test_matrix[1][4] = 11;
		test_matrix[2][0] = 2; test_matrix[2][1] = 10; test_matrix[2][2] = 9;  test_matrix[2][3] = 7;  test_matrix[2][4] = 40;
		test_matrix[3][0] = 3; test_matrix[3][1] = 8;  test_matrix[3][2] = 9;  test_matrix[3][3] = 2;  test_matrix[3][4] = 37;
	}

	SerialGaussMethod(test_matrix, test_matrix_lines, result);

	if (test) {
		printf("Solution:\n");

		for (i = 0; i < test_matrix_lines; ++i)
		{
			printf("x(%d) = %lf\n", i, result[i]);
		}
	}

	// инициализация тестовой матрицы
	if (test) {
		test_matrix[0][0] = 2; test_matrix[0][1] = 5;  test_matrix[0][2] = 4;  test_matrix[0][3] = 1;  test_matrix[0][4] = 20;
		test_matrix[1][0] = 1; test_matrix[1][1] = 3;  test_matrix[1][2] = 2;  test_matrix[1][3] = 1;  test_matrix[1][4] = 11;
		test_matrix[2][0] = 2; test_matrix[2][1] = 10; test_matrix[2][2] = 9;  test_matrix[2][3] = 7;  test_matrix[2][4] = 40;
		test_matrix[3][0] = 3; test_matrix[3][1] = 8;  test_matrix[3][2] = 9;  test_matrix[3][3] = 2;  test_matrix[3][4] = 37;
	}

	ParallelGaussMethod(test_matrix, test_matrix_lines, result);

	if (test) {
		printf("Solution:\n");

		for (i = 0; i < test_matrix_lines; ++i)
		{
			printf("x(%d) = %lf\n", i, result[i]);
		}
	}

	for (i = 0; i < test_matrix_lines; ++i)
	{
		delete[]test_matrix[i];
	}
	delete[] test_matrix;

	delete[] result;

	return 0;
}

int main()
{
	srand((unsigned)time(0));

	test_lab3(true, 0);
	test_lab3(false, MATRIX_SIZE);

	printf("Boost is %lf\n", duration_sf.count() / duration_pf.count());

	return 0;
}