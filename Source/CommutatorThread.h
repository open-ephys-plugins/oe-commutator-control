#ifndef COMMUTATORTHREAD_H_DEFINFED
#define COMMUTATORTHREAD_H_DEFINFED
#include <BasicJuceHeader.h>
#include <SerialLib.h>
#include <cmath>
#include <limits>
#include <atomic>

class CommutatorThread :
    public HighResolutionTimer
{
public:

    void setSerial(String port);
    bool start();
    void stop();
    void hiResTimerCallback() override;
    void manualTurn(double turn);
    void setHeading(double heading);

private:
    void sendTurn(double turn);

    ofSerial serial;
    double lastHeading = std::numeric_limits<double>::quiet_NaN();
    std::atomic<double> runningHeading;
    bool open = false;

    CriticalSection serialLock;
};

#endif