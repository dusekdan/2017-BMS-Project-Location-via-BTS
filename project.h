/**
 * Author: Daniel Dusek, xdusek21
 * Brno, University of Technology
 * BMS class of 2017/2018, Project #1
 */

#define USER_EQUIPMENT_HEIGTH 1.2
#define ANTENNA_CORRECTION_FACTOR -0.749018
#define FREQUENCE_OF_TRANSMISSION 900
#define GOOGLE_MAPS_URL_BASE "maps.google.com/maps?q="
#define BMS_OUTPUT_FILE "out.txt"

#define EMPTY_STRING ""

#include <iostream>
#include <stdlib.h>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <algorithm>
#include <math.h>


/**
 * Joins latitude and longitude into one structure.
 */
typedef struct
{
	double latitude;
	double longitude;
} T_GPS;


/**
 * Represents station loaded from input *.csv file containing nearby stations
 */
typedef struct 
{
	uint16_t lac;
	uint16_t cid;
	int rssi;
	int signal;
	int antH;
	int power;
} T_NearestStation;


/**
 * Represents station loaded from BTS.csv file
 * TODO: Check whether loading bch is necessary (and remove if not)
 */
typedef struct 
{
	uint16_t cid;
	uint16_t lac;
	uint16_t bch;
	std::string GPS;
} T_Station;


/**
 * Cotains values merged and calculated from both T_NearestStation and 
 * T_Station important for determining user equipment's location.
 */
typedef struct  
{
	uint16_t cid;
	uint16_t lac;
	std::string GPS;
	T_GPS GPSCords;
	double distance; 
} T_MatchedStation;


/**
 * Function headers
 */
std::string processParameters(int argc, char *argv[]);

std::vector<T_Station> loadBTSRecords(std::string BTSFile);
std::vector<T_NearestStation> loadNearestStations(std::string csvFile);
std::vector<T_MatchedStation> prepareMatchingStation(std::vector<T_NearestStation> nearbyStations, std::vector<T_Station> allStations); 

T_GPS convertStringGPS(std::string GPS);
double getDegreesOnly(double degrees, double minutes, double seconds);
double calculateDistanceToStation(double antennaCorrectionFactor, double transmissionFrequency, double mobileAntenaHeight);

void writeOutputFile(std::string data);
std::string generateGoogleMapsLink(T_GPS coords);

double helper_calculateAntennaCorrectionFactor(double transmissionFrequency, double mobileAntennaHeight);
