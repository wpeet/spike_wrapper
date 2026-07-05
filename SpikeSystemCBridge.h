#include <systemc.h>
#include <tlm.h>
#include "tlm_utils/simple_initiator_socket.h"
#include "decode.h"
#include "processor.h"
#include "simif.h"

// Define categories for the interception
enum class AccessType {
    FETCH,
    DATA_READ,
    DATA_WRITE
};

class SpikeSystemCBridge : public simif_t {
public:

    // TLM-2.0 Initiator port to talk to the SystemC Bus/Memory
    // FIXME: tlm_utils::simple_initiator_socket<SpikeSystemCBridge> ibus;
    tlm_utils::simple_initiator_socket<SpikeSystemCBridge> bus;

    // Core pointer to the processor instance to check execution states
    processor_t* proc_ptr = nullptr;

    // CRITICAL: Always return nullptr to completely disable Spike's internal TLB caching.
    // This forces Spike to hit mmio_load/mmio_store for EVERY single fetch and data access.
    char* addr_to_mem(reg_t addr) override { 
        return nullptr; 
    } 

    bool mmio_fetch(reg_t addr, size_t len, uint8_t* bytes) override {
        AccessType type = AccessType::FETCH;
        cout << "mmio_fetch: addr = " << hex << addr << ", len = " << dec << len << endl;
        return route_transaction(type, addr, len, bytes);
    }

    bool mmio_load(reg_t addr, size_t len, uint8_t* bytes) override {
        AccessType type = AccessType::DATA_READ;
        cout << "mmio_load: addr = " << hex << addr << ", len = " << dec << len << endl;        
        return route_transaction(type, addr, len, bytes);
    }

    bool mmio_store(reg_t addr, size_t len, const uint8_t* bytes) override {
        // Stores are always data operations
        cout << "mmio_store: addr = " << hex << addr << ", len = " << dec << len << endl;        
        return route_transaction(AccessType::DATA_WRITE, addr, len, const_cast<uint8_t*>(bytes));
    }

    // Callback for processors to let the simulation know they were reset.
    void proc_reset(unsigned id) override {

        cout << "SpikeSystemCBridge: proc_reset called, id = " << id << endl;

    }

    const char* get_symbol(uint64_t addr) override {
        // FIXME: return proc_ptr->sim->get_symbol(addr);
        return nullptr; 
    }

    const cfg_t &get_cfg() const override {
        return proc_ptr->get_cfg();
    }
    
    const std::map<size_t, processor_t*>& get_harts() const override {
        std::map<size_t, processor_t*> hartMap = { { proc_ptr->get_cfg().hartids.size(), proc_ptr } };
        return hartMap;
    }

    // Interception Callback tied to the SystemC module
    std::function<bool(AccessType, reg_t, size_t, uint8_t*)> bus_transport_cb;

    bool route_transaction(AccessType type, reg_t addr, size_t len, uint8_t* data) {
        cout << "SpikeSystemCBridge: Inside route_transaction" << endl;

        tlm::tlm_generic_payload trans;
        sc_core::sc_time delay = sc_core::SC_ZERO_TIME;
        trans.set_address(addr);
        trans.set_data_ptr(data);
        trans.set_data_length(len);
        trans.set_streaming_width(len);
        if (type == AccessType::FETCH) {
          // Forward to SystemC instruction cache or instruction bus
          trans.set_command(tlm::TLM_READ_COMMAND);
          bus->b_transport(trans, delay);
          std::cout << "[FETCH] PC: 0x" << std::hex << addr << " Len: " << len << std::endl;
        } 
        else if (type == AccessType::DATA_READ) {
          // Forward to SystemC data bus / RAM
          trans.set_command(tlm::TLM_READ_COMMAND);
          bus->b_transport(trans, delay);
          std::cout << "[DATA READ] Addr: 0x" << std::hex << addr << std::endl;
        } 
        else if (type == AccessType::DATA_WRITE) {
          trans.set_command(tlm::TLM_WRITE_COMMAND);
          bus->b_transport(trans, delay);
          std::cout << "[DATA WRITE] Addr: 0x" << std::hex << addr << std::endl;
        }
        if (delay > sc_core::SC_ZERO_TIME) sc_core::wait(delay);
        return trans.is_response_ok();
    }

};

