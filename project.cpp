/**
 * Author: Daniel Dusek, xdusek21
 * Brno, University of Technology
 * BMS class of 2017/2018, Project #1
 */

#define ANTENNA_CORRECTION_FACTOR -0.749018
#define FREQUENCE_OF_TRANSMISSION 900

#include <iostream>
#include <stdlib.h>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <algorithm>
#include <math.h>
#include "project.h"

void processParameters(int argc, char *argv[]);
void terminateExecutionWith(int exitCode, std::string message = "");
void writeOutputFile(/* DATA PLACEHOLDER */);
void loadBTSRecords();
void loadNearestStations();
std::vector<T_MatchedStation> prepareMatchingStation(std::vector<T_NearestStation> nearbyStations, std::vector<T_Station> allStations); 
T_GPS convertStringGPS(std::string GPS);
double getDegreesOnly(double degrees, double minutes, double seconds);
double calculateDistanceToStation(double antennaCorrectionFactor, double transmissionFrequency, double mobileAntenaHeight);

double helper_calculateAntennaCorrectionFactor(double transmissionFrequency, double mobileAntennaHeight);
// Input file name
std::string csvFile;
std::string btsCsvFile = "bts.csv";

std::vector<T_NearestStation> nearestStations;
std::vector<T_Station> AllStations;

// TODO: Rewrite functions so they are not unnecessary void
// TODO: Get rid of global variables
// TODO: Improve error handling within main || introduce exceptions
// TODO: Consider reading signal, antH and power as double. Or will it always be int?!
// TODO: Move function definitions to header file

int main(int argc, char *argv[])
{
    // TODO: Polish this to use return values/exception handling
    processParameters(argc, argv);
    loadNearestStations();
    loadBTSRecords();

    std::vector<T_MatchedStation> matchingStations = prepareMatchingStation(nearestStations, AllStations);

    /* DEBUG Returned matching stations */
    int counter = 0;
    for(std::vector<T_MatchedStation>::iterator it = matchingStations.begin(); it != matchingStations.end(); ++it)
    {
        std::cout << "Relevant station " << std::to_string(counter)  << " CID " << it->cid << " and gps for the doubtful " << it->GPS << "\n";
        std::cout << "\tGPS struct: latitude=" << std::to_string(it->GPSCords.latitude) << ", longitude=" << std::to_string(it->GPSCords.longitude) << "\n";
        std::cout << "\tDistance: " << std::to_string(it->distance) << "\n";
        counter++;
    }

    // TODO: Compute your location
    // TODO: Generate Google Map link
    // TODO: Write output to file "out.txt"
    writeOutputFile();

    return 0;   
}


/**
 * Calculates Antenna Correction Factor.
 *
 * Based on transmission frequency and mobile antenna height calculates antenna
 * correction factor. This function was used to calculate the AFC at the 
 * beginning and result was used as a constant ANTENNA_CORRECTION_FACTOR. 
 *
 * Magical constants used come from the Wikipedia formula, to be found here:
 * https://en.wikipedia.org/wiki/Hata_model
 *
 * double transmissionFrequency Transmission frequency in MHz.
 * double mobileAntennaHeight Mobile station antena length, in meters.
 *
 * return double Antenna Correction Factor.
 */
double helper_calculateAntennaCorrectionFactor(double transmissionFrequency, double mobileAntennaHeight)
{
    // Logarithm with a base of 10 precalculated for optimization purposes 
    double log10Freq = log10(transmissionFrequency);
    return (0.8 + ((1.1 * log10Freq - 0.7) * mobileAntennaHeight) - 1.56 * log10Freq);
}


/**
 * Calculates distance of user equipment from the station.
 *
 * Based on antenna height, power and signal calculates distance of user 
 * equipment to the station.
 *
 * double antennaHeight Specified in meters.
 * double power Power transmitted in dB (needs to be converted to dBm).
 * double signal Power received in dBm.
 *
 * return double Distance to the station in kilometers.
 */
double calculateDistanceToStation(double antennaHeight, double power, double signal)
{
    // Precalculate logarithm to avoid repeated log10 function call
    double log10AntennaHeight = log10(antennaHeight);

    // dBm conversion
    double powerTransmitted = 10 * log10((power * 1000));
    
    // Pathloss (Lu) = Power transmitted - power received
    double pathLoss = (powerTransmitted - signal); 

    // Distance calculation
    double exponent = (double) (-69.55 - (26.16 * log10(FREQUENCE_OF_TRANSMISSION)) + (13.82 * log10AntennaHeight) + ANTENNA_CORRECTION_FACTOR + pathLoss) / (44.9-(6.55*log10AntennaHeight));
    double distance = pow(10, exponent);

    return distance;
}


/**
 * Prepares vector of station information required for location determination.
 *
 * Loops over all stations and tries to match them with nearest stations, while
 * converting GPS from string to its double representation and calculating 
 * distance from station to user equipment.
 *
 * std::vector<T_NearestStation> nearbyStation Vector of all nearby stations.
 * std::vector<T_Station> allStations Vector of all station records.
 *
 * return std::vector<T_MatchedStation> Vector of all relevant stations.
 */
std::vector<T_MatchedStation> prepareMatchingStation(std::vector<T_NearestStation> nearbyStations, std::vector<T_Station> allStations)
{
    std::vector<T_MatchedStation> relevantStations;
    
    // Iterate over all stations & match them wih nearbyStations 
    for (std::vector<T_Station>::iterator it = allStations.begin(); it != allStations.end(); ++it)
    {
        for (std::vector<T_NearestStation>::iterator nsIt = nearbyStations.begin(); nsIt != nearbyStations.end(); ++nsIt)
        {
            // Filter only matching stations, calculate its distance from user equipment
            if (nsIt->lac == it->lac && nsIt->cid == it->cid)
            {
                T_MatchedStation newStation;
                newStation.cid = it->cid;
                newStation.lac = it->lac;
                newStation.GPS = it->GPS;
                newStation.GPSCords = convertStringGPS(it->GPS);
                newStation.distance = calculateDistanceToStation((double) nsIt->antH, (double) nsIt->power, (double) nsIt->signal);

                relevantStations.push_back(newStation);
            }
        }
    }

    return relevantStations;
}


/**
 * Parses GPS coordinates from string to T_GPS structure.
 *
 * std::string GPS String representation with degrees, minutes and seconds (E,N)
 *
 * return T_GPS Structure representing GPS as two double values.
 */
T_GPS convertStringGPS(std::string GPS)
{
    double degreesN, degreesE, minutesN, minutesE, secondsN, secondsE;
    std::stringstream ss;
    ss.str(GPS);

    ss >> degreesN;
    ss.get();
    ss >> minutesN;
    ss.get();
    ss >> secondsN;
    ss.get();

    // Get rid of N & comma
    ss.get();
    ss.get();

    ss >> degreesE;
    ss.get();
    ss >> minutesE;
    ss.get();
    ss >> secondsE;

    // Convert to double and create T_GPS structure
    T_GPS coordinates;
    coordinates.latitude = getDegreesOnly(degreesN, minutesN, secondsN);
    coordinates.longitude = getDegreesOnly(degreesE, minutesE, secondsE);

    return coordinates;
}


/**
 * Merges degrees, minutes and seconds into one double value.
 *
 * double degrees Degrees part of GPS location.
 * double minutes Minutes pars of GPS location.
 * double seconds Seconds part of GPS location.
 *
 * return double Single double value representing GPS location.
 */
double getDegreesOnly(double degrees, double minutes, double seconds)
{
    double result = degrees;
    result += (double) minutes/60;
    result += (double) seconds/3600;

    return result;
}


/**
 * Loads BTS records from BTS.csv input file.
 * TODO: Make this return vector of records.
 * TODO: Remove debug commented code.
 * TODO: Do not store values that are not used.
 */
void loadBTSRecords()
{
    std::ifstream file(btsCsvFile);

    if (!file.is_open())
    {
        // TODO: Terminate with proper code
        std::cerr << "Unable to open input csv file.\n";
        exit(11);
    }

    std::string lineValue;
    bool skip = true;
    while (getline(file, lineValue))
    {
        // Omit first line as it contains file headers
        if (skip)
        {
            skip = !skip;
            continue;
        }

        std::istringstream iis(lineValue);
        std::string token;
        int lineOffset = 0;
        T_Station station;

        // Process values 
        while (getline(iis, token, ';'))
        {
            switch (lineOffset)
            {
                case 0:
                    station.cid = atoi(token.c_str());
                break;
                case 1:
                    station.lac = atoi(token.c_str());
                break;
                case 2:
                    station.bch = atoi(token.c_str());
                break;
                case 3:
                    // Do not store localization information as there is no use for it
                break;
                case 4:
                    station.GPS = token;    
                    // Calculate decimal values for GPS
                break;
            }

            if (lineOffset != 4)
            {
                lineOffset++;
            }
            else 
            {
                lineOffset = 0;
            }
        }

        AllStations.push_back(station);
    }
}


/**
 * Load records from input bts file.
 *
 * TODO: Remove dependency on global data structures
 * TODO: Make it return vector of T_NearestStation
 */
void loadNearestStations()
{
    std::ifstream file(csvFile);

    if(!file.is_open())
    {
        // TODO Terminate with proper code
        std::cerr << "Unable to open input csv file.\n";
        exit(11);
    }

    std::string lineValue;
    bool skip = true;
    while (getline(file, lineValue))
    {

        // Omit first line as it contains file headers
        if (skip)
        {
            skip = !skip;
            continue;
        }

        // Parse line values and store them
        std::istringstream iss(lineValue);
        std::string token;
        int lineOffset = 0;
        T_NearestStation station;
        
        while (getline(iss, token, ';'))
        {
            switch (lineOffset)
            {
                // LAC
                case 0:
                    station.lac = atoi(token.c_str());
                break;
                // CID
                case 1:
                    station.cid = atoi(token.c_str());
                break;
                // RSSI
                case 2:
                    station.rssi = atoi(token.c_str()); // TODO: Maybe omit this as it is not used (yet)
                break;
                // Signal
                case 3:
                    station.signal = atoi(token.c_str());
                break;
                // Ant H
                case 4:
                    station.antH = atoi(token.c_str());
                break;
                // Power
                case 5:
                    station.power = atoi(token.c_str());
                break;
            }

            // Keep track of currently processed csv cell
            if (lineOffset != 5)
            {
                lineOffset++;
            }
            else 
            {
                lineOffset = 0;
            }
        }
        
        nearestStations.push_back(station);
    }

    // Debug dump nearest stations
    /*std::cout << "Iterating over nearest stations structure \n";
    for (std::vector<T_NearestStation>::iterator it = nearestStations.begin(); it != nearestStations.end(); ++it)
    {
        std::cout << "LAC: " << std::to_string(it->lac) << " CID: " << std::to_string(it->cid) << " RSSI: " << std::to_string(it->rssi);
        std::cout << " SIGNAL:"  << std::to_string(it->signal) << " ANTH: " << std::to_string(it->antH) << " Power:" << std::to_string(it->power);
        std::cout << "\n";
    }*/
}


/**
 * Processes commandline parameters passed to the application.
 *
 * int argc Number of parameters with which the application was called.
 * char** argv Array of parameters provided on input.
 */
void processParameters(int argc, char *argv[])
{
    // Terminate execution if no path to input file is provided.
    if (argc < 2)
    {
        terminateExecutionWith(1, 
            "Please specify input file as the first parameter.");
    }

    csvFile = std::string(argv[1]);
}


/**
 * Terminates program execution.
 *
 * Prints string message on stderr and terminates program execution with 
 * given exit code.
 *
 * int exitCode Numeric exit code with which program terminates.
 * std::string message Message displayed before termination (may be empty).
 */
void terminateExecutionWith(int exitCode, std::string message)
{
    if (!message.empty())
    {
        std::cerr << message << "\n";
    }

    // TODO: If necessary, execute cleanup here. 

    exit(exitCode);
}

/**
 *
 */
void writeOutputFile(/* DATA PLACEHOLDER */)
{
    std::string data = "Dummy placeholder data.";

    std::ofstream outFile;
    outFile.open("out.txt");
    outFile << data;
    outFile.close(); 
}

// TODO: If this gets implemented, work in header commentary
void showHelp()
{
    // TODO: Implement this (or not)
}