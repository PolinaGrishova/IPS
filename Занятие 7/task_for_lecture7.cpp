#include <omp.h>
#include <stdio.h>
#include <stdlib.h>

long long num = 100000000;
double step;

double par(void)
{
	int num_of_threads = 3;
	long long inc = 0;
	long long i = 0;
	double x = 0.0;
	double pi;
	double S = 0.0;
	step = 1.0 / (double)num;
	double t = omp_get_wtime();
	omp_lock_t writelock;
	omp_init_lock(&writelock);

#pragma omp parallel for reduction(+:S) private(x)
	for (i = 0; i < num; i++)
	{
		x = (i + 0.5)*step;
		S = S + 4.0 / (1.0 + (x*x));
//#pragma omp atomic
//#pragma omp critical
		omp_set_lock(&writelock);
		inc++;
		omp_unset_lock(&writelock);
	}
	t = omp_get_wtime() - t;
	pi = step * S;printf("Par: pi = %.14f\n", pi);
	omp_destroy_lock(&writelock);
	return t;
}

int main()
{
	omp_set_dynamic(0);
	omp_set_num_threads(3);

	printf("time: %f sec.\n\n", par());

	return 0;
}
