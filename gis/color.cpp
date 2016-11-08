/*-----------------------------------------------------------------------------------

    CRITERIA 3D
    Copyright (C) 2011 Fausto Tomei, Gabriele Antolini, Antonio Volta
    Alberto Pistocchi, Marco Bittelli

    This file is part of CRITERIA3D.
    CRITERIA3D has been developed under contract issued by A.R.P.A. Emilia-Romagna

    CRITERIA3D is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    CRITERIA3D is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with CRITERIA3D.  If not, see <http://www.gnu.org/licenses/>.

    contacts:
    ftomei@arpa.emr.it
    fausto.tomei@gmail.com
    gantolini@arpa.emr.it
    avolta@arpa.emr.it
    alberto.pistocchi@gecosistema.it
    marco.bittelli@unibo.it
-----------------------------------------------------------------------------------*/

#include "commonConstants.h"
#include "gis.h"


namespace gis
{
    Crit3DColor::Crit3DColor()
    {
        red = 0;
        green = 0;
        blue = 0;
    }

    Crit3DColor::Crit3DColor(short myRed, short myGreen, short myBlue)
    {
        red = myRed;
        green = myGreen;
        blue = myBlue;
    }

    Crit3DColorScale::Crit3DColorScale()
    {
        nrKeyColors = 1;
        nrColors = 1;
        keyColor = new Crit3DColor();
        color = new Crit3DColor();
        minimum = NODATA;
        maximum = NODATA;
        classification = classificationMethod::EqualInterval;
    }

    bool Crit3DColorScale::classify()
    {
        int i, j, n, nrIntervals;
        float step, dRed, dGreen, dBlue;

        if (classification == classificationMethod::EqualInterval)
        {
            nrIntervals = nrKeyColors - 1;
            step = (float)nrColors / (float)nrIntervals;

            for (i = 0; i < nrIntervals; i++)
            {
                dRed = (keyColor[i+1].red - keyColor[i].red) / step;
                dGreen = (keyColor[i+1].green - keyColor[i].green) / step;
                dBlue = (keyColor[i+1].blue - keyColor[i].blue) / step;

                for (j = 0; j < step; j++)
                {
                    n = (int)step * i + j;
                    color[n].red = keyColor[i].red + (short)(dRed * j);
                    color[n].green = keyColor[i].green + (short)(dGreen * j);
                    color[n].blue = keyColor[i].blue + (short)(dBlue * j);
                }
            }
            color[nrColors-1] = keyColor[nrKeyColors -1];
        }

        return (true);
    }


    Crit3DColor* Crit3DColorScale::getColor(float myValue)
    {
        int myIndex = 0;

        if (classification == classificationMethod::EqualInterval)
        {
            myIndex = (int)((nrColors-1) * ((myValue - minimum) / (maximum - minimum)));
        }

        return(&color[myIndex]);
    }


    bool setDefaultDTMScale(Crit3DColorScale* myScale)
    {
        myScale->nrKeyColors = 4;
        myScale->nrColors = 256;
        myScale->keyColor = new Crit3DColor[myScale->nrKeyColors];
        myScale->color = new Crit3DColor[myScale->nrColors];
        myScale->classification = classificationMethod::EqualInterval;

        myScale->keyColor[0] = Crit3DColor(32, 128, 16);         //green
        myScale->keyColor[1] = Crit3DColor(255, 196, 18);        //yellow
        myScale->keyColor[2] = Crit3DColor(110, 64, 18);         //brown
        myScale->keyColor[3] = Crit3DColor(200, 200, 200);       //lightgrey

        return(myScale->classify());
    }

    bool setTemperatureScale(Crit3DColorScale* myScale)
    {
        myScale->nrKeyColors = 3;
        myScale->nrColors = 256;
        myScale->keyColor = new Crit3DColor[myScale->nrKeyColors];
        myScale->color = new Crit3DColor[myScale->nrColors];
        myScale->classification = classificationMethod::EqualInterval;

        myScale->keyColor[0] = Crit3DColor(0, 0, 255);         //blue
        myScale->keyColor[1] = Crit3DColor(255, 255, 0);       //yellow
        myScale->keyColor[2] = Crit3DColor(255, 0, 0);         //red

        return(myScale->classify());
    }

    bool setPrecipitationScale(Crit3DColorScale* myScale)
    {
        myScale->nrKeyColors = 3;
        myScale->nrColors = 256;
        myScale->keyColor = new Crit3DColor[myScale->nrKeyColors];
        myScale->color = new Crit3DColor[myScale->nrColors];
        myScale->classification = classificationMethod::EqualInterval;

        myScale->keyColor[0] = Crit3DColor(255, 255, 255);
        myScale->keyColor[1] = Crit3DColor(0, 255, 255);
        myScale->keyColor[2] = Crit3DColor(0, 0, 255);

        return(myScale->classify());
    }

    bool setRelativeHumidityScale(Crit3DColorScale* myScale)
    {
        myScale->nrKeyColors = 3;
        myScale->nrColors = 256;
        myScale->keyColor = new Crit3DColor[myScale->nrKeyColors];
        myScale->color = new Crit3DColor[myScale->nrColors];
        myScale->classification = classificationMethod::EqualInterval;

        myScale->keyColor[0] = Crit3DColor(128, 0, 0);
        myScale->keyColor[1] = Crit3DColor(255, 255, 0);
        myScale->keyColor[2] = Crit3DColor(0, 0, 128);

        return(myScale->classify());
    }

    bool setLeafWetnessScale(Crit3DColorScale* myScale)
    {
        myScale->nrKeyColors = 3;
        myScale->nrColors = 256;
        myScale->keyColor = new Crit3DColor[myScale->nrKeyColors];
        myScale->color = new Crit3DColor[myScale->nrColors];
        myScale->classification = classificationMethod::EqualInterval;

        myScale->keyColor[0] = Crit3DColor(128, 0, 0);
        myScale->keyColor[1] = Crit3DColor(255, 255, 0);
        myScale->keyColor[2] = Crit3DColor(0, 0, 128);

        return(myScale->classify());
    }

    bool setWindIntensityScale(Crit3DColorScale* myScale)
    {
        myScale->nrKeyColors = 2;
        myScale->nrColors = 256;
        myScale->keyColor = new Crit3DColor[myScale->nrKeyColors];
        myScale->color = new Crit3DColor[myScale->nrColors];
        myScale->classification = classificationMethod::EqualInterval;

        myScale->keyColor[0] = Crit3DColor(64, 255, 0);
        myScale->keyColor[1] = Crit3DColor(255, 64, 0);

        return(myScale->classify());
    }

    bool setRadiationScale(Crit3DColorScale* myScale)
    {
        myScale->nrKeyColors = 3;
        myScale->nrColors = 256;
        myScale->keyColor = new Crit3DColor[myScale->nrKeyColors];
        myScale->color = new Crit3DColor[myScale->nrColors];
        myScale->classification = classificationMethod::EqualInterval;

        myScale->keyColor[0] = Crit3DColor(0, 0, 128);
        myScale->keyColor[1] = Crit3DColor(255, 255, 0);
        myScale->keyColor[2] = Crit3DColor(255, 0, 0);

        return(myScale->classify());
    }

}

