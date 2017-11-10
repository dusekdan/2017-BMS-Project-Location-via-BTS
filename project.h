/**
 * Author: Daniel Dusek, xdusek21
 * Brno, University of Technology
 * BMS class of 2017/2018, Project #1
 */
int variable = 5; // aby se nereklo

typedef struct 
{
	uint16_t lac;
	uint16_t cid;
	int rssi;
	int signal;
	int antH;
	int power;

	// Calculated values
	double GPSlatitude;
	double GPSlongtitude;
	double distance;
} T_NearestStation;

typedef struct 
{
	uint16_t cid;
	uint16_t lac;
	uint16_t bch;
	std::string localization;
	std::string GPS;
	
	// Calculated values
	// TODO: Introduce decimal values for GPS - probably convert during parse time
	double GPSlatitude;
	double GPSlongtitude;
} T_Station;