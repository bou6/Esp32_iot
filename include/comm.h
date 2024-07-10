#ifndef COMM_H
#define COMM_H
#include <ESP8266WebServer.h>

class Comm{
    // singelton class
    public:
        static Comm* get_instance();
        bool is_connected();
        void try_connect();
        //### ToDo declare it as a private member
        ESP8266WebServer server;
    private:
        Comm();

        // Static instance pointer
        static Comm* instance;
};

#endif