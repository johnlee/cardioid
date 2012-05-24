#ifndef TT06FUNC_HH
#define TT06FUNC_HH
#include <map>
#include <string>
#include <vector>
#include "TT06Func.h" 

struct PADE;

#define SQ(x) ((x)*(x))
#define CUBE(x) ((x)*(x)*(x))

/*
 * RATES[0] is d/dt V in component membrane (millivolt).
 * RATES[4] is d/dt Xr1 in component rapid_time_dependent_potassium_current_Xr1_gate (dimensionless).
 * RATES[5] is d/dt Xr2 in component rapid_time_dependent_potassium_current_Xr2_gate (dimensionless).
 * RATES[6] is d/dt Xs in component slow_time_dependent_potassium_current_Xs_gate (dimensionless).
 * RATES[7] is d/dt m in component fast_sodium_current_m_gate (dimensionless).
 * RATES[8] is d/dt h in component fast_sodium_current_h_gate (dimensionless).
 * RATES[9] is d/dt j in component fast_sodium_current_j_gate (dimensionless).
 * RATES[11] is d/dt d in component L_type_Ca_current_d_gate (dimensionless).
 * RATES[12] is d/dt f in component L_type_Ca_current_f_gate (dimensionless).
 * RATES[13] is d/dt f2 in component L_type_Ca_current_f2_gate (dimensionless).
 * RATES[14] is d/dt fCass in component L_type_Ca_current_fCass_gate (dimensionless).
 * RATES[15] is d/dt s in component transient_outward_current_s_gate (dimensionless).
 * RATES[16] is d/dt r in component transient_outward_current_r_gate (dimensionless).
 * RATES[18] is d/dt R_prime in component calcium_dynamics (dimensionless).
 * RATES[3] is d/dt Ca_i in component calcium_dynamics (millimolar).
 * RATES[17] is d/dt Ca_SR in component calcium_dynamics (millimolar).
 * RATES[10] is d/dt Ca_ss in component calcium_dynamics (millimolar).
 * RATES[2] is d/dt Na_i in component sodium_dynamics (millimolar).
 * RATES[1] is d/dt K_i in component potassium_dynamics (millimolar).
*/
// int mapCell2Dev[]                  {1,2,3,10,17,18,14,7,8,9,4,5,6,16,11,12,13,15};
namespace TT06Func
{


struct STATE
{
   STATE(){};
   STATE(int type, int index, double value)
   : type(type), index(index), value(value){};

   int index, type;
   double value;
}; 

struct CellTypeParmsFull
{
   std::string name;
   std::vector<int> anatomyIndices;
   int s_switch;
   double P_NaK, g_Ks, g_to, g_NaL, Vm;
   std::map<std::string,STATE> state;
};

struct WORK
{
   int offsetCell, nCell, offsetEq, nEq;
}; 

void initCnst();
//void updateNonGate(double dt, CellTypeParms *cellTypeParms, int n, int *cellType, double *Vm, int offset, double **state, double *dVdt);
void updateGate(double dt, int n, int *cellType, double *Vm, int offset, double **state, WORK& work);
void updateGateFast(double dt, int n, int *cellType, double *Vm, int offset, double **state, WORK& work);
PADE **makeFit(double tol, double V0, double V1, double deltaV, int mod);
void writeFit(PADE **fit); 
std::map<std::string,CellTypeParmsFull> getStandardCellTypes();
};

#endif
