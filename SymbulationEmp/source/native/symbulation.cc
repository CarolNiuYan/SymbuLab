// This is the main function for the NATIVE version of this project.

#include <iostream>
#include "../SymWorld.h"
#include "source/config/ArgManager.h"

using namespace std;

EMP_BUILD_CONFIG( SymConfigBase,
                 VALUE(SEED, double, 20, "What value should the random seed be?"),
                   VALUE(WORLD_TYPE, int, 1, "Type of the world (1 for Mixed and 2 for Grid"),
                   VALUE(UPDATES, int, 10, "Number of updates to run before quitting"),
                  VALUE(ECOLI_NUM, size_t, 200, "Initial number of E.coli bacteria"),
                  VALUE(SHIGELLA_NUM, size_t, 200, "Initial number of shigella bacteria"),
                  VALUE(MOI_E, double, 1.0, "Initial MOI of E.coli Virus"),
                  VALUE(MOI_S, double, 1.0, "Initial MOI of shigella Virus"),
                  VALUE(BURST_VAL, double, 10, "Burst Timer"),
                  VALUE(REPRO_RATE, double, 1.0, "Rate of reproduction for symbiont"),
                 )
	
int main(int argc, char * argv[])
{    
    SymConfigBase config;
    
    config.Read("SymSettings.cfg");

    auto args = emp::cl::ArgManager(argc, argv);
    if (args.ProcessConfigOptions(config, std::cout, "SymSettings.cfg") == false) {
	cout << "There was a problem in processing the options file." << endl;
	exit(0);
      }
    if (args.TestUnknown() == false) exit(0); //Leftover args no good

    double numupdates = config.UPDATES();
    size_t ECOLI_NUM = config.ECOLI_NUM();
    size_t SHIGELLA_NUM = config.SHIGELLA_NUM();
    double MOI_E = config.MOI_E();
    double MOI_S = config.MOI_S();
    double POP_SIZE = ECOLI_NUM + SHIGELLA_NUM;
    double BURSTVAL = config.BURST_VAL();
    double REPRO_RATE = config.REPRO_RATE();
    double resource = (ECOLI_NUM + SHIGELLA_NUM)*28;
    
    emp::Random random(config.SEED());
        
    SymWorld world(random);
    world.SetPopStruct_Grow();
    
    //Set up files
    world.SetupPopulationFile().SetTimingRepeat(10);
    world.SetupResultFile("Result_"+to_string(config.SEED())+"_Ecoli_"+to_string(ECOLI_NUM)+ "_Shi_" +to_string(MOI_E)+".data").SetTimingRepeat(1);

    //inject organisms
    for (size_t i = 0; i < ECOLI_NUM; i++){
      Host *new_org;
      new_org = new Host("Ecoli");
      world.Inject(*new_org);
    }
    for (size_t i = 0; i < SHIGELLA_NUM; i++){
      Host *new_org;
      new_org = new Host("Shigella");
      world.Inject(*new_org);
    }

    
    //inject symbionts
    for(size_t i = 0; i < POP_SIZE * MOI_E; i++) {
      world.InjectSymbiont("Ecoli", BURSTVAL, REPRO_RATE);
    }
    for(size_t i = 0; i < POP_SIZE * MOI_S; i++) {
      world.InjectSymbiont("Shigella", BURSTVAL, REPRO_RATE);
    }

    //Loop through updates      
    for (int i = 0; i < numupdates; i++) {
      cout << i << endl;
      world.Update();
    }

}
