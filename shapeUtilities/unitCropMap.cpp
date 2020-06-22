#include "unitCropMap.h"
#include "zonalStatistic.h"
#include "shapeToRaster.h"
#include "shapeUtilities.h"
#include <QPolygon>
#include <QFile>
#include <QFileInfo>


bool computeUCMprevailing(Crit3DShapeHandler *ucm, Crit3DShapeHandler *crop, Crit3DShapeHandler *soil, Crit3DShapeHandler *meteo,
                 std::string idCrop, std::string idSoil, std::string idMeteo, double cellSize,
                 QString ucmFileName, std::string *error, bool showInfo)
{

    // make a copy of shapefile and return cloned shapefile complete path
    QString refFileName = QString::fromStdString(crop->getFilepath());
    QString ucmShapeFileName = cloneShapeFile(refFileName, ucmFileName);

    if (!ucm->open(ucmShapeFileName.toStdString()))
    {
        *error = "Load shapefile failed: " + ucmShapeFileName.toStdString();
        return false;
    }

    // create reference and value raster
    gis::Crit3DRasterGrid* rasterRef = new(gis::Crit3DRasterGrid);
    gis::Crit3DRasterGrid* rasterVal = new(gis::Crit3DRasterGrid);
    initializeRasterFromShape(ucm, rasterRef, cellSize);
    initializeRasterFromShape(ucm, rasterVal, cellSize);

    // ECM --> reference
    fillRasterWithShapeNumber(rasterRef, ucm, showInfo);

    // zonal statistic on meteo grid
    fillRasterWithShapeNumber(rasterVal, meteo, showInfo);
    bool isOk = zonalStatisticsShape(ucm, meteo, rasterRef, rasterVal, idMeteo, "ID_METEO", MAJORITY, error, showInfo);

    // zonal statistic on soil map
    if (isOk)
    {
        fillRasterWithShapeNumber(rasterVal, soil, showInfo);
        isOk = zonalStatisticsShape(ucm, soil, rasterRef, rasterVal, idSoil, "ID_SOIL", MAJORITY, error, showInfo);
    }

    if (! isOk)
    {
        *error = "ZonalStatisticsShape: " + *error;
    }

    delete rasterRef;
    delete rasterVal;
    if (! isOk) return false;

    // read indexes
    int nShape = ucm->getShapeCount();
    int cropIndex = ucm->getFieldPos(idCrop);
    int soilIndex = ucm->getFieldPos(idSoil);
    int meteoIndex = ucm->getFieldPos(idMeteo);

    // add ID CASE
    ucm->addField("ID_CASE", FTString, 20, 0);
    int idCaseIndex = ucm->getFieldPos("ID_CASE");

    // add ID CROP
    bool existIdCrop = ucm->existField("ID_CROP");
    if (! existIdCrop) ucm->addField("ID_CROP", FTString, 5, 0);
    int idCropIndex = ucm->getFieldPos("ID_CROP");

    // FILL ID_CROP and ID_CASE
    for (int shapeIndex = 0; shapeIndex < nShape; shapeIndex++)
    {
        std::string cropStr = ucm->readStringAttribute(shapeIndex, cropIndex);
        if (cropStr == "-9999") cropStr = "";

        std::string soilStr = ucm->readStringAttribute(shapeIndex, soilIndex);
        if (soilStr == "-9999") soilStr = "";

        std::string meteoStr = ucm->readStringAttribute(shapeIndex, meteoIndex);
        if (meteoStr == "-9999") meteoStr = "";

        std::string caseStr = "";
        if (meteoStr != "" && soilStr != "" && cropStr != "")
            caseStr = "M" + meteoStr + "S" + soilStr + "C" + cropStr;

        if (! existIdCrop) ucm->writeStringAttribute(shapeIndex, idCropIndex, cropStr.c_str());
        ucm->writeStringAttribute(shapeIndex, idCaseIndex, caseStr.c_str());

        if (caseStr == "")
            ucm->deleteRecord(shapeIndex);
    }

    cleanShapeFile(ucm);

    return isOk;
}

bool computeUCMintersection(Crit3DShapeHandler *ucm, Crit3DShapeHandler *crop, Crit3DShapeHandler *soil, Crit3DShapeHandler *meteo,
                 std::string idCrop, std::string idSoil, std::string idMeteo, double cellSize,
                 QString ucmFileName, std::string *error, bool showInfo)
{

    // PolygonShapefile
    int type = 2;

    ucm->newFile(ucmFileName.toStdString(), type);
    ucm->open(ucmFileName.toStdString());
    // add ID CASE
    ucm->addField("ID_CASE", FTString, 20, 0);
    // add ID SOIL
    ucm->addField("ID_SOIL", FTString, 5, 0);
    // add ID CROP
    ucm->addField("ID_CROP", FTString, 5, 0);
    // add ID METEO
    ucm->addField("ID_METEO", FTString, 5, 0);

    if (crop == nullptr)
    {
        // soil and meteo intersection, add constant idCrop
    }
    else if (soil == nullptr)
    {
        // crop and meteo intersection, add constant idSoil
    }
    else if (meteo == nullptr)
    {
        // crop and soil intersection, add constant idMeteo
    }
    else
    {
        // soil and meteo intersection, shape result and crop intersection
    }
    return true;
}

bool shapeIntersection(Crit3DShapeHandler *intersecHandler, Crit3DShapeHandler *firstHandler, Crit3DShapeHandler *secondHandler, std::string fieldNameFirst, std::string fieldNameSecond, std::string *error, bool showInfo)
{
    ShapeObject myFirstShape;
    ShapeObject mySecondShape;
    Box<double> firstBounds;
    Box<double> secondBounds;
    int nrFirstShape = firstHandler->getShapeCount();
    int nrSecondShape = secondHandler->getShapeCount();
    int nIntersections = 0;
    std::vector< std::vector<ShapeObject::Part>> firstShapeParts;
    std::vector< std::vector<ShapeObject::Part>> secondShapeParts;

    QPolygonF firstPolygon;
    QPolygonF secondPolygon;
    QPolygonF intersectionPolygon;

    int IDfirstShape = firstHandler->getFieldPos(fieldNameFirst);
    int IDsecondShape = secondHandler->getFieldPos(fieldNameSecond);
    int IDCloneFirst = intersecHandler->getFieldPos(fieldNameFirst);
    int IDCloneSecond = intersecHandler->getFieldPos(fieldNameSecond);

    for (unsigned int firstShapeIndex = 0; firstShapeIndex < nrFirstShape; firstShapeIndex++)
    {

        firstPolygon.clear();
        firstHandler->getShape(firstShapeIndex, myFirstShape);
        std::string fieldFirst = firstHandler->readStringAttribute(firstShapeIndex, IDfirstShape);  //Field to copy
        // get bounds
        firstBounds = myFirstShape.getBounds();
        firstShapeParts[firstShapeIndex] = myFirstShape.getParts();
        for (unsigned int partIndex = 0; partIndex < firstShapeParts[firstShapeIndex].size(); partIndex++)
        {
            Box<double> partBB = myFirstShape.getPart(partIndex).boundsPart;
            int offset = myFirstShape.getPart(partIndex).offset;
            int length = myFirstShape.getPart(partIndex).length;

            if (firstShapeParts[firstShapeIndex][partIndex].hole)
            {
                continue;
            }
            else
            {
                for (unsigned long v = 0; v < length; v++)
                {
                    Point<double> vertex = myFirstShape.getVertex(v+offset);
                    QPoint point(vertex.x, vertex.y);
                    firstPolygon.append(point);
                }
                // check holes TO DO
            }
        }
        for (unsigned int secondShapeIndex = 0; secondShapeIndex < nrSecondShape; secondShapeIndex++)
        {

            secondPolygon.clear();
            secondHandler->getShape(secondShapeIndex, mySecondShape);
            std::string fieldSecond = secondHandler->readStringAttribute(secondShapeIndex, IDsecondShape); //Field to copy
            // get bounds
            secondBounds = mySecondShape.getBounds();
            bool noOverlap = firstBounds.xmin > secondBounds.xmax ||
                                 secondBounds.xmin > firstBounds.xmax ||
                                 firstBounds.ymin > secondBounds.ymax ||
                                 secondBounds.ymin > firstBounds.ymax;
            if (noOverlap)
            {
                continue;
            }
            else
            {
                secondShapeParts[secondShapeIndex] = mySecondShape.getParts();
                for (unsigned int partIndex = 0; partIndex < secondShapeParts[secondShapeIndex].size(); partIndex++)
                {
                    Box<double> partBB = mySecondShape.getPart(partIndex).boundsPart;
                    int offset = mySecondShape.getPart(partIndex).offset;
                    int length = mySecondShape.getPart(partIndex).length;

                    if (secondShapeParts[secondShapeIndex][partIndex].hole)
                    {
                        continue;
                    }
                    else
                    {
                        nIntersections = nIntersections + 1;
                        for (unsigned long v = 0; v < length; v++)
                        {
                            Point<double> vertex = mySecondShape.getVertex(v+offset);
                            QPoint point(vertex.x, vertex.y);
                            secondPolygon.append(point);
                        }
                        // check holes TO DO
                    }
                }
                intersectionPolygon = firstPolygon.intersected(secondPolygon);
                std::vector<double> coordinates;
                for (int i = 0; i<intersectionPolygon.size(); i++)
                {
                    coordinates.push_back(intersectionPolygon[i].x());
                    coordinates.push_back(intersectionPolygon[i].y());
                }
                intersecHandler->addShape(coordinates);
                intersecHandler->writeStringAttribute(nIntersections, IDCloneFirst, fieldFirst.c_str());
                intersecHandler->writeStringAttribute(nIntersections, IDCloneSecond, fieldSecond.c_str());
            }
        }
    }

    return true;
}

