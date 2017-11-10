/**
 * Author: Daniel Dusek, xdusek21
 * Brno, University of Technology
 * BMS class of 2017/2018, Project #1
 */

#include <iostream>
#include <stdlib.h>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include "project.h"

void processParameters(int argc, char *argv[]);
void terminateExecutionWith(int exitCode, std::string message = "");
void writeOutputFile(/* DATA PLACEHOLDER */);
/*void loadBTSRecords(uint16_t lac, uint16_t cid);*/
void loadBTSRecords();
void loadNearestStations();


// Input file name
std::string csvFile;
std::string btsCsvFile = "bts.csv";

std::vector<T_NearestStation> nearestStations;
std::vector<T_Station> AllStations;

int main(int argc, char *argv[])
{
    processParameters(argc, argv);

    // TODO: Store values from CSV in internal structure loadBTSInfo()
        // * Load data from input csv file
    loadNearestStations();
    loadBTSRecords();
        // * Validate input file (permissions) - validateCSVFile()
        // * loadBTSRecords()
        // * Possible exit-code 2: invalid input file / unable to read ...

    // TODO: Compute your location

    // TODO: Generate Google Map link

    // TODO: Write output to file "out.txt"
    writeOutputFile();



    // DEBUG: To be deleted once the application is ready.
    // std::cout << "Parameter count: " << std::to_string(argc) << "\n";
    // std::cout << "Input csv file: " << csvFile << "\n";
    return 0;   
}

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
                    station.localization = token;
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

    // Debug dump nearest stations
    std::cout << "Iterating over all stations structure \n";
    for (std::vector<T_Station>::iterator it = AllStations.begin(); it != AllStations.end(); ++it)
    {
        std::cout << "LAC: " << std::to_string(it->lac) << " CID: " << std::to_string(it->cid) << " BCH: " << std::to_string(it->bch);
        std::cout << " Localization:"  << it->localization << " GPS: " << it->GPS;
        std::cout << "\n";
    }
}


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

        std::istringstream iss(lineValue);
        std::string token;
        int lineOffset = 0;
        T_NearestStation station;
        
        // Process values
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
                    station.rssi = atoi(token.c_str());
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

            // std::cout << "Line offset: " << std::to_string(lineOffset) << " Token: " << token << " | ";

            if (lineOffset != 5)
            {
                lineOffset++;
            }
            else 
            {
                lineOffset = 0;
                // std::cout << "\n";
            }

        }

        
        nearestStations.push_back(station);
    }

    // Debug dump nearest stations
    std::cout << "Iterating over nearest stations structure \n";
    for (std::vector<T_NearestStation>::iterator it = nearestStations.begin(); it != nearestStations.end(); ++it)
    {
        std::cout << "LAC: " << std::to_string(it->lac) << " CID: " << std::to_string(it->cid) << " RSSI: " << std::to_string(it->rssi);
        std::cout << " SIGNAL:"  << std::to_string(it->signal) << " ANTH: " << std::to_string(it->antH) << " Power:" << std::to_string(it->power);
        std::cout << "\n";
    }
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






























/*void loadBTSRecords(uint16_t lac, uint16_t cid)
{
    std::ifstream file(btsCsvFile);

    if (!file.is_open())
    {
        // TODO Terminate with proper code
        std::cerr << "Unable to open BTS cvs file.\n";
        exit(11);
    }

    std::string lineValue;
    bool skip = true;
    while (getline(file, lineValue))
    {
        // Omit first line as it contains csv headers
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
            // 0th index is CID in BTS.csv, compare on match
            if (lineOffset == 0 && atoi(token.c_str()) == cid)
            {
                // If CID matches, check LAC, if it both matches, extract rest
                std::string rawLac; 
                getline(iis, rawLac, ';');
                if (atoi(rawLac.c_str()) == lac)
                {
                    // Both values match 
                    std::cout << "pLac: " << std::to_string(lac) << "_lac:" << token << " [MATCHES] \n";

                    // Extract remaining values and fill the structure
                }
                else
                {
                    // Not interesting record, rewind reader
                    std::string disposable;
                    getline(iis, disposable);
                    lineOffset = 0;
                }
            }
            else
            {
                // Not interesting record, rewind reader
                std::string disposable;
                getline(iis, disposable);
                lineOffset = 0;
            }
        }
    }
}*/
