#ifndef THERMOCOUPLE_K_H
#define THERMOCOUPLE_K_H

#include <stdbool.h>


double thermocouple_from_E_to_T(double e);
double thermocouple_from_T_to_E(double temp);
bool thermocouple_find_temp(float v, float *temp);

#endif
