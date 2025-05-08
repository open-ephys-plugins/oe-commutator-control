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

#ifndef PROCESSORPLUGIN_H_DEFINED
#define PROCESSORPLUGIN_H_DEFINED

#include "CommutatorThread.h"
#include <ProcessorHeaders.h>

class OECommutator : public GenericProcessor
{
public:
    OECommutator();

    ~OECommutator() {};

    void registerParameters() override;

    AudioProcessorEditor* createEditor() override;

    void process (AudioBuffer<float>& buffer) override;

    void parameterValueChanged (Parameter* parameter) override;

    void manualTurn (double turn);

    bool startAcquisition() override;
    bool stopAcquisition() override;
    bool isReady() override;

    Vector3D<double> getRotationAxis (String axis) const;

    static constexpr int NUM_QUATERNION_CHANNELS = 4;

    enum class QuaternionChannel : uint32_t
    {
        W = 0,
        X = 1,
        Y = 2,
        Z = 3,
    };

    /** Sets the quaternion channel indices within a specific stream. Quaternion indices are expected to be ordered X/Y/Z/W. */
    void setChannelIndices (std::array<int, NUM_QUATERNION_CHANNELS> indices);

    /** Check that all indices are unique, and greater than or equal to zero. */
    static bool verifyQuaternionChannelIndices (std::array<int, NUM_QUATERNION_CHANNELS> indices);

private:
    uint16 currentStream = 0;
    std::unique_ptr<CommutatorThread> commutator;

    bool streamExists (uint16 streamId) const;

    std::array<int, NUM_QUATERNION_CHANNELS> channelIndices {};
};

#endif
