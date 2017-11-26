/**
 * Author: Daniel Dusek, xdusek21
 * Brno, University of Technology
 * BMS class of 2017/2018, Project #1
 */
#include "project.h"

int main(int argc, char *argv[])
{
    // Input files 
    std::string BTSFile = "bts.csv";
    std::string inputFile = processParameters(argc, argv);
    if (inputFile.compare(EMPTY_STRING) == 0)
    {
        std::cerr << "Please specify input file as the first parameter.\n";
        return EXIT_FAILURE_PARAMS;
    }

    // Load information about stations
    std::vector<T_NearestStation> nearestStations = loadNearestStations(inputFile);
    if (nearestStations.empty())
    {
        std::cerr << "Input file could not be opened, or error occured while reading it. Fix the file and try again, please.\n";
        return EXIT_FAILURE_INPUTFILE;
    }

    std::vector<T_Station> allStations = loadBTSRecords(BTSFile);
    if (allStations.empty())
    {
        std::cerr << "Input file BTS.csv could not be opened, or error occured while reading it. Fix the file and try again, please.\n";
        return EXIT_FAILURE_INPUTFILE;
    }

    std::vector<T_MatchedStation> matchingStations = prepareMatchingStation(nearestStations, allStations);
    
    T_GPS UELocation = calculateUELocation(matchingStations);
    if (UELocation.latitude <= -1 && UELocation.longitude <= -1)
    {
        std::cerr << "You need at least 3 stations to determine location precisely.\n";
        return EXIT_FAILURE_CALCULATION;
    }
    
    writeOutputFile( generateGoogleMapsLink(UELocation) );

    return EXIT_SUCCESS;   
}


/*
 * Calculates User Equipment location.
 *
 * Approximates location of User Equipment using three elipses and their 
 * average midpoints. 
 * Conversion of to 'degree-distance' source:
 * https://en.wikipedia.org/wiki/Geographic_coordinate_system.
 *
 * std::vector<T_MatchedStation> matchingStations Vector of BTS stations.
 *
 * return T_GPS Location of User equipment on success, -1,-1 on failure.
 */
T_GPS calculateUELocation(std::vector<T_MatchedStation> matchingStations)
{
    T_GPS UELocation;

    // Calculate 'degree-distance' for relevant stations
    for (std::vector<T_MatchedStation>::iterator it = matchingStations.begin(); it != matchingStations.end(); ++it)
    {
        double latCord = it->GPSCords.latitude;
        
        double verticalDistance = 111132.92 - (559.82 * (cos((2*latCord) * M_PI / 180.0))) + (1.175 * cos(((4*latCord)*M_PI)/180.0)) - (0.0023 * cos(((6*latCord) * M_PI)/180.0));
        double horizontalDistance = 111412.84*cos(latCord*M_PI/180.0) - (93.5 * cos((3*latCord)*M_PI/180.0)) + (0.118 * cos((5*latCord)*M_PI/180.0));  

        it->verticalDistance = (double) ((it->distance*1000)/verticalDistance);
        it->horizontalDistance = (double) ((it->distance*1000)/horizontalDistance);
    }

    // Calculation can not be done, return false-y value
    if (matchingStations.size() < 3)
    {
        UELocation.latitude = -1;
        UELocation.longitude = -1;
        return UELocation;
    }

    // Define elipses by its 4 point and midpoint
    T_Elipse elipse01 = createElipse(matchingStations.at(0));
    T_Elipse elipse02 = createElipse(matchingStations.at(1));
    T_Elipse elipse03 = createElipse(matchingStations.at(2));

    // Calculate average mid point for each elipse-elipse combination
    T_Point midpoint12 = getAverageMidPoint(elipse01, elipse02);    
    T_Point midpoint23 = getAverageMidPoint(elipse02, elipse03);    
    T_Point midpoint31 = getAverageMidPoint(elipse03, elipse01);    

    // Average value of all the midpoints is close approximation of UE location
    double finalLat = (double) (midpoint12.latitude + midpoint23.latitude + midpoint31.latitude) / 3.0;
    double finalLon = (double) (midpoint12.longitude + midpoint23.longitude + midpoint31.longitude) / 3.0;

    UELocation.latitude = finalLat;
    UELocation.longitude = finalLon;
    return UELocation;
}


/**
 * Calculates average midpoint for two elipses.
 *
 * T_Elipse elipse01 First elipse to be used in calculation.
 * T_Elipse elipse02 Second elipse to be used in calculation.
 *
 * return T_Point Average midpoint.
 */
T_Point getAverageMidPoint(T_Elipse elipse01, T_Elipse elipse02)
{
    double midPointLon, midPointLat;

    // Elipse02 is closer to the right    
    if (elipse01.midPoint.longitude < elipse02.midPoint.longitude)
    {
        midPointLon = (double) (elipse02.mostLeft.longitude + elipse01.mostRight.longitude) / 2.0;
    }
    else
    {
        midPointLon = (double) (elipse01.mostLeft.longitude + elipse02.mostRight.longitude) / 2.0;
    }

    // Elipse02 is closer to the top
    if (elipse01.midPoint.latitude < elipse02.midPoint.latitude)
    {
        midPointLat = (double) (elipse02.mostBottom.latitude + elipse01.mostTop.latitude) / 2.0;
    }
    else
    {
        midPointLat = (double) (elipse01.mostBottom.latitude + elipse02.mostTop.latitude) / 2.0;
    }   
    
    T_Point midpoint;
    midpoint.latitude = midPointLat;
    midpoint.longitude = midPointLon;

    return midpoint;
}


// TODO: Ensure it works even for locations close to 0,0
/**
 * Creates elipse representation of a T_MatchedStation.
 *
 * T_MatchedStation station Station representation.
 *
 * return T_Elipse 4 points + midpoint representation of elipse.
 */
T_Elipse createElipse(T_MatchedStation station)
{
    T_Elipse elipse;
    T_Point mostLeft, mostRight, mostTop, mostBottom, midPoint;
    
    mostLeft.latitude = station.GPSCords.latitude;
    mostLeft.longitude = station.GPSCords.longitude - station.horizontalDistance; 
    mostRight.latitude = station.GPSCords.latitude;
    mostRight.longitude = station.GPSCords.longitude + station.horizontalDistance;
    mostTop.latitude = station.GPSCords.latitude + station.verticalDistance;
    mostTop.longitude = station.GPSCords.longitude;
    mostBottom.latitude = station.GPSCords.latitude - station.verticalDistance;
    mostBottom.longitude = station.GPSCords.longitude;
    midPoint.latitude = station.GPSCords.latitude;
    midPoint.longitude = station.GPSCords.longitude;

    elipse.mostLeft = mostLeft;
    elipse.mostRight = mostRight;
    elipse.mostTop = mostTop;
    elipse.mostBottom = mostBottom;
    elipse.midPoint = midPoint;
    
    return elipse;
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
                newStation.distance = calculateDistanceToStation(nsIt->antH, nsIt->power, nsIt->signal);

                // Store average values for same stations
                bool skipPushBack = false;
                for (std::vector<T_MatchedStation>::iterator it = relevantStations.begin(); it != relevantStations.end(); ++it)
                {
                    if (it->GPS == newStation.GPS)
                    {
                        double meanDistance = (double) ((newStation.distance + it->distance) / 2.0);
                        it->distance = meanDistance;
                        skipPushBack = true;
                    }
                }
                
                if (!skipPushBack)
                {
                    relevantStations.push_back(newStation);
                }
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
                    // Do not store.
                break;
                case 3:
                    // Do not store.
                break;
                case 4:
                    station.GPS = token;    
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
        char *pEnd;
        T_NearestStation station;
        
        while (getline(iss, token, ';'))
        {
            switch (lineOffset)
            {
                
                case 0: // LAC
                    station.lac = atoi(token.c_str());
                break;
                case 1: // CID
                    station.cid = atoi(token.c_str());
                break;
                case 2: // RSSI
                    // Do not store.
                break;
                case 3: // Signal
                    station.signal = strtod(token.c_str(), &pEnd);
                break;
                case 4: // Ant H
                    station.antH = strtod(token.c_str(), &pEnd);
                break;
                case 5: // Power
                    station.power = strtod(token.c_str(), &pEnd);
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
 * Prints out points of which elipse consists.
 *
 * T_Elipse elipse Elipse to be printed out.
 */
void helper_printElipsePoints(T_Elipse elipse)
{
    std::cout << "[DEBUG] Printing elipse:\n"; 
    std::cout << "\tMid [LAT,LON]: [" << elipse.midPoint.latitude << "," << elipse.midPoint.longitude << "]\n";
    std::cout << "\tTop [LAT,LON]: [" << elipse.mostTop.latitude << "," << elipse.mostTop.longitude << "]\n";
    std::cout << "\tBottom [LAT,LON]: [" << elipse.mostBottom.latitude << "," << elipse.mostBottom.longitude << "]\n";
    std::cout << "\tLeft [LAT,LON]: [" << elipse.mostLeft.latitude << "," << elipse.mostLeft.longitude << "]\n";
    std::cout << "\tRight [LAT,LON]: [" << elipse.mostRight.latitude << "," << elipse.mostRight.longitude << "]\n";
}