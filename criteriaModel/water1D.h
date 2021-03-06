#ifndef WATER1D_H
#define WATER1D_H

    #include <vector>

    #ifndef SOIL_H
        #include "soil.h"
    #endif

    #define MAX_EVAPORATION_DEPTH 0.15

    class Crit3DCrop;

    void initializeWater(std::vector<soil::Crit3DLayer> &soilLayers);

    double computeInfiltration(std::vector<soil::Crit3DLayer> &soilLayers, double inputWater, double ploughedSoilDepth);

    double computeEvaporation(std::vector<soil::Crit3DLayer> &soilLayers, double maxEvaporation);
    double computeSurfaceRunoff(const Crit3DCrop &myCrop, std::vector<soil::Crit3DLayer> &soilLayers);
    double computeLateralDrainage(std::vector<soil::Crit3DLayer> &soilLayers);
    double computeCapillaryRise(std::vector<soil::Crit3DLayer> &soilLayers, double waterTableDepth);

    double computeOptimalIrrigation(std::vector<soil::Crit3DLayer> &soilLayers, double irrigationMax);

    double getSoilWaterContent(const std::vector<soil::Crit3DLayer> &soilLayers);
    double getSoilWaterDeficit(const std::vector<soil::Crit3DLayer> &soilLayers);
    double getCropReadilyAvailableWater(const Crit3DCrop &myCrop, const std::vector<soil::Crit3DLayer> &soilLayers);


#endif // WATER1D_H

