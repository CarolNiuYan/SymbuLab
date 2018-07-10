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
  
  emp::Ptr<emp::DataMonitor<double, emp::data::Histogram>> data_node_hostintval;
  emp::Ptr<emp::DataMonitor<double, emp::data::Histogram>> data_node_symintval;

 public:
 //set fun_print_org to equal function that prints hosts/syms correctly
 SymWorld(emp::Random &random) : emp::World<Host>(random) {
    fun_print_org = [](Host & org, std::ostream & os) {
      //os << PrintHost(&org);
    };
  }
  
  /*
  emp::DataFile & SetupSymIntValFile(const std::string & filename) {
    auto & file = SetupFile(filename);
    auto & node = GetSymIntValDataNode();
    node.SetupBins(-1.0, 1.0, 20);
    file.AddVar(update, "update", "Update");
    file.AddMean(node, "mean_intval", "Average symbiont interaction value");
    file.AddHistBin(node, 0, "Hist_-1", "Count for histogram bin -1 to <-0.9");
    file.AddHistBin(node, 1, "Hist_-0.9", "Count for histogram bin -0.9 to <-0.8");
    file.AddHistBin(node, 2, "Hist_-0.8", "Count for histogram bin -0.8 to <-0.7");
    file.AddHistBin(node, 3, "Hist_-0.7", "Count for histogram bin -0.7 to <-0.6");
    file.AddHistBin(node, 4, "Hist_-0.6", "Count for histogram bin -0.6 to <-0.5");
    file.AddHistBin(node, 5, "Hist_-0.5", "Count for histogram bin -0.5 to <-0.4");
    file.AddHistBin(node, 6, "Hist_-0.4", "Count for histogram bin -0.4 to <-0.3");
    file.AddHistBin(node, 7, "Hist_-0.3", "Count for histogram bin -0.3 to <-0.2");
    file.AddHistBin(node, 8, "Hist_-0.2", "Count for histogram bin -0.2 to <-0.1");
    file.AddHistBin(node, 9, "Hist_-0.1", "Count for histogram bin -0.1 to <0.0");
    file.AddHistBin(node, 10, "Hist_0.0", "Count for histogram bin 0.0 to <0.1");
    file.AddHistBin(node, 11, "Hist_0.1", "Count for histogram bin 0.1 to <0.2");
    file.AddHistBin(node, 12, "Hist_0.2", "Count for histogram bin 0.2 to <0.3");
    file.AddHistBin(node, 13, "Hist_0.3", "Count for histogram bin 0.3 to <0.4");
    file.AddHistBin(node, 14, "Hist_0.4", "Count for histogram bin 0.4 to <0.5");
    file.AddHistBin(node, 15, "Hist_0.5", "Count for histogram bin 0.5 to <0.6");
    file.AddHistBin(node, 16, "Hist_0.6", "Count for histogram bin 0.6 to <0.7");
    file.AddHistBin(node, 17, "Hist_0.7", "Count for histogram bin 0.7 to <0.8");
    file.AddHistBin(node, 18, "Hist_0.8", "Count for histogram bin 0.8 to <0.9");
    file.AddHistBin(node, 19, "Hist_0.9", "Count for histogram bin 0.9 to 1.0");


    file.PrintHeaderKeys();

    return file;
  }

    double CalcSymIntVal(size_t i) {    
      return pop[i]->GetSymbiont().GetIntVal();
      }
  emp::DataMonitor<double, emp::data::Histogram>& GetSymIntValDataNode() {
    if (!data_node_symintval) {
      data_node_symintval.New();
      OnUpdate(
	       [this](size_t){
		 data_node_symintval->Reset();
		 for (size_t i = 0; i< pop.size(); i++) {
                    CHANGED: Check if the organism has a symbiont before adding the data
		   if (IsOccupied(i) && pop[i]->HasSym()) data_node_symintval->AddDatum(CalcSymIntVal(i));
		 }
	       }
	       );
    }
    return *data_node_symintval;
  }
  
  */
  void Update(size_t new_resources=10) {
    emp::World<Host>::Update();

    emp::vector<size_t> schedule = emp::GetPermutation(random, GetSize());
    
    double resource_portion = new_resources/GetNumOrgs();
    
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
        for (int j = 0; j < pop[i]->GetSymbionts().size(); j++) {
          Symbiont parent = pop[i]->GetSymbionts()[j];
          for(int k = 0; k < parent.GetReproRate(); k++) {
            Symbiont * offspring = new Symbiont(parent.GetStrainType(), parent.GetTargetType(), 10.0, 0.0, parent.GetReproRate(), -1);
            pop[i]->AddReproSymbionts(offspring);
          }
          pop[i]->GetSymbionts()[j].IncrementBurstTimer();
        }

        //check burst for every symbiont
        for(int j = 0; j < pop[i]->GetSymbionts().size(); j++) {
          if(pop[i]->GetSymbionts()[j].CheckBurst()) {
            //syms and repro_sym get ready
            for (int symNum = 0; symNum < pop[i]->GetSymbionts().size(); symNum++) {
              Symbiont refreshedSym = pop[i]->GetSymbionts()[symNum];
              refreshedSym.ResetSymbiont();
              pop[i]->GetReproSymbionts().push_back(refreshedSym);
            }
            //now burst
            for (int burstCount = 0; burstCount < pop[i]->GetReproSymbionts().size(); burstCount++) {
              Symbiont burstSym = pop[i]->GetReproSymbionts()[burstCount];
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


