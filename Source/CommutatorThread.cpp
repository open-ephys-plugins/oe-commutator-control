#include "CommutatorThread.h"
#include <algorithm>

void CommutatorThread::setSerial(String port)
{
	ScopedLock lock(serialLock);
	serial.close();
	open = serial.setup(port.toRawUTF8(), 9600);
	std::cout << "attempt to open " << port << " - " << open << std::endl;
}

void CommutatorThread::setHeading(double heading)
{
	runningHeading = heading;
}

bool CommutatorThread::start()
{
	lastHeading = std::numeric_limits<double>::quiet_NaN();
	if (open) startTimer(100);
	else return false;
}

void CommutatorThread::stop()
{
	stopTimer();
}

void CommutatorThread::manualTurn(double turn)
{
	if (open) sendTurn(turn);
}

void CommutatorThread::sendTurn(double turn)
{
	String json = "{turn: " + String(turn, 5, false) + "}\r\n";
	const char* str = json.toRawUTF8();
	int len = json.getNumBytesAsUTF8();
	{
		int n;
		ScopedLock lock(serialLock);
		n = serial.writeBytes(reinterpret_cast<unsigned char*>(const_cast<char*>(str)), len);
	}
}

void CommutatorThread::hiResTimerCallback()
{
	double currentHeading = runningHeading;
	if (!isnan(lastHeading))
	{
		std::array<double, 3> data;
		data[0] = currentHeading;
		data[1] = currentHeading + MathConstants<double>::twoPi;
		data[2] = currentHeading - MathConstants<double>::twoPi;
		std::sort(data.begin(), data.end(), [=](double a, double b) {
			return std::abs(a - lastHeading) < std::abs(b - lastHeading);
			});
		double p = data[0];

		double turn = (p - lastHeading) / MathConstants<double>::twoPi;
		if (std::abs(turn) > 0.01) {
			sendTurn(turn);
			lastHeading = currentHeading;
		}
	}
	else
	{
		lastHeading = currentHeading;
	}
}