////==================================================
/// An interface to functions of fortran prog SCF.
////==================================================

#ifndef _SCF_
#define _SCF_

#include <stdio.h>
#include <time.h>

extern void get_parameter_();
extern void get_pot_(double *x, double *y, double *z, double *pot);
extern void get_acc_(double *x, double *y, double *z, double *accx, double* accy, double* accz);

#endif