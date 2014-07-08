#------------------------------------------------------------------------------
#
#  OSM history parser makefile
#
#------------------------------------------------------------------------------

PREFIX ?= /usr

#CXX = g++
CXX = clang++

CXXFLAGS = -g -O3 -Wall -Wextra -pedantic
CXXFLAGS += `getconf LFS_CFLAGS`
#CXXFLAGS += -Wredundant-decls -Wdisabled-optimization
#CXXFLAGS += -Wpadded -Winline

# compile & link against libxml to have xml writing support
#CXXFLAGS += -DOSMIUM_WITH_OUTPUT_OSM_XML
CXXFLAGS += `xml2-config --cflags`
LDFLAGS = -L/usr/local/lib -lexpat -lpthread
LDFLAGS += `xml2-config --libs`

# compile &  link against libs needed for protobuf reading and writing
LDFLAGS += -lz -lprotobuf-lite -losmpbf

# compile &  link against geos for multipolygon extracts
#CXXFLAGS += `geos-config --cflags`
#CXXFLAGS += -DOSMIUM_WITH_GEOS
#LDFLAGS += `geos-config --libs`

.PHONY: all clean install

all: user-edit-location

user-edit-location: user_edit_location.cpp
	$(CXX) $(CXXFLAGS) -o $@ $< $(LDFLAGS)

install: user-edit-location
	install -m 755 -g root -o root -d $(DESTDIR)$(PREFIX)/bin
	install -m 755 -g root -o root user-edit-location $(DESTDIR)$(PREFIX)/bin/user-edit-location

clean:
	rm -f *.o core user-edit-location
