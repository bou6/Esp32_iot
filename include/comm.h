#ifndef COMM_H
#define COMM_H
#include <ESP8266WebServer.h>

class Comm{
    // singelton class
    public:
        enum State {
            INIT,
            AP_STARTED,
            CONNECTING,
            CONNECTED,
        };
        static Comm* get_instance();
        bool is_connected();
        void try_connect();
        bool read_saved_credentials(String* network, String* password);
        bool save_credentials(String network, String password);
        bool delete_credentials();

        void comm_state_machine();
        void comm_state_transition(State new_state);
        String comm_state_to_string(Comm::State state);
        State get_state();

        void set_network(String network);
        void set_password(String password);

        //### ToDo declare it as a private member
        ESP8266WebServer server;
    private:
        String m_network;
        String m_password;
        State m_state;
        Comm();

        // Static instance pointer
        static Comm* instance;
};

#endif