#include "vector_serial.h"

void vector_serial(const double* valorA, const double* valorb,double* valorc, int veces ){
    for (int i = 0; i < veces; i++)
    {
        valorc[i] = valorA[i] + valorb[i];
    }
}
