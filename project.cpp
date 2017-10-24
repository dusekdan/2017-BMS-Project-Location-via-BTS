/**
 * Author: Daniel Dusek, xdusek21
 * Brno, University of Technology
 * BMS class of 2017/2018, Project #1
 */

#include<iostream>
#include<stdlib.h>
#include<string>
#include<fstream>

void processParameters(int argc, char *argv[]);
void terminateExecutionWith(int exitCode, std::string message = "");
void writeOutputFile(/* DATA PLACEHOLDER */);

// Input file name
std::string csvFile;

int main(int argc, char *argv[])
{
    processParameters(argc, argv);

    // TODO: Store values from CSV in internal structure loadBTSInfo()
        // * Validate input file (permissions) - validateCSVFile()
        // * loadBTSRecords()
        // * Possible exit-code 2: invalid input file / unable to read ...

    // TODO: Compute your location

    // TODO: Generate Google Map link

    // TODO: Write output to file "out.txt"
    writeOutputFile();



    // DEBUG: To be deleted once the application is ready.
    std::cout << "Parameter count: " << std::to_string(argc) << "\n";
    std::cout << "Input csv file:" << csvFile << "\n";
    return 0;   
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
