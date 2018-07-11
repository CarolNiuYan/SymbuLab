// This is the main function for the NATIVE version of this project.

#include <iostream>
#include "../SymWorld.h"
#include "source/config/ArgManager.h"

using namespace std;

EMP_BUILD_CONFIG( SymConfigBase,
                 VALUE(SEED, double, 10, "What value should the random seed be?"),
                 VALUE(MUTATION_RATE, double, 0.002, "Standard deviation of the distribution to mutate by"),
                 VALUE(SYNERGY, double, 5, "Amount symbiont's returned resources should be multiplied by"),
                 VALUE(VERTICAL_TRANSMISSION, double, 1, "Value 0 to 1 of probability of symbiont vertically transmitting when host reproduces"),
		  VALUE(HOST_INT, double, 0, "Interaction value from -1 to 1 that hosts should have initially, -2 for random"),
		  VALUE(SYM_INT, double, 0, "Interaction value from -1 to 1 that symbionts should have initially, -2 for random"),
                  VALUE(WORLD_TYPE, int, 1, "Type of the world (1 for Mixed and 2 for Grid"),
                 VALUE(GRID_X, int, 5, "Width of the world"),
                 VALUE(GRID_Y, int, 5, "Height of world"),
                 VALUE(UPDATES, int, 1, "Number of updates to run before quitting"),
                  VALUE(POP_SIZE, int, 400, "Number of initial organisms"),
                  VALUE(MOI, double, 1.0, "Ratio of symbionts to hosts"),

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
    //double POP_SIZE = config.GRID_X() * config.GRID_Y();
    double POP_SIZE = config.POP_SIZE();
    double MOI = config.MOI();

    emp::Random random(config.SEED());
        
    SymWorld world(random);
    //world.SetPopStruct_Grid(config.GRID_X(), config.GRID_Y();
    //world.SetPopStruct_Mixed(false);
    world.SetPopStruct_Grow();
    //Set up files
    world.SetupPopulationFile().SetTimingRepeat(10);
    world.SetupResultFile("Result_"+to_string(config.SEED())+"_"+to_string(config.MOI())+".data").SetTimingRepeat(1);

    //inject organisms
    for (size_t i = 0; i < POP_SIZE; i++){
      Host *new_org;
      new_org = new Host("E");
      world.Inject(*new_org);
    }

    //inject symbionts
    for(size_t i = 0; i < POP_SIZE * MOI; i++) {
      world.InjectSymbiont();
    }

    //Loop through updates      
    for (int i = 0; i < numupdates; i++) {
      cout << i << endl;
      world.Update();
    }

}
