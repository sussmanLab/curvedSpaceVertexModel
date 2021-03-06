#include "std_include.h" // std library includes, definition of scalar, etc.. has a "using namespace std" in it, because I'm lazy

//we'll use TCLAP as our command line parser
#include <tclap/CmdLine.h>
#include "cuda_profiler_api.h"

#include "functions.h"
#include "profiler.h"
#include "gpuarray.h"
#include "periodicBoundaryConditions.h"
#include "simulation.h"
#include "simpleModel.h"
#include "sphericalVertexModel.h"
#include "sphericalVectorialVicsek.h"
#include "baseUpdater.h"
#include "energyMinimizerFIRE.h"
#include "velocityVerlet.h"
#include "noseHooverNVT.h"
#include "brownianDynamics.h"
#include "noiseSource.h"
#include "harmonicRepulsion.h"
#include "lennardJones6_12.h"
#include "indexer.h"
#include "hyperrectangularCellList.h"
#include "neighborList.h"
#include "poissonDiskSampling.h"
#include "sphericalVoronoi.h"
#include "vectorValueNetCDF.h"
#include "simpleUtilities.h"
#include "analysisPackage.h"

using namespace std;
using namespace TCLAP;


/*!
core of spherical vertexmodel
*/
int main(int argc, char*argv[])
{
    // wrap tclap in a try block
    try
    {
    //First, we set up a basic command line parser...
    //      cmd("command description message", delimiter, version string)
    CmdLine cmd("basic testing of dDimSim", ' ', "V0.1");

    //define the various command line strings that can be passed in...
    //ValueArg<T> variableName("shortflag","longFlag","description",required or not, default value,"value type",CmdLine object to add to
    ValueArg<int> programSwitchArg("z","programSwitch","an integer controlling program branch",false,0,"int",cmd);
    ValueArg<int> gpuSwitchArg("g","USEGPU","an integer controlling which gpu to use... g < 0 uses the cpu",false,-1,"int",cmd);
    ValueArg<int> nSwitchArg("n","Number","number of particles in the simulation",false,100,"int",cmd);
    ValueArg<int> maxIterationsSwitchArg("i","iterations","number of timestep iterations",false,0,"int",cmd);
    ValueArg<int> fileIdxSwitch("f","file","file Index",false,-1,"int",cmd);
    ValueArg<scalar> lengthSwitchArg("l","sideLength","size of simulation domain",false,10.0,"double",cmd);
    ValueArg<scalar> krSwitchArg("k","springRatio","kA divided by kP",false,1.0,"double",cmd);
    ValueArg<scalar> temperatureSwitchArg("t","temperature","temperature of simulation",false,.001,"double",cmd);

    //allow setting of system size by either volume fraction or density (assuming N has been set)
    ValueArg<scalar> p0SwitchArg("p","p0","preferred perimeter",false,3.78,"double",cmd);
    ValueArg<scalar> a0SwitchArg("a","a0","preferred area per cell",false,1.0,"double",cmd);
    ValueArg<scalar> rhoSwitchArg("r","rho","density",false,-1.0,"double",cmd);
    ValueArg<scalar> dtSwitchArg("e","dt","timestep",false,0.001,"double",cmd);
    ValueArg<scalar> v0SwitchArg("v","v0","v0",false,0.5,"double",cmd);
    //parse the arguments
    cmd.parse( argc, argv );

    int programSwitch = programSwitchArg.getValue();
    int fIdx = fileIdxSwitch.getValue();
    int N = nSwitchArg.getValue();
    int maximumIterations = maxIterationsSwitchArg.getValue();
    scalar L = lengthSwitchArg.getValue();
    scalar Temperature = temperatureSwitchArg.getValue();
    scalar rho = rhoSwitchArg.getValue();
    scalar dt = dtSwitchArg.getValue();
    scalar v0 = v0SwitchArg.getValue();
    scalar p0 = p0SwitchArg.getValue();
    scalar a0 = a0SwitchArg.getValue();
    scalar kr = krSwitchArg.getValue();

    int gpuSwitch = gpuSwitchArg.getValue();
    bool GPU = false;
    if(gpuSwitch >=0)
        GPU = chooseGPU(gpuSwitch);

    int dim =DIMENSION;
    bool reproducible = fIdx <=0 ? true : false;
    noiseSource noise(reproducible);
    shared_ptr<sphericalVertexModel> Configuration = make_shared<sphericalVertexModel>(N,noise,a0,p0,GPU,!GPU);
    Configuration->setScalarModelParameter(kr);

    shared_ptr<brownianDynamics> BD = make_shared<brownianDynamics>(reproducible);
    shared_ptr<Simulation> sim = make_shared<Simulation>();
    sim->setConfiguration(Configuration);
    sim->addUpdater(BD,Configuration);
    sim->setIntegrationTimestep(dt);

    if(gpuSwitch >=0)
        {
        sim->setCPUOperation(false);
        };
    sim->setReproducible(reproducible);

    int stepsPerTau = floor(1./dt);
    //initialize
    BD->setT(0);
    Configuration->setPreferredParameters(1.0,1.0);
    profiler stabProf("stabilization");
    cout << "stabilization..." << endl;
    for (int ii = 0; ii < maximumIterations; ++ii)
        {
        stabProf.start();
        sim->performTimestep();
        stabProf.end();
        }
    BD->setT(Temperature);
    Configuration->setPreferredParameters(a0,p0);
    cout << "initialization..." << endl;
    profiler initProf("initialization ");
    for (int ii = 0; ii < maximumIterations; ++ii)
        {
        initProf.start();
        sim->performTimestep();
        initProf.end();
        }

    dVec meanForce;
    Configuration->getMeanForce(meanForce);
    printf("mean force: %g %g %g\n",meanForce[0],meanForce[1],meanForce[2]);
    Configuration->geoProf.setName("geometry");
    Configuration->forceProf.setName("force");
    Configuration->moveProf.setName("movement and topology");
    Configuration->geoProf.print();
    Configuration->forceProf.print();
    Configuration->moveProf.print();
    BD->updateProfiler.print();
    stabProf.print();
    initProf.print();
//
//The end of the tclap try
//
    } catch (ArgException &e)  // catch any exceptions
    { cerr << "error: " << e.error() << " for arg " << e.argId() << endl; }
    return 0;
};
