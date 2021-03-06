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
#include <iostream>
#include <cstdlib>
#include <string>
#include <cstring>

#include "neurongpu.h"
#include "neurongpu_C.h"
#include "propagate_error.h"

extern "C" {
  static NeuronGPU *NeuronGPU_instance = NULL;
  ConnSpec ConnSpec_instance;
  SynSpec SynSpec_instance;

  void checkNeuronGPUInstance() {
    if (NeuronGPU_instance == NULL) {
      NeuronGPU_instance = new NeuronGPU();
    }
  }
  
  char *NeuronGPU_GetErrorMessage()
  {
    checkNeuronGPUInstance();
    char *cstr = NeuronGPU_instance->GetErrorMessage();
    return cstr;
  }

  unsigned char NeuronGPU_GetErrorCode()
  {
    checkNeuronGPUInstance();
    return NeuronGPU_instance->GetErrorCode();
  }

  void NeuronGPU_SetOnException(int on_exception)
  {
    checkNeuronGPUInstance();
    NeuronGPU_instance->SetOnException(on_exception);
  }

  unsigned int *RandomInt(size_t n);
  
  int NeuronGPU_SetRandomSeed(unsigned long long seed)
  { int ret = 0; BEGIN_ERR_PROP {
    ret = NeuronGPU_instance->SetRandomSeed(seed);
  } END_ERR_PROP return ret; }

  int NeuronGPU_SetTimeResolution(float time_res)
  { int ret = 0; BEGIN_ERR_PROP {
    ret = NeuronGPU_instance->SetTimeResolution(time_res);
  } END_ERR_PROP return ret; }

  float NeuronGPU_GetTimeResolution()
  { float ret = 0.0; BEGIN_ERR_PROP {
    ret = NeuronGPU_instance->GetTimeResolution();
  } END_ERR_PROP return ret; }

  int NeuronGPU_SetMaxSpikeBufferSize(int max_size)
  { int ret = 0; BEGIN_ERR_PROP {
    ret = NeuronGPU_instance->SetMaxSpikeBufferSize(max_size);
  } END_ERR_PROP return ret; }

  int NeuronGPU_GetMaxSpikeBufferSize()
  { int ret = 0; BEGIN_ERR_PROP {
    ret = NeuronGPU_instance->GetMaxSpikeBufferSize();
  } END_ERR_PROP return ret; }

  int NeuronGPU_SetSimTime(float sim_time)
  { int ret = 0; BEGIN_ERR_PROP {
    ret = NeuronGPU_instance->SetSimTime(sim_time);
  } END_ERR_PROP return ret; }

  int NeuronGPU_SetVerbosityLevel(int verbosity_level)
  { int ret = 0; BEGIN_ERR_PROP {
    ret = NeuronGPU_instance->SetVerbosityLevel(verbosity_level);
  } END_ERR_PROP return ret; }

  
  int NeuronGPU_Create(char *model_name, int n_neuron, int n_port)
  { int ret = 0; BEGIN_ERR_PROP {
    std::string model_name_str = std::string(model_name);
    NodeSeq neur = NeuronGPU_instance->Create(model_name_str, n_neuron,
						    n_port);
    ret = neur[0];
  } END_ERR_PROP return ret; }

  int NeuronGPU_CreatePoissonGenerator(int n_node, float rate)
  { int ret = 0; BEGIN_ERR_PROP {
    NodeSeq pg = NeuronGPU_instance->CreatePoissonGenerator(n_node, rate);

    ret = pg[0];
  } END_ERR_PROP return ret; }
  
  int NeuronGPU_CreateRecord(char *file_name, char *var_name_arr[],
			     int *i_node_arr, int *port_arr,
			     int n_node)
  { int ret = 0; BEGIN_ERR_PROP {
    std::string file_name_str = std::string(file_name);
    std::vector<std::string> var_name_vect;
    for (int i=0; i<n_node; i++) {
      std::string var_name = std::string(var_name_arr[i]);
      var_name_vect.push_back(var_name);
    }
    ret = NeuronGPU_instance->CreateRecord
      (file_name_str, var_name_vect.data(), i_node_arr, port_arr,
       n_node);		       
  } END_ERR_PROP return ret; }
  
  int NeuronGPU_GetRecordDataRows(int i_record)
  { int ret = 0; BEGIN_ERR_PROP {
    std::vector<std::vector<float> > *data_vect_pt
      = NeuronGPU_instance->GetRecordData(i_record);

    ret = data_vect_pt->size();
  } END_ERR_PROP return ret; }
  
  int NeuronGPU_GetRecordDataColumns(int i_record)
  { int ret = 0; BEGIN_ERR_PROP {
    std::vector<std::vector<float> > *data_vect_pt
      = NeuronGPU_instance->GetRecordData(i_record);
    
    ret = data_vect_pt->at(0).size();
  } END_ERR_PROP return ret; }

  float **NeuronGPU_GetRecordData(int i_record)
  { float **ret = NULL; BEGIN_ERR_PROP {
    std::vector<std::vector<float> > *data_vect_pt
      = NeuronGPU_instance->GetRecordData(i_record);
    int nr = data_vect_pt->size();
    ret = new float*[nr];
    for (int i=0; i<nr; i++) {
      ret[i] = data_vect_pt->at(i).data();
    }
  } END_ERR_PROP return ret; }

  int NeuronGPU_SetNeuronScalParam(int i_node, int n_neuron, char *param_name,
				   float val)
  { int ret = 0; BEGIN_ERR_PROP {
    
    std::string param_name_str = std::string(param_name);
    ret = NeuronGPU_instance->SetNeuronParam(i_node, n_neuron,
					     param_name_str, val);
  } END_ERR_PROP return ret; }

  int NeuronGPU_SetNeuronArrayParam(int i_node, int n_neuron,
				    char *param_name, float *param,
				    int array_size)
  { int ret = 0; BEGIN_ERR_PROP {
      std::string param_name_str = std::string(param_name);    
      ret = NeuronGPU_instance->SetNeuronParam(i_node, n_neuron,
					       param_name_str, param,
					       array_size);
  } END_ERR_PROP return ret; }

  int NeuronGPU_SetNeuronPtScalParam(int *i_node, int n_neuron,
				     char *param_name,float val)
  { int ret = 0; BEGIN_ERR_PROP {
    std::string param_name_str = std::string(param_name);
    ret = NeuronGPU_instance->SetNeuronParam(i_node, n_neuron,
					     param_name_str, val);
  } END_ERR_PROP return ret; }

  int NeuronGPU_SetNeuronPtArrayParam(int *i_node, int n_neuron,
				     char *param_name, float *param,
				     int array_size)
  { int ret = 0; BEGIN_ERR_PROP {
    std::string param_name_str = std::string(param_name);    
    ret = NeuronGPU_instance->SetNeuronParam(i_node, n_neuron,
					     param_name_str, param,
					     array_size);
  } END_ERR_PROP return ret; }
  
  int NeuronGPU_IsNeuronScalParam(int i_node, char *param_name)
  { int ret = 0; BEGIN_ERR_PROP {
    std::string param_name_str = std::string(param_name);
    
    ret = NeuronGPU_instance->IsNeuronScalParam(i_node, param_name_str);
  } END_ERR_PROP return ret; }
  
  int NeuronGPU_IsNeuronPortParam(int i_node, char *param_name)
  { int ret = 0; BEGIN_ERR_PROP {
    std::string param_name_str = std::string(param_name);
    
    ret = NeuronGPU_instance->IsNeuronPortParam(i_node, param_name_str);
  } END_ERR_PROP return ret; }
  
  int NeuronGPU_IsNeuronArrayParam(int i_node, char *param_name)
  { int ret = 0; BEGIN_ERR_PROP {
    std::string param_name_str = std::string(param_name);
    
    ret = NeuronGPU_instance->IsNeuronArrayParam(i_node, param_name_str);
  } END_ERR_PROP return ret; }
  

  int NeuronGPU_SetNeuronIntVar(int i_node, int n_neuron, char *var_name,
				int val)
  { int ret = 0; BEGIN_ERR_PROP {
    
    std::string var_name_str = std::string(var_name);
    ret = NeuronGPU_instance->SetNeuronIntVar(i_node, n_neuron,
					     var_name_str, val);
  } END_ERR_PROP return ret; }

  int NeuronGPU_SetNeuronScalVar(int i_node, int n_neuron, char *var_name,
				   float val)
  { int ret = 0; BEGIN_ERR_PROP {
    
    std::string var_name_str = std::string(var_name);
    ret = NeuronGPU_instance->SetNeuronVar(i_node, n_neuron,
					     var_name_str, val);
  } END_ERR_PROP return ret; }

  int NeuronGPU_SetNeuronArrayVar(int i_node, int n_neuron,
				    char *var_name, float *var,
				    int array_size)
  { int ret = 0; BEGIN_ERR_PROP {
      std::string var_name_str = std::string(var_name);    
      ret = NeuronGPU_instance->SetNeuronVar(i_node, n_neuron,
					       var_name_str, var,
					       array_size);
  } END_ERR_PROP return ret; }

  int NeuronGPU_SetNeuronPtIntVar(int *i_node, int n_neuron,
				     char *var_name, int val)
  { int ret = 0; BEGIN_ERR_PROP {
    std::string var_name_str = std::string(var_name);
    ret = NeuronGPU_instance->SetNeuronIntVar(i_node, n_neuron,
					      var_name_str, val);
  } END_ERR_PROP return ret; }

  int NeuronGPU_SetNeuronPtScalVar(int *i_node, int n_neuron,
				     char *var_name, float val)
  { int ret = 0; BEGIN_ERR_PROP {
    std::string var_name_str = std::string(var_name);
    ret = NeuronGPU_instance->SetNeuronVar(i_node, n_neuron,
					     var_name_str, val);
  } END_ERR_PROP return ret; }

  int NeuronGPU_SetNeuronPtArrayVar(int *i_node, int n_neuron,
				     char *var_name, float *var,
				     int array_size)
  { int ret = 0; BEGIN_ERR_PROP {
    std::string var_name_str = std::string(var_name);    
    ret = NeuronGPU_instance->SetNeuronVar(i_node, n_neuron,
					     var_name_str, var,
					     array_size);
  } END_ERR_PROP return ret; }
  
  int NeuronGPU_IsNeuronIntVar(int i_node, char *var_name)
  { int ret = 0; BEGIN_ERR_PROP {
    std::string var_name_str = std::string(var_name);

    ret = NeuronGPU_instance->IsNeuronIntVar(i_node, var_name_str);
  } END_ERR_PROP return ret; }
  
  int NeuronGPU_IsNeuronScalVar(int i_node, char *var_name)
  { int ret = 0; BEGIN_ERR_PROP {
    std::string var_name_str = std::string(var_name);
    
    ret = NeuronGPU_instance->IsNeuronScalVar(i_node, var_name_str);
  } END_ERR_PROP return ret; }
  
  int NeuronGPU_IsNeuronPortVar(int i_node, char *var_name)
  { int ret = 0; BEGIN_ERR_PROP {
    std::string var_name_str = std::string(var_name);
    
    ret = NeuronGPU_instance->IsNeuronPortVar(i_node, var_name_str);
  } END_ERR_PROP return ret; }
  
  int NeuronGPU_IsNeuronArrayVar(int i_node, char *var_name)
  { int ret = 0; BEGIN_ERR_PROP {
    std::string var_name_str = std::string(var_name);
    
    ret = NeuronGPU_instance->IsNeuronArrayVar(i_node, var_name_str);
  } END_ERR_PROP return ret; }
  

  int NeuronGPU_GetNeuronParamSize(int i_node, char *param_name)
  { int ret = 0; BEGIN_ERR_PROP {
    std::string param_name_str = std::string(param_name);
    
    ret = NeuronGPU_instance->GetNeuronParamSize(i_node, param_name_str);
  } END_ERR_PROP return ret; }
  
  
  int NeuronGPU_GetNeuronVarSize(int i_node, char *var_name)
  { int ret = 0; BEGIN_ERR_PROP {
    std::string var_name_str = std::string(var_name);
    
    ret = NeuronGPU_instance->GetNeuronVarSize(i_node, var_name_str);
  } END_ERR_PROP return ret; }
  
  
  float *NeuronGPU_GetNeuronParam(int i_node, int n_neuron,
				  char *param_name)
  { float *ret = NULL; BEGIN_ERR_PROP {
    
    std::string param_name_str = std::string(param_name);
    ret = NeuronGPU_instance->GetNeuronParam(i_node, n_neuron,
					     param_name_str);
  } END_ERR_PROP return ret; }


  float *NeuronGPU_GetNeuronPtParam(int *i_node, int n_neuron,
				 char *param_name)
  { float *ret = NULL; BEGIN_ERR_PROP {
    std::string param_name_str = std::string(param_name);
    ret = NeuronGPU_instance->GetNeuronParam(i_node, n_neuron,
					     param_name_str);
  } END_ERR_PROP return ret; }


  float *NeuronGPU_GetArrayParam(int i_node, char *param_name)
  { float *ret = NULL; BEGIN_ERR_PROP {
    
    std::string param_name_str = std::string(param_name);
    ret = NeuronGPU_instance->GetArrayParam(i_node, param_name_str);
  } END_ERR_PROP return ret; }

  
  int *NeuronGPU_GetNeuronIntVar(int i_node, int n_neuron,
				 char *param_name)
  { int *ret = NULL; BEGIN_ERR_PROP {
    
    std::string param_name_str = std::string(param_name);
    ret = NeuronGPU_instance->GetNeuronIntVar(i_node, n_neuron,
					      param_name_str);
  } END_ERR_PROP return ret; }


  int *NeuronGPU_GetNeuronPtIntVar(int *i_node, int n_neuron,
				   char *param_name)
  { int *ret = NULL; BEGIN_ERR_PROP {
    std::string param_name_str = std::string(param_name);
    ret = NeuronGPU_instance->GetNeuronIntVar(i_node, n_neuron,
					      param_name_str);
  } END_ERR_PROP return ret; }

  float *NeuronGPU_GetNeuronVar(int i_node, int n_neuron,
				char *param_name)
  { float *ret = NULL; BEGIN_ERR_PROP {
    
    std::string param_name_str = std::string(param_name);
    ret = NeuronGPU_instance->GetNeuronVar(i_node, n_neuron,
					   param_name_str);
  } END_ERR_PROP return ret; }


  float *NeuronGPU_GetNeuronPtVar(int *i_node, int n_neuron,
				 char *param_name)
  { float *ret = NULL; BEGIN_ERR_PROP {
    std::string param_name_str = std::string(param_name);
    ret = NeuronGPU_instance->GetNeuronVar(i_node, n_neuron,
					   param_name_str);
  } END_ERR_PROP return ret; }

  float *NeuronGPU_GetArrayVar(int i_node, char *var_name)
  { float *ret = NULL; BEGIN_ERR_PROP {
    
    std::string var_name_str = std::string(var_name);
    ret = NeuronGPU_instance->GetArrayVar(i_node, var_name_str);
  } END_ERR_PROP return ret; }


  int NeuronGPU_Calibrate()
  { int ret = 0; BEGIN_ERR_PROP {
    ret = NeuronGPU_instance->Calibrate();
  } END_ERR_PROP return ret; }

  int NeuronGPU_Simulate()
  { int ret = 0; BEGIN_ERR_PROP {
    ret = NeuronGPU_instance->Simulate();
  } END_ERR_PROP return ret; }

  int NeuronGPU_StartSimulation()
  { int ret = 0; BEGIN_ERR_PROP {
    ret = NeuronGPU_instance->StartSimulation();
  } END_ERR_PROP return ret; }

  int NeuronGPU_SimulationStep()
  { int ret = 0; BEGIN_ERR_PROP {
    ret = NeuronGPU_instance->SimulationStep();
  } END_ERR_PROP return ret; }

  int NeuronGPU_EndSimulation()
  { int ret = 0; BEGIN_ERR_PROP {
    ret = NeuronGPU_instance->EndSimulation();
  } END_ERR_PROP return ret; }

  int NeuronGPU_ConnectMpiInit(int argc, char *argv[])
  { int ret = 0; BEGIN_ERR_PROP {
    ret = NeuronGPU_instance->ConnectMpiInit(argc, argv);
  } END_ERR_PROP return ret; }

  int NeuronGPU_MpiId()
  { int ret = 0; BEGIN_ERR_PROP {
    ret = NeuronGPU_instance->MpiId();
  } END_ERR_PROP return ret; }

  int NeuronGPU_MpiNp()
  { int ret = 0; BEGIN_ERR_PROP {
    ret = NeuronGPU_instance->MpiNp();
  } END_ERR_PROP return ret; }
  int NeuronGPU_ProcMaster()
  { int ret = 0; BEGIN_ERR_PROP {
    ret = NeuronGPU_instance->ProcMaster();
  } END_ERR_PROP return ret; }

  int NeuronGPU_MpiFinalize()
  { int ret = 0; BEGIN_ERR_PROP {
    ret = NeuronGPU_instance->MpiFinalize();
  } END_ERR_PROP return ret; }

  unsigned int *NeuronGPU_RandomInt(size_t n)
  { unsigned int *ret = NULL; BEGIN_ERR_PROP {
    ret = NeuronGPU_instance->RandomInt(n);
  } END_ERR_PROP return ret; }
  
  float *NeuronGPU_RandomUniform(size_t n)
  { float* ret = NULL; BEGIN_ERR_PROP {
    ret = NeuronGPU_instance->RandomUniform(n);
  } END_ERR_PROP return ret; }
  
  float *NeuronGPU_RandomNormal(size_t n, float mean, float stddev)
  { float *ret = NULL; BEGIN_ERR_PROP {
    ret = NeuronGPU_instance->RandomNormal(n, mean, stddev);
  } END_ERR_PROP return ret; }
  
  float *NeuronGPU_RandomNormalClipped(size_t n, float mean, float stddev,
				       float vmin, float vmax, float vstep)
  { float *ret = NULL; BEGIN_ERR_PROP {
    ret = NeuronGPU_instance->RandomNormalClipped(n, mean, stddev, vmin,
						  vmax, vstep);
  } END_ERR_PROP return ret; }
  
  int NeuronGPU_Connect(int i_source_node, int i_target_node,
			unsigned char port, unsigned char syn_group,
			float weight, float delay)
  { int ret = 0; BEGIN_ERR_PROP {
    ret = NeuronGPU_instance->Connect(i_source_node, i_target_node,
				      port, syn_group, weight, delay);
  } END_ERR_PROP return ret; }

  int NeuronGPU_ConnSpecInit()
  { int ret = 0; BEGIN_ERR_PROP {
    ret = ConnSpec_instance.Init();
  } END_ERR_PROP return ret; }

  int NeuronGPU_SetConnSpecParam(char *param_name, int value)
  { int ret = 0; BEGIN_ERR_PROP {
    std::string param_name_str = std::string(param_name);
    ret = ConnSpec_instance.SetParam(param_name_str, value);
  } END_ERR_PROP return ret; }

  int NeuronGPU_ConnSpecIsParam(char *param_name)
  { int ret = 0; BEGIN_ERR_PROP {
    std::string param_name_str = std::string(param_name);
    ret = ConnSpec::IsParam(param_name_str);
  } END_ERR_PROP return ret; }

  int NeuronGPU_SynSpecInit()
  { int ret = 0; BEGIN_ERR_PROP {
    ret = SynSpec_instance.Init();
  } END_ERR_PROP return ret; }

  int NeuronGPU_SetSynSpecIntParam(char *param_name, int value)
  { int ret = 0; BEGIN_ERR_PROP {
    std::string param_name_str = std::string(param_name);
    ret = SynSpec_instance.SetParam(param_name_str, value);
  } END_ERR_PROP return ret; }

  int NeuronGPU_SetSynSpecFloatParam(char *param_name, float value)
  { int ret = 0; BEGIN_ERR_PROP {
    std::string param_name_str = std::string(param_name);
    ret = SynSpec_instance.SetParam(param_name_str, value);
  } END_ERR_PROP return ret; }

  int NeuronGPU_SetSynSpecFloatPtParam(char *param_name, float *array_pt)
  { int ret = 0; BEGIN_ERR_PROP {
    std::string param_name_str = std::string(param_name);
    ret = SynSpec_instance.SetParam(param_name_str, array_pt);
  } END_ERR_PROP return ret; }

  int NeuronGPU_SynSpecIsIntParam(char *param_name)
  { int ret = 0; BEGIN_ERR_PROP {
    std::string param_name_str = std::string(param_name);
    ret = SynSpec_instance.IsIntParam(param_name_str);
  } END_ERR_PROP return ret; }

  int NeuronGPU_SynSpecIsFloatParam(char *param_name)
  { int ret = 0; BEGIN_ERR_PROP {
    std::string param_name_str = std::string(param_name);
    ret = SynSpec_instance.IsFloatParam(param_name_str);
  } END_ERR_PROP return ret; }

  int NeuronGPU_SynSpecIsFloatPtParam(char *param_name)
  { int ret = 0; BEGIN_ERR_PROP {
    std::string param_name_str = std::string(param_name);
    ret = SynSpec_instance.IsFloatPtParam(param_name_str);
  } END_ERR_PROP return ret; }

  int NeuronGPU_ConnectSeqSeq(int i_source, int n_source, int i_target,
			      int n_target)
  { int ret = 0; BEGIN_ERR_PROP {
    ret = NeuronGPU_instance->Connect(i_source, n_source, i_target, n_target,
				      ConnSpec_instance, SynSpec_instance); 
  } END_ERR_PROP return ret; }

  int NeuronGPU_ConnectSeqGroup(int i_source, int n_source, int *i_target,
				int n_target)
  { int ret = 0; BEGIN_ERR_PROP {
    ret = NeuronGPU_instance->Connect(i_source, n_source, i_target, n_target,
				      ConnSpec_instance, SynSpec_instance); 
  } END_ERR_PROP return ret; }

  int NeuronGPU_ConnectGroupSeq(int *i_source, int n_source, int i_target,
				int n_target)
  { int ret = 0; BEGIN_ERR_PROP {
    ret = NeuronGPU_instance->Connect(i_source, n_source, i_target, n_target,
				      ConnSpec_instance, SynSpec_instance);
  } END_ERR_PROP return ret; }

  int NeuronGPU_ConnectGroupGroup(int *i_source, int n_source, int *i_target,
				  int n_target)
  { int ret = 0; BEGIN_ERR_PROP {
    ret = NeuronGPU_instance->Connect(i_source, n_source, i_target, n_target,
				      ConnSpec_instance, SynSpec_instance);
  } END_ERR_PROP return ret; }

  int NeuronGPU_RemoteConnectSeqSeq(int i_source_host, int i_source,
				    int n_source, int i_target_host,
				    int i_target, int n_target)
  { int ret = 0; BEGIN_ERR_PROP {
    ret = NeuronGPU_instance->RemoteConnect(i_source_host, i_source, n_source,
					    i_target_host, i_target, n_target,
					    ConnSpec_instance,
					    SynSpec_instance); 
  } END_ERR_PROP return ret; }

  int NeuronGPU_RemoteConnectSeqGroup(int i_source_host, int i_source,
				      int n_source, int i_target_host,
				      int *i_target, int n_target)
  { int ret = 0; BEGIN_ERR_PROP {
    ret = NeuronGPU_instance->RemoteConnect(i_source_host, i_source, n_source,
					    i_target_host, i_target, n_target,
					    ConnSpec_instance,
					    SynSpec_instance); 
  } END_ERR_PROP return ret; }

  int NeuronGPU_RemoteConnectGroupSeq(int i_source_host, int *i_source,
				      int n_source, int i_target_host,
				      int i_target, int n_target)
  { int ret = 0; BEGIN_ERR_PROP {
    ret = NeuronGPU_instance->RemoteConnect(i_source_host, i_source, n_source,
					    i_target_host, i_target, n_target,
					    ConnSpec_instance,
					    SynSpec_instance);
  } END_ERR_PROP return ret; }


  int NeuronGPU_RemoteConnectGroupGroup(int i_source_host, int *i_source,
					int n_source, int i_target_host,
					int *i_target, int n_target)
  { int ret = 0; BEGIN_ERR_PROP {
    ret = NeuronGPU_instance->RemoteConnect(i_source_host, i_source, n_source,
					    i_target_host, i_target, n_target,
					    ConnSpec_instance,
					    SynSpec_instance);
  } END_ERR_PROP return ret; }


  char **NeuronGPU_GetIntVarNames(int i_node)
  { char **ret = NULL; BEGIN_ERR_PROP {
    std::vector<std::string> var_name_vect =
      NeuronGPU_instance->GetIntVarNames(i_node);
    char **var_name_array = (char**)malloc(var_name_vect.size()
					   *sizeof(char*));
    for (unsigned int i=0; i<var_name_vect.size(); i++) {
      char *var_name = (char*)malloc((var_name_vect[i].length() + 1)
				      *sizeof(char));
      
      strcpy(var_name, var_name_vect[i].c_str());
      var_name_array[i] = var_name;
    }
    ret = var_name_array;
    
  } END_ERR_PROP return ret; }

  char **NeuronGPU_GetScalVarNames(int i_node)
  { char **ret = NULL; BEGIN_ERR_PROP {
    std::vector<std::string> var_name_vect =
      NeuronGPU_instance->GetScalVarNames(i_node);
    char **var_name_array = (char**)malloc(var_name_vect.size()
					   *sizeof(char*));
    for (unsigned int i=0; i<var_name_vect.size(); i++) {
      char *var_name = (char*)malloc((var_name_vect[i].length() + 1)
				      *sizeof(char));
      
      strcpy(var_name, var_name_vect[i].c_str());
      var_name_array[i] = var_name;
    }
    ret = var_name_array;
    
  } END_ERR_PROP return ret; }

  int NeuronGPU_GetNIntVar(int i_node)
  { int ret = 0; BEGIN_ERR_PROP {
    ret = NeuronGPU_instance->GetNIntVar(i_node);
  } END_ERR_PROP return ret; }

  int NeuronGPU_GetNScalVar(int i_node)
  { int ret = 0; BEGIN_ERR_PROP {
    ret = NeuronGPU_instance->GetNScalVar(i_node);
  } END_ERR_PROP return ret; }


  char **NeuronGPU_GetPortVarNames(int i_node)
  { char **ret = NULL; BEGIN_ERR_PROP {
    std::vector<std::string> var_name_vect =
      NeuronGPU_instance->GetPortVarNames(i_node);
    char **var_name_array = (char**)malloc(var_name_vect.size()
					   *sizeof(char*));
    for (unsigned int i=0; i<var_name_vect.size(); i++) {
      char *var_name = (char*)malloc((var_name_vect[i].length() + 1)
				      *sizeof(char));
      
      strcpy(var_name, var_name_vect[i].c_str());
      var_name_array[i] = var_name;
    }
    ret = var_name_array;
    
  } END_ERR_PROP return ret; }

  
  int NeuronGPU_GetNPortVar(int i_node)
  { int ret = 0; BEGIN_ERR_PROP {
    ret = NeuronGPU_instance->GetNPortVar(i_node);
  } END_ERR_PROP return ret; }

  
  char **NeuronGPU_GetScalParamNames(int i_node)
  { char **ret = NULL; BEGIN_ERR_PROP {
    std::vector<std::string> var_name_vect =
      NeuronGPU_instance->GetScalParamNames(i_node);
    char **var_name_array = (char**)malloc(var_name_vect.size()
					   *sizeof(char*));
    for (unsigned int i=0; i<var_name_vect.size(); i++) {
      char *var_name = (char*)malloc((var_name_vect[i].length() + 1)
				      *sizeof(char));
      
      strcpy(var_name, var_name_vect[i].c_str());
      var_name_array[i] = var_name;
    }
    ret = var_name_array;
    
  } END_ERR_PROP return ret; }

  
  int NeuronGPU_GetNScalParam(int i_node)
  { int ret = 0; BEGIN_ERR_PROP {
    ret = NeuronGPU_instance->GetNScalParam(i_node);
  } END_ERR_PROP return ret; }


  char **NeuronGPU_GetGroupParamNames(int i_node)
  { char **ret = NULL; BEGIN_ERR_PROP {
    std::vector<std::string> var_name_vect =
      NeuronGPU_instance->GetGroupParamNames(i_node);
    char **var_name_array = (char**)malloc(var_name_vect.size()
					   *sizeof(char*));
    for (unsigned int i=0; i<var_name_vect.size(); i++) {
      char *var_name = (char*)malloc((var_name_vect[i].length() + 1)
				      *sizeof(char));
      
      strcpy(var_name, var_name_vect[i].c_str());
      var_name_array[i] = var_name;
    }
    ret = var_name_array;
    
  } END_ERR_PROP return ret; }

  
  int NeuronGPU_GetNGroupParam(int i_node)
  { int ret = 0; BEGIN_ERR_PROP {
    ret = NeuronGPU_instance->GetNGroupParam(i_node);
  } END_ERR_PROP return ret; }


  char **NeuronGPU_GetPortParamNames(int i_node)
  { char **ret = NULL; BEGIN_ERR_PROP {
    std::vector<std::string> var_name_vect =
      NeuronGPU_instance->GetPortParamNames(i_node);
    char **var_name_array = (char**)malloc(var_name_vect.size()
					   *sizeof(char*));
    for (unsigned int i=0; i<var_name_vect.size(); i++) {
      char *var_name = (char*)malloc((var_name_vect[i].length() + 1)
				      *sizeof(char));
      
      strcpy(var_name, var_name_vect[i].c_str());
      var_name_array[i] = var_name;
    }
    ret = var_name_array;
    
  } END_ERR_PROP return ret; }

  
  int NeuronGPU_GetNPortParam(int i_node)
  { int ret = 0; BEGIN_ERR_PROP {
    ret = NeuronGPU_instance->GetNPortParam(i_node);
  } END_ERR_PROP return ret; }


  char **NeuronGPU_GetArrayParamNames(int i_node)
  { char **ret = NULL; BEGIN_ERR_PROP {
    std::vector<std::string> var_name_vect =
      NeuronGPU_instance->GetArrayParamNames(i_node);
    char **var_name_array = (char**)malloc(var_name_vect.size()
					   *sizeof(char*));
    for (unsigned int i=0; i<var_name_vect.size(); i++) {
      char *var_name = (char*)malloc((var_name_vect[i].length() + 1)
				      *sizeof(char));
      
      strcpy(var_name, var_name_vect[i].c_str());
      var_name_array[i] = var_name;
    }
    ret = var_name_array;
    
  } END_ERR_PROP return ret; }

  
  int NeuronGPU_GetNArrayParam(int i_node)
  { int ret = 0; BEGIN_ERR_PROP {
    ret = NeuronGPU_instance->GetNArrayParam(i_node);
  } END_ERR_PROP return ret; }

  char **NeuronGPU_GetArrayVarNames(int i_node)
  { char **ret = NULL; BEGIN_ERR_PROP {
    std::vector<std::string> var_name_vect =
      NeuronGPU_instance->GetArrayVarNames(i_node);
    char **var_name_array = (char**)malloc(var_name_vect.size()
					   *sizeof(char*));
    for (unsigned int i=0; i<var_name_vect.size(); i++) {
      char *var_name = (char*)malloc((var_name_vect[i].length() + 1)
				      *sizeof(char));
      
      strcpy(var_name, var_name_vect[i].c_str());
      var_name_array[i] = var_name;
    }
    ret = var_name_array;
    
  } END_ERR_PROP return ret; }

  int NeuronGPU_GetNArrayVar(int i_node)
  { int ret = 0; BEGIN_ERR_PROP {
    ret = NeuronGPU_instance->GetNArrayVar(i_node);
  } END_ERR_PROP return ret; }


  int *NeuronGPU_GetSeqSeqConnections(int i_source, int n_source, int i_target,
				      int n_target, int syn_group, int *n_conn)
  { int *ret = NULL; BEGIN_ERR_PROP {
      std::vector<ConnectionId> conn_id_vect =
	NeuronGPU_instance->GetConnections(i_source, n_source, i_target,
					   n_target, syn_group);
      *n_conn = conn_id_vect.size();
      int *conn_id_array = (int*)malloc((*n_conn)*3*sizeof(int));
      for (int i=0; i<(*n_conn); i++) {
	conn_id_array[i*3] = conn_id_vect[i].i_source_;
	conn_id_array[i*3 + 1] = conn_id_vect[i].i_group_;
	conn_id_array[i*3 + 2] = conn_id_vect[i].i_conn_;
      }
      ret = conn_id_array;
  } END_ERR_PROP return ret; }

  int *NeuronGPU_GetSeqGroupConnections(int i_source, int n_source,
					int *i_target, int n_target,
					int syn_group, int *n_conn)
  { int *ret = NULL; BEGIN_ERR_PROP {
      std::vector<ConnectionId> conn_id_vect =
	NeuronGPU_instance->GetConnections(i_source, n_source, i_target,
					   n_target, syn_group);
      *n_conn = conn_id_vect.size();
      int *conn_id_array = (int*)malloc((*n_conn)*3*sizeof(int));
      for (int i=0; i<(*n_conn); i++) {
	conn_id_array[i*3] = conn_id_vect[i].i_source_;
	conn_id_array[i*3 + 1] = conn_id_vect[i].i_group_;
	conn_id_array[i*3 + 2] = conn_id_vect[i].i_conn_;
      }
      ret = conn_id_array;
  } END_ERR_PROP return ret; }

  int *NeuronGPU_GetGroupSeqConnections(int *i_source, int n_source,
					int i_target, int n_target,
					int syn_group, int *n_conn)
  { int *ret = NULL; BEGIN_ERR_PROP {
      std::vector<ConnectionId> conn_id_vect =
	NeuronGPU_instance->GetConnections(i_source, n_source, i_target,
					   n_target, syn_group);
      *n_conn = conn_id_vect.size();
      int *conn_id_array = (int*)malloc((*n_conn)*3*sizeof(int));
      for (int i=0; i<(*n_conn); i++) {
	conn_id_array[i*3] = conn_id_vect[i].i_source_;
	conn_id_array[i*3 + 1] = conn_id_vect[i].i_group_;
	conn_id_array[i*3 + 2] = conn_id_vect[i].i_conn_;
      }
      ret = conn_id_array;
  } END_ERR_PROP return ret; }

  int *NeuronGPU_GetGroupGroupConnections(int *i_source, int n_source,
					 int *i_target, int n_target,
					 int syn_group, int *n_conn)
  { int *ret = NULL; BEGIN_ERR_PROP {
      std::vector<ConnectionId> conn_id_vect =
	NeuronGPU_instance->GetConnections(i_source, n_source, i_target,
					   n_target, syn_group);
      *n_conn = conn_id_vect.size();
      int *conn_id_array = (int*)malloc((*n_conn)*3*sizeof(int));
      for (int i=0; i<(*n_conn); i++) {
	conn_id_array[i*3] = conn_id_vect[i].i_source_;
	conn_id_array[i*3 + 1] = conn_id_vect[i].i_group_;
	conn_id_array[i*3 + 2] = conn_id_vect[i].i_conn_;
      }
      ret = conn_id_array;
  } END_ERR_PROP return ret; }

  int NeuronGPU_GetConnectionStatus(int i_source, int i_group, int i_conn,
				    int *i_target, unsigned char *port,
				    unsigned char *syn_group, float *delay,
				    float *weight)
  { int ret = 0; BEGIN_ERR_PROP {
      ConnectionId conn_id;
      conn_id.i_source_ = i_source;
      conn_id.i_group_ = i_group;
      conn_id.i_conn_ = i_conn;
      ConnectionStatus conn_stat
	= NeuronGPU_instance->GetConnectionStatus(conn_id);
      *i_target = conn_stat.i_target;
      *port = conn_stat.port;
      *syn_group = conn_stat.syn_group;
      *delay = conn_stat.delay;
      *weight = conn_stat.weight;
      
      ret = 0;
  } END_ERR_PROP return ret; }



  int NeuronGPU_CreateSynGroup(char *model_name)
  { int ret = 0; BEGIN_ERR_PROP {
    std::string model_name_str = std::string(model_name);
    ret = NeuronGPU_instance->CreateSynGroup(model_name_str);
  } END_ERR_PROP return ret; }


  int NeuronGPU_GetSynGroupNParam(int i_syn_group)
  { int ret = 0; BEGIN_ERR_PROP {
    ret = NeuronGPU_instance->GetSynGroupNParam(i_syn_group);
  } END_ERR_PROP return ret; }

  
  char **NeuronGPU_GetSynGroupParamNames(int i_syn_group)
  { char **ret = NULL; BEGIN_ERR_PROP {
    std::vector<std::string> name_vect =
      NeuronGPU_instance->GetSynGroupParamNames(i_syn_group);
    char **name_array = (char**)malloc(name_vect.size()
				       *sizeof(char*));
    for (unsigned int i=0; i<name_vect.size(); i++) {
      char *param_name = (char*)malloc((name_vect[i].length() + 1)
				       *sizeof(char));
      
      strcpy(param_name, name_vect[i].c_str());
      name_array[i] = param_name;
    }
    ret = name_array;
    
  } END_ERR_PROP return ret; }

  
  int NeuronGPU_IsSynGroupParam(int i_syn_group, char *param_name)
  { int ret = 0; BEGIN_ERR_PROP {
    std::string param_name_str = std::string(param_name);
    
    ret = NeuronGPU_instance->IsSynGroupParam(i_syn_group, param_name_str);
  } END_ERR_PROP return ret; }

  
  int NeuronGPU_GetSynGroupParamIdx(int i_syn_group, char *param_name)
  { int ret = 0; BEGIN_ERR_PROP {
    std::string param_name_str = std::string(param_name);
    
    ret = NeuronGPU_instance->GetSynGroupParamIdx(i_syn_group, param_name_str);
  } END_ERR_PROP return ret; }

  
  float NeuronGPU_GetSynGroupParam(int i_syn_group, char *param_name)
  { float ret = 0.0; BEGIN_ERR_PROP {
    
    std::string param_name_str = std::string(param_name);
    ret = NeuronGPU_instance->GetSynGroupParam(i_syn_group, param_name_str);
  } END_ERR_PROP return ret; }

  
  int NeuronGPU_SetSynGroupParam(int i_syn_group, char *param_name, float val)
  { float ret = 0.0; BEGIN_ERR_PROP {
    
    std::string param_name_str = std::string(param_name);
    ret = NeuronGPU_instance->SetSynGroupParam(i_syn_group, param_name_str,
					       val);
  } END_ERR_PROP return ret; }

  
  int NeuronGPU_ActivateSpikeCount(int i_node, int n_node)
  { int ret = 0; BEGIN_ERR_PROP {
    
    ret = NeuronGPU_instance->ActivateSpikeCount(i_node, n_node);
  } END_ERR_PROP return ret; }

  
  int NeuronGPU_ActivateRecSpikeTimes(int i_node, int n_node,
				      int max_n_rec_spike_times)
  { int ret = 0; BEGIN_ERR_PROP {
    
      ret = NeuronGPU_instance->ActivateRecSpikeTimes(i_node, n_node,
						      max_n_rec_spike_times);
  } END_ERR_PROP return ret; }

  
  int NeuronGPU_GetNRecSpikeTimes(int i_node)
  { int ret = 0; BEGIN_ERR_PROP {
    
      ret = NeuronGPU_instance->GetNRecSpikeTimes(i_node);
  } END_ERR_PROP return ret; }

  float* NeuronGPU_GetRecSpikeTimes(int i_node)
  { float *ret = NULL; BEGIN_ERR_PROP {
    std::vector<float> spike_time_vect
      = NeuronGPU_instance->GetRecSpikeTimes(i_node);

    int ns = (int)spike_time_vect.size();
    ret = new float[ns];
    for (int i=0; i<ns; i++) {
      ret[i] = spike_time_vect[i];
    }
    
  } END_ERR_PROP return ret; }
  
  int NeuronGPU_PushSpikesToNodes(int n_spikes, int *node_id)
  { int ret = 0; BEGIN_ERR_PROP {
    
      ret = NeuronGPU_instance->PushSpikesToNodes(n_spikes, node_id);
  } END_ERR_PROP return ret; }

  int NeuronGPU_GetExtNeuronInputSpikes(int *n_spikes, int **node, int **port,
			      float **spike_height, int include_zeros)
  { int ret = 0; BEGIN_ERR_PROP {
    
      ret = NeuronGPU_instance->GetExtNeuronInputSpikes(n_spikes, node, port,
							spike_height,
							include_zeros>0);
  } END_ERR_PROP return ret; }

  int NeuronGPU_SetNeuronGroupParam(int i_node, int n_node, char *param_name,
				    float val)
  { float ret = 0.0; BEGIN_ERR_PROP {
    
    std::string param_name_str = std::string(param_name);
    ret = NeuronGPU_instance->SetNeuronGroupParam(i_node, n_node,
						  param_name_str,
						  val);
  } END_ERR_PROP return ret; }

  int NeuronGPU_IsNeuronGroupParam(int i_node, char *param_name)
  { int ret = 0; BEGIN_ERR_PROP {
    std::string param_name_str = std::string(param_name);
    
    ret = NeuronGPU_instance->IsNeuronGroupParam(i_node, param_name_str);
  } END_ERR_PROP return ret; }

  float NeuronGPU_GetNeuronGroupParam(int i_node, char *param_name)
  { float ret = 0; BEGIN_ERR_PROP {
    std::string param_name_str = std::string(param_name);
    
    ret = NeuronGPU_instance->GetNeuronGroupParam(i_node, param_name_str);
  } END_ERR_PROP return ret; }


  int NeuronGPU_GetNFloatParam()
  { int ret = 0; BEGIN_ERR_PROP {
    ret = NeuronGPU_instance->GetNFloatParam();
  } END_ERR_PROP return ret; }

  
  char **NeuronGPU_GetFloatParamNames()
  { char **ret = NULL; BEGIN_ERR_PROP {
    std::vector<std::string> name_vect =
      NeuronGPU_instance->GetFloatParamNames();
    char **name_array = (char**)malloc(name_vect.size()
				       *sizeof(char*));
    for (unsigned int i=0; i<name_vect.size(); i++) {
      char *param_name = (char*)malloc((name_vect[i].length() + 1)
				       *sizeof(char));
      
      strcpy(param_name, name_vect[i].c_str());
      name_array[i] = param_name;
    }
    ret = name_array;
    
  } END_ERR_PROP return ret; }

  
  int NeuronGPU_IsFloatParam(char *param_name)
  { int ret = 0; BEGIN_ERR_PROP {
    std::string param_name_str = std::string(param_name);
    
    ret = NeuronGPU_instance->IsFloatParam(param_name_str);
  } END_ERR_PROP return ret; }

  
  int NeuronGPU_GetFloatParamIdx(char *param_name)
  { int ret = 0; BEGIN_ERR_PROP {
    std::string param_name_str = std::string(param_name);
    
    ret = NeuronGPU_instance->GetFloatParamIdx(param_name_str);
  } END_ERR_PROP return ret; }

  
  float NeuronGPU_GetFloatParam(char *param_name)
  { float ret = 0.0; BEGIN_ERR_PROP {
    std::string param_name_str = std::string(param_name);
    
    ret = NeuronGPU_instance->GetFloatParam(param_name_str);
  } END_ERR_PROP return ret; }

  
  int NeuronGPU_SetFloatParam(char *param_name, float val)
  { int ret = 0; BEGIN_ERR_PROP {
    std::string param_name_str = std::string(param_name);
    
    ret = NeuronGPU_instance->SetFloatParam(param_name_str, val);
  } END_ERR_PROP return ret; }

  int NeuronGPU_GetNIntParam()
  { int ret = 0; BEGIN_ERR_PROP {
    ret = NeuronGPU_instance->GetNIntParam();
  } END_ERR_PROP return ret; }

  
  char **NeuronGPU_GetIntParamNames()
  { char **ret = NULL; BEGIN_ERR_PROP {
    std::vector<std::string> name_vect =
      NeuronGPU_instance->GetIntParamNames();
    char **name_array = (char**)malloc(name_vect.size()
				       *sizeof(char*));
    for (unsigned int i=0; i<name_vect.size(); i++) {
      char *param_name = (char*)malloc((name_vect[i].length() + 1)
				       *sizeof(char));
      
      strcpy(param_name, name_vect[i].c_str());
      name_array[i] = param_name;
    }
    ret = name_array;
    
  } END_ERR_PROP return ret; }

  
  int NeuronGPU_IsIntParam(char *param_name)
  { int ret = 0; BEGIN_ERR_PROP {
    std::string param_name_str = std::string(param_name);
    
    ret = NeuronGPU_instance->IsIntParam(param_name_str);
  } END_ERR_PROP return ret; }

  
  int NeuronGPU_GetIntParamIdx(char *param_name)
  { int ret = 0; BEGIN_ERR_PROP {
    std::string param_name_str = std::string(param_name);
    
    ret = NeuronGPU_instance->GetIntParamIdx(param_name_str);
  } END_ERR_PROP return ret; }

  
  int NeuronGPU_GetIntParam(char *param_name)
  { int ret = 0; BEGIN_ERR_PROP {
    
    std::string param_name_str = std::string(param_name);
    ret = NeuronGPU_instance->GetIntParam(param_name_str);
  } END_ERR_PROP return ret; }

  
  int NeuronGPU_SetIntParam(char *param_name, int val)
  { int ret = 0; BEGIN_ERR_PROP {
    
    std::string param_name_str = std::string(param_name);
    ret = NeuronGPU_instance->SetIntParam(param_name_str, val);
  } END_ERR_PROP return ret; }

  int NeuronGPU_RemoteCreate(int i_host, char *model_name, int n_neuron,
			     int n_port)
  { int ret = 0; BEGIN_ERR_PROP {
    std::string model_name_str = std::string(model_name);
    RemoteNodeSeq rneur = NeuronGPU_instance->RemoteCreate(i_host,
							   model_name_str,
							   n_neuron,
							   n_port);
    ret = rneur.node_seq[0];
  } END_ERR_PROP return ret; }

}

