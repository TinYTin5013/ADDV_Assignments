#define SC_INCLUDE_DYNAMIC_PROCESSES

#include <systemc.h>
#include "tlm.h"
#include "tlm_utils/simple_initiator_socket.h"
#include "tlm_utils/simple_target_socket.h"

using namespace sc_core;
using namespace sc_dt;
using namespace std;

const int ADD = 0;
const int SUB = 1;
const int EQ = 2;
const int REM = 3;

// Hardware module (Target) - implements arithmetic operations
SC_MODULE(Hardware_System) {
    tlm_utils::simple_target_socket<Hardware_System> target_socket;

    SC_CTOR(Hardware_System) : target_socket("target_socket") {
        target_socket.register_b_transport(this, &Hardware_System::b_transport);
    }

    virtual void b_transport(tlm::tlm_generic_payload& trans, sc_time& delay);

    // Arithmetic operations
    int addition(int x, int y);
    int subtraction(int x, int y);
    int equality(int x, int y);
    int remainder(int x, int y);
};

void Hardware_System::b_transport(tlm::tlm_generic_payload& trans, sc_time& delay) {
    unsigned int* ptr = reinterpret_cast<unsigned int*>(trans.get_data_ptr()); // Get data from payload
    int command = ptr[0]; // First element is the command
    int x = ptr[1]; // First operand
    int y = ptr[2]; // Second operand
    int result = -1;

    // Command handling
    switch (command) {
        case ADD:
            result = addition(x, y);
            break;
        case SUB:
            result = subtraction(x, y);
            break;
        case EQ:
            result = equality(x, y);
            break;
        case REM:
            result = remainder(x, y);
            break;
        default:
            SC_REPORT_ERROR("TLM-2", "Unsupported command");
            break;
    }

    ptr[0] = result; // Store result in the first element of data
    trans.set_response_status(tlm::TLM_OK_RESPONSE); // Indicate success
}

int Hardware_System::addition(int x, int y) {
    int z = x + y;
    wait(10, SC_NS); // Simulate delay
    return z;
}

int Hardware_System::subtraction(int x, int y) {
    int z = x - y;
    wait(11, SC_NS); // Simulate delay
    return z;
}

int Hardware_System::equality(int x, int y) {
    wait(4, SC_NS); // Simulate delay
    return (x == y) ? 1 : 0;
}

int Hardware_System::remainder(int x, int y) {
    int z = x % y;
    wait(15, SC_NS); // Simulate delay
    return z;
}

// Software module (Initiator) - sends commands to the hardware and verifies results
SC_MODULE(Software) {
    tlm_utils::simple_initiator_socket<Software> initiator_socket;

    SC_CTOR(Software) : initiator_socket("initiator_socket") {
        SC_THREAD(process_commands); // Run commands in a separate thread
    }

    // Function to send commands to the hardware and verify the result
    void process_commands() {
        sc_time delay = SC_ZERO_TIME; // Initialize delay

        // Test data and commands with expected results
        verify_command(ADD, 4, 36, 40, delay);
        verify_command(EQ, 49, 55, 0, delay);
        verify_command(REM, 20, 2, 0, delay);
        verify_command(SUB, 9, 12, -3, delay);
        verify_command(ADD, 12, 1000, 1012, delay);
        verify_command(REM, 5, 8, 5, delay);
        verify_command(REM, 100, 7, 2, delay);
        verify_command(SUB, 125, 25, 100, delay);
        verify_command(EQ, 47, 47, 1, delay);
        verify_command(ADD, 5, 6, 11, delay);

        delay = sc_time_stamp();
        std::cout << "Total simulation time: " << delay << std::endl;
    }

    // Function to send command and verify the result
    void verify_command(int cmd, int x, int y, int expected_result, sc_time &delay) {
        tlm::tlm_generic_payload trans;
        int data[3] = {cmd, x, y}; // Command and operands

        trans.set_data_ptr(reinterpret_cast<unsigned char*>(data)); // Attach data to payload
        trans.set_data_length(12); // 3 integers (4 bytes each)

        initiator_socket->b_transport(trans, delay); // Blocking transport call

        // Check if the result is correct
        if (data[0] == expected_result) {
            std::cout << "Command: " << cmd_to_string(cmd) << " (" << x << ", " << y << ")"
                      << ", Result: " << data[0] << " - correct" << std::endl;
        } else {
            std::cout << "Command: " << cmd_to_string(cmd) << " (" << x << ", " << y << ")"
                      << ", Result: " << data[0] << " - incorrect, expected: " << expected_result << std::endl;
        }
    }

    // Helper function to convert command codes to strings
    std::string cmd_to_string(int cmd) {
        switch (cmd) {
            case ADD: return "add";
            case SUB: return "sub";
            case EQ: return "eq";
            case REM: return "rem";
            default: return "unknown";
        }
    }
};

// Top-level module to connect software and hardware modules
SC_MODULE(Top) {
    Software *software;
    Hardware_System *hardware;

    SC_CTOR(Top) {
        software = new Software("software");
        hardware = new Hardware_System("hardware");

        // Bind sockets
        software->initiator_socket.bind(hardware->target_socket);
    }
};

// Main entry point for SystemC simulation
int sc_main(int sc_argc, char* sc_argv[]) {
    Top top("top"); // Instantiate the top module
    sc_start(); // Start the simulation
    return 0;
}
