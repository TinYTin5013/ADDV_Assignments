#define SC_INCLUDE_DYNAMIC_PROCESSES

#include <systemc.h>
#include "tlm.h"
#include "tlm_utils/simple_initiator_socket.h"
#include "tlm_utils/simple_target_socket.h"

using namespace sc_core;
using namespace sc_dt;
using namespace std;

SC_MODULE(Top){
	Software* software;
	Hardware_System* hardware;
	
	SC_CTOR(Top){
		software = new Software("software");
		hardware = new Hardware_System("hardware");
	}
	
	software->socket.bind(hardware->socket);
}

SC_MODULE(Hardware_System) {

	tlm_utils::simple_target_socket<Hardware_System> target_socket;
	
	SC_CTOR(Hardware_System) : socket("socket") {
		socket.register_b_transport(this, &Hardware_System::b_transport);
	}
	
	virtual void b_transport( tlm::tlm_generic_payload& trans, sc_time& delay);
	int addition(int x, int y);
	int subtraction(int x, int y);
	int equality(int x, int y);
	int remainder(int x, int y);
};

void Hardware_System::b_transport( tlm::tlm_generic_payload& trans, sc_time& delay){
	string cmd = trans.get_command();
	sc_dt::uint64 adr = trans.getaddress();
	unsigned int* ptr = trans.get_data_ptr();
	unsigned int len = trans.get_data_length();
	unsigned char* byt = trans.get_byte_enable_ptr();
	unsigned int wid = trans.get_streaming_width();
	
	if(addr >= sc_dt::uint64(SIZE) || byt!=0 || len>4 || wid<len){
		SC_REPORT_ERROR("TLM-2", "Target does not support given generic payload transaction");
	}
	
	int* terms;
	memcpy(&terms, ptr, len);
	int x = terms[0];
	int y = terms[1];
	int result = -1;
	
	if (cmd == "add"){result = addition(x, y);}
	else if(cmd == "sub"{result = subtraction(x,y);}
	else if(cmd == "eq"){result = equality(x,y);}
	else if(cmd == "rem"){result = remainder(x,y);}
	
	ptr[2] = result; //Test the pointer conditions
	trans.set_response_status(tlm:TLM_OK_RESPONSE);
}
int Hardware_System::addition(int x, int y){
	int z = x+y;
	wait(10, SC_NS);
	return z;
}

int Hardware_System::subtraction(int x, int y){
	int z = x-y;
	wait(11, SC_NS);
	return z;
}

int Hardware_System::equality(int x, int y){
	wait(5, SC_NS);
	return (x==y)?1:0;
}
int Hardware_System::remainder(int x, int y){
	int z = x%y;
	wait(15, SC_NS);
	return z;
}
int sc_main(int sc_argc, char* sc_argv[]){
	Hello_SystemC HelloWorld_i("HelloWorld_i");
	sc_start();
	return 0;
}

