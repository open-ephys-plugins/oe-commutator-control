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

#include <ProcessorHeaders.h>
#include "CommutatorThread.h"

class OECommutator : public GenericProcessor
{
public:
	/** The class constructor, used to initialize any members. */
	OECommutator();

	/** The class destructor, used to deallocate memory */
	~OECommutator();

	/** If the processor has a custom editor, this method must be defined to instantiate it. */
	AudioProcessorEditor* createEditor() override;


	/** Defines the functionality of the processor.
		The process method is called every time a new data buffer is available.
		Visualizer plugins typically use this method to send data to the canvas for display purposes */
	void process(AudioBuffer<float>& buffer) override;


	void parameterValueChanged(Parameter* parameter) override;

	void manualTurn(double turn);

	bool startAcquisition() override;
	bool stopAcquisition() override;

private:
	uint16 currentStream;
	std::unique_ptr<CommutatorThread> commutator;

	int selAngle = 0;
	int selPol = 0;

	bool streamExists(uint16 streamId);

};

#endif