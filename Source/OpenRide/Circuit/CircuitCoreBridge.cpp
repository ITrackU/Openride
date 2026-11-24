// UnrealBuildTool only compiles files inside the module folder, but the circuit math
// lives in the engine-independent core/ folder so it can be unit-tested on the Pi.
// Including the .cpp here compiles that single source of truth into the game module
// without duplicating it.
#include "../../../core/circuit/StadiumCircuit.cpp"
