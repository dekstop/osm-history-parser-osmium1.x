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

// main
class HistoryHandler : public Osmium::Handler::Base {
protected:
    std::unordered_map<osm_user_id_t, std::string> uid_name;
    std::ofstream &m_outfile;
    long m_numNodes, m_numUNodes;
    long m_numWays, m_numUWays;
    long m_numRels, m_numURels;
    
    // http://stackoverflow.com/questions/1494399/how-do-i-search-find-and-replace-in-a-standard-string
    std::string ReplaceString(std::string subject, const std::string& search,
        const std::string& replace) const {

        size_t pos = 0;
        while((pos = subject.find(search, pos)) != std::string::npos) {
            subject.replace(pos, search.length(), replace);
            pos += replace.length();
        }
        return subject;
    }

    std::string escape(const std::string str) const {
        return ReplaceString(ReplaceString(ReplaceString(ReplaceString(str, "\\", "\\\\"), "\n", "\\n"), "\t", "\\t"), "\r", "\\r");
    }

    bool add_user(const osm_user_id_t uid, const char* name) {
        std::unordered_map<osm_user_id_t, std::string>::const_iterator it = uid_name.find(uid);
        if (it == uid_name.end()) {
            std::string str_name(name);
            uid_name[uid] = str_name;
            return true;
        }
        return false;
    }

    void save_map() const {
        for (auto it = uid_name.cbegin(); it != uid_name.cend(); ++it) {
            m_outfile <<  
                it->first << "\t" <<
                escape(it->second) << std::endl;
        }
    }

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

        add_user(node->uid(), node->user());
    }

    void after_nodes() const {
        std::cout << "Node versions: " << m_numNodes << std::endl;
        std::cout << "Node versions with uid: " << m_numUNodes << std::endl;
        
        // Terminate early: don't parse ways and relations.
        // throw Osmium::Handler::StopReading();
    }

    void way(const shared_ptr< Osmium::OSM::Way const > &way) {
        m_numWays++;
        if (way->user_is_anonymous()) return;
        m_numUWays++;

        add_user(way->uid(), way->user());
    }

    void after_ways() const {
        std::cout << "Way versions: " << m_numWays << std::endl;
        std::cout << "Way versions with uid: " << m_numUWays << std::endl;
    }

    void relation(const shared_ptr< Osmium::OSM::Relation const > &relation) {
        m_numRels++;
        if (relation->user_is_anonymous()) return;
        m_numURels++;

        add_user(relation->uid(), relation->user());
    }
    
    void after_relations() const {
        std::cout << "Relation versions: " << m_numRels << std::endl;
        std::cout << "Relation versions with uid: " << m_numURels << std::endl;

        save_map();
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
  
