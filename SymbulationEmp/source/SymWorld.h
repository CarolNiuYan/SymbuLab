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
  
  emp::Ptr<emp::DataMonitor<int>> data_node_hostcount;
  emp::Ptr<emp::DataMonitor<int>> data_node_symcount;
  emp::Ptr<emp::DataMonitor<int>> data_node_infectedhostnum; 

 public:
 //set fun_print_org to equal function that prints hosts/syms correctly
 SymWorld(emp::Random &random) : emp::World<Host>(random) {
    fun_print_org = [](Host & org, std::ostream & os) {
      //os << PrintHost(&org);
    };
  }

  void InjectSymbiont() {
    Symbiont * newSym = new Symbiont("18", "E", 10, 0, 1, -1);
    int newLoc = GetRandomCellID();
    while (!IsOccupied(newLoc) || pop[newLoc]->HasSym()){
      newLoc = GetRandomCellID();
    }
    //for future multiple pairs check, not needed for now
    //if(newSym.GetTargetType() == pop[newLoc]->GetName()){
    pop[newLoc]->AddSymbionts(*newSym); 
  }
  
  
  emp::DataFile & SetupResultFile(const std::string & filename) {
    auto & file = SetupFile(filename);
    auto & HostCount = GetHostCountDataNode();
    auto & SymCount = GetSymCountDataNode();
    auto & InfectedHost = GetInfectedHostNumDataNode();
    
    file.AddVar(update, "update", "Update");
    file.AddTotal(HostCount, "host_count", "Number of host in the population");
    file.AddTotal(SymCount, "sym_count", "Number of symbionts in the population");
    file.AddTotal(InfectedHost, "infected_host", "Number of infected hosts in the population");

    file.PrintHeaderKeys();

    return file;
  }

  size_t CalcSymNum(size_t i) {
    return pop[i]->NumSym();
  }
  
  size_t CalcInfectedHostNum(size_t i) {
    if (pop[i]->HasSym()) return 1;
    else return 0;
  }

  emp::DataMonitor<int>& GetHostCountDataNode() {
    if (!data_node_hostcount) {
      data_node_hostcount.New();
      OnUpdate(
	       [this](size_t){
		 data_node_hostcount->Reset();
		 for (size_t i = 0; i< pop.size(); i++) {
                   //CHANGED: Check if the organism has a symbiont before adding the data
		   if (IsOccupied(i)) data_node_hostcount->AddDatum(1);
		 }
	       }
	       );
    }
    return *data_node_hostcount;
  }
  
  emp::DataMonitor<int>& GetSymCountDataNode() {
    if (!data_node_symcount) {
      data_node_symcount.New();
      OnUpdate(
	       [this](size_t){
		 data_node_symcount->Reset();
		 for (size_t i = 0; i< pop.size(); i++) {
                   //CHANGED: Check if the organism has a symbiont before adding the data
		   if (IsOccupied(i)) data_node_symcount->AddDatum(CalcSymNum(i));
		 }
	       }
	       );
    }
    return *data_node_symcount;
  }
    
  emp::DataMonitor<int>& GetInfectedHostNumDataNode() {
    if (!data_node_infectedhostnum) {
      data_node_infectedhostnum.New();
      OnUpdate(
	       [this](size_t){
		 data_node_infectedhostnum->Reset();
		 for (size_t i = 0; i< pop.size(); i++) {
                   //CHANGED: Check if the organism has a symbiont before adding the data
		   if (IsOccupied(i)) data_node_infectedhostnum->AddDatum(CalcInfectedHostNum(i));
		 }
	       }
	       );
    }
    return *data_node_infectedhostnum;
  }
  
  
  void Update(size_t new_resources=10) {
    emp::World<Host>::Update();
    emp::vector<size_t> schedule = emp::GetPermutation(random, GetSize());
    double resource_portion = new_resources;
    if (GetNumOrgs() != 0){
      resource_portion = new_resources/GetNumOrgs();
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
            Symbiont * offspring = new Symbiont(parent.GetStrainType(), parent.GetTargetType(), 10.0, 0.0, parent.GetReproRate(), -1);
            pop[i]->AddReproSymbionts(offspring);
          }
          (*(pop[i]->GetSymbionts()))[j].IncrementBurstTimer();
        }

        //check burst for every symbiont
        for(size_t j = 0; j < (pop[i]->GetSymbionts())->size(); j++) {
          if((*(pop[i]->GetSymbionts()))[j].CheckBurst()) {
            //std::cout << "bursted" <<(*(pop[i]->GetSymbionts()))[j].GetBurstTimer() << std::endl;
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


