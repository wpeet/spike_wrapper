#pragma once
#include <systemc.h>
#include <tlm.h>
#include <tlm_utils/simple_initiator_socket.h>

// FIXME: #include "config.h"
#include "cfg.h"
#include "sim.h"
#include "mmu.h"
#include "arith.h"
#include "remote_bitbang.h"
#include "cachesim.h"
#include "extension.h"
#include <dlfcn.h>
#include <fesvr/option_parser.h>
#include <stdexcept>
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <string>
#include <memory>
#include <fstream>
#include <limits>
#include <cinttypes>
#include <sstream>
#include <map>
#include "../VERSION"
#include "SpikeSystemCBridge.h"

using namespace sc_core;
using namespace sc_dt;
using namespace std;

class spike_iss: ::sc_core::sc_module {
public:
    // Clock and Reset inputs for synchronization
    sc_core::sc_in<bool> clk;
    sc_core::sc_in<bool> rst;

    int    argc;
    char** argv;

    SpikeSystemCBridge* bridge;

private:
    static void help(int exit_code);
    static void suggest_help();
    static bool check_file_exists(const char *fileName);
    static std::ifstream::pos_type get_file_size(const char *filename);
    static void read_file_bytes(const char *filename,size_t fileoff, abstract_mem_t* mem, size_t memoff, size_t read_sz);
    bool sort_mem_region(const mem_cfg_t &a, const mem_cfg_t &b);
    static bool check_mem_overlap(const mem_cfg_t& L, const mem_cfg_t& R);
    static bool check_if_merge_covers_64bit_space(const mem_cfg_t& L, const mem_cfg_t& R);
    static mem_cfg_t merge_mem_regions(const mem_cfg_t& L, const mem_cfg_t& R);
    static std::vector<mem_cfg_t> merge_overlapping_memory_regions(std::vector<mem_cfg_t> mems);
    static mem_cfg_t create_mem_region(unsigned long long base, unsigned long long size);
    static std::vector<mem_cfg_t> parse_mem_layout(const char* arg);
    static std::vector<std::pair<reg_t, abstract_mem_t*>> make_mems(const std::vector<mem_cfg_t> &layout);
    static unsigned long atoul_safe(const char* s);
    static unsigned long atoul_nonzero_safe(const char* s);
    static std::vector<size_t> parse_hartids(const char *s);

    void   before_end_of_elaboration( );
    void   end_of_elaboration( );
    // FIXME: void   start_of_simulation();
    void   simulation_worker();

    sim_t* spike_sim;

public:
    SC_CTOR( spike_iss );

};

