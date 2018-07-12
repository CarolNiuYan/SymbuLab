#include "source/Evolve/World.h"
#include "source/tools/Random.h"
#include <set>
#include "SymOrg.h"
#include "source/tools/random_utils.h"
#include "source/data/DataFile.h"

class SymWorld : public emp::World<Host>{
 private:
  double vertTrans = 0; 
  double mut_rate = 0;
  emp::Random random;
  
  emp::Ptr<emp::DataMonitor<int>> data_node_ecoli_count;
  emp::Ptr<emp::DataMonitor<int>> data_node_shi_count;
  emp::Ptr<emp::DataMonitor<int>> data_node_e_sym_count;
  emp::Ptr<emp::DataMonitor<int>> data_node_s_sym_count;

 public:
 SymWorld(emp::Random &random) : emp::World<Host>(random) {
    fun_print_org = [](Host & org, std::ostream & os) {
      //os << PrintHost(&org);
    };
  }

  void InjectSymbiont(std::string strain, double burstVal, double reproRate) {
    //current model only fits for one symbiont type for each host
    Symbiont * newSym = new Symbiont("strain18", strain, burstVal, 0, reproRate, -1);
    int newLoc = GetRandomCellID();
    if (IsOccupied(newLoc) == true) {              
      if(newSym->GetTargetType() == pop[newLoc]->GetName()){
        if (!pop[newLoc]->HasSym()) {
          pop[newLoc]->AddSymbionts(*newSym);
        } else {
          if(random.GetInt(0, 100000) <= 1) {
            pop[newLoc]->AddSymbionts(*newSym);
          }
        }
      }
    }
  }
  
  
  emp::DataFile & SetupResultFile(const std::string & filename) {
    auto & file = SetupFile(filename);
    auto & EcoliCount = GetEcoliDataNode();
    auto & ShiCount = GetShigellaDataNode();
    auto & EcoliSymCount = GetEcoliSymCountDataNode();
    auto & ShiSymCount = GetShiSymCountDataNode();
    
    file.AddVar(update, "update", "Update");
    file.AddTotal(EcoliCount, "Ecoli_count", "Number of Ecoli host in the population");
    file.AddTotal(ShiCount, "Shigella_count", "Number of Shigella host in the population");
    file.AddTotal(EcoliSymCount, "e_sym_count", "Number of ecoli symbionts in the population");
    file.AddTotal(ShiSymCount, "s_sym_count", "Number of shigella symbionts in the population");

    file.PrintHeaderKeys();

    return file;
  }

  size_t CalcSymNum(size_t i) {
    return pop[i]->NumSym();
  }
  
  emp::DataMonitor<int>& GetEcoliDataNode() {
    if (!data_node_ecoli_count) {
      data_node_ecoli_count.New();
      OnUpdate(
	       [this](size_t){
		 data_node_ecoli_count->Reset();
		 for (size_t i = 0; i< pop.size(); i++) {
                   //CHANGED: Check if the organism has a symbiont before adding the data
		   if (IsOccupied(i) && pop[i]->GetName() == "Ecoli") data_node_ecoli_count->AddDatum(1);
                 }
               }
               );
    }
    return *data_node_ecoli_count;
  }
    
  emp::DataMonitor<int>& GetShigellaDataNode() {
    if (!data_node_shi_count) {
      data_node_shi_count.New();
      OnUpdate(
               [this](size_t){
                 data_node_shi_count->Reset();
                 for (size_t i = 0; i< pop.size(); i++) {
                   //CHANGED: Check if the organism has a symbiont before adding the data
                   if (IsOccupied(i)&& pop[i]->GetName() == "Shigella") data_node_shi_count->AddDatum(1);
                 }
               }
               );
    }
    return *data_node_shi_count;
  }
    
  emp::DataMonitor<int>& GetEcoliSymCountDataNode() {
    if (!data_node_e_sym_count) {
      data_node_e_sym_count.New();
      OnUpdate(
               [this](size_t){
                 data_node_e_sym_count->Reset();
                 for (size_t i = 0; i< pop.size(); i++) {
                   //CHANGED: Check if the organism has a symbiont before adding the data
                   if (IsOccupied(i) && pop[i]->GetName() == "Ecoli") data_node_e_sym_count->AddDatum(CalcSymNum(i));
                 }
               }
               );
    }
    return *data_node_e_sym_count;
  }
  
  emp::DataMonitor<int>& GetShiSymCountDataNode() {
    if (!data_node_s_sym_count) {
      data_node_s_sym_count.New();
      OnUpdate(
               [this](size_t){
                 data_node_s_sym_count->Reset();
                 for (size_t i = 0; i< pop.size(); i++) {
                   if (IsOccupied(i) && pop[i]->GetName() == "Shigella") data_node_s_sym_count->AddDatum(CalcSymNum(i));
                 }
               }
               );
    }
    return *data_node_s_sym_count;
  }
    
  
  void Update(double new_resources=50) {
    emp::World<Host>::Update();
    emp::vector<size_t> schedule = emp::GetPermutation(random, GetSize());
    double resource_portion = new_resources;
    if (pop.size() != 0){
      resource_portion = new_resources/pop.size();
    }
    
    for (size_t i : schedule) {
      if (IsOccupied(i) == false) continue;  // no organism at that cell
      
      pop[i]->Process(random, resource_portion);  //distribute    
      
      //if not infected
      if (!pop[i]->HasSym() && pop[i]->GetResources() >= 1000) {  // host replication
	Host *host_baby = new Host(pop[i]->GetName());  
	pop[i]->SubtractResources(1000);
        //TODO: Check once more to make sure it automatically deals with Grow World Model
	DoBirth(*host_baby, i); //should append new org to end of population


      //if infected  
      } else if (pop[i]->HasSym()) {
        //repro for every symbionts
        for (size_t j = 0; j < (pop[i]->GetSymbionts())->size(); j++) {
          Symbiont parent = (*(pop[i]->GetSymbionts()))[j];
          for(int k = 0; k < parent.GetReproRate(); k++) {
            Symbiont * offspring = new Symbiont(parent.GetStrainType(),parent.GetTargetType(),
                                                parent.GetBurstTimer(), 0.0, parent.GetReproRate(), -1);
            pop[i]->AddReproSymbionts(offspring);
          }
          (*(pop[i]->GetSymbionts()))[j].IncrementBurstTimer();
        }

        //check burst for every symbiont
        for(size_t j = 0; j < (pop[i]->GetSymbionts())->size(); j++) {
          if((*(pop[i]->GetSymbionts()))[j].CheckBurst()) {            
            //syms and repro_sym get ready
            for (size_t symNum = 0; symNum < (pop[i]->GetSymbionts())->size(); symNum++) {
              Symbiont refreshedSym = (*(pop[i]->GetSymbionts()))[symNum];
              refreshedSym.ResetSymbiont();
              (pop[i]->GetReproSymbionts())->push_back(refreshedSym);
            }
            //now burst
            for (size_t burstCount = 0; burstCount < (pop[i]->GetReproSymbionts())->size(); burstCount++) {
              Symbiont burstSym = (*(pop[i]->GetReproSymbionts()))[burstCount];
              int newLoc = GetRandomCellID();
              if (IsOccupied(newLoc) == true) {              
                if(burstSym.GetTargetType() == pop[newLoc]->GetName()){
                  if (!pop[newLoc]->HasSym()) {
                    pop[newLoc]->AddSymbionts(burstSym);
                  } else {
                    if(random.GetInt(0, 100000) <= 1) {
                      pop[newLoc]->AddSymbionts(burstSym);
                    }
                  }
                }
              }//if occupied
            }
            DoDeath(i);
            break;
          }//if check burst
        }        
      }//if infected
    }//update                                                           
  }
};


