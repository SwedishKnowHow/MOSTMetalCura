/** Copyright (C) 2017 Yuenyong Nilsiam */
/** based on */
/** Copyright (C) 2013 David Braam - Released under terms of the AGPLv3 License */
#ifndef GCODEEXPORT_H
#define GCODEEXPORT_H

#include <stdio.h>
#include <deque> // for extrusionAmountAtPreviousRetractions

#include "settings.h"
#include "utils/intpoint.h"
#include "timeEstimate.h"

namespace cura {

class RetractionConfig
{
public:
    double amount; //!< The amount
    int speed;
    int primeSpeed;
    double primeAmount;
    int zHop;
};

//The GCodePathConfig is the configuration for moves/extrusion actions. This defines at which width the line is printed and at which speed.
class GCodePathConfig
{
private:
    int speed;
    int line_width;
    int flow;
    int layer_thickness;
    double extrusion_mm3_per_mm;
public:
    const char* name;
    bool spiralize;
    RetractionConfig* retraction_config;

    GCodePathConfig() : speed(0), line_width(0), extrusion_mm3_per_mm(0.0), name(nullptr), spiralize(false), retraction_config(nullptr) {}
    GCodePathConfig(RetractionConfig* retraction_config, const char* name) : speed(0), line_width(0), extrusion_mm3_per_mm(0.0), name(name), spiralize(false), retraction_config(retraction_config) {}

    void setSpeed(int speed)
    {
        this->speed = speed;
    }

    void setLineWidth(int line_width)
    {
        this->line_width = line_width;
        calculateExtrusion();
    }

    void setLayerHeight(int layer_height)
    {
        this->layer_thickness = layer_height;
        calculateExtrusion();
    }

    void setFlow(int flow)
    {
        this->flow = flow;
        calculateExtrusion();
    }

    void smoothSpeed(int min_speed, int layer_nr, int max_speed_layer)
    {
        speed = (speed*layer_nr)/max_speed_layer + (min_speed*(max_speed_layer-layer_nr)/max_speed_layer);
    }

    double getExtrusionMM3perMM()
    {
        return extrusion_mm3_per_mm;
    }

    int getSpeed()
    {
        return speed;
    }

    int getLineWidth()
    {
        return line_width;
    }

private:
    void calculateExtrusion()
    {
        extrusion_mm3_per_mm = INT2MM(line_width) * INT2MM(layer_thickness) * double(flow) / 100.0;
    }
};

//The GCodeExport class writes the actual GCode. This is the only class that knows how GCode looks and feels.
//  Any customizations on GCodes flavors are done in this class.
class GCodeExport
{
private:
    std::ostream* output_stream;
    double extrusion_amount; // in mm or mm^3
    double extruderSwitchRetraction;
    int extruderSwitchRetractionSpeed;
    int extruderSwitchPrimeSpeed;
    double retraction_extrusion_window;
    int retraction_count_max;
    std::deque<double> extrusion_amount_at_previous_n_retractions; // in mm or mm^3
    Point3 currentPosition;
    Point3 startPosition;
    Point extruderOffset[MAX_EXTRUDERS];
    char extruderCharacter[MAX_EXTRUDERS];
    int currentTemperature[MAX_EXTRUDERS];
    int currentSpeed;
    int zPos;
    bool isRetracted;
    bool isZHopped;
    int retractionPrimeSpeed;
    int current_extruder;
    int currentFanSpeed;
    EGCodeFlavor flavor;
    std::string preSwitchExtruderCode[MAX_EXTRUDERS];
    std::string postSwitchExtruderCode[MAX_EXTRUDERS];

    double totalFilament[MAX_EXTRUDERS]; // in mm^3
    double filament_diameter[MAX_EXTRUDERS]; // in mm^3
    double totalPrintTime;
    TimeEstimateCalculator estimateCalculator;

    bool is_volumatric;
    //std::string welder_on = "G4 P0\nM42 P0 S1\n";//@ GCode to turn welder on
    //std::string welder_off = "G4 P0\nM42 P0 S0\n";//@ Gcode to turn welder off
    std::string welder_on;//@ GCode to turn welder on
    std::string welder_off;//@ GCode to turn welder off
    double min_dist_welder_off; //@ minimum distance to move with welder off, unit mm
    bool isWelding; //@ true = welder is on, false = welder is off
    bool isMetalPrinting; //@ true = metal printing, false = not metal printing
public:

    GCodeExport();
    ~GCodeExport();

    void setOutputStream(std::ostream* stream);

    void setExtruderOffset(int id, Point p);
    Point getExtruderOffset(int id);
    void setSwitchExtruderCode(int id, std::string preSwitchExtruderCode, std::string postSwitchExtruderCode);

    void setFlavor(EGCodeFlavor flavor);
    EGCodeFlavor getFlavor();

    void setRetractionSettings(int extruderSwitchRetraction, int extruderSwitchRetractionSpeed, int extruderSwitchPrimeSpeed, int minimalExtrusionBeforeRetraction, int retraction_count_max);

    void setZ(int z);

    Point3 getPosition();

    Point getPositionXY();

    void resetStartPosition();

    Point getStartPositionXY();

    int getPositionZ();

    int getExtruderNr();

    void setFilamentDiameter(unsigned int n, int diameter);
    double getFilamentArea(unsigned int extruder);

    double getExtrusionAmountMM3(unsigned int extruder);

    double getTotalFilamentUsed(int e);

    double getTotalPrintTime();
    void updateTotalPrintTime();
    void resetTotalPrintTimeAndFilament();

    void writeComment(std::string comment);
    void writeTypeComment(const char* type);
    void writeLayerComment(int layer_nr);

    void writeLine(const char* line);

    void resetExtrusionValue();

    void writeDelay(double timeAmount);

    void writeMove(Point p, int speed, double extrusion_per_mm);

    void writeMove(Point3 p, int speed, double extrusion_per_mm);
private:
    void writeMove(int x, int y, int z, int speed, double extrusion_per_mm);
public:
    void writeRetraction(RetractionConfig* config, bool force=false);

    void switchExtruder(int newExtruder);

    void writeCode(const char* str);

    void writeFanCommand(int speed);

    void writeTemperatureCommand(int extruder, int temperature, bool wait = false);
    void writeBedTemperatureCommand(int temperature, bool wait = false);

    void finalize(int maxObjectHeight, int moveSpeed, const char* endCode);
    void setWelderOn(std::string welder_on_gcode);
    void setWelderOff(std::string welder_off_gcode);
    void setMinDistWelderOff(double machine_min_dist_welder_off);
    void setIsMetalPrinting(bool machine_metal_printing);
    void setIsWelding(bool is_welding);
};

}

#endif//GCODEEXPORT_H
