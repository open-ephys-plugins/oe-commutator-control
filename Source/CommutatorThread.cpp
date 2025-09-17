/*
------------------------------------------------------------------

This file is part of the Open Ephys GUI
Copyright (C) 2022 Open Ephys

------------------------------------------------------------------

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "CommutatorThread.h"
#include <algorithm>

void CommutatorThread::setSerial (String port)
{
    if (port.isEmpty())
        return;

    ScopedLock lock (serialLock);
    serial.close();
    open = serial.setup (port.toRawUTF8(), 9600);

    if (! open)
    {
        LOGE ("Unable to open serial port \"" + port + "\".");
    }
    else
    {
        LOGD ("Opened serial port \"" + port + "\".");
    }
}

void CommutatorThread::setRotationAxis (Vector3D<double> axis)
{
    if (! isRunning)
    {
        rotationAxis = axis;
    }
}

void CommutatorThread::setQuaternion (std::array<double, 4> quaternion)
{
    runningQuaternion = quaternion;
}

bool CommutatorThread::isReady() const
{
    if (! open)
    {
        LOGE ("Serial port is not open. Cannot start until the port is opened.");
        CoreServices::sendStatusMessage ("Commutator: Serial port is not open.");
    }

    if (rotationAxis.length() != 1)
    {
        LOGE ("Rotation axis is invalid. Expected a total length of 1, but length is ", rotationAxis.length());
        CoreServices::sendStatusMessage ("Commutator: Invalid rotation axis");
    }

    return open && rotationAxis.length() == 1;
}

bool CommutatorThread::start()
{
    lastTwist = std::numeric_limits<double>::quiet_NaN();
    lastQuaternion = defaultQuaternion;
    runningQuaternion = defaultQuaternion;

    if (open && rotationAxis.length() == 1)
    {
        startTimer (100);
        isRunning = true;
        return true;
    }
    else
    {
        isRunning = false;
        return false;
    }
}

void CommutatorThread::stop()
{
    stopTimer();
    isRunning = false;
}

void CommutatorThread::manualTurn (double turn)
{
    if (open)
        sendTurn (turn);
}

void CommutatorThread::sendTurn (double turn)
{
    String json = "{turn: " + String (turn, 5, false) + "}\r\n";
    const char* str = json.toRawUTF8();
    int len = json.getNumBytesAsUTF8();

    ScopedLock lock (serialLock);
    int n = serial.writeBytes (reinterpret_cast<unsigned char*> (const_cast<char*> (str)), len);
}

double CommutatorThread::quaternionToTwist (Quaternion<double> currentQuaternion, Quaternion<double> lastQuaternion)
{
    const Vector3D<double> commutatorAxis (0, 0, 1);
    const Vector3D<double> headstageAxis (0, 0, 1);

    Vector3D<double> localAxis = headstageAxis;
    Vector3D<double> globalAxis = rotationAxis;

    Quaternion<double> lastConjugated(-lastQuaternion.vector, lastQuaternion.scalar);

    // Get incremental rotation
    Quaternion<double> deltaQuaternion(currentQuaternion);
    deltaQuaternion *= lastConjugated; //juce quaternions do not implement a*b operator, only a *= b.
    deltaQuaternion = deltaQuaternion.normalised(); //remove any possible floating point error

    // Get device rotation axis in global coordinates
    Quaternion<double> deviceAxis (localAxis, 0);
    Quaternion<double> projection (lastQuaternion);
    projection *= deviceAxis;
    projection *= lastConjugated;
    projection = projection.normalised();


    // Calculate rotation across the axis in global coordinates
    double localDotProduct = deltaQuaternion.vector * projection.vector;

    double incrementalAngleLocal = 2 * std::atan2 (localDotProduct, deltaQuaternion.scalar);

    //calculate the rotation across the commutator axis
    double globalDotProduct = deltaQuaternion.vector * globalAxis;
    double incrementalAngleGlobal = 2 * std::atan2 (globalDotProduct, deltaQuaternion.scalar);

    //get the cosine from the rotated axis and the commutator axis
    //since vectors are normalised, this is just the dot product
    double cos_angle = projection.vector * globalAxis;

    //Remove the local twist from the global rotation to get a weighted total rotation
    double totalAngle = incrementalAngleLocal +(incrementalAngleGlobal - incrementalAngleLocal * cos_angle);


    return -totalAngle / MathConstants<double>::twoPi;

}

void CommutatorThread::hiResTimerCallback()
{
    std::array<double, 4> currentQuaternion = runningQuaternion;

    if (currentQuaternion == defaultQuaternion)
        return;

    double currentTwist = 0;
    if (lastQuaternion != defaultQuaternion)
        currentTwist = quaternionToTwist (Quaternion<double> (currentQuaternion[1], currentQuaternion[2], currentQuaternion[3], currentQuaternion[0]).normalised(),
                                          Quaternion<double> (lastQuaternion[1], lastQuaternion[2], lastQuaternion[3], lastQuaternion[0]).normalised());

    lastQuaternion = currentQuaternion;

    if (! isnan (lastTwist))
    {
        if (std::abs (currentTwist) > 0.01)
        {
            sendTurn (currentTwist);
            lastTwist = currentTwist;
        }
    }
    else
    {
        lastTwist = currentTwist;
    }
}
