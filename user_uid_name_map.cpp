#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#include <iostream>
#include <fstream>

#include <getopt.h>
#include <unistd.h>
#include <unordered_map>

#define OSMIUM_MAIN
#define OSMIUM_WITH_PBF_INPUT
#define OSMIUM_WITH_XML_INPUT
//#define OSMIUM_WITH_PBF_OUTPUT
//#define OSMIUM_WITH_XML_OUTPUT
#include <osmium.hpp>
//#include <osmium/output/pbf.hpp>
//#include <osmium/output/xml.hpp>


class HistoryHandler : public Osmium::Handler::Base {
protected:
    std::unordered_map<osm_user_id_t, const char *> uid_name;
    std::ofstream &m_outfile;
    long m_numNodes, m_numUNodes;
    long m_numWays, m_numUWays;
    long m_numRels, m_numURels;

public:
    HistoryHandler(std::ofstream &outfile) : Base(), m_outfile(outfile), m_numNodes(0), 
        m_numUNodes(0), m_numWays(0), m_numUWays(0), m_numRels(0), m_numURels(0) {
    }

    void init (Osmium::OSM::Meta &meta) const {
        if (meta.has_multiple_object_versions()) {
            std::cout << "History file" << std::endl;
        } else {
            std::cout << "Non-history file" << std::endl;
        }
    }

    void node(const shared_ptr<Osmium::OSM::Node const> &node) {
        m_numNodes++;
        if (node->user_is_anonymous()) return;
        m_numUNodes++;

        osm_user_id_t uid = node->uid();
        std::unordered_map<osm_user_id_t, const char *>::const_iterator it = uid_name.find(uid);
        if (it == uid_name.end()) {
            char *name = new char[strlen(node->user()) + 1];
            strcpy(name, node->user());
            uid_name[uid] = name;
        }
    }

    void after_nodes() const {
        std::cout << "Node versions: " << m_numNodes << std::endl;
        std::cout << "Node versions with uid: " << m_numUNodes << std::endl;
        
        for (auto it = uid_name.cbegin(); it != uid_name.cend(); ++it) {
            m_outfile <<  
                it->first << "\t" <<
                it->second << std::endl;
            delete[] it->second;
        }

        // Terminate early: don't parse ways and relations.
        throw Osmium::Handler::StopReading();
    }

    void way(const shared_ptr< Osmium::OSM::Way const > &way) {
        m_numWays++;
        if (way->user_is_anonymous()) return;
        m_numUWays++;
//        std::cout << "Way: " << 
//            way->timestamp() << "\t" <<
//            way->uid() << "\t" <<
//            //way->position().lon() << "," << way->position().lat() << "\t" <<
//            std::endl;
    }

    void after_ways() const {
        std::cout << "Way versions: " << m_numWays << std::endl;
        std::cout << "Way versions with uid: " << m_numUWays << std::endl;
    }

    void relation(const shared_ptr< Osmium::OSM::Relation const > &relation) {
        m_numRels++;
        if (relation->user_is_anonymous()) return;
        m_numURels++;
//        std::cout << "Relation: " << 
//            relation->timestamp() << "\t" <<
//            relation->uid() << "\t" <<
//            //relation->position().lon() << "," << relation->position().lat() << "\t" <<
//            std::endl;
    }
    
    void after_relations() const {
        std::cout << "Relation versions: " << m_numRels << std::endl;
        std::cout << "Relation versions with uid: " << m_numURels << std::endl;
    }

};


int main(int argc, char *argv[]) {
    char *infilename, *outfilename;

    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " OSM-HISTORY-FILE OUTFILE" << std::endl;
        return 1;
    }
    
    infilename = argv[1];
    Osmium::OSMFile infile(infilename);
    if (infile.encoding()->is_pbf()) {
        std::cout << "PBF file" << std::endl;
    } else {
        std::cout << "XML file" << std::endl;
    }
    
    outfilename = argv[2];
    std::ofstream outfile;
    outfile.open(outfilename);

    HistoryHandler hh(outfile);
    Osmium::Input::read(infile, hh);

    outfile.close();
    return 0;
}
  
