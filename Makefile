CXX = /home/roman/openwrt-sdk-23.05.2-x86-64_gcc-12.3.0_musl.Linux-x86_64/staging_dir/toolchain-x86_64_gcc-12.3.0_musl/bin/x86_64-openwrt-linux-g++
CXXFLAGS = -std=c++11 -Wall -g -Iinclude

OBJ_DIR = obj
SRC_DIR = src
INCLUDE_DIR = include

EXEC = network_diagnostic_utility_openwrt

OBJ = $(OBJ_DIR)/network_utils.o $(OBJ_DIR)/raw_socket.o $(OBJ_DIR)/tracerout.o $(OBJ_DIR)/main.o

OPENWRT_LIB_DIR = /home/roman/openwrt-sdk-23.05.2-x86-64_gcc-12.3.0_musl.Linux-x86_64/staging_dir/toolchain-x86_64_gcc-12.3.0_musl/lib
OPENWRT_INCLUDE_DIR = /home/roman/openwrt-sdk-23.05.2-x86-64_gcc-12.3.0_musl.Linux-x86_64/staging_dir/toolchain-x86_64_gcc-12.3.0_musl/include

LDFLAGS = -L$(OPENWRT_LIB_DIR) -lstdc++

$(EXEC): $(OBJ)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $(EXEC) $(OBJ)

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

$(OBJ_DIR)/network_utils.o: $(SRC_DIR)/network_utils.cpp $(INCLUDE_DIR)/network_utils.h | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $(SRC_DIR)/network_utils.cpp -o $(OBJ_DIR)/network_utils.o

$(OBJ_DIR)/raw_socket.o: $(SRC_DIR)/raw_socket.cpp $(INCLUDE_DIR)/raw_socket.h | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $(SRC_DIR)/raw_socket.cpp -o $(OBJ_DIR)/raw_socket.o

$(OBJ_DIR)/tracerout.o: $(SRC_DIR)/tracerout.cpp $(INCLUDE_DIR)/tracerout.h | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $(SRC_DIR)/tracerout.cpp -o $(OBJ_DIR)/tracerout.o

$(OBJ_DIR)/main.o: $(SRC_DIR)/main.cpp | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $(SRC_DIR)/main.cpp -o $(OBJ_DIR)/main.o

clean:
	rm -rf $(OBJ_DIR) $(EXEC)