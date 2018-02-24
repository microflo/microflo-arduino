/* microflo_component yaml
name: UltrasoundSR04
description: "Read distance using a HC-SR04 ultrasonic sensor"
icon: "arrows-h"
inports:
  trigger:
    type: bang
    description: "Trigger a new read-out"
  trigpin:
    type: integer
    description: "Pin connected to Trig on SR04 board"
  echopin:
    type: integer
    description: "Ping connected to Echo on SR04 board"
  maxdistance:
    type: integer
    description: "Maximum distance that is considered valid (in cm)"
outports:
  triggered:
    type: bang
    description: "A read-out was just triggered"
  distance:
    type: number
    description: "Distance to object (in cm)"
microflo_component */

#ifdef HAVE_NEWPING
#include <NewPing.h>
#endif


class UltrasoundSR04 : public SingleOutputComponent {

public:
#ifdef HAVE_NEWPING
    static void echoCheck(NewPing *ping) {
        // If ping received, set the sensor distance to array.
        if (ping->check_timer()) {
            const long distance = ping->ping_result / US_ROUNDTRIP_CM;
            UltrasoundSR04 *component = (UltrasoundSR04 *)ping->user_data;
            if (component) {
                component->sendDistance(distance);
            }
        }
    }
#endif

public:
    UltrasoundSR04()
        : echoPin(-1)
        , trigPin(-1)
        , maxDistance(200)
#ifdef HAVE_NEWPING
        , ping(-1, -1, maxDistance)
#endif
    {
    }

    virtual void process(Packet in, MicroFlo::PortId port) {
        using namespace UltrasoundSR04Ports;

        if (port == InPorts::trigger) {
#ifdef HAVE_NEWPING
            ping.timer_stop(); // assurance
            ping.ping_timer(echoCheck); // setup callback
#endif
            send((bool)true, OutPorts::triggered);
        } else if (port == InPorts::trigpin && in.isNumber()) {
            trigPin = in.asInteger();
            checkInitialize();
        } else if (port == InPorts::echopin && in.isNumber()) {
            echoPin = in.asInteger();
            checkInitialize();
        }
    }

    void sendDistance(long distance) {
        using namespace UltrasoundSR04Ports;
        send(distance, OutPorts::distance);
    }
private:
    inline const bool pinsValid() {
        return (echoPin > 0 && trigPin > 0);
    }

    void checkInitialize() {
        if (!pinsValid()) {
            return;
        }
#ifdef HAVE_NEWPING
        ping = NewPing(echoPin, trigPin, maxDistance);
        ping.user_data = (void *)this;
#endif
    }

private:
    int8_t echoPin;
    int8_t trigPin;
#ifdef HAVE_NEWPING
    NewPing ping;
#endif
    long maxDistance;
};
