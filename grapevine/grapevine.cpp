#include <stdio.h>
#include <math.h>
#include <malloc.h>
#include "crit3dDate.h"
#include "commonConstants.h"
#include "furtherMathFunctions.h"
#include "physics.h"
#include "gammaFunction.h"
#include "grapevine.h"

Vine3D_Grapevine::Vine3D_Grapevine()
{
}


bool Vine3D_Grapevine::compute(bool computeDaily, int secondsPerStep, TvineField* vineField, double chlorophyll)
{
    simulationStepInSeconds = (double)(secondsPerStep) ;
    isAmphystomatic = true ;
    myLeafWidth = 0.2;       // [m]
    // Stomatal conductance Adjust stom conductance-photosynth ratio for soil water (Pa)
    alphaLeuning = vineField->cultivar->parameterWangLeuning.alpha;
    getFixSimulationParameters();
    initializeWaterStress(vineField->cultivar);

    if (chlorophyll == NODATA) chlorophyllContent = CHLDEFAULT;
    else chlorophyllContent = chlorophyll;

    this->statePlant.stateGrowth.meanTemperatureLastMonth = meanLastMonthTemperature(this->statePlant.stateGrowth.meanTemperatureLastMonth);
    myThermalUnit = minValue(maxValue(0.0 ,(meanDailyTemperature - parameterBindiMigliettaFix.baseTemperature)),
                             parameterBindiMigliettaFix.tempMaxThreshold);

    bool isVegetativeSeason;
    this->computePhenology(computeDaily, &isVegetativeSeason, vineField->cultivar);

    // vine not initialized ->Fallow
    if (statePlant.statePheno.stage == NOT_INITIALIZED_VINE)
    {
        double fallowLAI = 3.0;
        double sensitivityToVPD = 1000;
        fallowTranspiration(fallowLAI, sensitivityToVPD);
        return(true);
    }

    //PHENOLOGY
    this->statePlant.stateGrowth.tartaricAcid =  Vine3D_Grapevine::getTartaricAcid();
    Vine3D_Grapevine::getPotentialBrix();

    if (isVegetativeSeason)
    {
        if (computeDaily)
        {
            Vine3D_Grapevine::getLAIVine(vineField);
        }
        //Vine3D_Grapevine::photosynthesisRadiationUseEfficiency(); // da usare solo quando non funziona Farquhar
        Vine3D_Grapevine::photosynthesisAndTranspiration(vineField->cultivar);

        // check fruit set
        if(this->statePlant.statePheno.daysAfterBloom >= 5)
        {
            if ((this->statePlant.statePheno.stage <= physiologicalMaturity)&& (this->statePlant.stateGrowth.isHarvested != 1))
            {
                 /*this->statePlant.stateGrowth.fruitBiomass = this->statePlant.stateGrowth.cumulatedBiomass *
                    (vineField->cultivar->parameterBindiMiglietta.fruitBiomassOffset + vineField->cultivar->parameterBindiMiglietta.fruitBiomassSlope * minValue(80,(this->statePlant.statePheno.daysAfterBloom - 5)));
                 */
                double denominator, numerator , cumulatedFruitBiomass, incrementalRatio;
                incrementalRatio = this->statePlant.stateGrowth.fruitBiomassIndex * vineField->shootsPerPlant/11.0;
                numerator = (vineField->cultivar->parameterBindiMiglietta.fruitBiomassOffset + incrementalRatio * minValue(80,(this->statePlant.statePheno.daysAfterBloom - 5)));
                denominator = (vineField->cultivar->parameterBindiMiglietta.fruitBiomassOffset + incrementalRatio * minValue(80,(this->statePlant.statePheno.daysAfterBloom-1 - 5)));
                cumulatedFruitBiomass = deltaTime.netAssimilation * numerator;
                if (computeDaily)
                {
                    this->statePlant.stateGrowth.fruitBiomass *= numerator / denominator ;
                }
                this->statePlant.stateGrowth.fruitBiomass += cumulatedFruitBiomass;
            }

        }
        else
        {
            this->statePlant.stateGrowth.fruitBiomass = 0;
        }
        if((this->statePlant.statePheno.stage >= flowering))
        {
            if (computeDaily)
            {
                this->statePlant.statePheno.daysAfterBloom++ ;
            }
        }
    }
    fruitBiomassRateDecreaseDueToRainfall();
    grassTranspiration(vineField->maxLAIGrass, vineField->cultivar->parameterWangLeuning.sensitivityToVapourPressureDeficit, vineField->plantDensity * parameterBindiMigliettaFix.shadedSurface);

    return(true);

}


void Vine3D_Grapevine::initializeRootProperties (soil::Crit3DSoil* mySoil,
                                                  int nrLayers, float maxRootDepth,
                                                  double* myLayerDepth, double* myLayerThickness,
                                                  int soilLayersWithoutRootUp, int soilLayerWithRoot,
                                                  rootDistribution type, double depthMode, double depthMean)
{
    soilTotalDepth = maxRootDepth;
    soilLayersNr = nrLayers;
    layerRootDensity =  (double *) calloc(soilLayersNr, sizeof(double));

    layerDepth =  (double *) calloc(soilLayersNr, sizeof(double));
    layerThickness =  (double *) calloc(soilLayersNr, sizeof(double));
    for (int i = 0; i <nrLayers; i++)
    {
        layerDepth[i] = myLayerDepth[i];
        layerThickness[i] = myLayerThickness[i];
    }
    layerRootDensity = rootDensity(mySoil, soilLayersNr, soilLayerWithRoot,
               soilLayersWithoutRootUp, type, layerDepth, layerThickness, depthMode, depthMean);

    psiSoilProfile = (double *) calloc(soilLayersNr, sizeof(double));
    soilWaterContentProfile = (double *) calloc(soilLayersNr, sizeof(double));
    soilWaterContentProfileFC = (double *) calloc(soilLayersNr, sizeof(double));
    soilWaterContentProfileWP = (double *) calloc(soilLayersNr, sizeof(double));
    fractionTranspirableSoilWaterProfile = (double *) calloc(soilLayersNr, sizeof(double));
    stressCoefficientProfile = (double *) calloc(soilLayersNr, sizeof(double));
    soilTempProfile = (double *) calloc (soilLayersNr, sizeof(double));
    soilFieldCapacity = (double *) calloc (soilLayersNr, sizeof(double));
    transpirationInstantLayer = (double *) calloc (soilLayersNr, sizeof(double));
    transpirationLayer = (double *) calloc (soilLayersNr, sizeof(double));
    transpirationCumulatedGrass = (double *) calloc (soilLayersNr, sizeof(double));

    for (int i=0 ; i< soilLayersNr ; i++)
    {
        psiSoilProfile[i] = NODATA ;
        soilWaterContentProfile[i]= NODATA ;
        soilWaterContentProfileFC[i]= NODATA;
        soilWaterContentProfileWP[i]= NODATA;
        fractionTranspirableSoilWaterProfile[i]= NODATA;
        soilTempProfile[i] = NODATA ;
        soilFieldCapacity[i] = NODATA;
        stressCoefficientProfile[i] = NODATA;
        transpirationInstantLayer[i] = NODATA;
        transpirationLayer[i] = NODATA;
        transpirationCumulatedGrass[i] = NODATA;
    }
}

void Vine3D_Grapevine::setDate (Crit3DTime myTime)
{
    myDoy = getDoyFromDate(myTime.date);
    myYear = myTime.date.year;
    myHour = myTime.getHour();
}


bool Vine3D_Grapevine::setWeather(double meanDailyTemp, double temp, double irradiance , double prec , double relativeHumidity , double windSpeed, double atmospheriCP_AIRressure)
{
    bool isReadingOK = false ;
    myIrradiance = irradiance ;
    myInstantTemp = temp ;
    myPrec = prec ;
    myRelativeHumidity = relativeHumidity ;
    myWindSpeed = windSpeed ;
    myAtmosphericPressure = atmospheriCP_AIRressure ;
    meanDailyTemperature = meanDailyTemp;
    double deltaRelHum = maxValue(100.0 - myRelativeHumidity, 0.01);
    vaporPressureDeficit = 0.01 * deltaRelHum * 613.75 * exp(17.502 * myInstantTemp / (240.97 + myInstantTemp));
    //globalRadiation = globRad;
    if ((prec != NODATA) && (temp != NODATA) && (windSpeed != NODATA) && (irradiance != NODATA) && (relativeHumidity != NODATA) && (atmospheriCP_AIRressure != NODATA)) isReadingOK = true ;
    return isReadingOK ;
}

bool Vine3D_Grapevine::setDerivedVariables ( double diffuseIrradiance, double directIrradiance, double transmissivity, double sunElevation, float clearSkyTransmissivity, double etp)
{
    bool isReadingOK = false;
    myDiffuseIrradiance = diffuseIrradiance ;
    myDirectIrradiance = directIrradiance ;
    //myLongWaveIrradiance = longWaveIrradiance ;
    myCloudiness = minValue(1,maxValue(0,1. - transmissivity/clearSkyTransmissivity)) ;
    //myAirVapourPressure = airVapourPressure ;
    mySunElevation =  sunElevation;
    potentialEvapotranspiration = etp;
    if ((etp != NODATA) && (sunElevation != NODATA)&&(diffuseIrradiance != NODATA) && (directIrradiance != NODATA)  && (transmissivity != NODATA) ) isReadingOK = true ;
    return isReadingOK ;
}


void Vine3D_Grapevine::initializeWaterStress(Tvine* cultivar)
{
    for (int i = 0; i < soilLayersNr; i++)
    {
         stressCoefficientProfile[i] = getWaterStressSawFunction(i, cultivar);
    }
}


bool Vine3D_Grapevine::setSoilProfile(double* myWiltingPoint, double* myFieldCapacity,
                            double* myPsiSoilProfile, double* mySoilWaterContentProfile,
                            double* mySoilWaterContentFC, double* mySoilWaterContentWP)
{
    bool isReadingOK = true ;
    psiSoilAverage = 0.;
    psiFieldCapacityAverage = 0.;
    double logPsiSoilAverage = 0.;
    double logPsiFCAverage = 0.;
    wiltingPoint = myWiltingPoint[int(soilLayersNr/2)] / 101.97;     // conversion from mH2O to MPa
    //layer 0: surface, no soil
    for (int i = 1; i < soilLayersNr; i++)
    {
        if (myPsiSoilProfile[i] == NODATA) isReadingOK = false;
        soilFieldCapacity[i] = myFieldCapacity[i]/101.97; // conversion from mH2O to MPa
        psiSoilProfile[i] = minValue(myPsiSoilProfile[i],-1.)/101.97 ; // conversion from mH2O to MPa
        logPsiSoilAverage += log(-psiSoilProfile[i]) * layerRootDensity[i];
        logPsiFCAverage += log(-soilFieldCapacity[i]) * layerRootDensity[i];
    }
    psiSoilAverage = -exp(logPsiSoilAverage);
    psiFieldCapacityAverage = -exp(logPsiFCAverage);
    //double psiLogPosition = (log(-wiltingPoint) - logPsiSoilAverage) / (log(-wiltingPoint) - logPsiFCAverage);
    fractionTranspirableSoilWaterAverage = 0;


    for (int i = 0; i < soilLayersNr; i++)
    {
        soilWaterContentProfile[i] = mySoilWaterContentProfile[i];
        soilWaterContentProfileFC[i] = mySoilWaterContentFC[i];
        soilWaterContentProfileWP[i] = mySoilWaterContentWP[i];

        fractionTranspirableSoilWaterProfile[i]= maxValue(0,minValue(1,(soilWaterContentProfile[i]-soilWaterContentProfileWP[i])/(soilWaterContentProfileFC[i]-soilWaterContentProfileWP[i])));
        fractionTranspirableSoilWaterAverage += fractionTranspirableSoilWaterProfile[i] * layerRootDensity[i];
        transpirationLayer[i] = 0.;
        transpirationCumulatedGrass[i] = 0. ;
    }
    return isReadingOK ;
}

bool Vine3D_Grapevine::setStatePlant(TstatePlant myStatePlant, bool isVineyard)
{
    statePlant = myStatePlant;
    this->statePlant.outputPlant.transpirationNoStress = 0.;

    if (! isVineyard)
    {
        statePlant.outputPlant.brixBerry = NODATA;
        statePlant.statePheno.stage = NODATA;
        statePlant.stateGrowth.cumulatedBiomass = NODATA;
        statePlant.stateGrowth.leafAreaIndex = NODATA;
        statePlant.stateGrowth.fruitBiomass = NODATA;
    }
    return true;
}

TstatePlant Vine3D_Grapevine::getStatePlant()
{
    return(this->statePlant);
}

ToutputPlant Vine3D_Grapevine::getOutputPlant()
{
    return (this->statePlant.outputPlant);
}


void Vine3D_Grapevine::getFixSimulationParameters()
{
    // Bindi Miglietta parameters
    parameterBindiMigliettaFix.a = -0.28;
    parameterBindiMigliettaFix.b = 0.04;
    parameterBindiMigliettaFix.c = -0.015;
    parameterBindiMigliettaFix.baseTemperature = 10;
    parameterBindiMigliettaFix.tempMaxThreshold = 35;
    parameterBindiMigliettaFix.extinctionCoefficient = 0.5;
    parameterBindiMigliettaFix.shadedSurface = 0.8;
    // Wang Leuning parameters
    parameterWangLeuningFix.minimalStomatalConductance = 0.008;
    parameterWangLeuningFix.optimalTemperatureForPhotosynthesis = 298.15;
    // fenovitis parameters
    parameterPhenoVitisFix.a= 0.005;
    parameterPhenoVitisFix.optimalChillingTemp = 2.8;
    parameterPhenoVitisFix.co2 = -0.015;
    parameterPhenoVitisFix.startingDay = 244;
}


bool Vine3D_Grapevine::initializeStatePlant(int doy, TvineField* vineField)
{
    getFixSimulationParameters();

    int dayAfterStartEOD = (doy - parameterPhenoVitisFix.startingDay+730) % 365;
    int dayAfterEcoDormancy = dayAfterStartEOD - 115;
    long squareDayAfterEOD = dayAfterStartEOD* dayAfterStartEOD;
    long squareDayAfterEcoD = dayAfterEcoDormancy * dayAfterEcoDormancy;

    this->statePlant.stateGrowth.cumulatedBiomass = 0.0;
    this->statePlant.stateGrowth.fruitBiomass = 0.0;
    this->statePlant.stateGrowth.isHarvested = 0;
    this->statePlant.stateGrowth.shootLeafNumber = minShootLeafNr;
    this->statePlant.stateGrowth.meanTemperatureLastMonth = 0.0012*squareDayAfterEOD -0.3603*dayAfterStartEOD + 29.718;
    this->statePlant.stateGrowth.tartaricAcid = NODATA;
    this->statePlant.statePheno.daysAfterBloom = 0.0;
    this->statePlant.stateGrowth.fruitBiomassIndex = vineField->cultivar->parameterBindiMiglietta.fruitBiomassSlope;
    this->statePlant.outputPlant.brixBerry = NODATA;
    this->statePlant.statePheno.degreeDaysFromFirstMarch = NODATA;
    this->statePlant.statePheno.degreeDaysAtFruitSet = NODATA;
    this->statePlant.statePheno.forceStateVegetativeSeason = 0;

    meanDailyTemperature = 0.0016*squareDayAfterEOD -0.412*dayAfterStartEOD + 28.013;

    if (doy > 90 && doy < parameterPhenoVitisFix.startingDay)
        {
        //not initializing period
        this->statePlant.statePheno.chillingState = -200;
        this->statePlant.statePheno.forceStateBudBurst = 0;
        this->statePlant.statePheno.stage = NOT_INITIALIZED_VINE;
        this->statePlant.stateGrowth.leafAreaIndex = NODATA;
        }
   else
        {
        //pheno
        this->statePlant.statePheno.chillingState = maxValue(0, -0.0011*squareDayAfterEOD + 1.1516*dayAfterStartEOD -27.364);

        if (dayAfterStartEOD < 114)
            this->statePlant.statePheno.forceStateBudBurst = 0;
        else
            this->statePlant.statePheno.forceStateBudBurst = maxValue(0,0.0026*squareDayAfterEcoD -0.2164*dayAfterEcoDormancy +4.3808);

        if (doy > parameterPhenoVitisFix.startingDay && doy < 357)
            this->statePlant.statePheno.stage =  endoDormancy;
        else
            this->statePlant.statePheno.stage = ecoDormancy;
        }
    return true;
}


bool Vine3D_Grapevine::fieldBookAction(TvineField* vineField, TfieldOperation action, float quantity)
{
    /*enum TfieldOperation {irrigationOperation, grassSowing, grassRemoving, trimming, leafRemoval,
                          clusterThinning, harvesting, tartaricAnalysis};*/
    if (action == trimming)
    {
        double shootLeafArea;
        this->statePlant.stateGrowth.shootLeafNumber -= quantity;
        shootLeafArea = vineField->cultivar->parameterBindiMiglietta.d * pow(this->statePlant.stateGrowth.shootLeafNumber, vineField->cultivar->parameterBindiMiglietta.f) ;
        // water unstressed leaf area index
        this->statePlant.stateGrowth.leafAreaIndex = shootLeafArea * vineField->shootsPerPlant * vineField->plantDensity / parameterBindiMigliettaFix.shadedSurface;
    }
    if (action == leafRemoval)
    {
        double shootLeafArea;
        this->statePlant.stateGrowth.shootLeafNumber -= quantity;
        shootLeafArea = vineField->cultivar->parameterBindiMiglietta.d * pow(this->statePlant.stateGrowth.shootLeafNumber, vineField->cultivar->parameterBindiMiglietta.f) ;
        // water unstressed leaf area index
        this->statePlant.stateGrowth.leafAreaIndex = shootLeafArea * vineField->shootsPerPlant * vineField->plantDensity / parameterBindiMigliettaFix.shadedSurface;
    }
    if (action == clusterThinning)
    {
        this->statePlant.stateGrowth.fruitBiomass *= 0.01*(100.0 - quantity);
        this->statePlant.stateGrowth.fruitBiomassIndex *= 0.01*(100.0 - quantity*0.6);
    }
    if (action == harvesting)
    {
        this->statePlant.stateGrowth.isHarvested = 1;
    }

    return true;
}


void Vine3D_Grapevine::photosynthesisRadiationUseEfficiency(Tvine* cultivar)
{
    stepPhotosynthesis = (myIrradiance*(simulationStepInSeconds)*1e-6)
    *(1-exp(-parameterBindiMigliettaFix.extinctionCoefficient*statePlant.stateGrowth.leafAreaIndex))*cultivar->parameterBindiMiglietta.radiationUseEfficiency
    *(1-0.0025*pow(myInstantTemp-25.,2));
    stepPhotosynthesis = maxValue(0,stepPhotosynthesis);
    this->statePlant.stateGrowth.cumulatedBiomass += stepPhotosynthesis ;
}


void Vine3D_Grapevine::photosynthesisAndTranspiration(Tvine* cultivar)
{
    Vine3D_Grapevine::weatherVariables();
    Vine3D_Grapevine::radiationAbsorption();
    //Vine3D_Grapevine::leafTemperature();
    Vine3D_Grapevine::aerodynamicalCoupling();
    Vine3D_Grapevine::upscale(cultivar);
    Vine3D_Grapevine::carbonWaterFluxesProfileNoStress(cultivar);
    Vine3D_Grapevine::carbonWaterFluxesProfile(cultivar);
    Vine3D_Grapevine::cumulatedResults();
    Vine3D_Grapevine::getStressCoefficient();

}

double Vine3D_Grapevine::getCO2()
{
    double atmCO2 ; // fitting from data of Mauna Loa,Hawaii
    if (myYear < 1990)
    {
        atmCO2= 280 * exp(0.0014876*(myYear -1840));//exponential change in CO2 concentration (ppm)
    }
    else
    {
        atmCO2= 350 * exp(0.00630*(myYear - 1990));
    }
    atmCO2 += 3*cos(2*PI*myDoy/365.0);		     // to consider the seasonal effects
    return atmCO2*myAtmosphericPressure/1000000  ;   // [Pa] in +- ppm/10
}


double Vine3D_Grapevine::acclimationFunction(double Ha , double Hd, double leafTemp,
                                              double entropicTerm,double optimumTemp)
{
    // taken from Hydrall Model, Magnani UNIBO
    return exp(Ha*(leafTemp - optimumTemp)/(optimumTemp*R_GAS*leafTemp))
            *(1+exp((optimumTemp*entropicTerm-Hd)/(optimumTemp*R_GAS)))
            /(1+exp((leafTemp*entropicTerm-Hd)/(leafTemp*R_GAS))) ;
}


double Vine3D_Grapevine::acclimationFunction2(double preFactor, double expFactor,
                                               double leafTemp, double optimumTemp)
{
    // taken from Hydrall Model, Magnani UNIBO
    return preFactor*exp((expFactor*(leafTemp-optimumTemp)/(optimumTemp*R_GAS*leafTemp)));
}

void Vine3D_Grapevine::weatherVariables()
{
    // taken from Hydrall Model, Magnani UNIBO
    myAirVapourPressure = SaturationVaporPressure(myInstantTemp)*myRelativeHumidity/100.;
    emissivitySky = 1.24*pow((myAirVapourPressure/100.0)/(myInstantTemp+ZEROCELSIUS),(1.0/7.0))*(1 - 0.84*myCloudiness)+ 0.84*myCloudiness;
    myLongWaveIrradiance = pow(myInstantTemp+ZEROCELSIUS,4)* emissivitySky * STEFAN_BOLTZMANN ;
    slopeSatVapPressureVSTemp = 2588464.2/pow(240.97+myInstantTemp,2) * exp(17.502* myInstantTemp /(240.97+ myInstantTemp)) ;
}


void Vine3D_Grapevine::radiationAbsorption()
{
    // taken from Hydrall Model, Magnani UNIBO
    double sineSolarElevation;

    static double   leafAbsorbanceNIR= 0.2;
    static double   hemisphericalIsotropyParameter = 0. ; // in order to change the hemispherical isotropy from -0.4 to 0.6 Wang & Leuning 1998
    static double   clumpingParameter = 1.0 ; // from 0 to 1 <1 for needles
    double  diffuseLightSector1K,diffuseLightSector2K,diffuseLightSector3K ;
    double scatteringCoefPAR, scatteringCoefNIR ;
    double dum[17];
    double  sunlitAbsorbedNIR ,  shadedAbsorbedNIR, sunlitAbsorbedLW , shadedAbsorbedLW;
    double directIncomingPAR, directIncomingNIR , diffuseIncomingPAR , diffuseIncomingNIR,leafAbsorbancePAR;
    double directReflectionCoefficientPAR , directReflectionCoefficientNIR , diffuseReflectionCoefficientPAR , diffuseReflectionCoefficientNIR;
    //projection of the unit leaf area in the direction of the sun's beam, following Sellers 1985 (in Wang & Leuning 1998)


    sineSolarElevation = maxValue(0.0001,sin(mySunElevation*DEG_TO_RAD));
    directLightK = (0.5 - hemisphericalIsotropyParameter*(0.633-1.11*sineSolarElevation) - pow(hemisphericalIsotropyParameter,2)*(0.33-0.579*sineSolarElevation))/ sineSolarElevation;

    /*Extinction coeff for canopy of black leaves, diffuse radiation
    The average extinctio coefficient is computed considering three sky sectors,
    assuming SOC conditions (Goudriaan & van Laar 1994, p 98-99)*/
    diffuseLightSector1K = (0.5 - hemisphericalIsotropyParameter*(0.633-1.11*0.259) - hemisphericalIsotropyParameter*hemisphericalIsotropyParameter*(0.33-0.579*0.259))/0.259;  //projection of unit leaf for first sky sector (0-30 elevation)
    diffuseLightSector2K = (0.5 - hemisphericalIsotropyParameter*(0.633-1.11*0.707) - hemisphericalIsotropyParameter*hemisphericalIsotropyParameter*(0.33-0.579*0.707))/0.707 ; //second sky sector (30-60 elevation)
    diffuseLightSector3K = (0.5 - hemisphericalIsotropyParameter*(0.633-1.11*0.966) - hemisphericalIsotropyParameter*hemisphericalIsotropyParameter*(0.33-0.579*0.966))/ 0.966 ; // third sky sector (60-90 elevation)
    diffuseLightK =- 1.0/statePlant.stateGrowth.leafAreaIndex * log(0.178 * exp(-diffuseLightSector1K*statePlant.stateGrowth.leafAreaIndex) + 0.514 * exp(-diffuseLightSector2K*statePlant.stateGrowth.leafAreaIndex)
            + 0.308 * exp(-diffuseLightSector3K*statePlant.stateGrowth.leafAreaIndex));  //approximation based on relative radiance from 3 sky sectors
    //Include effects of leaf clumping (see Goudriaan & van Laar 1994, p 110)
    directLightK  *= clumpingParameter ;//direct light
    diffuseLightK *= clumpingParameter ;//diffuse light
    if (0.001 < sineSolarElevation)
    {
        //Leaf area index of sunlit (1) and shaded (2) big-leaf
        sunlit.leafAreaIndex = UPSCALINGFUNC(directLightK,statePlant.stateGrowth.leafAreaIndex);
        shaded.leafAreaIndex = statePlant.stateGrowth.leafAreaIndex - sunlit.leafAreaIndex ;
        //Extinction coefficients for direct and diffuse PAR and NIR radiation, scattering leaves
        //Based on approximation by Goudriaan 1977 (in Goudriaan & van Laar 1994)
        leafAbsorbancePAR = 1 - pow(10,-pow(10,0.28 + 0.63*log10(chlorophyllContent*0.85/1000)));//from Agusti et al (1994), Eq. 1, assuming Chl a = 0.85 Chl (a+b)
        scatteringCoefPAR = 1.0 - leafAbsorbancePAR ; //scattering coefficient for PAR
        scatteringCoefNIR = 1.0 - leafAbsorbanceNIR ; //scattering coefficient for NIR
        dum[0]= sqrt(1-scatteringCoefPAR);
        dum[1]= sqrt(1-scatteringCoefNIR);
        diffuseLightKPAR = diffuseLightK * dum[0] ;//extinction coeff of PAR, direct light
        diffuseLightKNIR = diffuseLightK * dum[1]; //extinction coeff of NIR radiation, direct light
        directLightKPAR  = directLightK * dum[0]; //extinction coeff of PAR, diffuse light
        directLightKNIR  = directLightK * dum[1]; //extinction coeff of NIR radiation, diffuse light
        //Canopy+soil reflection coefficients for direct, diffuse PAR, NIR radiation
        dum[2]= (1-dum[0]) / (1+dum[0]);
        dum[3]= (1-dum[1]) / (1+dum[1]);
        dum[4]= 2.0 * directLightK / (directLightK + diffuseLightK) ;
        directReflectionCoefficientNIR = directReflectionCoefficientPAR = dum[4] * dum[2];
        diffuseReflectionCoefficientNIR = diffuseReflectionCoefficientPAR = dum[4] * dum[3];
        //Incoming direct PAR and NIR (W m-2)
        directIncomingNIR = directIncomingPAR = myDirectIrradiance * 0.5 ;
        //Incoming diffuse PAR and NIR (W m-2)
        diffuseIncomingNIR = diffuseIncomingPAR = myDiffuseIrradiance * 0.5 ;
        //Preliminary computations
        dum[5]= diffuseIncomingPAR * (1.0-diffuseReflectionCoefficientPAR) * diffuseLightKPAR ;
        dum[6]= directIncomingPAR * (1.0-directReflectionCoefficientPAR) * directLightKPAR ;
        dum[7]= directIncomingPAR * (1.0-scatteringCoefPAR) * directLightK ;
        dum[8]=  diffuseIncomingNIR * (1.0-diffuseReflectionCoefficientNIR) * diffuseLightKNIR;
        dum[9]= directIncomingNIR * (1.0-directReflectionCoefficientNIR) * directLightKNIR;
        dum[10]= directIncomingNIR * (1.0-scatteringCoefNIR) * directLightKNIR ;
        dum[11]= UPSCALINGFUNC((diffuseLightKPAR+directLightK),statePlant.stateGrowth.leafAreaIndex);
        dum[12]= UPSCALINGFUNC((directLightKPAR+directLightK),statePlant.stateGrowth.leafAreaIndex);
        dum[13]= UPSCALINGFUNC((diffuseLightKPAR+directLightK),statePlant.stateGrowth.leafAreaIndex);
        dum[14]= UPSCALINGFUNC((directLightKNIR+directLightK),statePlant.stateGrowth.leafAreaIndex);
        dum[15]= UPSCALINGFUNC(directLightK,statePlant.stateGrowth.leafAreaIndex) - UPSCALINGFUNC((2.0*directLightK),statePlant.stateGrowth.leafAreaIndex) ;
        // PAR absorbed by sunlit (1) and shaded (2) big-leaf (W m-2) from Wang & Leuning 1998
        sunlit.absorbedPAR = dum[5] * dum[11] + dum[6] * dum[12] + dum[7] * dum[15] ;
        shaded.absorbedPAR = dum[5]*(UPSCALINGFUNC(diffuseLightKPAR,statePlant.stateGrowth.leafAreaIndex)- dum[11]) + dum[6]*(UPSCALINGFUNC(directLightKPAR,statePlant.stateGrowth.leafAreaIndex)- dum[12]) - dum[7] * dum[15];
        // NIR absorbed by sunlit (1) and shaded (2) big-leaf (W m-2) fromWang & Leuning 1998
        sunlitAbsorbedNIR = dum[8]*dum[13]+dum[9]*dum[14]+dum[10]*dum[15];
        shadedAbsorbedNIR = dum[8]*(UPSCALINGFUNC(diffuseLightKNIR,statePlant.stateGrowth.leafAreaIndex)-dum[13])+dum[9]*(UPSCALINGFUNC(directLightKNIR,statePlant.stateGrowth.leafAreaIndex)- dum[14]) - dum[10] * dum[15];
        // Long-wave radiation balance by sunlit (1) and shaded (2) big-leaf (W m-2) from Wang & Leuning 1998
        dum[16]= myLongWaveIrradiance -STEFAN_BOLTZMANN*pow(myInstantTemp+ZEROCELSIUS,4); //negativo
        dum[16] *= diffuseLightK ;
        double emissivityLeaf, emissivitySoil;
        emissivityLeaf = 0.96 ; // supposed constant because variation is very small
        emissivitySoil= 0.94 ;   // supposed constant because variation is very small
        sunlitAbsorbedLW = (dum[16] * UPSCALINGFUNC((directLightK+diffuseLightK),statePlant.stateGrowth.leafAreaIndex))*emissivityLeaf+(1.0-emissivitySoil)*(emissivityLeaf-emissivitySky)* UPSCALINGFUNC((2*diffuseLightK),statePlant.stateGrowth.leafAreaIndex)* UPSCALINGFUNC((directLightK-diffuseLightK),statePlant.stateGrowth.leafAreaIndex);
        shadedAbsorbedLW = dum[16] * UPSCALINGFUNC(diffuseLightK,statePlant.stateGrowth.leafAreaIndex) - sunlitAbsorbedLW ;
        // Isothermal net radiation for sunlit (1) and shaded (2) big-leaf
        sunlit.isothermalNetRadiation= sunlit.absorbedPAR + sunlitAbsorbedNIR + sunlitAbsorbedLW ;
        shaded.isothermalNetRadiation = shaded.absorbedPAR + shadedAbsorbedNIR + shadedAbsorbedLW ;
    }
    else
    {
         sunlit.leafAreaIndex =  0.0 ;
         sunlit.absorbedPAR = 0.0 ;
         sunlitAbsorbedNIR = 0.0 ;
         sunlitAbsorbedLW = 0.0 ;
         sunlit.isothermalNetRadiation =  0.0 ;

         shaded.leafAreaIndex = statePlant.stateGrowth.leafAreaIndex;
         shaded.absorbedPAR = 0.0 ;
         shadedAbsorbedNIR = 0.0 ;
         dum[16]= myLongWaveIrradiance -STEFAN_BOLTZMANN*pow(myInstantTemp + ZEROCELSIUS,4) ;
         dum[16] *= diffuseLightK ;
         shadedAbsorbedLW= dum[16] * (UPSCALINGFUNC(diffuseLightK,statePlant.stateGrowth.leafAreaIndex) - UPSCALINGFUNC(directLightK+diffuseLightK,statePlant.stateGrowth.leafAreaIndex)) ;
         shaded.isothermalNetRadiation = shaded.absorbedPAR + shadedAbsorbedNIR + shadedAbsorbedLW ;
    }
     // Convert absorbed PAR into units of mol m-2 s-1
     sunlit.absorbedPAR *= 4.57E-6 ;
     shaded.absorbedPAR *= 4.57E-6 ;

}

void Vine3D_Grapevine::leafTemperature()
{
    // taken from Hydrall Model, Magnani UNIBO
    double sineSolarElevation;

    static double   hemisphericalIsotropyParameter = 0. ; // in order to change the hemispherical isotropy from -0.4 to 0.6 Wang & Leuning 1998
    static double   clumpingParameter = 1.0 ; // from 0 to 1 <1 for needles
    double  diffuseLightSector1K,diffuseLightSector2K,diffuseLightSector3K ;
    //projection of the unit leaf area in the direction of the sun's beam, following Sellers 1985 (in Wang & Leuning 1998)
    sineSolarElevation = maxValue(0.001,sin(mySunElevation*DEG_TO_RAD));
    if (sineSolarElevation > 0.01){
        directLightK = (0.5 - hemisphericalIsotropyParameter*(0.633-1.11*sineSolarElevation) - pow(hemisphericalIsotropyParameter,2)*(0.33-0.579*sineSolarElevation))/ sineSolarElevation;
        /*Extinction coeff for canopy of black leaves, diffuse radiation
        The average extinctio coefficient is computed considering three sky sectors,
        assuming SOC conditions (Goudriaan & van Laar 1994, p 98-99)*/
        diffuseLightSector1K = (0.5 - hemisphericalIsotropyParameter*(0.633-1.11*0.259) - hemisphericalIsotropyParameter*hemisphericalIsotropyParameter*(0.33-0.579*0.259))/0.259;  //projection of unit leaf for first sky sector (0-30 elevation)
        diffuseLightSector2K = (0.5 - hemisphericalIsotropyParameter*(0.633-1.11*0.707) - hemisphericalIsotropyParameter*hemisphericalIsotropyParameter*(0.33-0.579*0.707))/0.707 ; //second sky sector (30-60 elevation)
        diffuseLightSector3K = (0.5 - hemisphericalIsotropyParameter*(0.633-1.11*0.966) - hemisphericalIsotropyParameter*hemisphericalIsotropyParameter*(0.33-0.579*0.966))/ 0.966 ; // third sky sector (60-90 elevation)
        diffuseLightK =- 1.0/statePlant.stateGrowth.leafAreaIndex * log(0.178 * exp(-diffuseLightSector1K*statePlant.stateGrowth.leafAreaIndex) + 0.514 * exp(-diffuseLightSector2K*statePlant.stateGrowth.leafAreaIndex)
                                                                        + 0.308 * exp(-diffuseLightSector3K*statePlant.stateGrowth.leafAreaIndex));  //approximation based on relative radiance from 3 sky sectors
        //Include effects of leaf clumping (see Goudriaan & van Laar 1994, p 110)
        directLightK  *= clumpingParameter ;//direct light
        diffuseLightK *= clumpingParameter ;//diffuse light


        //double directIncomingNIR,directIncomingPAR,diffuseIncomingNIR,diffuseIncomingPAR;
        //irectIncomingNIR = directIncomingPAR = myDirectIrradiance * 0.5 ;
        //Incoming diffuse PAR and NIR (W m-2)
        //diffuseIncomingNIR = diffuseIncomingPAR = myDiffuseIrradiance * 0.5 ;
        //double sunlitIrradiance,shadedIrradiance;
        double sunlitGlobalRadiation,shadedGlobalRadiation;

        //shadedIrradiance = myDiffuseIrradiance * shaded.leafAreaIndex / statePlant.stateGrowth.leafAreaIndex;
        shadedGlobalRadiation = myDiffuseIrradiance * simulationStepInSeconds ;
        shaded.leafTemperature = myInstantTemp + 1.67*1.0e-6 * shadedGlobalRadiation - 0.25 * vaporPressureDeficit / GAMMA ; // by Stanghellini 1987 phd thesis

        // sunlitIrradiance = myDiffuseIrradiance * sunlit.leafAreaIndex/ statePlant.stateGrowth.leafAreaIndex;
        //sunlitIrradiance = myDirectIrradiance * sunlit.leafAreaIndex/ statePlant.stateGrowth.leafAreaIndex;
        sunlitGlobalRadiation = myIrradiance * simulationStepInSeconds ;
        sunlit.leafTemperature = myInstantTemp + 1.67*1.0e-6 * sunlitGlobalRadiation - 0.25 * vaporPressureDeficit / GAMMA ; // by Stanghellini 1987 phd thesis
    }
    else
    {
        sunlit.leafTemperature = shaded.leafTemperature = myInstantTemp;
    }
    sunlit.leafTemperature += ZEROCELSIUS;
    shaded.leafTemperature += ZEROCELSIUS;
}

void Vine3D_Grapevine::aerodynamicalCoupling()
{
        // taken from Hydrall Model, Magnani UNIBO
        static double A = 0.0067;
        static double BETA = 3.0;
        static double KARM = 0.41;
        double heightReference , roughnessLength,zeroPlaneDisplacement, sensibleHeat, frictionVelocity, windSpeedTopCanopy, leafBoundaryLayerConductance;
        double canopyAerodynamicConductanceToMomentum,aerodynamicConductanceToCO2,dummy,aerodynamicConductanceForHeat,sunlitDeltaTemp,shadedDeltaTemp;
        double windSpeed;
        windSpeed = maxValue(5,myWindSpeed);
        heightReference = myPlantHeight + 5 ; // [m]
        dummy = 0.2 * statePlant.stateGrowth.leafAreaIndex ;
        zeroPlaneDisplacement = minValue(myPlantHeight * (log(1+pow(dummy,0.166)) + 0.03*log(1+pow(dummy,6))), 0.99*myPlantHeight) ;
        if (dummy < 0.2) roughnessLength = 0.01 + 0.28*sqrt(dummy) * myPlantHeight ;
        else roughnessLength = 0.3 * myPlantHeight * (1.0 - zeroPlaneDisplacement/myPlantHeight);
        // Canopy energy balance.
        // Compute iteratively:
        // - leaf temperature (different for sunlit and shaded foliage)
        // - aerodynamic conductance (non-neutral conditions)


        // Initialize sensible heat flux and friction velocity
        sensibleHeat = sunlit.isothermalNetRadiation + shaded.isothermalNetRadiation ;
        frictionVelocity = maxValue(1.0e-4,KARM*windSpeed/log((heightReference-zeroPlaneDisplacement)/roughnessLength));


        short i = 0 ;
        double sensibleHeatOld = -9999;
        double threshold = fabs(sensibleHeat/10000.0);
        while( (20 > i++) && (fabs(sensibleHeat - sensibleHeatOld)> threshold)) {
        // Monin-Obukhov length (m) and nondimensional height
        // Note: imposed a limit to non-dimensional height under stable
        // conditions, corresponding to a value of 0.2 for the generalized
        // stability factor F (=1/FIM/FIH)
            sensibleHeatOld = sensibleHeat ;
            double moninObukhovLength,zeta,deviationFunctionForMomentum,deviationFunctionForHeat,radiativeConductance,totalConductanceToHeatExchange,stomatalConductanceWater ;

            moninObukhovLength = -(pow(frictionVelocity,3))*CP_AIR*myAtmosphericPressure;
            moninObukhovLength /= (R_GAS*(KARM*9.8*sensibleHeat));

            zeta = minValue((heightReference-zeroPlaneDisplacement)/moninObukhovLength,0.25) ;

            if (zeta < 0)
            {
                //Stability function for momentum and heat (-)
                double x,y,stabilityFunctionForMomentum;
                stabilityFunctionForMomentum = pow((1.0-16.0*zeta),-0.25);
                x= 1.0/stabilityFunctionForMomentum;
                y= 1.0/pow(stabilityFunctionForMomentum,2);
                //Deviation function for momentum and heat (-)
                deviationFunctionForMomentum = 2.0*log((1+x)/2.) + log((1+x*x)/2.0)- 2.0*atan(x) + PI/2.0 ;
                deviationFunctionForHeat = 2*log((1+y)/2) ;
            }
            else
            {
                // Stable conditions
                //stabilityFunctionForMomentum = (1+5*zeta);
                // Deviation function for momentum and heat (-)
                deviationFunctionForMomentum = deviationFunctionForHeat = - 5*zeta ;
            }
            //friction velocity
            frictionVelocity = KARM*windSpeed/(log((heightReference-zeroPlaneDisplacement)/roughnessLength) - deviationFunctionForMomentum);
            frictionVelocity = maxValue(frictionVelocity,1.0e-4);

            // Wind speed at canopy top	(m s-1)
            windSpeedTopCanopy = (frictionVelocity/KARM) * log((myPlantHeight - zeroPlaneDisplacement)/roughnessLength);
            windSpeedTopCanopy = maxValue(windSpeedTopCanopy,1.0e-4);

            // Average leaf boundary-layer conductance cumulated over the canopy (m s-1)
            leafBoundaryLayerConductance = A*sqrt(windSpeedTopCanopy/(leafWidth()))*((2.0/BETA)*(1-exp(-BETA/2.0))) * statePlant.stateGrowth.leafAreaIndex;
            //       Total canopy aerodynamic conductance for momentum exchange (s m-1)
            canopyAerodynamicConductanceToMomentum= frictionVelocity / (windSpeed/frictionVelocity + (deviationFunctionForMomentum-deviationFunctionForHeat)/KARM);
            // Aerodynamic conductance for heat exchange (mol m-2 s-1)
            dummy =	(myAtmosphericPressure/R_GAS)/(myInstantTemp + ZEROCELSIUS);// conversion factor m s-1 into mol m-2 s-1
            aerodynamicConductanceForHeat =  ((canopyAerodynamicConductanceToMomentum*leafBoundaryLayerConductance)/(canopyAerodynamicConductanceToMomentum + leafBoundaryLayerConductance)) * dummy ; //whole canopy
            sunlit.aerodynamicConductanceHeatExchange = aerodynamicConductanceForHeat * sunlit.leafAreaIndex/statePlant.stateGrowth.leafAreaIndex ;//sunlit big-leaf
            shaded.aerodynamicConductanceHeatExchange = aerodynamicConductanceForHeat - sunlit.aerodynamicConductanceHeatExchange ; //  shaded big-leaf
            // Canopy radiative conductance (mol m-2 s-1)
            radiativeConductance= 4*(slopeSatVapPressureVSTemp/GAMMA)*(STEFAN_BOLTZMANN/CP_AIR)*pow((myInstantTemp + ZEROCELSIUS),3);
            // Total conductance to heat exchange (mol m-2 s-1)
            totalConductanceToHeatExchange =  aerodynamicConductanceForHeat + radiativeConductance; //whole canopy
            sunlit.totalConductanceHeatExchange = totalConductanceToHeatExchange * sunlit.leafAreaIndex/statePlant.stateGrowth.leafAreaIndex;	//sunlit big-leaf
            shaded.totalConductanceHeatExchange = totalConductanceToHeatExchange - sunlit.totalConductanceHeatExchange;  //shaded big-leaf

            // Temperature of big-leaf (approx. expression)
            if (sunlit.leafAreaIndex > 1.0E-6)
            {
                stomatalConductanceWater= (10.0/sunlit.leafAreaIndex); //dummy stom res for sunlit big-leaf
                //if (sunlit.isothermalNetRadiation > 100) stomatalConductanceWater *= pow(100/sunlit.isothermalNetRadiation,0.5);
                sunlitDeltaTemp = ((stomatalConductanceWater+1.0/sunlit.aerodynamicConductanceHeatExchange)*GAMMA*sunlit.isothermalNetRadiation/CP_AIR - vaporPressureDeficit*(1+0.001*sunlit.isothermalNetRadiation))/sunlit.totalConductanceHeatExchange/(GAMMA*(stomatalConductanceWater+1.0/sunlit.aerodynamicConductanceCO2Exchange) + slopeSatVapPressureVSTemp/sunlit.totalConductanceHeatExchange);
            }
            else
            {
                sunlitDeltaTemp = 0.0;
            }
            sunlitDeltaTemp = 0.0;
            sunlit.leafTemperature = myInstantTemp + sunlitDeltaTemp	+ ZEROCELSIUS ; //sunlit big-leaf
            stomatalConductanceWater = 10.0/shaded.leafAreaIndex ; //dummy stom res for shaded big-leaf
            //if (shaded.isothermalNetRadiation > 100) stomatalConductanceWater *= pow(100/shaded.isothermalNetRadiation,0.5);
            shadedDeltaTemp = ((stomatalConductanceWater + 1.0/shaded.aerodynamicConductanceHeatExchange)*GAMMA*shaded.isothermalNetRadiation/CP_AIR- vaporPressureDeficit*(1+0.001*shaded.isothermalNetRadiation))/shaded.totalConductanceHeatExchange/(GAMMA*(stomatalConductanceWater + 1.0/shaded.aerodynamicConductanceHeatExchange) + slopeSatVapPressureVSTemp/shaded.totalConductanceHeatExchange);
            shadedDeltaTemp = 0.0;
            shaded.leafTemperature = myInstantTemp + shadedDeltaTemp + ZEROCELSIUS;  //shaded big-leaf
            // Sensible heat flux from the whole canopy
            sensibleHeat = CP_AIR * (sunlit.aerodynamicConductanceHeatExchange*sunlitDeltaTemp + shaded.aerodynamicConductanceHeatExchange*shadedDeltaTemp);
        }
        if (isAmphystomatic) aerodynamicConductanceToCO2 = 0.78 * aerodynamicConductanceForHeat; //amphystomatous species. Ratio of diffusivities from Wang & Leuning 1998
        else aerodynamicConductanceToCO2 = 0.78 * (canopyAerodynamicConductanceToMomentum*leafBoundaryLayerConductance)/(leafBoundaryLayerConductance + 2.0*canopyAerodynamicConductanceToMomentum) * dummy; //hypostomatous species

        sunlit.aerodynamicConductanceCO2Exchange = aerodynamicConductanceToCO2 * sunlit.leafAreaIndex/statePlant.stateGrowth.leafAreaIndex ; //sunlit big-leaf
        shaded.aerodynamicConductanceCO2Exchange = aerodynamicConductanceToCO2 * shaded.leafAreaIndex/statePlant.stateGrowth.leafAreaIndex ;  //shaded big-leaf
}

void Vine3D_Grapevine::upscale(Tvine *cultivar)
{
    // taken from Hydrall Model, Magnani UNIBO
    double sineSolarElevation;
    sineSolarElevation = maxValue(0.0001,sin(mySunElevation*DEG_TO_RAD));
    static double BETA = 0.5 ;
    double dum[10],darkRespirationT0;
    double optimalCarboxylationRate,optimalElectronTransportRate ;
    double leafConvexityFactor ;
    //     Preliminary computations
    dum[0]= R_GAS/1000.0 * sunlit.leafTemperature ; //[kJ mol-1]
    dum[1]= R_GAS/1000.0 * shaded.leafTemperature ;
    dum[2]= sunlit.leafTemperature - ZEROCELSIUS ; // [oC]
    dum[3]= shaded.leafTemperature - ZEROCELSIUS ;

    // optimalCarboxylationRate = (nitrogen.interceptLeaf + nitrogen.slopeLeaf * nitrogen.leafNitrogen/specificLeafArea*1000)*1e-6; // carboxylation rate based on nitrogen leaf
    optimalCarboxylationRate = cultivar->parameterWangLeuning.maxCarboxRate * 1.0e-6; // [mol m-2 s-1] from Greer et al. 2011
    darkRespirationT0 = 0.0089 * optimalCarboxylationRate ;
    //   Adjust unit dark respiration rate for temperature (mol m-2 s-1)
    sunlit.darkRespiration = darkRespirationT0 * exp(CRD - HARD/dum[0])* UPSCALINGFUNC((directLightK + diffuseLightKPAR),statePlant.stateGrowth.leafAreaIndex); //sunlit big-leaf
    shaded.darkRespiration = darkRespirationT0 * exp(CRD - HARD/dum[1]); //shaded big-leaf
    shaded.darkRespiration *= (UPSCALINGFUNC(diffuseLightKPAR,statePlant.stateGrowth.leafAreaIndex) - UPSCALINGFUNC((directLightK + diffuseLightKPAR),statePlant.stateGrowth.leafAreaIndex));
    entropicFactorElectronTransporRate = (-0.75*(this->statePlant.stateGrowth.meanTemperatureLastMonth)+660);  // entropy term for J (kJ mol-1 oC-1)
    entropicFactorCarboxyliation = (-1.07*(this->statePlant.stateGrowth.meanTemperatureLastMonth)+668); // entropy term for VCmax (kJ mol-1 oC-1)
    if (sineSolarElevation > 1.0e-3)
    {
        //Stomatal conductance to CO2 in darkness (molCO2 m-2 s-1)
        sunlit.minimalStomatalConductance = parameterWangLeuningFix.minimalStomatalConductance  * UPSCALINGFUNC((directLightK+diffuseLightKPAR),statePlant.stateGrowth.leafAreaIndex)	;
        shaded.minimalStomatalConductance = parameterWangLeuningFix.minimalStomatalConductance  * (UPSCALINGFUNC(diffuseLightKPAR,statePlant.stateGrowth.leafAreaIndex) - UPSCALINGFUNC((directLightK+diffuseLightKPAR),statePlant.stateGrowth.leafAreaIndex));
        // Carboxylation rate
        //sunlit.maximalCarboxylationRate = optimalCarboxylationRate * exp(CVCM - HAVCM/dum[0]); //sunlit big leaf
        //shaded.maximalCarboxylationRate = optimalCarboxylationRate * exp(CVCM - HAVCM/dum[1]); //shaded big leaf
        sunlit.maximalCarboxylationRate = optimalCarboxylationRate * acclimationFunction(HAVCM*1000,HD*1000,sunlit.leafTemperature,entropicFactorCarboxyliation,parameterWangLeuningFix.optimalTemperatureForPhotosynthesis); //sunlit big leaf
        shaded.maximalCarboxylationRate = optimalCarboxylationRate * acclimationFunction(HAVCM*1000,HD*1000,shaded.leafTemperature,entropicFactorCarboxyliation,parameterWangLeuningFix.optimalTemperatureForPhotosynthesis); //shaded big leaf
        // Scale-up maximum carboxylation rate (mol m-2 s-1)
        sunlit.maximalCarboxylationRate *= UPSCALINGFUNC((directLightK+diffuseLightKPAR),statePlant.stateGrowth.leafAreaIndex);
        shaded.maximalCarboxylationRate *= (UPSCALINGFUNC(diffuseLightKPAR,statePlant.stateGrowth.leafAreaIndex) - UPSCALINGFUNC((directLightK+diffuseLightKPAR),statePlant.stateGrowth.leafAreaIndex));
        //CO2 compensation point in dark
        sunlit.carbonMichaelisMentenConstant = exp(CKC - HAKC/dum[0]) * 1.0e-6 * myAtmosphericPressure ;
        shaded.carbonMichaelisMentenConstant = exp(CKC - HAKC/dum[1]) * 1.0E-6 * myAtmosphericPressure ;
        // Adjust Michaelis constant of oxygenation for temp (Pa)
        sunlit.oxygenMichaelisMentenConstant = exp(CKO - HAKO/dum[0])* 1.0e-3 * myAtmosphericPressure ;
        shaded.oxygenMichaelisMentenConstant = exp(CKO - HAKO/dum[1])* 1.0E-3 * myAtmosphericPressure ;
        // CO2 compensation point with no dark respiration (Pa)
        sunlit.compensationPoint = exp(CGSTAR - HAGSTAR/dum[0]) * 1.0e-6 * myAtmosphericPressure ;
        shaded.compensationPoint = exp(CGSTAR - HAGSTAR/dum[1]) * 1.0e-6 * myAtmosphericPressure ;
        // Electron transport
        // Compute potential e- transport at ref temp (mol e m-2 s-1) from correlation with Vcmax
        optimalElectronTransportRate = 1.5 * optimalCarboxylationRate ; //general correlation based on Leuning (1997)
        // check value and compare with 2.5 value in Medlyn et al (1999) and 1.67 value in Medlyn et al (2002) Based on greer Weedon 2011
        // Adjust maximum potential electron transport for temperature (mol m-2 s-1)
        //sunlit.maximalElectronTrasportRate = optimalElectronTransportRate * exp(CJM - HAJM/dum[0]);
        //shaded.maximalElectronTrasportRate = optimalElectronTransportRate * exp(CJM - HAJM/dum[1]);
        sunlit.maximalElectronTrasportRate = optimalElectronTransportRate * acclimationFunction(HAJM*1000,HD*1000,sunlit.leafTemperature,entropicFactorElectronTransporRate,parameterWangLeuningFix.optimalTemperatureForPhotosynthesis);
        shaded.maximalElectronTrasportRate = optimalElectronTransportRate * acclimationFunction(HAJM*1000,HD*1000,shaded.leafTemperature,entropicFactorElectronTransporRate,parameterWangLeuningFix.optimalTemperatureForPhotosynthesis);

        // Compute maximum PSII quantum yield, light-acclimated (mol e- mol-1 quanta absorbed)
        sunlit.quantumYieldPS2 = 0.352 + 0.022*dum[2] - 3.4E-4*pow(dum[2],2);      //sunlit big-leaf
        shaded.quantumYieldPS2 = 0.352 + 0.022*dum[3] - 3.4E-4*pow(dum[3],2);      //shaded big-leaf
        // Compute convexity factor of light response curve (-)
        // The value derived from leaf Chl content is modified for temperature effects, according to Bernacchi et al. (2003)
        leafConvexityFactor = 1 - chlorophyllContent * 6.93E-4 ;    //from Pons & Anten (2004), fig. 3b
        sunlit.convexityFactorNonRectangularHyperbola = leafConvexityFactor/0.98 * (0.76 + 0.018*dum[2] - 3.7E-4*pow(dum[2],2));  //sunlit big-leaf
        shaded.convexityFactorNonRectangularHyperbola = leafConvexityFactor/0.98 * (0.76 + 0.018*dum[3] - 3.7E-4*pow(dum[3],2));  //shaded big-leaf
        // Scale-up potential electron transport of sunlit big-leaf (mol m-2 s-1)
        sunlit.maximalElectronTrasportRate *= UPSCALINGFUNC((directLightK+diffuseLightKPAR),statePlant.stateGrowth.leafAreaIndex);
        // Adjust electr transp of sunlit big-leaf for PAR effects (mol e- m-2 s-1)
        dum[4]= sunlit.absorbedPAR * sunlit.quantumYieldPS2 * BETA ; //  potential PSII e- transport of sunlit big-leaf (mol m-2 s-1)
        dum[5]= dum[4] + sunlit.maximalElectronTrasportRate ;
        dum[6]= dum[4] * sunlit.maximalElectronTrasportRate ;
        sunlit.maximalElectronTrasportRate = (dum[5] - sqrt(pow(dum[5],2) - 4.0*sunlit.convexityFactorNonRectangularHyperbola*dum[6])) / (2.0*sunlit.convexityFactorNonRectangularHyperbola);
        // Scale-up potential electron transport of shaded big-leaf (mol m-2 s-1)
        // The simplified formulation proposed by de Pury & Farquhar (1999) is applied
        shaded.maximalElectronTrasportRate *= (UPSCALINGFUNC(diffuseLightKPAR,statePlant.stateGrowth.leafAreaIndex) - UPSCALINGFUNC((directLightK+diffuseLightKPAR),statePlant.stateGrowth.leafAreaIndex));
        // Adjust electr transp of shaded big-leaf for PAR effects (mol e- m-2 s-1)
        dum[4]= shaded.absorbedPAR * shaded.quantumYieldPS2 * BETA ; // potential PSII e- transport of sunlit big-leaf (mol m-2 s-1)
        dum[5]= dum[4] + shaded.maximalElectronTrasportRate ;
        dum[6]= dum[4] * shaded.maximalElectronTrasportRate ;
        shaded.maximalElectronTrasportRate = (dum[5] - sqrt(pow(dum[5],2) - 4.0*shaded.convexityFactorNonRectangularHyperbola*dum[6])) / (2.0*shaded.convexityFactorNonRectangularHyperbola);
    }
    else
    {  //night-time computations
        sunlit.maximalElectronTrasportRate = 0.0;
        shaded.maximalElectronTrasportRate = 0.0;
        sunlit.darkRespiration = 0.0;
        sunlit.maximalCarboxylationRate = 0.0;
    }


}

void Vine3D_Grapevine::photosynthesisKernel(Tvine* cultivar, double COMP,double GAC,double GHR,double GSCD,double J,double KC,double KO
                     ,double RD,double RNI,double STOMWL,double VCmax,double *ASS,double *GSC,double *TR)
{
    // taken from Hydrall Model, Magnani UNIBO
    // daily time computation
    #define NODATA_TOLERANCE 9999
    double myStromalCarbonDioxide, VPDS, WC,WJ,VC,DUM1,CS; //, myPreviousVPDS;
    double ASSOLD, deltaAssimilation, myTolerance; //, myPreviousDelta;
    int I,Imax ;
    Imax = 1000 ;
    myTolerance = 1e-7;
    deltaAssimilation = NODATA_TOLERANCE;
    //myPreviousDelta = deltaAssimilation;
      if (J >= 1.0e-7)
      {
            // Initialize variables
            myStromalCarbonDioxide = 0.7 * getCO2() ;
            VPDS = vaporPressureDeficit;
            //myPreviousVPDS = VPDS;
            ASSOLD = NODATA ;
            DUM1 = 1.6*slopeSatVapPressureVSTemp/GAMMA+ GHR/GAC;
            I = 0 ; // initialize the cycle variable
            while ((I++ < Imax) && (deltaAssimilation > myTolerance))
            {
                //Assimilation
                WC = VCmax * myStromalCarbonDioxide / (myStromalCarbonDioxide + KC * (1.0 + OSS / KO));  //RuBP-limited carboxylation (mol m-2 s-1)
                WJ = J * myStromalCarbonDioxide / (4.5 * myStromalCarbonDioxide + 10.5 * COMP);  //electr transp-limited carboxyl (mol m-2 s-1)
                VC = minValue(WC,WJ);  //carboxylation rate (mol m-2 s-1)

                *ASS = maxValue(0.0, VC * (1.0 - COMP / myStromalCarbonDioxide));  //gross assimilation (mol m-2 s-1)
                CS = getCO2() - myAtmosphericPressure * (*ASS - RD) / GAC;	//CO2 concentration at leaf surface (Pa)
                CS = maxValue(1e-4,CS);
                //Stomatal conductance
                *GSC = GSCD + STOMWL * (*ASS-RD) / (CS-COMP) * cultivar->parameterWangLeuning.sensitivityToVapourPressureDeficit / (cultivar->parameterWangLeuning.sensitivityToVapourPressureDeficit +VPDS); //stom conduct to CO2 (mol m-2 s-1)
                *GSC = maxValue(*GSC,1.0e-5);
                // Stromal CO2 concentration
                myStromalCarbonDioxide = CS - myAtmosphericPressure * (*ASS - RD) / (*GSC);	 //CO2 concentr at carboxyl sites (Pa)
                myStromalCarbonDioxide = maxValue(1.0e-2,myStromalCarbonDioxide) ;
                //Vapour pressure deficit at leaf surface
                VPDS = (slopeSatVapPressureVSTemp/CP_AIR*RNI + vaporPressureDeficit*GHR) / (GHR+(*GSC)*DUM1);  //VPD at the leaf surface (Pa)
                deltaAssimilation = fabs((*ASS) - ASSOLD);
                ASSOLD = *ASS ;
            }
      }
      else //night time computation
      {
        *ASS= 0.0;
        *GSC= GSCD;
        VPDS= vaporPressureDeficit ;
      }
      //  Transpiration rate
      *TR = (*GSC / 0.64) * VPDS/myAtmosphericPressure ;  //Transpiration rate (mol m-2 s-1). Ratio of diffusivities from Wang & Leuning 1998
      *TR = maxValue(1.0E-8,*TR) ;
}

void Vine3D_Grapevine::photosynthesisKernelSimplified(Tvine* cultivar, double COMP,double GSCD,double J,double KC,double KO
                     ,double RD,double STOMWL,double VCmax,double *ASS,double *GSC,double *TR)
{
    // taken from Hydrall Model, Magnani UNIBO
    // daily time computation
    #define NODATA_TOLERANCE 9999
    double myStromalCarbonDioxide, VPDS, WC,WJ,VC,CS; //myPreviousVPDS
    double ASSOLD, deltaAssimilation, myTolerance; //myPreviousDelta
    int I,Imax ;
    Imax = 1000 ;
    myTolerance = 1e-7;
    deltaAssimilation = NODATA_TOLERANCE;
    //myPreviousDelta = deltaAssimilation;
      if (J >= 1.0e-7)
      {
            // Initialize variables
            myStromalCarbonDioxide = 0.7 * getCO2() ;
            CS = getCO2();
            VPDS = vaporPressureDeficit;
            //myPreviousVPDS = VPDS;
            ASSOLD = NODATA ;
            //DUM1 = 1.6*slopeSatVapPressureVSTemp/GAMMA+ GHR/GAC;
            I = 0 ; // initialize the cycle variable
            while ((I++ < Imax) && (deltaAssimilation > myTolerance))
            {
                //Assimilation
                WC = VCmax * myStromalCarbonDioxide / (myStromalCarbonDioxide + KC * (1.0 + OSS / KO));  //RuBP-limited carboxylation (mol m-2 s-1)
                WJ = J * myStromalCarbonDioxide / (4.5 * myStromalCarbonDioxide + 10.5 * COMP);  //electr transp-limited carboxyl (mol m-2 s-1)
                VC = minValue(WC,WJ);  //carboxylation rate (mol m-2 s-1)

                *ASS = maxValue(0.0, VC * (1.0 - COMP / myStromalCarbonDioxide));  //gross assimilation (mol m-2 s-1)
                //CS = getCO2() - myAtmosphericPressure * (*ASS - RD) / GAC;	//CO2 concentration at leaf surface (Pa)
                //CS = maxValue(1e-4,CS);
                //Stomatal conductance
                *GSC = GSCD + STOMWL * (*ASS-RD) / (CS-COMP) * cultivar->parameterWangLeuning.sensitivityToVapourPressureDeficit / (cultivar->parameterWangLeuning.sensitivityToVapourPressureDeficit+VPDS); //stom conduct to CO2 (mol m-2 s-1)
                *GSC = maxValue(*GSC,1.0e-5);
                // Stromal CO2 concentration
                myStromalCarbonDioxide = CS - myAtmosphericPressure * (*ASS - RD) / (*GSC);	 //CO2 concentr at carboxyl sites (Pa)
                myStromalCarbonDioxide = maxValue(1.0e-2,myStromalCarbonDioxide) ;
                //Vapour pressure deficit at leaf surface
                //VPDS = (slopeSatVapPressureVSTemp/CP_AIR*RNI + vaporPressureDeficit*GHR) / (GHR+(*GSC)*DUM1);  //VPD at the leaf surface (Pa)
                deltaAssimilation = fabs((*ASS) - ASSOLD);
                ASSOLD = *ASS ;
            }
      }
      else //night time computation
      {
        *ASS= 0.0;
        *GSC= GSCD;
        VPDS= vaporPressureDeficit ;
      }
      //  Transpiration rate
      *TR = (*GSC / 0.64) * VPDS/myAtmosphericPressure ;  //Transpiration rate (mol m-2 s-1). Ratio of diffusivities from Wang & Leuning 1998
      *TR = maxValue(1E-8, *TR) ;
}

void Vine3D_Grapevine::carbonWaterFluxes(Tvine* cultivar)
    {
        // taken from Hydrall Model, Magnani UNIBO
        assimilationInstant = 0 ;
        transpirationInstant = 0 ;
        totalStomatalConductance = 0 ;
        if(sunlit.leafAreaIndex > 0)
        {
            Vine3D_Grapevine::photosynthesisKernelSimplified(cultivar, sunlit.compensationPoint,sunlit.minimalStomatalConductance,sunlit.maximalElectronTrasportRate,sunlit.carbonMichaelisMentenConstant,sunlit.oxygenMichaelisMentenConstant,sunlit.darkRespiration,alphaLeuning,sunlit.maximalCarboxylationRate,&sunlit.assimilation,&sunlit.stomatalConductance,&sunlit.transpiration);	//sunlit big-leaf
        }
        else
        {
            sunlit.assimilation = 0.0;
            sunlit.stomatalConductance = 0.0;
            sunlit.transpiration = 0.0;
        }
        assimilationInstant += sunlit.assimilation ;
        transpirationInstant += sunlit.transpiration ;
        totalStomatalConductance += sunlit.stomatalConductance ;
        // shaded big leaf
        Vine3D_Grapevine::photosynthesisKernelSimplified(cultivar, shaded.compensationPoint,shaded.minimalStomatalConductance,shaded.maximalElectronTrasportRate,shaded.carbonMichaelisMentenConstant,shaded.oxygenMichaelisMentenConstant,shaded.darkRespiration,alphaLeuning,shaded.maximalCarboxylationRate,&shaded.assimilation,&shaded.stomatalConductance,&shaded.transpiration);
        assimilationInstant += shaded.assimilation ; //canopy gross assimilation (mol m-2 s-1)
        transpirationInstant += shaded.transpiration ; //canopy transpiration (mol m-2 s-1)
        totalStomatalConductance += shaded.stomatalConductance ; //canopy conductance to CO2 (mol m-2 s-1)
    }

void Vine3D_Grapevine::carbonWaterFluxesProfile(Tvine* cultivar)
    {
        // taken from Hydrall Model, Magnani UNIBO
        assimilationInstant = 0 ;

        totalStomatalConductance = 0 ;
        for (int i=0;i<soilLayersNr;i++)
        {
            transpirationInstantLayer[i] = 0 ;
            if(sunlit.leafAreaIndex > 0)
            {
                Vine3D_Grapevine::photosynthesisKernelSimplified(cultivar, sunlit.compensationPoint,sunlit.minimalStomatalConductance,
                                                                  sunlit.maximalElectronTrasportRate,sunlit.carbonMichaelisMentenConstant,
                                                                  sunlit.oxygenMichaelisMentenConstant,sunlit.darkRespiration,
                                                                  alphaLeuning*stressCoefficientProfile[i], sunlit.maximalCarboxylationRate,
                                                                  &(sunlit.assimilation),&(sunlit.stomatalConductance),
                                                                  &(sunlit.transpiration));	//sunlit big-leaf
            }
            else
            {
                sunlit.assimilation = 0.0;
                sunlit.stomatalConductance = 0.0;
                sunlit.transpiration = 0.0;
            }
            assimilationInstant += sunlit.assimilation*layerRootDensity[i] ;
            transpirationInstantLayer[i] += sunlit.transpiration*layerRootDensity[i] ;
            totalStomatalConductance += sunlit.stomatalConductance*layerRootDensity[i] ;
            // shaded big leaf
            Vine3D_Grapevine::photosynthesisKernelSimplified(cultivar, shaded.compensationPoint,shaded.minimalStomatalConductance,
                                                              shaded.maximalElectronTrasportRate,shaded.carbonMichaelisMentenConstant,
                                                              shaded.oxygenMichaelisMentenConstant,shaded.darkRespiration,
                                                              alphaLeuning * stressCoefficientProfile[i],
                                                              shaded.maximalCarboxylationRate,&(shaded.assimilation),
                                                              &(shaded.stomatalConductance),&(shaded.transpiration));
            assimilationInstant += shaded.assimilation*layerRootDensity[i] ; //canopy gross assimilation (mol m-2 s-1)
            transpirationInstantLayer[i] += shaded.transpiration*layerRootDensity[i] ; //canopy transpiration (mol m-2 s-1)
            totalStomatalConductance += shaded.stomatalConductance*layerRootDensity[i] ; //canopy conductance to CO2 (mol m-2 s-1)
        }
    }

void Vine3D_Grapevine::carbonWaterFluxesProfileNoStress(Tvine* cultivar)
    {
        // taken from Hydrall Model, Magnani UNIBO
        double assimilationInstantNoStress = 0 ;
        transpirationInstantNoStress = 0;
        totalStomatalConductanceNoStress = 0 ;
        for (int i=0;i<soilLayersNr;i++)
        {
            //transpirationInstantLayer[i] = 0 ;
            if(sunlit.leafAreaIndex > 0)
            {
                Vine3D_Grapevine::photosynthesisKernelSimplified(cultivar, sunlit.compensationPoint,sunlit.minimalStomatalConductance,sunlit.maximalElectronTrasportRate,sunlit.carbonMichaelisMentenConstant,sunlit.oxygenMichaelisMentenConstant,sunlit.darkRespiration, alphaLeuning, sunlit.maximalCarboxylationRate,&sunlit.assimilation,&sunlit.stomatalConductance,&sunlit.transpiration);	//sunlit big-leaf
            }
            else
            {
                sunlit.assimilation = 0.0;
                sunlit.stomatalConductance = 0.0;
                sunlit.transpiration = 0.0;
            }
            assimilationInstantNoStress += sunlit.assimilation*layerRootDensity[i] ;
            transpirationInstantNoStress += sunlit.transpiration*layerRootDensity[i] ;
            totalStomatalConductanceNoStress += sunlit.stomatalConductance*layerRootDensity[i] ;
            // shaded big leaf
            Vine3D_Grapevine::photosynthesisKernelSimplified(cultivar, shaded.compensationPoint,shaded.minimalStomatalConductance,shaded.maximalElectronTrasportRate,shaded.carbonMichaelisMentenConstant,shaded.oxygenMichaelisMentenConstant,shaded.darkRespiration, alphaLeuning, shaded.maximalCarboxylationRate,&shaded.assimilation,&shaded.stomatalConductance,&shaded.transpiration);
            assimilationInstantNoStress += shaded.assimilation*layerRootDensity[i] ; //canopy gross assimilation (mol m-2 s-1)
            transpirationInstantNoStress += shaded.transpiration*layerRootDensity[i] ; //canopy transpiration (mol m-2 s-1)
            totalStomatalConductanceNoStress += shaded.stomatalConductance*layerRootDensity[i] ; //canopy conductance to CO2 (mol m-2 s-1)
        }
    }

double Vine3D_Grapevine::getStressCoefficient()
{
    double stomatalRatio;
    if (totalStomatalConductanceNoStress == 0.0)
        stomatalRatio = 1.0;
    else
        stomatalRatio = totalStomatalConductance / totalStomatalConductanceNoStress;
    return 1.0 - stomatalRatio;
}

void Vine3D_Grapevine::cumulatedResults()
{
    // taken from Hydrall Model, Magnani UNIBO
    // Cumulate hourly values of gas exchange
    deltaTime.absorbedPAR = simulationStepInSeconds*(sunlit.absorbedPAR+shaded.absorbedPAR);  //absorbed PAR (mol m-2 yr-1)
    deltaTime.grossAssimilation = simulationStepInSeconds * assimilationInstant ; // canopy gross assimilation (mol m-2)
    deltaTime.respiration = simulationStepInSeconds * Vine3D_Grapevine::plantRespiration() ;
    deltaTime.netAssimilation = deltaTime.grossAssimilation- deltaTime.respiration ;
    deltaTime.netAssimilation = deltaTime.netAssimilation*12/1000.0/FC ;
    statePlant.stateGrowth.cumulatedBiomass += deltaTime.netAssimilation ;

    deltaTime.transpiration = 0.;

    for (int i=0; i<soilLayersNr; i++)
    {
        transpirationLayer[i]= simulationStepInSeconds * H2OMOLECULARWEIGHT * transpirationInstantLayer[i]; // [mm]
        deltaTime.transpiration += transpirationLayer[i];
    }
    this->statePlant.outputPlant.transpirationNoStress = simulationStepInSeconds * H2OMOLECULARWEIGHT * transpirationInstantNoStress; //mm

}

double Vine3D_Grapevine::plantRespiration()
{
    // taken from Hydrall Model, Magnani UNIBO
    double leafRespiration,rootRespiration,sapwoodRespiration,shootRespiration ;
    double totalRespiration;
    nitrogen.leaf = 0.02;    //[kg kgDM-1]
    nitrogen.shoot = 0.012;  //[kg kgDM-1]
    nitrogen.root = 0.0078;  //[kg kgDM-1]
    nitrogen.stem = 0.0021;  //[kg kgDM-1]

    biomass.leaf = biomass.shoot = (statePlant.stateGrowth.cumulatedBiomass - statePlant.stateGrowth.fruitBiomass)/2 ;
    biomass.fineRoot = 1.5e-4 * minValue(1,statePlant.statePheno.daysAfterBloom); // from Schreiner et al. 2006
    biomass.sapwood = 2.0e-4 * minValue(1,statePlant.statePheno.daysAfterBloom); // from Schreiner et al. 2006
    // Compute hourly stand respiration at 10 oC (mol m-2 h-1)
    leafRespiration = 0.0106/2.0 * (biomass.leaf * nitrogen.leaf/0.014) ;
    shootRespiration = 0.0106/2.0 * (biomass.shoot * nitrogen.shoot/0.014) ;
    sapwoodRespiration = 0.0106/2.0 * (biomass.sapwood * nitrogen.stem/0.014) ;
    rootRespiration = 0.0106/2.0 * (biomass.fineRoot * nitrogen.root/0.014) ;

    // Adjust for temperature effects
    //leafRespiration *= maxValue(0,minValue(1,Vine3D_Grapevine::temperatureMoistureFunction(myInstantTemp + ZEROCELSIUS))) ;
    //sapwoodRespiration *= maxValue(0,minValue(1,Vine3D_Grapevine::temperatureMoistureFunction(myInstantTemp + ZEROCELSIUS))) ;
    //shootRespiration *= maxValue(0,minValue(1,Vine3D_Grapevine::temperatureMoistureFunction(myInstantTemp + ZEROCELSIUS))) ;
    mySoilTemp = Vine3D_Grapevine::soilTemperatureModel();
    rootRespiration *= maxValue(0,minValue(1,Vine3D_Grapevine::temperatureMoistureFunction(mySoilTemp + ZEROCELSIUS))) ;
    // hourly canopy respiration (sapwood+fine roots)
    totalRespiration =(leafRespiration + sapwoodRespiration + rootRespiration + shootRespiration);
    // per second respiration
    totalRespiration /= (double)(HOUR_SECONDS);
    return totalRespiration;
}

double Vine3D_Grapevine::soilTemperatureModel()
{
    // taken from Hydrall Model, Magnani UNIBO
    double temp;
    temp = 0.8 * this->statePlant.stateGrowth.meanTemperatureLastMonth + 0.2 * myInstantTemp;
    return temp;
}

double Vine3D_Grapevine::temperatureMoistureFunction(double temperature)
{
    // taken from Hydrall Model, Magnani UNIBO
    int   MODEL;
    double temperatureMoistureFactor,correctionSoilMoisture; //K_VW
    //K_VW= 1.5;
    temperatureMoistureFactor = 1. ;
    MODEL = 2;
    //T = climate.instantT;
    //1. AP_H model
    if (MODEL == 1) {

        if(psiSoilAverage >= psiFieldCapacityAverage)
        {
            correctionSoilMoisture = 1.0; //effects of soil water potential
        }
        else if (psiSoilAverage <= wiltingPoint)
        {
            correctionSoilMoisture = 0.0;
        }
        else
        {
            correctionSoilMoisture = log(wiltingPoint/psiSoilAverage) / log(wiltingPoint/psiFieldCapacityAverage);
        }
        temperatureMoistureFactor = pow(2.0,((temperature - parameterWangLeuningFix.optimalTemperatureForPhotosynthesis)/10.0)); // temperature dependence of respiration, based on Q10 approach
        temperatureMoistureFactor *= correctionSoilMoisture;
    }
    // 2. AP_LT model
    else if (MODEL == 2){
         //effects of soil water potential
        if (psiSoilAverage >= psiFieldCapacityAverage)
        {
            correctionSoilMoisture = 1.0;
        }
        else if (psiSoilAverage <= wiltingPoint)
        {
            correctionSoilMoisture = 0.0;
        }
        else
        {
            correctionSoilMoisture = log(wiltingPoint/psiSoilAverage) / log(wiltingPoint/psiFieldCapacityAverage);
        }
        temperatureMoistureFactor= exp(308.56 * (1.0/(parameterWangLeuningFix.optimalTemperatureForPhotosynthesis + 46.02) - 1.0/(temperature+46.02)));  //temperature dependence of respiration, based on Lloyd & Taylor (1994)
        temperatureMoistureFactor *= correctionSoilMoisture;
    }
    return temperatureMoistureFactor;
}


void Vine3D_Grapevine::plantInterception(double fieldCoverByPlant)
{
    // taken from Hydrall Model, Magnani UNIBO
    double plantInterceptedWater;
    // to check if units are ok
    specificLeafArea = statePlant.stateGrowth.leafAreaIndex * biomass.leaf ;
    //canopy capacity (mm per hour)
    plantInterceptedWater = 0.07 * specificLeafArea * (biomass.leaf * fieldCoverByPlant)*simulationStepInSeconds/86400.0;

    //TODO Antonio: check
    //canopy rainfall interception (mm per hour)
    if (plantInterceptedWater > (0.5*myPrec))  plantInterceptedWater = myPrec*0.5;

    deltaTime.interceptedWater = plantInterceptedWater;
}

double Vine3D_Grapevine::meanLastMonthTemperature(double previousLastMonthTemp)
{
    double newTemperature;
    double monthFraction;
    monthFraction = simulationStepInSeconds/(2592000.0); // seconds of 30 days
    newTemperature = previousLastMonthTemp * (1 - monthFraction) + myInstantTemp * monthFraction ;
    return newTemperature;
}



double* Vine3D_Grapevine::rootDensity(soil::Crit3DSoil* mySoil, int nrLayers,
                                       int nrLayersWithRoot, int nrUpperLayersWithoutRoot,
                                       rootDistribution type, double* layerDepth,
                                       double* layerThickness, double mode , double mean)
{

    double shapeFactor=2.;

    double *layerRootDensity =  (double *) calloc(nrLayers, sizeof(double));

    if (type == CARDIOID_DISTRIBUTION)
    {
        double *lunette =  (double *) calloc(2*nrLayersWithRoot, sizeof(double));
        double *lunetteDensity =  (double *) calloc(2*nrLayersWithRoot, sizeof(double));
        for (int i = 0 ; i<nrLayersWithRoot ; i++)
        {
            double sinAlfa,cosAlfa,alfa;
            sinAlfa = 1.0 - (double)(1+i)/((double)(nrLayersWithRoot)) ;
            cosAlfa = maxValue(sqrt(1.0 - pow(sinAlfa,2)),0.0001);
            alfa = atan(sinAlfa/cosAlfa);
            lunette[i]= ((PI/2) - alfa - sinAlfa*cosAlfa)/PI;
        }
        lunetteDensity[2*nrLayersWithRoot - 1]= lunetteDensity[0] = lunette[0];
        for (int i = 1 ; i<nrLayersWithRoot ; i++)
        {
            lunetteDensity[2*nrLayersWithRoot - i - 1]=lunetteDensity[i]=lunette[i]-lunette[i-1] ;            
        }
        // cardioid deformation
        double LiMin,Limax,k,normalizationFactor ;
        LiMin = -log(0.2)/nrLayersWithRoot;
        Limax = -log(0.05)/nrLayersWithRoot;
        k = LiMin + (Limax - LiMin)*(shapeFactor-1);
        normalizationFactor = 0 ;
        for (int i = 0 ; i<(2*nrLayersWithRoot) ; i++)
        {
            lunetteDensity[i] *= exp(-k*(i+0.5)); //changed from basic to C
            normalizationFactor += lunetteDensity[i];
        }
        for (int i = 0 ; i<(2*nrLayersWithRoot) ; i++)
        {
            lunetteDensity[i] /= normalizationFactor ;
        }
        for  (int i = 0 ; i<(nrLayers) ; i++)
        {
            layerRootDensity[i]=0;
        }
        for (int i = 0 ; i<( nrLayersWithRoot) ; i++)
        {
            layerRootDensity[nrUpperLayersWithoutRoot+i]=lunetteDensity[2*i] + lunetteDensity[2*i+1] ;
        }
    }

    else if (type == GAMMA_DISTRIBUTION)
    {
        double normalizationFactor ;
        double kappa, theta;
        double a, b, skeleton;
        int indexHorizon;
        theta = mean - mode;
        kappa = mean / theta;
        // complete gamma function
        normalizationFactor = Gamma_Function(kappa);
        double rootDensitySum = 0. ;
        for (int i=0 ; i<nrLayers ; i++)
        {
            layerRootDensity[i] = 0;
            if (i >= nrUpperLayersWithoutRoot)
            {
                a = layerDepth[i] - layerThickness[i]*0.5;
                b = layerDepth[i] + layerThickness[i]*0.5;
                layerRootDensity[i] = Incomplete_Gamma_Function(b/theta,kappa) - Incomplete_Gamma_Function(a/theta,kappa);
                layerRootDensity[i] /= normalizationFactor;

                //skeleton
                indexHorizon = soil::getHorizonIndex(mySoil, (float)layerDepth[i]);
                skeleton = mySoil->horizon[indexHorizon].coarseFragments;
                layerRootDensity[i] *= (1.0-skeleton);

                rootDensitySum += layerRootDensity[i];
            }
        }
        for (int i=0 ; i<nrLayers ; i++)
        {
            layerRootDensity[i] /= rootDensitySum;
        }
    }

    return layerRootDensity ;
}


bool Vine3D_Grapevine::getExtractedWater(double* myWaterExtractionProfile)
{
    for(int i=0; i<soilLayersNr; i++)
    {
        myWaterExtractionProfile[i] = 0;
        if (transpirationLayer[i] != NODATA)
            myWaterExtractionProfile[i] += transpirationLayer[i];
        //if ((myWaterExtractionProfile[i] > 0.1) || (myWaterExtractionProfile[i]<0)) printf("estrazione  %f\n",myWaterExtractionProfile[i]);
        if (transpirationCumulatedGrass[i] != NODATA)
            myWaterExtractionProfile[i]+= transpirationCumulatedGrass[i];


    }
    return true;
}

double Vine3D_Grapevine::getRealTranspirationGrapevine()
{
    double sum = 0.0;
    for(int i=0; i<soilLayersNr; i++)
    {
        if (transpirationLayer[i] != NODATA)
            sum += transpirationLayer[i];
    }

    return sum;
}

double Vine3D_Grapevine::getRealTranspirationGrass()
{
    double sum = 0.0;
    for(int i=0; i<soilLayersNr; i++)
    {
        if (transpirationCumulatedGrass[i] != NODATA)
            sum += transpirationCumulatedGrass[i];
    }

    return sum;
}


bool Vine3D_Grapevine::getExtractedWaterFromGrassTranspirationandEvaporation(double* myWaterExtractionProfile)
{
    bool* underWiltingPointLayers = (bool*) calloc(soilLayersNr, sizeof(bool));
    if (myWaterExtractionProfile == NULL) return false;

    for(int i=0; i<soilLayersNr; i++)
    {
        if (psiSoilProfile[i] < wiltingPoint) underWiltingPointLayers[i]= true ;
        else underWiltingPointLayers[i]= false ;
    }


    for (int i=0 ; i<soilLayersNr ; i++)
    {
        double a, b;
        double rootDepthGrass=0.3;
        //controllare
        a = layerDepth[i] - layerThickness[i]*0.5;
        b = layerDepth[i] + layerThickness[i]*0.5;
        if (a >= rootDepthGrass) myWaterExtractionProfile[i] += 0;
        else
        {
            if(!underWiltingPointLayers[i])
            {
                myWaterExtractionProfile[i] += (this->statePlant.outputPlant.evaporation + this->statePlant.outputPlant.grassTranspiration)
                        * (fabs(a - minValue(rootDepthGrass,b))/rootDepthGrass);

            }
            else myWaterExtractionProfile[i] += 0;
        }
    }
    free(underWiltingPointLayers);
    return true;
}


// phenology by Caffarra and Eccel 2011
double Vine3D_Grapevine::chillingRate(double temp, double aParameter, double CP_AIRarameter)
{
    return (2./(1+exp(aParameter*(temp-CP_AIRarameter)*(temp-CP_AIRarameter))));
}

double Vine3D_Grapevine::criticalForceState(double chillingState,double co1 , double co2)
{
    return co1*exp(co2*chillingState);
}

double Vine3D_Grapevine::forceStateFunction(double forceState , double temp, double degDaysVeraison)
{
    forceState += (1./(1+exp(-0.26*(temp-16.06))));
    if ((statePlant.statePheno.degreeDaysFromFirstMarch > degDaysVeraison) && (this->statePlant.statePheno.daysAfterBloom < 100))
    {
        double coldCorrection;
        double a,b,c, offset, maxCorrection;
        a = 4.0;
        c = 14.5;
        maxCorrection = 0.33;
        offset = -0.05;
        if (temp < c) b = 5.0;
        else b = 1.2;

        coldCorrection = offset + maxCorrection*(1.0 / (1.0+pow(fabs((temp-c)/a),2.0*b)));
        forceState += coldCorrection;
    }
    return forceState;
}

double Vine3D_Grapevine::forceStateFunction(double forceState , double temp)
{
    forceState += (1./(1+exp(-0.26*(temp-16.06))));
    return forceState;
}
void Vine3D_Grapevine::printResults(std::string fileName, bool* isFirst, Tvine* cultivar)
{
    FILE *fp;
    if (*isFirst)
    {
        fp = fopen(fileName.c_str(),"w");
        *isFirst = false;
    }
    else
    {
        fp = fopen(fileName.c_str(),"a");
        printf("GPP:%.3f\t lai:%.2f \tStage:%.1f \n",assimilationInstant*1e6, statePlant.stateGrowth.leafAreaIndex, this->statePlant.statePheno.stage );
        printf("%.1f  temperature   %.1f  sunlit %.1f  shaded", myInstantTemp,sunlit.leafTemperature-ZEROCELSIUS, shaded.leafTemperature-ZEROCELSIUS);
    }
    fprintf(fp,"%d\t%f\t%f\t%f\t%f\t%f\n", myDoy,myHour, potentialEvapotranspiration, deltaTime.transpiration, deltaTime.transpirationGrass ,getWaterStressSawFunctionAverage(cultivar));
    fclose(fp);
}



void Vine3D_Grapevine::computePhenology(bool computeDaily, bool* isVegSeason, Tvine* cultivar)
{
    double criticalForceStateBudBurst;

    if (this->statePlant.statePheno.stage < ecoDormancy)
    {
        this->statePlant.stateGrowth.isHarvested = 0;
        this->statePlant.stateGrowth.cumulatedBiomass = 0.; // [g m-2]
        this->statePlant.stateGrowth.fruitBiomass = 0.;
        this->statePlant.stateGrowth.fruitBiomassIndex = cultivar->parameterBindiMiglietta.fruitBiomassSlope;
        this->statePlant.stateGrowth.leafAreaIndex = LAIMIN;
        this->statePlant.stateGrowth.shootLeafNumber = minShootLeafNr;
        this->statePlant.statePheno.daysAfterBloom = 0.;
        this->statePlant.statePheno.cumulatedRadiationFromFruitsetToVeraison = 0.;
        this->statePlant.statePheno.degreeDaysAtFruitSet = NODATA;
        this->statePlant.statePheno.degreeDaysFromFirstMarch = NODATA;

        leafNumberRate = 0. ;
        myThermalUnit = 0.;

        if (this->statePlant.statePheno.stage == NOT_INITIALIZED_VINE)
        {
            this->statePlant.stateGrowth.isHarvested = 0;
            this->statePlant.stateGrowth.leafAreaIndex = NODATA;
            this->statePlant.stateGrowth.shootLeafNumber = NODATA;
            this->statePlant.stateGrowth.fruitBiomass = NODATA;
            this->statePlant.stateGrowth.cumulatedBiomass = NODATA;
            this->statePlant.statePheno.daysAfterBloom = NODATA;
        }
    }

    if ((this->statePlant.statePheno.stage >= budBurst))
        *isVegSeason = true ;
    else
        *isVegSeason = false;

    if (computeDaily)
    {
        if (myDoy == parameterPhenoVitisFix.startingDay)
            this->statePlant.statePheno.chillingState = 0;

        else
            this->statePlant.statePheno.chillingState += chillingRate(meanDailyTemperature, parameterPhenoVitisFix.a,parameterPhenoVitisFix.optimalChillingTemp);

        if (this->statePlant.statePheno.stage != NOT_INITIALIZED_VINE)
            this->statePlant.statePheno.stage = endoDormancy + minValue(1,this->statePlant.statePheno.chillingState / cultivar->parameterPhenoVitis.criticalChilling);

        if ((this->statePlant.statePheno.chillingState > cultivar->parameterPhenoVitis.criticalChilling))
        {
            this->statePlant.statePheno.forceStateBudBurst = forceStateFunction(this->statePlant.statePheno.forceStateBudBurst , meanDailyTemperature);
            criticalForceStateBudBurst = criticalForceState(this->statePlant.statePheno.chillingState, cultivar->parameterPhenoVitis.co1 , parameterPhenoVitisFix.co2);
            this->statePlant.statePheno.stage = ecoDormancy + minValue(1,1 -
                        ((criticalForceStateBudBurst-this->statePlant.statePheno.forceStateBudBurst)/criticalForceStateBudBurst));
        }

        if ((this->statePlant.statePheno.forceStateBudBurst > criticalForceStateBudBurst))
        {
            *isVegSeason = true;
            this->statePlant.statePheno.forceStateVegetativeSeason = forceStateFunction(this->statePlant.statePheno.forceStateVegetativeSeason, meanDailyTemperature,cultivar->parameterPhenoVitis.degreeDaysAtVeraison);

            //veraison->physiologicalMaturity
            if (this->statePlant.statePheno.forceStateVegetativeSeason > cultivar->parameterPhenoVitis.criticalForceStateVeraison)
            {
                this->statePlant.statePheno.stage = veraison +
                      ((this->statePlant.statePheno.forceStateVegetativeSeason-cultivar->parameterPhenoVitis.criticalForceStateVeraison) /
                      (cultivar->parameterPhenoVitis.criticalForceStatePhysiologicalMaturity - cultivar->parameterPhenoVitis.criticalForceStateVeraison));
                if (this->statePlant.statePheno.stage > vineSenescence)
                    this->statePlant.statePheno.stage = vineSenescence;
            }
            //fruitset->veraison - mixed models
            else if (this->statePlant.statePheno.forceStateVegetativeSeason > cultivar->parameterPhenoVitis.criticalForceStateFruitSet)
            {
                if (this->statePlant.statePheno.degreeDaysAtFruitSet == NODATA)
                    this->statePlant.statePheno.stage = fruitSet;
                else
                    this->statePlant.statePheno.stage = fruitSet +
                        ((this->statePlant.statePheno.degreeDaysFromFirstMarch - this->statePlant.statePheno.degreeDaysAtFruitSet) /
                        (cultivar->parameterPhenoVitis.degreeDaysAtVeraison - this->statePlant.statePheno.degreeDaysAtFruitSet));

                if (this->statePlant.statePheno.stage >= veraison)
                    this->statePlant.statePheno.forceStateVegetativeSeason = cultivar->parameterPhenoVitis.criticalForceStateVeraison;
            }
            //flowering->fruitset
            else if (this->statePlant.statePheno.forceStateVegetativeSeason > cultivar->parameterPhenoVitis.criticalForceStateFlowering)
            {
                this->statePlant.statePheno.stage = flowering +
                        ((this->statePlant.statePheno.forceStateVegetativeSeason - cultivar->parameterPhenoVitis.criticalForceStateFlowering) /
                        (cultivar->parameterPhenoVitis.criticalForceStateFruitSet - cultivar->parameterPhenoVitis.criticalForceStateFlowering));
            }
            else
            {
                this->statePlant.statePheno.stage = budBurst +
                        (this->statePlant.statePheno.forceStateVegetativeSeason/cultivar->parameterPhenoVitis.criticalForceStateFlowering);
            }

        }
    }

    // brix calculation
    if ((this->statePlant.statePheno.stage >= veraison)&& (this->statePlant.statePheno.stage < vineSenescence))
    {

        /*if ((this->statePlant.statePheno.degreeDaysFromFirstMarch > cultivar->parameterPhenoVitis.degreeDaysAtVeraison) && (myHour == 14))
        {
            double coldCorrection;
            double a,b,c, offset, maxCorrection;
            a = 4.0;
            c = 14.5;
            maxCorrection = 0.33;
            offset = -0.05;
            if (meanDailyTemperature < c) b = 5.0;
            else b = 1.2;

            coldCorrection = offset + maxCorrection*(1.0 / (1.0+pow(fabs((meanDailyTemperature-c)/a),2.0*b)));
            this->statePlant.statePheno.forceStateVegetativeSeason += coldCorrection;
        }*/
        //per sangiovese
        this->statePlant.outputPlant.brixBerry = minValue(potentialBrix,0.28 * (this->statePlant.statePheno.forceStateVegetativeSeason - cultivar->parameterPhenoVitis.criticalForceStateVeraison)  + 11.5) ;
        //this->statePlant.outputPlant.brixBerry = minValue(potentialBrix,0.41 * (this->statePlant.statePheno.forceStateVegetativeSeason - cultivar->parameterPhenoVitis.criticalForceStateVeraison)  + 11.5) ;
        this->statePlant.outputPlant.brixMaximum = potentialBrix;
    }
    else
    {
        this->statePlant.outputPlant.brixBerry = NODATA;
        this->statePlant.outputPlant.brixMaximum = NODATA;
    }

    //15 november
    if (myDoy == 320)
    {
        this->statePlant.statePheno.stage = endoDormancy;
        this->statePlant.statePheno.forceStateBudBurst = 0.;
        this->statePlant.statePheno.forceStateVegetativeSeason = 0.;
        this->statePlant.outputPlant.brixBerry = NODATA;
        this->statePlant.outputPlant.brixMaximum = NODATA;
        this->statePlant.statePheno.degreeDaysAtFruitSet = NODATA;
        this->statePlant.statePheno.degreeDaysFromFirstMarch = NODATA;
        *isVegSeason = false;
    }

}

double  Vine3D_Grapevine::leafWidth()
{
    if (this->statePlant.statePheno.stage == budBurst) return (myLeafWidth*0.2);
    else if (this->statePlant.statePheno.stage == flowering) return (myLeafWidth*0.5);
    else return (myLeafWidth) ;
}

double Vine3D_Grapevine::getWaterStressByPsiSoil(double myPsiSoil,double psiSoilStressParameter,double exponentialFactorForPsiRatio)
{
    // Green et al. 2007 soil potentials in MPa and positive sign
    double waterStressFactor; // MPa
    waterStressFactor = 1 - 1.0/(1+pow((-myPsiSoil/psiSoilStressParameter),exponentialFactorForPsiRatio));
    return waterStressFactor;
}

double Vine3D_Grapevine::getWaterStressSawFunction(int index, Tvine* cultivar)
{
    if (cultivar->parameterWangLeuning.waterStressThreshold < fractionTranspirableSoilWaterProfile[index])
        return 1.; // no stress
    else
        return fractionTranspirableSoilWaterProfile[index] / cultivar->parameterWangLeuning.waterStressThreshold ;
}

double Vine3D_Grapevine::getWaterStressSawFunctionAverage(Tvine* cultivar)
{
    if (cultivar->parameterWangLeuning.waterStressThreshold < fractionTranspirableSoilWaterAverage)
        return 1.; // no stress
    else
        return fractionTranspirableSoilWaterAverage / cultivar->parameterWangLeuning.waterStressThreshold ;
}


//incolto
double Vine3D_Grapevine::getFallowTranspiration(double stress, double lai, double sensitivityToVPD)
{
    if (lai <= 0) return 0.0;

    double waterUseEfficiencyGrass = getCO2() * 0.3 / (1.6*vaporPressureDeficit);
    double photosyntheticActiveRadiation = 0.5 * myIrradiance;

    double grassAbsorbedPAR = photosyntheticActiveRadiation * (1-exp(-0.8*lai));
    double assimilationGrass = (RUEGRASS*1E-9 / 0.012) * grassAbsorbedPAR * stress;;

    // water-limited transpiration
    double assimilWaterLimited = waterUseEfficiencyGrass * 1.25e-3 * 0.1 * stress / H2OMOLECULARWEIGHT;
    assimilationGrass = minValue(assimilationGrass, assimilWaterLimited);

    double dum = R_GAS/1000. * (myInstantTemp + ZEROCELSIUS );
    double compensationPoint = exp(CGSTAR - HAGSTAR/dum) * 1.0e-6 * myAtmosphericPressure;
    double stomatalConductanceGrass = parameterWangLeuningFix.minimalStomatalConductance + alphaLeuning
            * ((assimilationGrass*(1-0.089)) / (getCO2()-compensationPoint))
            * (sensitivityToVPD / (sensitivityToVPD + vaporPressureDeficit)); //stom conduct to CO2 (mol m-2 s-1)

    stomatalConductanceGrass = maxValue(stomatalConductanceGrass, 1.0e-5);
    //Transpiration rate (mol m-2 s-1). Ratio of diffusivities from Wang & Leuning 1998
    double grassTransp = (stomatalConductanceGrass / 0.64) * vaporPressureDeficit/myAtmosphericPressure ;

    grassTransp = maxValue(1.0E-8, grassTransp) ;

    return grassTransp; // molH2O m-2 s-1
}



double Vine3D_Grapevine::getGrassTranspiration(double stress, double laiGrassMax, double sensitivityToVPD, double fieldCoverByPlant)
{

    if (laiGrassMax <= 0) return 0.0;

    // water-limited transpiration
    double waterUseEfficiencyGrass = getCO2() * 0.3 / (1.6*vaporPressureDeficit);
    double photosyntheticActiveRadiation = 0.5 * myIrradiance;

    double grassAbsorbedPARSunlit, grassAbsorbedPARShaded;
    grassAbsorbedPARSunlit = (photosyntheticActiveRadiation - sunlit.absorbedPAR) * (1 - fieldCoverByPlant);
    grassAbsorbedPARShaded = (photosyntheticActiveRadiation - sunlit.absorbedPAR - shaded.absorbedPAR) * fieldCoverByPlant;

    grassAbsorbedPARShaded *= 1.0 - exp(-0.8 * getLAIGrass(SHADEDGRASS, laiGrassMax));
    grassAbsorbedPARSunlit *= 1.0 - exp(-0.8 * getLAIGrass(SUNLITGRASS, laiGrassMax));
    double assimilationGrassSunlit = RUEGRASS*1E-9 / 0.012 ;
    double assimilationGrassShaded = assimilationGrassSunlit; // molC m-2 s-1
    assimilationGrassShaded *= grassAbsorbedPARShaded*stress; // light-limited assimilation calculated through RUE
    assimilationGrassSunlit *= grassAbsorbedPARSunlit*stress;
    double assimilWaterLimited = waterUseEfficiencyGrass * 1.25e-3 * 0.1 * stress / H2OMOLECULARWEIGHT;
    assimilationGrassShaded = minValue(assimilationGrassShaded,
                    assimilWaterLimited * fieldCoverByPlant);
    assimilationGrassSunlit = minValue(assimilationGrassSunlit,
                    assimilWaterLimited * (1.0 - fieldCoverByPlant));

    double stomatalConductanceGrass, compensationPoint,dum;
    dum = R_GAS/1000. * (myInstantTemp + ZEROCELSIUS );
    compensationPoint = exp(CGSTAR - HAGSTAR/dum) * 1.0e-6 * myAtmosphericPressure ;
    stomatalConductanceGrass = parameterWangLeuningFix.minimalStomatalConductance + alphaLeuning
            * (((assimilationGrassShaded+assimilationGrassSunlit)*(1-0.089)) / (getCO2()-compensationPoint))
            * (sensitivityToVPD / (sensitivityToVPD + vaporPressureDeficit)); //stom conduct to CO2 (mol m-2 s-1)
    stomatalConductanceGrass = maxValue(stomatalConductanceGrass,1.0e-5);

    //Transpiration rate (mol m-2 s-1). Ratio of diffusivities from Wang & Leuning 1998
    double grassTransp = (stomatalConductanceGrass / 0.64) * vaporPressureDeficit/myAtmosphericPressure ;
    grassTransp = maxValue(1.0E-8,grassTransp) ;

    return grassTransp; // molH2O m-2 s-1
}


double Vine3D_Grapevine::getGrassRootDensity(int layer, float startRootDepth, float totalRootDepth)
{
    double upperDepth = layerDepth[layer] - layerThickness[layer]*0.5;
    double lowerDepth = layerDepth[layer] + layerThickness[layer]*0.5;
    if (upperDepth > totalRootDepth) return 0.0;
    if (lowerDepth < startRootDepth) return 0.0;

    double x1 = maxValue(startRootDepth, upperDepth);
    double x2 = minValue(totalRootDepth, lowerDepth);
    double m = -2.0 / (totalRootDepth*totalRootDepth);
    double q = 2.0 / totalRootDepth;
    double y1 = m*x1 + q;
    double y2 = m*x2 + q;
    //trapezoid
    return (y1+y2) * fabs(x2-x1) * 0.5;
}

//----------------------------------------------------------------------
// Fallow = incolto
// LAI minimo = 1
// usato per le zone non definite e vigneti non ancora inizializzati
//----------------------------------------------------------------------
void Vine3D_Grapevine::fallowTranspiration(double lai, double sensitivityToVPD)
{
    deltaTime.transpirationGrass = 0.;
    double transpiration, rootDensity;
    for (int layer=0; layer < soilLayersNr; layer++)
    {
        transpiration = getFallowTranspiration(stressCoefficientProfile[layer], lai, sensitivityToVPD);
        rootDensity = getGrassRootDensity(layer, (float)0.02, (float)soilTotalDepth);
        transpirationCumulatedGrass[layer] = simulationStepInSeconds * H2OMOLECULARWEIGHT
                                        * transpiration * rootDensity; // conversion into mm
        deltaTime.transpirationGrass += transpirationCumulatedGrass[layer];
    }
    this->statePlant.outputPlant.transpirationNoStress += simulationStepInSeconds * H2OMOLECULARWEIGHT*getFallowTranspiration(1, lai, sensitivityToVPD);;
}


void Vine3D_Grapevine::grassTranspiration(double laiGrassMax, double sensitivityToVPD, double fieldCoverByPlant)
{
    deltaTime.transpirationGrass = 0.;
    double transpiration, rootDensity;
    for (int layer=0; layer < soilLayersNr; layer++)
    {
        transpiration = getGrassTranspiration(stressCoefficientProfile[layer], laiGrassMax, sensitivityToVPD, fieldCoverByPlant);
        rootDensity = getGrassRootDensity(layer, (float)0.02, float(soilTotalDepth * 0.66));
        transpirationCumulatedGrass[layer] = simulationStepInSeconds * H2OMOLECULARWEIGHT
                                        * transpiration * rootDensity; // conversion into mm
        deltaTime.transpirationGrass += transpirationCumulatedGrass[layer];
    }
    this->statePlant.outputPlant.transpirationNoStress += simulationStepInSeconds * H2OMOLECULARWEIGHT*getGrassTranspiration(1., laiGrassMax, sensitivityToVPD, fieldCoverByPlant);
}


double Vine3D_Grapevine::getLaiStressCoefficient()
{
    double stress;
    stress = 1/(1+12.9*exp(-14.1*fractionTranspirableSoilWaterAverage)); // from Bindi et al. 1995
    return stress;
}


double Vine3D_Grapevine::getLAIGrass(bool isShadow, double laiGrassMax)
{
    double laiGrass = laiGrassMax;
    if (isShadow) laiGrass = laiGrassMax * 0.5;

    double laiGrassMin = laiGrass * 0.5;
    double delta = laiGrass - laiGrassMin;

    if ((myDoy > 181 && myDoy < 244))
    {
        return laiGrassMin;
    }
    if (myDoy > 151 && myDoy <=181)
    {
        return (laiGrassMin + delta *(181. - myDoy)/30.);
    }
    if (myDoy > 243 && myDoy <= 273)
    {
        return (laiGrassMin + delta *(myDoy - 243.)/30.);
    }
    return laiGrass;
}

void Vine3D_Grapevine::getLAIVine(TvineField* vineField)
{
    double shootLeafArea;
    double deltaLai, laiOld;
    laiOld = NODATA;

    if (this->statePlant.stateGrowth.shootLeafNumber >= 5 )
    {
        laiOld = this->statePlant.stateGrowth.leafAreaIndex;
        shootLeafArea = this->statePlant.stateGrowth.leafAreaIndex * parameterBindiMigliettaFix.shadedSurface / (vineField->shootsPerPlant*vineField->plantDensity);
        this->statePlant.stateGrowth.shootLeafNumber = pow(shootLeafArea/vineField->cultivar->parameterBindiMiglietta.d,1./vineField->cultivar->parameterBindiMiglietta.f);
    }

    if (myDoy < 260)
    {
        leafNumberRate = maxValue(0,(parameterBindiMigliettaFix.a + parameterBindiMigliettaFix.b * meanDailyTemperature) * (1 + parameterBindiMigliettaFix.c * this->statePlant.stateGrowth.shootLeafNumber));
    }
    else leafNumberRate = 0;

    if((this->statePlant.statePheno.stage >= veraison) && (this->statePlant.statePheno.stage <= physiologicalMaturity))
    {
        leafNumberRate *= 1 - (vineField->cultivar->parameterBindiMiglietta.fruitBiomassOffset + this->statePlant.stateGrowth.fruitBiomassIndex * this->statePlant.statePheno.daysAfterBloom);
    }

    this->statePlant.stateGrowth.shootLeafNumber += leafNumberRate;
    shootLeafArea = vineField->cultivar->parameterBindiMiglietta.d * pow(this->statePlant.stateGrowth.shootLeafNumber, vineField->cultivar->parameterBindiMiglietta.f) ;

    // water unstressed leaf area index
    this->statePlant.stateGrowth.leafAreaIndex = shootLeafArea * vineField->shootsPerPlant * vineField->plantDensity / parameterBindiMigliettaFix.shadedSurface;
    if (laiOld != NODATA)
    {
        deltaLai = maxValue(0,this->statePlant.stateGrowth.leafAreaIndex-laiOld);
        deltaLai *= Vine3D_Grapevine::getLaiStressCoefficient();
        this->statePlant.stateGrowth.leafAreaIndex = laiOld + deltaLai;
        this->statePlant.stateGrowth.leafAreaIndex = minValue(6.0,this->statePlant.stateGrowth.leafAreaIndex);
    }

    if (this->statePlant.statePheno.stage >= physiologicalMaturity)
    {
        int deltaDoy = 320 - myDoy;  // 15 nov: laimin
        this->statePlant.stateGrowth.leafAreaIndex *= (1.0 - 1.0/deltaDoy);
        this->statePlant.stateGrowth.leafAreaIndex = maxValue(this->statePlant.stateGrowth.leafAreaIndex, LAIMIN);
    }
    else if ((this->statePlant.statePheno.stage < physiologicalMaturity) && (myDoy > 273))
    {
        int deltaDoy = 320 - myDoy;  // 15 nov: laimin
        this->statePlant.stateGrowth.leafAreaIndex *= (1.0 - 1.0/deltaDoy);
        this->statePlant.stateGrowth.leafAreaIndex = maxValue(this->statePlant.stateGrowth.leafAreaIndex, LAIMIN);
    }

}

void Vine3D_Grapevine::getPotentialBrix()
{
    //potentialBrix = 28;
    if ((this->statePlant.statePheno.stage >=fruitSet) && (this->statePlant.statePheno.stage <=veraison))
    {
        this->statePlant.statePheno.cumulatedRadiationFromFruitsetToVeraison += maxValue(0,1e-6*(simulationStepInSeconds*myIrradiance));
        potentialBrix = 0.015*this->statePlant.statePheno.cumulatedRadiationFromFruitsetToVeraison + 5.71;
        //per sangiovese
        potentialBrix = 0.019*this->statePlant.statePheno.cumulatedRadiationFromFruitsetToVeraison;
        //potentialBrix =  minValue(31,potentialBrix);
    }
    else if (this->statePlant.statePheno.stage > veraison)
    {
        //per sangiovese
        potentialBrix = 0.019*this->statePlant.statePheno.cumulatedRadiationFromFruitsetToVeraison;
        //potentialBrix = 0.015*this->statePlant.statePheno.cumulatedRadiationFromFruitsetToVeraison + 5.71;
        potentialBrix = minValue(31,potentialBrix);
    }
    else if (this->statePlant.statePheno.stage < fruitSet)
    {
        this->statePlant.statePheno.cumulatedRadiationFromFruitsetToVeraison = 0;
        potentialBrix = NODATA;
    }

}

double Vine3D_Grapevine::getTartaricAcid()
{
    double tartrate = NODATA;
    double berryVolume;
    if (this->statePlant.statePheno.stage >= veraison)
    {
        berryVolume = gompertzDistribution((double)(this->statePlant.statePheno.stage-veraison+0.2));
        tartrate = 1.0/berryVolume;
    }
    return tartrate;
}

double Vine3D_Grapevine::gompertzDistribution(double stage)
{
    double a,b,c,output;
    a = 2.5; // asymptotic value at +infty
    b = log(a);
    c = -log(-log(0.76)/b);
    output = a*exp(-b*exp(-c*stage)) ;
    return output;
}

void Vine3D_Grapevine::fruitBiomassRateDecreaseDueToRainfall()
{
    double reduction = 0;
    if (myPrec > 0)
    {
        if ((this->statePlant.statePheno.stage >= 2.8) && (this->statePlant.statePheno.stage <= 4))
        {
            reduction = 5.*(1 / (1 + exp(-5.*(myPrec - 10)))); // percentage damage due to rainfall between flowering and fruitset
            this->statePlant.stateGrowth.fruitBiomassIndex *= 0.01*(100 - reduction) ;
        }

    }
}

double Vine3D_Grapevine::getRootDensity(int myLayer)
{
    if ((myLayer <= 0) || (myLayer >= this->soilLayersNr)) return 0.0;
    return this->layerRootDensity[myLayer];
}

