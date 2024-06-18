#include <stdio.h>
#include <math.h>

double bbp_digit(int n) {
    double sum = 0.0;
    for (int k = 0; k < n; ++k) {
        double a = (1.0 / pow(16, k)) * ((4.0 / (8 * k + 1)) - (2.0 / (8 * k + 4)) - (1.0 / (8 * k + 5)) - (1.0 / (8 * k + 6)));
        sum += a;
    }
    return sum;
}

int main() {
    for (int n = 1; n <= 100; ++n) {
        double pi_digit = bbp_digit(n);
        printf("First %d digit(s) of Pi: %.*lf\n", n, n, pi_digit);
    }
    return 0;
}