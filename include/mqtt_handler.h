#include <Arduino.h>
#include <CertStoreBearSSL.h>
#include <PubSubClient.h>

class Mqtt_handler {
    public:
        enum Mqtt_state {
            SETUP,
            CONNECTED,
        };
        static Mqtt_handler* get_instance();
        void mqtt_handler_state_machine();


    private:
        Mqtt_handler();
        Mqtt_handler(const Mqtt_handler&) = delete;
        Mqtt_handler& operator=(const Mqtt_handler&) = delete;
        void reconnect() ;
        void mqtt_handler_state_transition(Mqtt_state new_state);
        String mqtt_state_to_string(Mqtt_state state);
        
        static Mqtt_handler* m_instance;
        Mqtt_state m_state;
        BearSSL::WiFiClientSecure *bear;
        BearSSL::CertStore certStore;
        PubSubClient * client;
};