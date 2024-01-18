PATH_BIN = bin
PATH_LIB = lib
PATH_OBJ = obj

PATH_NETRPC = netrpc
PATH_COMM = $(PATH_NETRPC)/common
PATH_NET = $(PATH_NETRPC)/net

PATH_TESTCASES = testcases

# will install lib to /usr/lib/libsocket.a
PATH_INSTALL_LIB_ROOT = /usr/lib

# will install all header file to /usr/include/socket
PATH_INSTALL_INC_ROOT = /usr/include

PATH_INSTALL_INC_COMM = $(PATH_INSTALL_INC_ROOT)/$(PATH_COMM)
PATH_INSTALL_INC_NET = $(PATH_INSTALL_INC_ROOT)/$(PATH_NET)


# PATH_PROTOBUF = /usr/include/google
# PATH_TINYXML = /usr/include/tinyxml

CXX := g++

CXXFLAGS += -g -O0 -std=c++11 -Wall -Wno-deprecated -Wno-unused-but-set-variable

CXXFLAGS += -I./ -I$(PATH_NETRPC)	-I$(PATH_COMM) -I$(PATH_NET)

# LIBS += /usr/lib/libprotobuf.a	/usr/lib/libtinyxml.a
LIBS += /usr/lib/libprotobuf.a /usr/lib/libtinyxml.a


COMM_OBJ := $(patsubst $(PATH_COMM)/%.cc, $(PATH_OBJ)/%.o, $(wildcard $(PATH_COMM)/*.cc))
NET_OBJ := $(patsubst $(PATH_NET)/%.cc, $(PATH_OBJ)/%.o, $(wildcard $(PATH_NET)/*.cc))

ALL_TESTS : $(PATH_BIN)/test_log  $(PATH_BIN)/test_eventloop

TEST_CASE_OUT := $(PATH_BIN)/test_log $(PATH_BIN)/test_eventloop

LIB_OUT := $(PATH_LIB)/libnetrpc.a

$(PATH_BIN)/test_log: $(LIB_OUT)
	$(CXX) $(CXXFLAGS) $(PATH_TESTCASES)/test_log.cc -o $@ $(LIB_OUT) $(LIBS) -ldl -pthread

$(PATH_BIN)/test_eventloop: $(LIB_OUT)
	$(CXX) $(CXXFLAGS) $(PATH_TESTCASES)/test_eventloop.cc -o $@ $(LIB_OUT) $(LIBS) -ldl -pthread


$(LIB_OUT): $(COMM_OBJ) $(NET_OBJ)
	cd $(PATH_OBJ) && ar rcv libnetrpc.a *.o && cp libnetrpc.a ../lib/

$(PATH_OBJ)/%.o : $(PATH_COMM)/%.cc
	$(CXX) $(CXXFLAGS) -c $< -o $@


$(PATH_OBJ)/%.o : $(PATH_NET)/%.cc
	$(CXX) $(CXXFLAGS) -c $< -o $@


# print something test
# like this: make PRINT-PATH_BIN, and then will print variable PATH_BIN
PRINT-% : ; @echo $* = $($*)


# to clean 
clean :
	rm -f $(COMM_OBJ) $(NET_OBJ) $(TESTCASES) $(TEST_CASE_OUT) $(PATH_LIB)/libnetrpc.a $(PATH_OBJ)/libnetrpc.a

# install
install:
	mkdir -p $(PATH_INSTALL_INC_COMM) $(PATH_INSTALL_INC_NET) \
		&& cp $(PATH_COMM)/*.h $(PATH_INSTALL_INC_COMM) \
		&& cp $(PATH_NET)/*.h $(PATH_INSTALL_INC_NET) \
		&& cp $(LIB_OUT) $(PATH_INSTALL_LIB_ROOT)/


# uninstall
uninstall:
	rm -rf $(PATH_INSTALL_INC_ROOT)/NETRPC && rm -f $(PATH_INSTALL_LIB_ROOT)/libnetrpc.a