#ifndef HEAT_H
#define HEAT_H

double ThermalVaporFlow(long i, TlinkedNode *myLink, int myProcess);
double IsothermalVaporConductivity(long i, double h, double myT);
double SoilRelativeHumidity(double h, double myT);
double SoilHeatCapacity(long i, double h, double T);
double SoilHeatConductivity(long i, double T, double h);
double VaporFromPsiTemp(double h, double T);
double VaporThetaV(double h, double T, long i);
void restoreHeat();
void initializeBalanceHeat();
void updateBalanceHeatWholePeriod();
void saveWaterFluxes();
void initializeHeatFluxes();
void saveHeatFlux(TlinkedNode* myLink, int fluxType, double myValue);
float readHeatFlux(TlinkedNode* myLink, int fluxType);
bool HeatComputation(double timeStep, double timeStepWater);

#endif
