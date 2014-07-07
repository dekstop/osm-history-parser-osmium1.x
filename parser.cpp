#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#include <iostream>
#include <fstream>

#include <getopt.h>
#include <unistd.h>

#define OSMIUM_MAIN
#define OSMIUM_WITH_PBF_INPUT
#define OSMIUM_WITH_XML_INPUT
//#define OSMIUM_WITH_PBF_OUTPUT
//#define OSMIUM_WITH_XML_OUTPUT
#include <osmium.hpp>
//#include <osmium/output/pbf.hpp>
//#include <osmium/output/xml.hpp>

int main(int argc, char *argv[]) {
    char *infilename, *outfilename;

    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " OSM-HISTORY-FILE OUTFILE" << std::endl;
        return 1;
    }
    
    infilename = argv[1];
    Osmium::OSMFile infile(infilename);
    
    outfilename = argv[2];
    std::ofstream outfile;
    outfile.open(outfilename);
    outfile << infilename << std::endl;

    //HistoryParser parser(outfile);
    //Osmium::Input::read(infile, parser);

    outfile.close();
    return 0;
}
  
