
#include <systemc.h>

SC_MODULE(Hello_SystemC) {
	SC_CTOR(Hello_SystemC) {
		SC_THREAD(module_thread);
	}

	void module_thread(void){
		SC_REPORT_INFO("Module Thread", "LOL");
	}
};

int sc_main(int sc_argc, char* sc_argv[]){
	Hello_SystemC HelloWorld_i("HelloWorld_i");

	sc_start();

	return 0;
}

