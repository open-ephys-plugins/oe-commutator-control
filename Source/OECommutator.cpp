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
    : GenericProcessor ("OE Commutator")
{
    addIntParameter (Parameter::PROCESSOR_SCOPE, "current_stream", "Current Stream", "Currently selected stream", 0, 0, 200000, false);

    addStringParameter (Parameter::PROCESSOR_SCOPE, "serial_name", "Serial Name", "Serial port name", "", true);

    Array<String> angles = Array<String>();
    angles.add("+Z");
    angles.add("-Z");
    angles.add("+Y");
    angles.add("-Y");
    angles.add("+X");
    angles.add("-X");

    addCategoricalParameter (Parameter::PROCESSOR_SCOPE, "angle", "Angle", "Selected angle", angles, 0, true);

    commutator = std::make_unique<CommutatorThread>();
}

AudioProcessorEditor* OECommutator::createEditor()
{
    editor = std::make_unique<OECommutatorEditor> (this);
    return editor.get();
}

void OECommutator::parameterValueChanged (Parameter* parameter)
{
    if (parameter->getName().equalsIgnoreCase ("current_stream"))
    {
        uint16 candidateStream = (uint16) (int) parameter->getValue();

        if (streamExists (candidateStream))
            currentStream = candidateStream;
    }
    else if (parameter->getName().equalsIgnoreCase ("serial_name"))
    {
        commutator->setSerial (parameter->getValueAsString());
    }
}

bool OECommutator::isReady()
{
    return commutator->isReady() && streamExists(currentStream);
}

bool OECommutator::startAcquisition()
{
    commutator->setRotationAxis (getRotationAxis (getParameter ("angle")->getValueAsString()));
    return commutator->start();
}

bool OECommutator::stopAcquisition()
{
    commutator->stop();
    return true;
}

void OECommutator::process (AudioBuffer<float>& buffer)
{
    if (currentStream != 0)
    {
        std::array<double, 4> data = { 0, 0, 0, 0 };

        int nSamples = getNumSamplesInBlock (currentStream);
        if (nSamples > 0)
        {
            for (int i = 0; i < 4; i++)
            {
                int chanIndex = getDataStream (currentStream)->getContinuousChannels()[i]->getGlobalIndex();
                data[i] = buffer.getSample (chanIndex, nSamples - 1);
            }

            commutator->setQuaternion (data);
        }
    }
}

bool OECommutator::streamExists (uint16 streamId) const
{
    for (auto stream : getDataStreams())
    {
        if (stream->getStreamId() == streamId)
            return true;
    }

    return false;
}

void OECommutator::manualTurn (double turn)
{
    commutator->manualTurn (turn);
}

Vector3D<double> OECommutator::getRotationAxis(String angle) const
{
    if (angle == "+Z")
    {
        return Vector3D<double> (0, 0, 1);
    }
    else if (angle == "-Z")
    {
        return Vector3D<double> (0, 0, -1);
    }
    else if (angle == "+Y")
    {
        return Vector3D<double> (0, 1, 0);
    }
    else if (angle == "-Y")
    {
        return Vector3D<double> (0, -1, 0);
    }
    else if (angle == "+X")
    {
        return Vector3D<double> (1, 0, 0);
    }
    else if (angle == "-X")
    {
        return Vector3D<double> (-1, 0, 0);
    }
    else
    {
        return Vector3D<double> (0, 0, 0);
    }
}
