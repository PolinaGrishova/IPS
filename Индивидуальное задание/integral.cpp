// integral.cpp : Этот файл содержит функцию "main". Здесь начинается и заканчивается выполнение программы.
//

#include <chrono>
#include <cmath>
#include <functional>
#include <iostream>

#include <cilk\cilk.h>
#include <cilk\cilk_api.h>
#include <cilk\reducer_opadd.h>

double func(double x) {
  return 4. / std::sqrt(4. - x * x);
}

double serial_integral_rect(std::function<double(double)> fun, double a, double b, size_t num_h) {
  double sum(0.);
  double h = (b - a) / static_cast<double>(num_h);
  size_t num_batch = 1024;
  
  for (size_t i = 0; i < num_h; i++){//+=num_batch) {
    sum += fun(a + h * i) + fun(a + h * (i + 1));
  }

  return sum * h / 2.;
}

double paralell_integral_rect(std::function<double(double)> fun, double a, double b, size_t num_h) {
  cilk::reducer_opadd<double> sum(0.);
  double h = (b - a) / static_cast<double>(num_h);
  size_t num_batch = 1024;

  cilk_for (size_t i = 0; i < num_h; i++){// += num_batch) {
    sum += fun(a + h * i) + fun(a + h * (i + 1));
  }

  return sum->get_value() * h / 2.;
}

int main() {
  double a(0.), b(1.);

 // for (size_t n = 100; n < 10000000; n *= 10) {
    size_t n = 1000000;
    std::chrono::high_resolution_clock::time_point t1 = std::chrono::high_resolution_clock::now();
    double result = serial_integral_rect(func, a, b, n);
    std::chrono::high_resolution_clock::time_point t2 = std::chrono::high_resolution_clock::now();

    std::cout << "Serial result " << result << std::endl;
    std::chrono::duration<double> duration_serial = (t2 - t1);
    std::cout << "Duration of serial run is: " << duration_serial.count() << " seconds" << std::endl;

    t1 = std::chrono::high_resolution_clock::now();
    result = paralell_integral_rect(func, a, b, n);
    t2 = std::chrono::high_resolution_clock::now();
    std::cout << "Paralell result " << result << std::endl;
    std::chrono::duration<double> duration_paralell = (t2 - t1);
    std::cout << "Duration of paralell run is: " << duration_paralell.count() << " seconds" << std::endl;
    std::cout << "Boost is: " << duration_serial.count() / duration_paralell.count() << " times" << std::endl;
    std::cout << std::endl;
 // }

    return 0;
}