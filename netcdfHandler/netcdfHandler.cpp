#include <iostream>
#include <sstream>
#include <vector>

#include <netcdf.h>

#include "commonConstants.h"
#include "netcdfHandler.h"

using namespace std;


namespace NetCDF
{
    bool provaNetCDF(string fileName, stringstream *buffer)
    {
        int ncId, retval;
        char name[NC_MAX_NAME+1];
        char varName[NC_MAX_NAME+1];
        char typeName[NC_MAX_NAME+1];
        char *valueStr;
        int valueInt;
        double value;
        size_t lenght;
        nc_type ncTypeId;
        int idDimX, idDimY, nrX, nrY;
        idDimX = NODATA;
        idDimY = NODATA;
        string nameDimX, nameDimY;

        //NC_NOWRITE tells netCDF we want read-only access
        if ((retval = nc_open(fileName.data(), NC_NOWRITE, &ncId)))
        {
            *buffer << nc_strerror(retval) << endl;
            return false;
        }

        // NC_INQ tells how many netCDF dimensions, variables and global attributes are in
        // the file, also the dimension id of the unlimited dimension, if there is one.
        int nrDimensions, nrVariables, nrGlobalAttributes, unlimDimensionId;
        if ((retval = nc_inq(ncId, &nrDimensions, &nrVariables, &nrGlobalAttributes, &unlimDimensionId)))
        {
            *buffer << nc_strerror(retval) << endl;
            return false;
        }

        *buffer << fileName << endl << endl;
        *buffer << "Global attributes:" << endl;
        for (int a = 0; a <nrGlobalAttributes; a++)
        {
           nc_inq_attname(ncId, NC_GLOBAL, a, name);
           nc_inq_attlen(ncId, NC_GLOBAL, name, &lenght);
           valueStr = (char *) calloc(lenght +1, sizeof(char));
           nc_get_att_text(ncId, NC_GLOBAL, name, valueStr);

           *buffer << name << " = " << valueStr << endl;
       }

       int varDimIds[NC_MAX_VAR_DIMS];
       int nrVarDimensions, nrVarAttributes;

       *buffer << "\nDimensions: " << endl;
       for (int i = 0; i < nrDimensions; i++)
       {
           nc_inq_dim(ncId, i, name, &lenght);

           if (string(name) == "x" || string(name) == "lon" || string(name) == "longitude")
           {
               nrX = int(lenght);
               nameDimX = string(name);
           }
           if (string(name) == "y" || string(name) == "lat" || string(name) == "latitude")
           {
               nrY = int(lenght);
               nameDimY = string(name);
           }

           *buffer << i << " - " << name << "\t values: " << lenght << endl;
       }

       *buffer <<"\n(x,y) = "<<nrX << "," <<nrY << endl;

       *buffer << "\nVariables: " << endl;
       for (int v = 0; v < nrVariables; v++)
       {
           nc_inq_var(ncId, v, varName, &ncTypeId, &nrVarDimensions, varDimIds, &nrVarAttributes);
           nc_inq_type(ncId, ncTypeId, typeName, &lenght);

           if (string(varName) == nameDimX)
               idDimX = v;
           if (string(varName) == nameDimY)
               idDimY = v;

           *buffer << endl << v << " - " << varName << "\t type: " << typeName << "\t dims: ";
           for (int d = 0; d < nrVarDimensions; d++)
           {
               nc_inq_dim(ncId, varDimIds[d], name, &lenght);
               *buffer << name << " ";
           }
           *buffer << endl;

           for (int a = 0; a < nrVarAttributes; a++)
           {
                nc_inq_attname(ncId, v, a, name);
                nc_inq_atttype(ncId, v, name, &ncTypeId);
                if (ncTypeId == NC_CHAR)
                {
                    nc_inq_attlen(ncId, v, name, &lenght);
                    valueStr = (char *) calloc(lenght +1, sizeof(char));
                    nc_get_att_text(ncId, v, name, valueStr);
                    *buffer << name << " = " << valueStr << endl;
                }
                else if (ncTypeId == NC_INT)
                {
                    nc_get_att(ncId, v, name, &valueInt);
                    *buffer << name << " = " << valueInt << endl;
                }
                else if (ncTypeId == NC_DOUBLE)
                {
                    nc_get_att(ncId, v, name, &value);
                    *buffer << name << " = " << value << endl;
                }
            }
        }

        if (idDimX != NODATA)
        {
            float* x = (float*) calloc(nrX, sizeof(float));
            if (retval = nc_get_var_float(ncId, idDimX, x))
            {
                *buffer << "\nERROR in reading var: " << nameDimX << " - " << nc_strerror(retval);
                nc_close(ncId);
                return false;
            }
        }

        if (idDimY != NODATA)
        {
            float* y = (float*) calloc(nrY, sizeof(float));
            if (retval = nc_get_var_float(ncId, idDimY, y))
            {
                *buffer << "\nERROR in reading var: " << nameDimY << " - " << nc_strerror(retval);
                nc_close(ncId);
                return false;
            }
        }

        if (nameDimX == "x")
        {
            float* lat = (float*) calloc(nrY*nrX, sizeof(float));
            float* lon = (float*) calloc(nrY*nrX, sizeof(float));

            if (retval = nc_get_var_float(ncId, 4, lon))
                *buffer << "\nERROR in reading longitude:" << nc_strerror(retval);

            if (retval = nc_get_var_float(ncId, 5, lat))
                *buffer << "\nERROR in reading latitude:" << nc_strerror(retval);

            *buffer << endl;
            for (int row = 0; row < nrY; row+=2)
            {
                *buffer << lat[row*nrX+row] << ",";
                *buffer << lon[row*nrX+row] << "  ";
            }
            *buffer << endl;
        }

        // CLOSE file, freeing all resources
        if ((retval = nc_close(ncId)))
            *buffer << nc_strerror(retval) << endl;

       return true;
    }

}