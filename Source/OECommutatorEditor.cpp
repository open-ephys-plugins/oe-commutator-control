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

#include "OECommutatorEditor.h"
#include "OECommutator.h"

OECommutatorEditor::OECommutatorEditor (GenericProcessor* parentNode)
    : GenericEditor (parentNode)
{
    desiredWidth = 170;

    vector<ofSerialDeviceInfo> devices = serial.getDeviceList();

    serialLabel = std::make_unique<Label> ("Serial label");
    serialLabel->setText ("Serial port", dontSendNotification);
    serialLabel->setBounds (5, 30, 90, 20);
    addAndMakeVisible (serialLabel.get());

    serialSelection = std::make_unique<ComboBox> ("Serial");
    serialSelection->setBounds (5, 50, 90, 20);
    for (int i = 0; i < devices.size(); i++)
    {
        serialSelection->addItem (devices[i].getDevicePath(), i + 1);
    }
    serialSelection->addListener (this);
    addAndMakeVisible (serialSelection.get());

    axisOverride = std::make_unique<UtilityButton> ("Override");
    axisOverride->setBounds (110, 30, 62, 18);
    axisOverride->setRadius (2.0f);
    axisOverride->setClickingTogglesState (true);
    axisOverride->setToggleState (false, dontSendNotification);
    axisOverride->setTooltip ("Override the default axis of rotation");
    axisOverride->addListener (this);
    addAndMakeVisible (axisOverride.get());

    angleSelection = std::make_unique<ComboBoxParameterEditor> (parentNode->getParameter ("angle"));
    angleSelection->setLayout (ParameterEditor::Layout::nameHidden);
    angleSelection->setBounds (110, 50, 62, 20);
    angleSelection->setEnabled (axisOverride->getToggleState());
    addAndMakeVisible (angleSelection.get());

    streamLabel = std::make_unique<Label> ("Stream label");
    streamLabel->setText ("Stream", dontSendNotification);
    streamLabel->setBounds (5, 75, 90, 20);
    addAndMakeVisible (streamLabel.get());

    streamSelection = std::make_unique<ComboBox> ("Stream");
    streamSelection->setBounds (5, 95, 90, 20);
    streamSelection->addListener (this);
    addAndMakeVisible (streamSelection.get());

    manualTurnLabel = std::make_unique<Label> ("manualTurn");
    manualTurnLabel->setText ("Turn", dontSendNotification);
    manualTurnLabel->setBounds (118, 75, 50, 20);
    addAndMakeVisible (manualTurnLabel.get());

    leftButton = std::make_unique<ArrowButton> ("left", 0.5f, juce::Colours::black);
    leftButton->setBounds (117, 95, 20, 20);
    leftButton->addListener (this);
    leftButton->setRepeatSpeed (500, 100);
    addAndMakeVisible (leftButton.get());

    rightButton = std::make_unique<ArrowButton> ("right", 0.0f, juce::Colours::black);
    rightButton->setBounds (142, 95, 20, 20);
    rightButton->addListener (this);
    rightButton->setRepeatSpeed (500, 100);
    addAndMakeVisible (rightButton.get());
}

void OECommutatorEditor::buttonClicked (Button* btn)
{
    OECommutator* proc = (OECommutator*) getProcessor();
    if (btn == leftButton.get())
    {
        proc->manualTurn (0.1f);
    }
    else if (btn == rightButton.get())
    {
        proc->manualTurn (-0.1f);
    }
    else if (btn == axisOverride.get())
    {
        angleSelection->setEnabled (btn->getToggleState());
    }
}

void OECommutatorEditor::comboBoxChanged (ComboBox* cb)
{
    if (cb == streamSelection.get())
    {
        currentStream = cb->getSelectedId();

        if (currentStream > 0)
        {
            getProcessor()->getParameter ("current_stream")->setNextValue (currentStream);
        }
    }
    else if (cb == serialSelection.get())
    {
        getProcessor()->getParameter ("serial_name")->setNextValue (cb->getText());
    }
}

void OECommutatorEditor::updateSettings()
{
    streamSelection->clear();

    for (auto stream : getProcessor()->getDataStreams())
    {
        if (stream->getIdentifier().contains (".bno"))
        {
            if (currentStream == 0)
                currentStream = stream->getStreamId();

            streamSelection->addItem (stream->getName(), stream->getStreamId());
        }
        if (streamSelection->indexOfItemId (currentStream) == -1)
        {
            if (streamSelection->getNumItems() > 0)
                currentStream = streamSelection->getItemId (0);
            else
                currentStream = 0;
        }

        if (currentStream > 0)
        {
            streamSelection->setSelectedId (currentStream, sendNotification);
        }
    }
}

void OECommutatorEditor::startAcquisition()
{
    streamSelection->setEnabled (false);
    serialSelection->setEnabled (false);
    angleSelection->setEnabled (false);
    axisOverride->setEnabled (false);
}

void OECommutatorEditor::stopAcquisition()
{
    streamSelection->setEnabled (true);
    serialSelection->setEnabled (true);
    angleSelection->setEnabled (true);
    axisOverride->setEnabled (true);
}
