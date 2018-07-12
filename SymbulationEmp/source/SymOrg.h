#include "source/tools/Random.h"
#include "source/tools/string_utils.h"
#include <set>
#include <iomanip> // setprecision
#include <sstream> // stringstream


class Symbiont {
 private:
  std::string strain;
  std::string target;
  double burst_value;
  double burst_timer;
  double repro_rate;
  double interaction_val; //should always be -1

 public:
 Symbiont(std::string _strain, std::string _target,double _burstValue = 10.0, double _burstTimer=0.0, double _reproRate=1.0, double _intval= -1.0)
   : strain(_strain), target(_target), burst_value(_burstValue), burst_timer(_burstTimer), repro_rate(_reproRate), interaction_val(_intval){;}
  Symbiont(const Symbiont &) = default;
  Symbiont(Symbiont &&) = default;
  

  Symbiont & operator=(const Symbiont &) = default;
  Symbiont & operator=(Symbiont &&) = default;

  std::string GetStrainType() const {return strain;}
  std::string GetTargetType() const {return target;}
  double GetBurstTimer() const {return burst_timer;}
  double GetBurstValue() const {return burst_value;}
  double GetReproRate() const {return repro_rate;}
  double GetIntVal() const {return interaction_val;}

  void SetIntVal(double _in) { interaction_val = _in;}
  void SetBurstTimer(double _in) {burst_timer = _in;}
  void SetReproRate(double _in) {repro_rate = _in;}
    
  void ResetSymbiont() {
    SetBurstTimer(0);
  }

  void IncrementBurstTimer() {
    burst_timer++;
  }
  
  bool CheckBurst() {
    return burst_timer >= burst_value;
  }
};

class Host {
 private:
  std::string name;
  emp::vector<Symbiont> syms;
  emp::vector<Symbiont> repro_syms; //place holder for newly built symbionts after infected
  double resources;

 public:
 Host(std::string _name, emp::vector<Symbiont> _syms = {}, emp::vector<Symbiont> _reproSyms = {}, double _resources = 0.0) : name(_name), syms(_syms), repro_syms(_reproSyms), resources(_resources){ ; }
  Host(const Host &) = default;
  Host(Host &&) = default;

  Host & operator=(const Host &) = default;
  Host & operator=(Host &&) = default;
  bool operator==(const Host &other) const { return (this == &other);}
  bool operator!=(const Host &other) const {return !(*this == other);}

  std::string GetName() {return name;}
  emp::vector<Symbiont>* GetSymbionts() { return &syms;}
  emp::vector<Symbiont>* GetReproSymbionts() { return &repro_syms;}  
  double GetResources() { return resources;}

  
  void SetSymbionts(emp::vector<Symbiont> _in) {syms = _in;}
  void SetResources(double _in) {resources = _in;}
  void SubtractResources(double _in){resources -= _in;}
  
  void AddResources(double _in) {resources += _in;}
  void AddSymbionts(Symbiont _in) {syms.push_back(_in);}
  void AddReproSymbionts(Symbiont * _in) {repro_syms.push_back(*_in);}

  
  bool HasSym() {
    if (syms.size() <= 0) { 
      return false;
    } else {
      return true;
    } 	
  }

  size_t NumSym() {
    return syms.size() + repro_syms.size();
  }


  //check whether host will get resources and update resource status
  void DistribResources(double resources) { 
    //If host is infected, stop taking resources
    if(this->HasSym()) {return;}
    //otherwise, host get all resources
    this->AddResources(resources);

  }

  void Process(emp::Random &random, double resources) {
    DistribResources(resources); 
  }
};


