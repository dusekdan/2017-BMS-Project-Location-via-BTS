/**
 * Author: Daniel Dusek, xdusek21
 * Brno, University of Technology
 * BMS class of 2017/2018, Project #1
 */

// TODO: Put function headers here


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

