/*!
    CRITERIA3D

    \copyright 2016 Fausto Tomei, Gabriele Antolini,
    Alberto Pistocchi, Marco Bittelli, Antonio Volta, Laura Costantini

    You should have received a copy of the GNU General Public License
    along with Nome-Programma.  If not, see <http://www.gnu.org/licenses/>.

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
    fausto.tomei@gmail.com
    ftomei@arpae.it
*/

#include <math.h>

#include "crit3dDate.h"
#include "commonConstants.h"


Crit3DTime::Crit3DTime()
{
    time = 0.; date.day = 0; date.month = 0; date.year = 0;
}

Crit3DTime::Crit3DTime(Crit3DDate myDate, float mySeconds)
{
    date = myDate;
    time = 0;
    *this = this->addSeconds(mySeconds);
}

int Crit3DTime::getHour() const
{
    return int(time / 3600);
}

int Crit3DTime::getMinutes() const
{
    return int((time - getHour()*3600) / 60);
}

float Crit3DTime::getSeconds() const
{
    return (time - getHour()*3600 - getMinutes()*60);
}

bool operator < (const Crit3DTime& time1, const Crit3DTime& time2)
{
    return time1.date < time2.date ||
            (time1.date == time2.date && time1.time < time2.time);
}

bool operator > (const Crit3DTime& time1, const Crit3DTime& time2)
{
    return time1.date > time2.date ||
            (time1.date == time2.date && time1.time > time2.time);
}

bool operator <= (const Crit3DTime& time1, const Crit3DTime& time2)
{
    return (time1 < time2 || time1.isEqual(time2));
}

bool operator >= (const Crit3DTime& time1, const Crit3DTime& time2)
{
    return (time1 > time2 || time1.isEqual(time2));
}

bool operator == (const Crit3DTime& time1, const Crit3DTime& time2)
{
    return (time1.isEqual(time2));
}

Crit3DTime getNullTime()
{
    Crit3DTime* nullTime = new Crit3DTime();
    return (*nullTime);
}

bool Crit3DTime::isEqual(const Crit3DTime& myTime) const
{
    return ((this->date == myTime.date) && (int(this->time) == int(myTime.time)));
}

Crit3DTime Crit3DTime::addSeconds(float mySeconds) const
{
    Crit3DTime myTime = *this;
    myTime.time += mySeconds;

    while (!((myTime.time >= 0.f) && (myTime.time < float(DAY_SECONDS))))
    {
        if (myTime.time >= float(DAY_SECONDS))
        {
            ++(myTime.date);
            myTime.time -= DAY_SECONDS;
        }
        else if (myTime.time < 0)
        {
            --(myTime.date);
            myTime.time += DAY_SECONDS;
        }
    }
    return myTime;
}

std::string Crit3DTime::toStdString()
{
    char myStr[17];
    sprintf (myStr, "%d-%02d-%02d %02d:%02d", this->date.year, this->date.month, this->date.day, this->getHour(), this->getMinutes());

    return std::string(myStr);
}
