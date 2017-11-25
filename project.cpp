/**
 * Author: Daniel Dusek, xdusek21
 * Brno, University of Technology
 * BMS class of 2017/2018, Project #1
 */
#include "project.h"

// TODO: Consider reading signal, antH and power as double. Or will it always be int?!

int main(int argc, char *argv[])
{
    // Input files 
    std::string BTSFile = "bts.csv";
    std::string inputFile = processParameters(argc, argv);
    if (inputFile.compare(EMPTY_STRING) == 0)
    {
        std::cerr << "Please specify input file as the first parameter.\n";
        return 1;
    }

    // Load information about stations
    std::vector<T_NearestStation> nearestStations = loadNearestStations(inputFile);
    if (nearestStations.empty())
    {
        std::cerr << "Input file could not be opened, or error occured while reading it. Fix the file and try again, please.\n";
        return 2;
    }

    std::vector<T_Station> allStations = loadBTSRecords(BTSFile);
    if (allStations.empty())
    {
        std::cerr << "Input file BTS.csv could not be opened, or error occured while reading it. Fix the file and try again, please.\n";
        return 2;
    }

    std::vector<T_MatchedStation> matchingStations = prepareMatchingStation(nearestStations, allStations); // TODO: What if no matching stations are found?

    // TODO: Compute UE location
    // - 
    for (std::vector<T_MatchedStation>::iterator it = matchingStations.begin(); it != matchingStations.end(); ++it)
    {
        double latCord = it->GPSCords.latitude;
        
        double verticalDistance = 111132.92 - (559.82 * (cos((2*latCord) * M_PI / 180.0))) + (1.175 * cos(((4*latCord)*M_PI)/180.0)) - (0.0023 * cos(((6*latCord) * M_PI)/180.0));
        double horizontalDistance = 111412.84*cos(latCord*M_PI/180.0) - (93.5 * cos((3*latCord)*M_PI/180.0)) + (0.118 * cos((5*latCord)*M_PI/180.0));  

        it->verticalDistance = (double) ((it->distance*1000)/verticalDistance);
        it->horizontalDistance = (double) ((it->distance*1000)/horizontalDistance);

    }


    /* DEBUG Returned matching stations */
    int counter = 0;
    for(std::vector<T_MatchedStation>::iterator it = matchingStations.begin(); it != matchingStations.end(); ++it)
    {
        std::cout << "Relevant station " << std::to_string(counter)  << " CID " << it->cid << " and gps for the doubtful " << it->GPS << "\n";
        std::cout << "\tGPS struct: latitude=" << std::to_string(it->GPSCords.latitude) << ", longitude=" << std::to_string(it->GPSCords.longitude) << "\n";
        std::cout << "\tDistance: " << std::to_string(it->distance) << " [km] \n";
        std::cout << "\tLink: " << generateGoogleMapsLink(it->GPSCords) << "\n";
        std::cout << "\tVertical distance: " << it->verticalDistance << "\n";
        std::cout << "\tHorizontal distance: " << it->horizontalDistance << "\n";
        std::cout << "\n";
        counter++;
    }

    
    // Dummy location -> TODO: Replace this with actual value
    T_GPS UELocation;
    UELocation.latitude = 0.0;
    UELocation.longitude = 0.0;
    
    writeOutputFile( generateGoogleMapsLink(UELocation) );

    return 0;   
}


/**
 * Crafts link to maps.google.com.
 *
 * T_GPS coords Coordinates pointing to location to be marked on map.
 *
 * return std::string Google map link in required format. 
 */
std::string generateGoogleMapsLink(T_GPS coords)
{
    return GOOGLE_MAPS_URL_BASE + std::to_string(coords.latitude) + "," + std::to_string(coords.longitude);
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
 * Magical constants used come from the Wikipedia formula, to be found here:
 * https://en.wikipedia.org/wiki/Hata_model
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
 *
 * std::string BTSFile Path to input BTS.csv file
 *
 * TODO: Do not store values that are not used.
 * return std::vector<T_Station> Vector of T_Station records.
 */
std::vector<T_Station> loadBTSRecords(std::string BTSFile)
{
    std::ifstream file(BTSFile);
    std::vector<T_Station> allStations;

    // Input file cannot be read, return empty vector
    if (!file.is_open())
    {
        return allStations;
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

        allStations.push_back(station);
    }

    return allStations;
}


/**
 * Load records from input bts file.
 *
 * std::string csvFile Path to input csv file.
 *
 * return std::vector<T_NearestStation> Vector of nearest stations
 */
std::vector<T_NearestStation> loadNearestStations(std::string csvFile)
{
    std::ifstream file(csvFile);
    std::vector<T_NearestStation> nearestStations;

    // File cannot be read, return no records.
    if(!file.is_open())
    {
        return nearestStations;
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

    return nearestStations;
}


/**
 * Processes commandline parameters passed to the application.
 *
 * int argc Number of parameters with which the application was called.
 * char** argv Array of parameters provided on input.
 *
 * return std::string Input csv file passed from parameters.
 */
std::string processParameters(int argc, char *argv[])
{
    // Terminate execution if no path to input file is provided.
    if (argc < 2)
    {
        return std::string(EMPTY_STRING);
    }

    return std::string(argv[1]);
}


/**
 * Writes string data to output file.
 *
 * std::string data Data to be written to output file.
 */
void writeOutputFile(std::string data)
{
    std::ofstream outFile;
    outFile.open(BMS_OUTPUT_FILE);
    outFile << data;
    outFile.close(); 
}




// TODO: Remove this if it is not used by the end of the Saturday 2017-11-25
/**
 * Terminates program execution.
 *
 * Prints string message on stderr and terminates program execution with 
 * given exit code.
 *
 * int exitCode Numeric exit code with which program terminates.
 * std::string message Message displayed before termination (may be empty).
 */
//void terminateExecutionWith(int exitCode, std::string message = "");
/*void terminateExecutionWith(int exitCode, std::string message)
{
    if (!message.empty())
    {
        std::cerr << message << "\n";
    }

    // TODO: If necessary, execute cleanup here. 

    exit(exitCode);
}*/