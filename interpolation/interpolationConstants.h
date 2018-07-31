#ifndef INTERPOLATIONCONSTS_H
#define INTERPOLATIONCONSTS_H

#define MIN_REGRESSION_POINTS 3
#define PEARSONSTANDARDTHRESHOLD 0.2
#define PREC_JRC_THRESHOLD 0.5
#define PREC_THRESHOLD 0.2

#ifndef _STRING_
    #include <string>
#endif

#ifndef _MAP_
    #include <map>
#endif

#ifndef _VECTOR_
    #include <vector>
#endif

namespace interpolationMethod
{
    enum { idw, kriging, shepard };
}

enum TProxyVar { height, urbanFraction, orogIndex, seaDistance, aspect, slope, noProxy };

const std::map<std::string, TProxyVar> ProxyVarNames = {
  { "elevation", height },
  { "altitude", height },
  { "orography", height },
  { "orogIndex", orogIndex },
  { "urbanFraction", urbanFraction },
  { "seaDistance", seaDistance },
  { "aspect", aspect },
  { "slope", slope }
};


enum TkrigingMode {KRIGING_SPHERICAL = 1,
                   KRIGING_EXPONENTIAL=2,
                   KRIGING_GAUSSIAN=3,
                   KRIGING_LINEAR=4
                  };



#endif // INTERPOLATIONCONSTS_H
