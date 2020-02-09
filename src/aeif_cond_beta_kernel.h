/*
Copyright (C) 2020 Bruno Golosio
This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef AEIFCONDBETAKERNELH
#define AEIFCONDBETAKERNELH

#include <string>
#include <math.h>
#include "spike_buffer.h"
#include "node_group.h"
#include "aeif_cond_beta.h"

#define MIN(a,b) (((a)<(b))?(a):(b))

extern __constant__ float NeuralGPUTimeResolution;

namespace aeif_cond_beta_ns
{
enum ScalVarIndexes {
  i_V_m = 0,
  i_w,
  N_SCAL_VAR
};

enum PortVarIndexes {
  i_g = 0,
  i_g1,
  N_PORT_VAR
};

enum ScalParamIndexes {
  i_V_th = 0,
  i_Delta_T,
  i_g_L,
  i_E_L,
  i_C_m,
  i_a,
  i_b,
  i_tau_w,
  i_I_e,
  i_V_peak,
  i_V_reset,
  i_t_ref,
  i_refractory_step,
  N_SCAL_PARAM
};

enum PortParamIndexes {
  i_E_rev = 0,
  i_taus_rise,
  i_taus_decay,
  i_g0,
  N_PORT_PARAM
};

const std::string aeif_cond_beta_scal_var_name[N_SCAL_VAR] = {
  "V_m",
  "w"
};

const std::string aeif_cond_beta_port_var_name[N_PORT_VAR] = {
  "g",
  "g1"
};

const std::string aeif_cond_beta_scal_param_name[N_SCAL_PARAM] = {
  "V_th",
  "Delta_T",
  "g_L",
  "E_L",
  "C_m",
  "a",
  "b",
  "tau_w",
  "I_e",
  "V_peak",
  "V_reset",
  "t_ref",
  "refractory_step"
};

const std::string aeif_cond_beta_port_param_name[N_PORT_PARAM] = {
  "E_rev",
  "taus_rise",
  "taus_decay",
  "g0"  
};

//
// I know that defines are "bad", but the defines below make the
// following equations much more readable.
// For every rule there is some exceptions!
//
#define V_m y[i_V_m]
#define w y[i_w]
#define g(i) y[N_SCAL_VAR + N_PORT_VAR*i + i_g]
#define g1(i) y[N_SCAL_VAR + N_PORT_VAR*i + i_g1]

#define dVdt dydx[i_V_m]
#define dwdt dydx[i_w]
#define dgdt(i) dydx[N_SCAL_VAR + N_PORT_VAR*i + i_g]
#define dg1dt(i) dydx[N_SCAL_VAR + N_PORT_VAR*i + i_g1]

#define V_th param[i_V_th]
#define Delta_T param[i_Delta_T]
#define g_L param[i_g_L]
#define E_L param[i_E_L]
#define C_m param[i_C_m]
#define a param[i_a]
#define b param[i_b]
#define tau_w param[i_tau_w]
#define I_e param[i_I_e]
#define V_peak param[i_V_peak]
#define V_reset param[i_V_reset]
#define t_ref param[i_t_ref]
#define refractory_step param[i_refractory_step]

#define E_rev(i) param[N_SCAL_PARAM + N_PORT_PARAM*i + i_E_rev]
#define taus_rise(i) param[N_SCAL_PARAM + N_PORT_PARAM*i + i_taus_rise]
#define taus_decay(i) param[N_SCAL_PARAM + N_PORT_PARAM*i + i_taus_decay]
#define g0(i) param[N_SCAL_PARAM + N_PORT_PARAM*i + i_g0]


 template<int NVAR, int NPARAM> //, class DataStruct>
__device__
    void Derivatives(float x, float *y, float *dydx, float *param,
		     aeif_cond_beta_rk5 data_struct)
{
  enum { n_port = (NVAR-N_SCAL_VAR)/N_PORT_VAR };
  float I_syn = 0.0;

  float V = ( refractory_step > 0 ) ? V_reset :  MIN(V_m, V_peak);
  for (int i = 0; i<n_port; i++) {
    I_syn += g(i)*(E_rev(i) - V);
  }
  float V_spike = Delta_T*exp((V - V_th)/Delta_T);

  dVdt = ( refractory_step > 0 ) ? 0 :
    ( -g_L*(V - E_L - V_spike) + I_syn - w + I_e) / C_m;
  // Adaptation current w.
  dwdt = (a*(V - E_L) - w) / tau_w;
  for (int i=0; i<n_port; i++) {
    // Synaptic conductance derivative
    dg1dt(i) = -g1(i) / taus_rise(i);
    dgdt(i) = g1(i) - g(i) / taus_decay(i);
  }
}

 template<int NVAR, int NPARAM> //, class DataStruct>
__device__
    void ExternalUpdate
    (float x, float *y, float *param, bool end_time_step,
			aeif_cond_beta_rk5 data_struct)
{
  if ( V_m < -1.0e3) { // numerical instability
    printf("V_m out of lower bound\n");
    V_m = V_reset;
    w=0;
    return;
  }
  if ( w < -1.0e6 || w > 1.0e6) { // numerical instability
    printf("w out of bound\n");
    V_m = V_reset;
    w=0;
    return;
  }
  if (refractory_step > 0.0) {
    V_m = V_reset;
    if (end_time_step) {
      refractory_step -= 1.0;
    }
  }
  else {
    if ( V_m >= V_peak ) { // send spike
      int neuron_idx = threadIdx.x + blockIdx.x * blockDim.x;
      PushSpike(data_struct.i_node_0_ + neuron_idx, 1.0);
      V_m = V_reset;
      w += b; // spike-driven adaptation
      refractory_step = (int)round(t_ref/NeuralGPUTimeResolution);
      if (refractory_step<0) {
	refractory_step = 0;
      }
    }
  }
}


};

template <>
int aeif_cond_beta::UpdateNR<0>(int it, float t1);

template<int N_PORT>
int aeif_cond_beta::UpdateNR(int it, float t1)
{
  if (N_PORT == n_port_) {
    const int NVAR = aeif_cond_beta_ns::N_SCAL_VAR
      + aeif_cond_beta_ns::N_PORT_VAR*N_PORT;
    const int NPARAM = aeif_cond_beta_ns::N_SCAL_PARAM
      + aeif_cond_beta_ns::N_PORT_PARAM*N_PORT;

    rk5_.Update<NVAR, NPARAM>(t1, h_min_, rk5_data_struct_);
  }
  else {
    UpdateNR<N_PORT - 1>(it, t1);
  }

  return 0;
}

template<int NVAR, int NPARAM>
__device__
void Derivatives(float x, float *y, float *dydx, float *param,
		 aeif_cond_beta_rk5 data_struct)
{
    aeif_cond_beta_ns::Derivatives<NVAR, NPARAM>(x, y, dydx, param,
						 data_struct);
}

template<int NVAR, int NPARAM>
__device__
void ExternalUpdate(float x, float *y, float *param, bool end_time_step,
		    aeif_cond_beta_rk5 data_struct)
{
    aeif_cond_beta_ns::ExternalUpdate<NVAR, NPARAM>(x, y, param,
						    end_time_step,
						    data_struct);
}


#endif