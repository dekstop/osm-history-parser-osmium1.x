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


class HistoryHandler : public Osmium::Handler::Base {
protected:
    std::ofstream &m_nodefile, &m_tagfile;
    long m_numNodes, m_numUNodes, m_numTNodes;
    long m_numWays, m_numUWays;
    long m_numRels, m_numURels;

    // http://stackoverflow.com/questions/1494399/how-do-i-search-find-and-replace-in-a-standard-string
    std::string ReplaceString(std::string subject, const std::string& search,
        const std::string& replace) {

        size_t pos = 0;
        while((pos = subject.find(search, pos)) != std::string::npos) {
            subject.replace(pos, search.length(), replace);
            pos += replace.length();
        }
        return subject;
    }

    std::string escape(const char* str) {
        return ReplaceString(ReplaceString(ReplaceString(ReplaceString(str, "\\", "\\\\"), "\n", "\\n"), "\t", "\\t"), "\r", "\\r");
    }

public:
    HistoryHandler(std::ofstream &nodefile, std::ofstream &tagfile) : Base(), 
        m_nodefile(nodefile), m_tagfile(tagfile), m_numNodes(0), m_numUNodes(0), 
        m_numTNodes(0), m_numWays(0), m_numUWays(0), m_numRels(0), m_numURels(0) {
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
        if (node->tags().empty()) return;
        m_numTNodes++;
        m_nodefile << 
            node->id() << "\t" <<
            node->version() << "\t" <<
            node->changeset() << "\t" <<
            node->timestamp() << "\t" <<
            node->uid() << "\t" <<
            escape(node->user()) << "\t" <<
            node->lat() << "\t" <<
            node->lon() << std::endl;
        for (const Osmium::OSM::Tag& tag : node->tags()) {
            m_tagfile <<
                node->id() << "\t" <<
                node->version() << "\t" <<
                escape(tag.key()) << "\t" <<
                escape(tag.value()) << std::endl;
        }
    }

    void after_nodes() const {
        std::cout << "Node versions: " << m_numNodes << std::endl;
        std::cout << "Node versions with uid: " << m_numUNodes << std::endl;
        std::cout << "Node versions with tags: " << m_numTNodes << std::endl;

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
    char *infilename, *nodefilename, *tagfilename;

    if (argc != 4) {
        std::cerr << "Usage: " << argv[0] << " OSM-HISTORY-FILE NODE_OUTFILE TAG_OUTFILE" << std::endl;
        return 1;
    }
    
    infilename = argv[1];
    Osmium::OSMFile infile(infilename);
    if (infile.encoding()->is_pbf()) {
        std::cout << "PBF file" << std::endl;
    } else {
        std::cout << "XML file" << std::endl;
    }
    
    nodefilename = argv[2];
    std::ofstream nodefile;
    nodefile.open(nodefilename);

    tagfilename = argv[3];
    std::ofstream tagfile;
    tagfile.open(tagfilename);

    HistoryHandler hh(nodefile, tagfile);
    Osmium::Input::read(infile, hh);

    nodefile.close();
    tagfile.close();
    return 0;
}
  
