# CRITERIA3D distribution
CRITERIA3D distribution provides a set of libraries and executables for agro-meteorological analysis.
It includes a numerical solution for three-dimensional water and heat flow in the soil, water balance, meteorological data interpolation, daily weather generator, radiation budget, snow accumulation and melt, crop development and crop water uptake.

Executable files are named with upper case, libraries with lower case. QT libraries (5.7 or following) are needed.

## soilFluxes3D library
SoilFluxes3D is a numerical solution for flow equations of water and heat in the soil, in a three-dimensional domain.
Surface water flow is described by the two-dimensional parabolic approximation of the St. Venant equation, using Manning’s equation of motion; subsurface water flow is described by the three-dimensional Richards’ equation for the unsaturated zone and by three-dimensional Darcy’s law for the saturated zone, using an integrated finite difference formulation.

Water fluxes equations may be coupled with the heat flux equations, which include diffusive, latent and advective terms. Atmospheric data (net irradiance, air temperature and relative humidity, wind speed) could be used as top boundary conditions.
http://www.sciencedirect.com/science/article/pii/S0309170809001754

## PRAGA (PRogram for AGrometeorological Analysis)
![](https://github.com/ARPA-SIMC/CRITERIA3D/blob/master/doc/img/PRAGA_screenshot.png)
PRAGA is a geographical application for agrometeorological and climate analysis.

Currently PRAGA imports only data from ARPAE-SIMC sources: [Arkimet](https://github.com/ARPA-SIMC/arkimet) for meteo points, ERG-5 (Emilia-Romagna Grid at 5x5km) as meteo grid.

The user interface of PRAGA uses a modified version of [MapGraphics](https://github.com/raptorswing/MapGraphics) library.

To compile it, firstly compile the project _MapGraphics_, then compile the project *Makeall_PRAGA*.

## TestCriteria1D
Test program for the 1D water balance using criteria1D, crop and soil libraries.

To compile it, use the project *Makeall_CRITERIA1D*. Example data are available in *DATA/Criteria1d* directory.

## HEAT1D
HEAT1D is a graphical interface for testing the soilFluxex3D library in a 1D domain. Users can set fixed or variable atmospheric boundary conditions and soil parameters. Output graph results include soil temperature, soil water content, heat fluxes (diffusive and latent), liquid (isothermal and thermal) and vapor (isothermal and thermal) water fluxes. 

Graphs are produced using the Qwt plot library (http://qwt.sourceforge.net/) which should be compiled before usage.

## criteria1D library
Algorithms for 1D water balance: soil water infiltration, drainage and capillary rise, crop water demand, evaporation and crop transpiration. 

## crop library
Algorithms for crop development, leaf area index, root growth and distribution, based on daily temperature.

## soil library
Modified Van Genuchten-Mualem model for soil water retention curve and water conductivity, USDA soil texture classification.

## solarRadiation library
Algorithms for potential/actual solar radiation computation.

## interpolation library
Algorithms for the spatialization of meteorological data.


# License
CRITERIA3D has been developed under contract issued by 
ARPAE Hydro-Meteo-Climate Service, Emilia-Romagna, Italy: http://www.arpae.it/sim/

CRITERIA3D is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by the Free Software Foundation, 
either version 3 of the License, or (at your option) any later version.

# Authors
**Fausto Tomei** <ftomei@arpae.it>

**Gabriele Antolini**	 <gantolini@arpae.it>

Laura Costantini  <laura.costantini0@gmail.com>

Antonio Volta		<antonio.volta@agromet.it>

## Contributions

Marco Bittelli   <marco.bittelli@unibo.it>

Vittorio Marletto <vmarletto@arpae.it>

Alberto Pistocchi	 <alberto.pistocchi@jrc.ec.europa.eu>

Tomaso Tonelli <ttonelli@arpae.it>

Margot Van Soetendaal <margot@farnet.eu>

Franco Zinoni <fzinoni@arpae.it>
