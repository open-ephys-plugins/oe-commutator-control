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

#ifndef COMMUTATORTHREAD_H_DEFINFED
#define COMMUTATORTHREAD_H_DEFINFED

#include "../../Source/Utils/Utils.h"
#include "../../Source/CoreServices.h"
#include <BasicJuceHeader.h>
#include <SerialLib.h>
#include <atomic>
#include <cmath>
#include <limits>

class CommutatorThread : public HighResolutionTimer
{
public:
    void setSerial (String port);
    bool start();
    void stop();
    void hiResTimerCallback() override;
    void manualTurn (double turn);
    /** Sets the values of a Quaternion object using an array. Expected to be ordered as X/Y/Z/W for indices 0-3. */
    void setQuaternion (std::array<double, 4> quaternion);
    void setRotationAxis (Vector3D<double> axis);
    bool isReady() const;

private:
    /** Converts quaternion data to a twist. Quaternion values are expected to be ordered X/Y/Z/W. */
    double quaternionToTwist (Quaternion<double> quaternion);
    void sendTurn (double turn);

    ofSerial serial;

    double lastTwist = std::numeric_limits<double>::quiet_NaN();
    double previousAngleAboutAxis = std::numeric_limits<double>::quiet_NaN();

    static inline const std::array<double, 4> defaultQuaternion { 0.0, 0.0, 0.0, 0.0 };

    std::atomic<std::array<double, 4>> runningQuaternion;
    Vector3D<double> rotationAxis = Vector3D<double> (0, 0, 0);

    bool open = false;
    std::atomic<bool> isRunning = false;

    CriticalSection serialLock;
};

#endif
