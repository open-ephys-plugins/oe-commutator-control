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

#include "OECommutator.h"

#include "OECommutatorEditor.h"
#include <CoreServicesHeader.h>


OECommutator::OECommutator()
    : GenericProcessor("OE Commutator")
{
    addIntParameter(Parameter::GLOBAL_SCOPE, "current_stream", "Currently selected stream",
        0, 0, 200000, false);

    addStringParameter(Parameter::GLOBAL_SCOPE, "serial_name", "Serial port name", "", true);
    addIntParameter(Parameter::GLOBAL_SCOPE, "angle", "Selected angle", 1, 1, 6);

    commutator = std::make_unique<CommutatorThread>();
}


OECommutator::~OECommutator()
{

}


AudioProcessorEditor* OECommutator::createEditor()
{
    editor = std::make_unique<OECommutatorEditor>(this);
    return editor.get();
}


void OECommutator::parameterValueChanged(Parameter* parameter)
{
    if (parameter->getName().equalsIgnoreCase("current_stream"))
    {

        uint16 candidateStream = (uint16)(int)parameter->getValue();

        if (streamExists(candidateStream))
            currentStream = candidateStream;

    }
    else if (parameter->getName().equalsIgnoreCase("serial_name"))
    {
        commutator->setSerial(parameter->getValueAsString());
    }
    else if (parameter->getName().equalsIgnoreCase("angle"))
    {
        int value = parameter->getValue();
        selAngle = (value - 1) / 2;
        selPol = (value - 1) % 2;
    }
}

bool OECommutator::startAcquisition()
{
    return commutator->start();
}

bool OECommutator::stopAcquisition()
{
    commutator->stop();
    return true;
}

void OECommutator::process(AudioBuffer<float>& buffer)
{
    double roll, pitch, heading;
    if (currentStream != 0)
    {
        std::array<double, 4> q;
        int nSamples = getNumSamplesInBlock(currentStream);
        if (nSamples > 0) {
            for (int i = 0; i < 4; i++)
            {
                int chanIndex = getDataStream(currentStream)->getContinuousChannels()[i]->getGlobalIndex();
                q[i] = buffer.getSample(chanIndex, nSamples - 1);
            }
            //test if there is a pitch singularity and correct
            double norm = q[0] * q[0] + q[1] * q[1] + q[2] * q[2] + q[3] * q[3];
            double sing = q[1] * q[2] + q[0] * q[3];

            if (sing > 0.499 * norm)
            {
                heading = 2 * std::atan2(q[1], q[0]);
                pitch = MathConstants<double>::halfPi;
                roll = 0;
            }
            else if (sing < -0.499 * norm)
            {
                heading = -2 * std::atan2(q[1], q[0]);
                pitch = -MathConstants<double>::halfPi;
                roll = 0;
            }
            else
            {

                // Roll (rotation around the x-axis)
                roll = std::atan2(2 * (q[0] * q[1] + q[2] * q[3]), 1 - 2 * (q[1] * q[1] + q[2] * q[2]));
                // Pitch (rotation around the y-axis)
                double arg = 2 * (q[0] * q[2] - q[3] * q[1]);
                arg = std::max(-1.0, std::min(1.0, arg));
                pitch = std::asin(arg);
                // Yaw (rotation around the z-axis, or heading)
                heading = std::atan2(2 * (q[0] * q[3] + q[1] * q[2]), 1 - 2 * (q[2] * q[2] + q[3] * q[3]));
            }
            double angle;
            if (selAngle == 0) angle = heading;
            else if (selAngle == 1) angle = pitch;
            else angle = roll;

            if (selPol != 0) angle = -angle;

            commutator->setHeading(angle);
            static_cast<OECommutatorEditor*>(editor.get())->setHeading(angle);
        }
    }
}

bool OECommutator::streamExists(uint16 streamId)
{
    for (auto stream : getDataStreams())
    {
        if (stream->getStreamId() == streamId)
            return true;
    }

    return false;

}

void OECommutator::manualTurn(double turn)
{
    commutator->manualTurn(turn);
}


