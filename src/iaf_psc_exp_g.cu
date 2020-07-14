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

#include <config.h>
#include <cmath>
#include <iostream>
#include "iaf_psc_exp_g.h"
#include "spike_buffer.h"

using namespace iaf_psc_exp_g_ns;

extern __constant__ float NeuronGPUTimeResolution;

#define I_syn var[i_I_syn]
#define V_m_rel var[i_V_m_rel]
#define refractory_step var[i_refractory_step]
#define I_e param[i_I_e]

__global__ void iaf_psc_exp_g_Update
( int n_node, int i_node_0, float *var_arr, float *param_arr, int n_var,
  int n_param, float Theta_rel, float V_reset_rel, int n_refractory_steps,
  float P11, float P22, float P21, float P20 )
{
  int i_neuron = threadIdx.x + blockIdx.x * blockDim.x;
  if (i_neuron<n_node) {
    float *var = var_arr + n_var*i_neuron;
    float *param = param_arr + n_param*i_neuron;
    
    if ( refractory_step > 0.0 ) {
      // neuron is absolute refractory
      refractory_step -= 1.0;
    }
    else { // neuron is not refractory, so evolve V
      V_m_rel = V_m_rel * P22 + I_syn * P21 + I_e * P20;
    }
    // exponential decaying PSC
    I_syn *= P11;
    
    if (V_m_rel >= Theta_rel ) { // threshold crossing
      PushSpike(i_node_0 + i_neuron, 1.0);
      V_m_rel = V_reset_rel;
      refractory_step = n_refractory_steps;
    }    
  }
}

double h_propagator_32( double tau_syn, double tau, double C, double h )
{
  const double P32_linear = 1.0 / ( 2.0 * C * tau * tau ) * h * h
    * ( tau_syn - tau ) * exp( -h / tau );
  const double P32_singular = h / C * exp( -h / tau );
  const double P32 =
    -tau / ( C * ( 1.0 - tau / tau_syn ) ) * exp( -h / tau_syn )
    * expm1( h * ( 1.0 / tau_syn - 1.0 / tau ) );

  const double dev_P32 = fabs( P32 - P32_singular );

  if ( tau == tau_syn || ( fabs( tau - tau_syn ) < 0.1 && dev_P32 > 2.0
			   * fabs( P32_linear ) ) )
  {
    return P32_singular;
  }
  else
  {
    return P32;
  }
}

iaf_psc_exp_g::~iaf_psc_exp_g()
{
  FreeVarArr();
  FreeParamArr();
}

int iaf_psc_exp_g::Init(int i_node_0, int n_node, int /*n_port*/,
			   int i_group, unsigned long long *seed)
{
  BaseNeuron::Init(i_node_0, n_node, 1 /*n_port*/, i_group, seed);
  node_type_ = i_iaf_psc_exp_g_model;

  n_scal_var_ = N_SCAL_VAR;
  n_var_ = n_scal_var_;
  n_scal_param_ = N_SCAL_PARAM;
  n_param_ = n_scal_param_;
  
  AllocParamArr();
  AllocVarArr();

  scal_var_name_ = iaf_psc_exp_g_scal_var_name;
  scal_param_name_ = iaf_psc_exp_g_scal_param_name;

  SetScalParam(0, n_node, "I_e", 0.0 );              // in pA

  SetScalVar(0, n_node, "I_syn", 0.0 );
  SetScalVar(0, n_node, "V_m_rel", 0.0 ); // in mV
  SetScalVar(0, n_node, "refractory_step", 0 );

  tau_m_ = 10.0;
  C_m_ = 250.0;
  E_L_ = -65.0;
  Theta_rel_ = 15.0;
  V_reset_rel_ = 0.0;
  tau_syn_ = 0.5;
  t_ref_ = 2.0;

  // multiplication factor of input signal is always 1 for all nodes
  float input_weight = 1.0;
  gpuErrchk(cudaMalloc(&port_weight_arr_, sizeof(float)));
  gpuErrchk(cudaMemcpy(port_weight_arr_, &input_weight,
			 sizeof(float), cudaMemcpyHostToDevice));
  port_weight_arr_step_ = 0;
  port_weight_port_step_ = 0;
  
  // input spike signal is stored in I_syn
  port_input_arr_ = GetVarArr() + GetScalVarIdx("I_syn");
  port_input_arr_step_ = n_var_;
  port_input_port_step_ = 0;

  return 0;
}

int iaf_psc_exp_g::Update(int it, float t1)
{
  // std::cout << "iaf_psc_exp_g neuron update\n";
  float h = time_resolution_;
  float P11 = exp( -h / tau_syn_ );
  float P22 = exp( -h / tau_m_ );
  float P21 = h_propagator_32( tau_syn_, tau_m_, C_m_, h );
  float P20 = tau_m_ / C_m_ * ( 1.0 - P22 );
  int n_refractory_steps = int(round(t_ref_ / h));

  iaf_psc_exp_g_Update<<<(n_node_+1023)/1024, 1024>>>
    (n_node_, i_node_0_, var_arr_, param_arr_, n_var_, n_param_,
      Theta_rel_, V_reset_rel_, n_refractory_steps, P11, P22, P21, P20 );
  //gpuErrchk( cudaDeviceSynchronize() );
  
  return 0;
}

int iaf_psc_exp_g::Free()
{
  FreeVarArr();  
  FreeParamArr();
  
  return 0;
}

int iaf_psc_exp_g::SetNeuronGroupParam(std::string param_name, float val)
{
  if (param_name=="tau_m") {
    tau_m_ = val;
  }
  else if (param_name=="C_m") {
    C_m_ = val;
  }
  else if (param_name=="E_L") {
     E_L_ = val;
  }
  else if (param_name=="Theta_rel") {
     Theta_rel_ = val;
  }
  else if (param_name=="V_reset_rel") {
     V_reset_rel_ = val;
  }
  else if (param_name=="tau_syn") {
     tau_syn_ = val;
  }
  else if (param_name=="t_ref") {
     t_ref_ = val;
  }
  else {
    throw ngpu_exception(std::string("Unrecognized neuron group parameter ")
			 + param_name);
  }
  
  return 0;
}