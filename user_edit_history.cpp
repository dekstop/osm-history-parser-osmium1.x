#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#include <fstream>
#include <iostream>
#include <sstream>
#include <unordered_set>

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


class HistoryHandler : public Osmium::Handler::Base {
protected:
    std::unordered_set<osm_user_id_t> m_filter_users;
    std::ofstream &m_nodefile, &m_wayfile, &m_relfile;
    long m_numNodes, m_numUNodes;
    long m_numWays, m_numUWays;
    long m_numRels, m_numURels;

    bool matches_user_filter(const shared_ptr<Osmium::OSM::Object const> &obj) {
        if (obj->user_is_anonymous()) return false;
        std::unordered_set<osm_user_id_t>::const_iterator it = m_filter_users.find(obj->uid());
        if (it == m_filter_users.end()) return false;
        return true;
    }

public:
    HistoryHandler(std::unordered_set<osm_user_id_t> &filter_users,
        std::ofstream &nodefile, std::ofstream &wayfile, std::ofstream &relfile) : Base(),
        m_filter_users(filter_users), 
        m_nodefile(nodefile), m_wayfile(wayfile), m_relfile(relfile), m_numNodes(0), m_numUNodes(0), 
        m_numWays(0), m_numUWays(0), m_numRels(0), m_numURels(0) {
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
        if (!matches_user_filter(node)) return;
        m_numUNodes++;
        m_nodefile << 
            node->id() << "\t" <<
            node->version() << "\t" <<
            node->changeset() << "\t" <<
            node->timestamp_as_string() << "\t" <<
            node->uid() << "\t";
        if (node->position().defined()) {
            m_nodefile << 
                node->position().lat() << "\t" <<
                node->position().lon() << std::endl;
        } else {
            m_nodefile << "\t" << std::endl;
        }
    }

    void after_nodes() const {
        std::cout << "Node versions: " << m_numNodes << std::endl;
        std::cout << "Node versions with uid: " << m_numUNodes << std::endl;

        // Terminate early: don't parse ways and relations.
        // throw Osmium::Handler::StopReading();
    }

    void way(const shared_ptr< Osmium::OSM::Way const > &way) {
        m_numWays++;
        if (!matches_user_filter(way)) return;
        m_numUWays++;
        m_wayfile << 
            way->id() << "\t" <<
            way->version() << "\t" <<
            way->changeset() << "\t" <<
            way->timestamp_as_string() << "\t" <<
            way->uid() << std::endl;
    }

    void after_ways() const {
        std::cout << "Way versions: " << m_numWays << std::endl;
        std::cout << "Way versions with uid: " << m_numUWays << std::endl;
    }

    void relation(const shared_ptr< Osmium::OSM::Relation const > &relation) {
        m_numRels++;
        if (!matches_user_filter(relation)) return;
        m_numURels++;
        m_relfile << 
            relation->id() << "\t" <<
            relation->version() << "\t" <<
            relation->changeset() << "\t" <<
            relation->timestamp_as_string() << "\t" <<
            relation->uid() << std::endl;
    }
    
    void after_relations() const {
        std::cout << "Relation versions: " << m_numRels << std::endl;
        std::cout << "Relation versions with uid: " << m_numURels << std::endl;
    }

};

// http://insanecoding.blogspot.co.uk/2011/11/how-to-read-in-file-in-c.html
std::string get_file_contents(std::string filename) {
    std::ifstream in(filename, std::ios::in | std::ios::binary);
    if (in) {
        std::string contents;
        in.seekg(0, std::ios::end);
        contents.resize(in.tellg());
        in.seekg(0, std::ios::beg);
        in.read(&contents[0], contents.size());
        in.close();
        return(contents);
    }
    throw(errno);
}

bool load_user_filter(std::string filename, std::unordered_set<osm_user_id_t> &filter_users) {
    try {
        std::string str = get_file_contents(filename); // throws int errno
        std::istringstream ss(str);
        ss.imbue(std::locale::classic()); // locale independent parsing
        osm_user_id_t uid;
        while (ss >> uid) { // throws std::exception
            filter_users.insert(uid);
            if (ss.peek() == '\n') ss.ignore();        
        }
        return true;
    }
    catch (int errno) {
        std::cout << "Error: " << strerror(errno) << std::endl;
    }
    catch (std::exception &e) {
        std::cout << "Exception: " << e.what() << std::endl;
    }
    return false;
}


int main(int argc, char *argv[]) {
    char *infilename, *useridfilename, *nodefilename, *wayfilename, *relfilename;

    if (argc != 6) {
        std::cerr << "Usage: " << argv[0] << " OSM-HISTORY-FILE USERID_INFILE NODE_OUTFILE WAY_OUTFILE REL_OUTFILE" << std::endl;
        return 1;
    }
    
    infilename = argv[1];
    Osmium::OSMFile infile(infilename);
    if (infile.encoding()->is_pbf()) {
        std::cout << "PBF file" << std::endl;
    } else {
        std::cout << "XML file" << std::endl;
    }

    useridfilename = argv[2];
    std::unordered_set<osm_user_id_t> filter_users;
    if(!load_user_filter(useridfilename, filter_users)) {
        return 1;
    }
    std::cout << "Loaded " << filter_users.size() << " user IDs to filter." << std::endl;
    
    nodefilename = argv[3];
    std::ofstream nodefile;
    nodefile.open(nodefilename);

    wayfilename = argv[4];
    std::ofstream wayfile;
    wayfile.open(wayfilename);

    relfilename = argv[5];
    std::ofstream relfile;
    relfile.open(relfilename);

    HistoryHandler hh(filter_users, nodefile, wayfile, relfile);
    Osmium::Input::read(infile, hh);

    nodefile.close();
    wayfile.close();
    relfile.close();
    return 0;
}
  
