#include "std_include.h" // std library includes, definition of scalar, etc.. has a "using namespace std" in it, because I'm lazy

//we'll use TCLAP as our command line parser
#include <tclap/CmdLine.h>
#include "cuda_profiler_api.h"

#include "functions.h"
#include "gpuarray.h"
#include "periodicBoundaryConditions.h"
#include "simulation.h"
#include "simpleModel.h"
#include "voronoiVicsek.h"
#include "baseUpdater.h"
#include "energyMinimizerFIRE.h"
#include "velocityVerlet.h"
#include "noseHooverNVT.h"
#include "noiseSource.h"
#include "harmonicRepulsion.h"
#include "lennardJones6_12.h"
#include "indexer.h"
#include "hyperrectangularCellList.h"
#include "neighborList.h"
#include "poissonDiskSampling.h"
#include "sphericalVoronoi.h"

using namespace std;
using namespace TCLAP;

//!What, after all, *is* the volume of a d-dimensional sphere?
scalar sphereVolume(scalar radius, int dimension)
    {
    if(dimension == 1)
        return 2*radius;
    else
        if(dimension == 2)
            return PI*radius*radius;
        else
            return (2.*PI*radius*radius)/((scalar) dimension)*sphereVolume(radius,dimension-2);
    };

/*!
This file runs some dynamics on particles interacting according to some
potential... when this repository is ever meant to be used this all should be
updated.
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
    ValueArg<int> maxIterationsSwitchArg("i","iterations","number of timestep iterations",false,100,"int",cmd);
    ValueArg<scalar> lengthSwitchArg("l","sideLength","size of simulation domain",false,10.0,"double",cmd);
    ValueArg<scalar> temperatureSwitchArg("t","temperature","temperature of simulation",false,.001,"double",cmd);

    //allow setting of system size by either volume fraction or density (assuming N has been set)
    scalar phiDest = 1.90225*exp(-(scalar)DIMENSION / 2.51907);
    ValueArg<scalar> phiSwitchArg("p","phi","volume fraction",false,phiDest,"double",cmd);
    ValueArg<scalar> rhoSwitchArg("r","rho","density",false,-1.0,"double",cmd);
    //parse the arguments
    cmd.parse( argc, argv );

    int programSwitch = programSwitchArg.getValue();
    int N = nSwitchArg.getValue();
    int maximumIterations = maxIterationsSwitchArg.getValue();
    scalar L = lengthSwitchArg.getValue();
    scalar Temperature = temperatureSwitchArg.getValue();
    scalar phi = phiSwitchArg.getValue();
    scalar rho = rhoSwitchArg.getValue();

    int gpuSwitch = gpuSwitchArg.getValue();
    bool GPU = false;
    if(gpuSwitch >=0)
        GPU = chooseGPU(gpuSwitch);

    if(phi >0)
        {
        L = pow(N*sphereVolume(.5,DIMENSION) / phi,(1.0/(scalar) DIMENSION));
        rho = N/pow(L,(scalar)DIMENSION);
        }
    else
        phi = N*sphereVolume(.5,DIMENSION) / pow(L,(scalar)DIMENSION);

    if(rho >0)
        {
        L = pow(((scalar)N/rho),(1.0/(scalar) DIMENSION));
        phi = rho * sphereVolume(.5,DIMENSION);
        }
    else
        rho = N/pow(L,(scalar)DIMENSION);


    int dim =DIMENSION;
    cout << "running a simulation in "<<dim << " dimensions with box sizes " << L << endl;
    cout << "density = " << rho << "\tvolume fracation = "<<phi<<endl;
    noiseSource noise(true);
    shared_ptr<simpleModel> Configuration = make_shared<sphericalVoronoi>(N,noise);
    shared_ptr<voronoiVicsek> vicsek = make_shared<voronoiVicsek>();
    vicsek->setEta(0.2);
    vicsek->setV0(0.5);
    vicsek->setDeltaT(.1);

    shared_ptr<Simulation> sim = make_shared<Simulation>();
    sim->setConfiguration(Configuration);
    sim->addUpdater(vicsek,Configuration);

    /*
    //after the simulation box has been set, we can set particle positions...do so via poisson disk sampling?
    vector<dVec> poissonPoints;
    scalar diameter = .75;
    clock_t tt1=clock();
    int loopCount = 0;
    while(poissonPoints.size() != N)
        {
        poissonDiskSampling(N,diameter,poissonPoints,noise,PBC);
        loopCount +=1;
         diameter *= 0.95;
        }
    clock_t tt2=clock();
    scalar seedingTimeTaken = (tt2-tt1)/(scalar)CLOCKS_PER_SEC;
    cout << "disk sampling took "<< loopCount << " diameter attempts and took " << seedingTimeTaken << " total seconds" <<endl;

    Configuration->setParticlePositions(poissonPoints);
    */

     //monodisperse harmonic spheres
    //shared_ptr<harmonicRepulsion> softSpheres = make_shared<harmonicRepulsion>();
    //softSpheres->setMonodisperse();
    //softSpheres->setNeighborList(neighList);
    //vector<scalar> stiffnessParameters(1,1.0);
    //softSpheres->setForceParameters(stiffnessParameters);
    //sim->addForce(softSpheres,Configuration);
    //cout << "simulation set-up finished" << endl;cout.flush();

    //shared_ptr<noseHooverNVT> nvt = make_shared<noseHooverNVT>(Configuration,Temperature);
    //nvt->setDeltaT(1e-2);
    //sim->addUpdater(nvt,Configuration);

    if(gpuSwitch >=0)
        {
        sim->setCPUOperation(false);
//        Configuration->setGPU();
//        softSpheres->setGPU();
//        fire->setGPU();
//        neighList->setGPU();
        };
for (int ii = 0; ii < maximumIterations; ++ii) sim->performTimestep();
//neighList->nlistTuner->printTimingData();


//
//The end of the tclap try
//
    } catch (ArgException &e)  // catch any exceptions
    { cerr << "error: " << e.error() << " for arg " << e.argId() << endl; }
    return 0;
};
