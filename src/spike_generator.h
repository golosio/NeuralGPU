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

#ifndef SPIKEGENERATORH
#define SPIKEGENERATORH

#include <iostream>
#include <string>
#include "cuda_error.h"
				    //#include "node_group.h"
#include "base_neuron.h"
#include "neuron_models.h"

/*
const int N_POISS_GEN_SCAL_PARAM = 4;
const std::string poiss_gen_scal_param_name[] = {
  "rate",
  "origin"
  "start",
  "stop",
};
*/

class spike_generator : public BaseNeuron
{
  int *d_n_spikes_;
  int *d_i_spike_;	    
  int **d_spike_time_idx_;
  float **d_spike_height_;
  int **h_spike_time_idx_;
  float ** h_spike_height_;
  std::vector<std::vector<float>> spike_time_vect_;
  std::vector<std::vector<float>> spike_height_vect_;

  int SetSpikes(int irel_node, int n_spikes, float *spike_time,
		float *spike_height, float time_min, float time_resolution);
  
 public:
  ~spike_generator();
  
  int Init(int i_node_0, int n_node, int n_port, int i_group,
	   unsigned long long *seed);

  int Free();
  
  int Update(int i_time, float t1);

  int Calibrate(float time_min, float time_resolution);

  bool IsArrayParam(std::string param_name);

  int SetArrayParam(int i_neuron, int n_neuron, std::string param_name,
		    float *array, int array_size);
  
  int SetArrayParam(int *i_neuron, int n_neuron, std::string param_name,
		    float *array, int array_size);

};


#endif
