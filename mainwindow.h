#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include <QMainWindow>
#include "functions.h"
#include "noiseSource.h"
#include "indexer.h"
#include "sphericalVoronoi.h"
#include "sphericalVertexModel.h"
#include "sphericalVectorialVicsek.h"
#include "sphericalSelfPropelledParticle.h"
#include "noseHooverNVT.h"
#include "energyMinimizerFIRE.h"
#include "brownianDynamics.h"
#include "simulation.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void simulationInitialize();
    void hideControls();
    void showControls();

private slots:
    //parameter setting

    void on_initializeButton_released();
    //simulation controls
    void on_addIterationsButton_released();

    void on_drawStuffButton_released();

    void on_xRotSlider_valueChanged(int value);

    void on_zRotSlider_valueChanged(int value);

    void on_zoomSlider_valueChanged(int value);

    void on_actionReset_the_system_triggered();

    void on_reprodicbleRNGBox_stateChanged(int arg1);

    void on_saveFileNowButton_released();

    void on_computeEnergyButton_released();

    void on_boxRadius_textEdited(const QString &arg1);

    void on_boxDensity_textEdited(const QString &arg1);

    void on_resetSystemButton_released();

    void on_computeEnergyButton_2_released();

    void on_cancelEvolutionParametersButton_pressed();

    void on_setParametersButton_released();

    void on_boxNTotalSize_textChanged(const QString &arg1);

    void on_forbidNeighborExchanges_released();

    void on_BDCheckBox_released();

    void on_FIRECheckBox_clicked();

    void on_cancelFIREButton_released();

    void on_switchUpdaterButton_released();

    void on_setFIREButton_released();

    void on_SPPCheckBox_clicked();

    void on_setSppParametersButton_clicked();

    void on_cancelSppParametersButton_clicked();

private:
    Ui::MainWindow *ui;

public:
    bool GPU = false;
    bool reproducible = true;
    int maximumIterations=0;

    int N=100;
    scalar v0 = 0.1;
    scalar eta = 0.1;
    scalar dt = 0.001;

    scalar radius = 1.0;
    scalar density = 0.0623;

    int zoom = 1;
    vector<scalar3> spherePositions;
    vector<scalar> sphereRadii;

    vector<QString> computationalNames;

    noiseSource noise;

    shared_ptr<sphericalModel> Configuration;
    shared_ptr<Simulation> sim;
    shared_ptr<noseHooverNVT> NVT;
    shared_ptr<brownianDynamics> BD;
    shared_ptr<vectorialVicsek> vicsek;
    shared_ptr<energyMinimizerFIRE> fire;
    shared_ptr<sphericalSelfPropelledParticle> spp;

};

#endif // MAINWINDOW_H
