#ifndef FIT_H
#define FIT_H
#include <stdio.h>
typedef struct {  char *name; double (*func)(double x, void *parms); void *parms; double (*afunc)(double x, void *aparms); void *aparms; int n; double deltaX, x0,x1,ymin,ymax, tol, errMax, errRMS; double *x, *y ; int cost; int l; int m; double *coef;} PADE; 
PADE   *padeApprox (char *name, double (*func)(double x, void *parms), void *parms, int size_parms,double tol, double deltaX, double x0, double x1, int maxOrder, int maxCost);
void padeErrorInfo(PADE pade);
void padeWrite(FILE *file,PADE pade);
double padeFunc(double x, PADE *parms) ;
#endif