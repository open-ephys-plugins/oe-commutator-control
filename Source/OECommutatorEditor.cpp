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
    desiredWidth = 185;

    vector<ofSerialDeviceInfo> devices = serial.getDeviceList();

    FontOptions labelFont ("Inter", "Regular", 14.0f);

    serialLabel = std::make_unique<Label> ("Serial label");
    serialLabel->setFont (labelFont);
    serialLabel->setText ("Serial port", dontSendNotification);
    serialLabel->setBounds (10, 30, 90, 20);
    addAndMakeVisible (serialLabel.get());

    serialSelection = std::make_unique<ComboBox> ("Serial");
    serialSelection->setBounds (10, 50, 90, 20);
    serialSelection->setTextWhenNothingSelected ("Select a port");
    for (int i = 0; i < devices.size(); i++)
    {
        serialSelection->addItem (devices[i].getDevicePath(), i + 1);
    }
    serialSelection->addListener (this);
    addAndMakeVisible (serialSelection.get());

    axisOverride = std::make_unique<UtilityButton> ("Override");
    axisOverride->setBounds (115, 30, 62, 18);
    axisOverride->setRadius (2.0f);
    axisOverride->setClickingTogglesState (true);
    axisOverride->setToggleState (false, dontSendNotification);
    axisOverride->setTooltip ("Override the default axis of rotation");
    axisOverride->addListener (this);
    addAndMakeVisible (axisOverride.get());

    axisSelection = std::make_unique<ComboBox> ("Axis Override");
    axisSelection->setBounds (115, 50, 62, 20);
    axisSelection->setEnabled (axisOverride->getToggleState());
    axisSelection->setTooltip ("Choose a specific axis to rotate around, based on the device orientation");
    int count = 1;
    for (const auto& axis : OECommutator::axes)
    {
        axisSelection->addItem (axis, count++);
    }
    axisSelection->setSelectedItemIndex (0, dontSendNotification);
    axisSelection->addListener (this);
    addAndMakeVisible (axisSelection.get());

    streamLabel = std::make_unique<Label> ("Stream label");
    streamLabel->setFont (labelFont);
    streamLabel->setText ("Stream", dontSendNotification);
    streamLabel->setBounds (10, 75, 90, 20);
    addAndMakeVisible (streamLabel.get());

    streamSelection = std::make_unique<ComboBox> ("Stream");
    streamSelection->setBounds (10, 95, 90, 20);
    streamSelection->addListener (this);
    addAndMakeVisible (streamSelection.get());

    manualTurnLabel = std::make_unique<Label> ("manualTurn");
    manualTurnLabel->setFont (labelFont);
    manualTurnLabel->setText ("Turn", dontSendNotification);
    manualTurnLabel->setJustificationType (Justification::centred);
    manualTurnLabel->setBounds (122, 75, 45, 20);
    addAndMakeVisible (manualTurnLabel.get());

    leftButton = std::make_unique<ArrowButton> ("left", 0.5f, juce::Colours::black);
    leftButton->setBounds (122, 95, 20, 20);
    leftButton->addListener (this);
    leftButton->setRepeatSpeed (500, 100);
    addAndMakeVisible (leftButton.get());

    rightButton = std::make_unique<ArrowButton> ("right", 0.0f, juce::Colours::black);
    rightButton->setBounds (147, 95, 20, 20);
    rightButton->addListener (this);
    rightButton->setRepeatSpeed (500, 100);
    addAndMakeVisible (rightButton.get());
}

void OECommutatorEditor::setSerialSelection (std::string selection)
{
    for (int i = 0; i < serialSelection->getNumItems(); i++)
    {
        if (serialSelection->getItemText (i).equalsIgnoreCase (String (selection)))
        {
            serialSelection->setSelectedItemIndex (i, dontSendNotification);
        }
    }
}

std::string OECommutatorEditor::getAxisSelection()
{
    return axisSelection->getText().toStdString();
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
        axisSelection->setEnabled (btn->getToggleState());
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

    std::array<int, OECommutator::NUM_QUATERNION_CHANNELS> indices {};
    indices.fill (-1);

    for (auto stream : getProcessor()->getDataStreams())
    {
        if (stream->getIdentifier().contains (".9dof"))
        {
            auto channels = stream->getContinuousChannels();

            for (int i = 0; i < channels.size(); i++)
            {
                auto ch = channels[i];

                if (ch->getIdentifier().contains (".quaternion.w"))
                {
                    indices[(uint32_t) OECommutator::QuaternionChannel::W] = i;
                }
                else if (ch->getIdentifier().contains (".quaternion.x"))
                {
                    indices[(uint32_t) OECommutator::QuaternionChannel::X] = i;
                }
                else if (ch->getIdentifier().contains (".quaternion.y"))
                {
                    indices[(uint32_t) OECommutator::QuaternionChannel::Y] = i;
                }
                else if (ch->getIdentifier().contains (".quaternion.z"))
                {
                    indices[(uint32_t) OECommutator::QuaternionChannel::Z] = i;
                }
            }

            if (! OECommutator::verifyQuaternionChannelIndices (indices))
            {
                LOGD ("Invalid channel indices. Cannot find all quaternion channels in this data stream.");
                continue;
            }

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

    ((OECommutator*) getProcessor())->setChannelIndices (indices);
}

void OECommutatorEditor::startAcquisition()
{
    streamSelection->setEnabled (false);
    serialSelection->setEnabled (false);
    axisSelection->setEnabled (false);
    axisOverride->setEnabled (false);
}

void OECommutatorEditor::stopAcquisition()
{
    streamSelection->setEnabled (true);
    serialSelection->setEnabled (true);
    axisOverride->setEnabled (true);
    axisSelection->setEnabled (axisOverride->getToggleState());
}

void OECommutatorEditor::saveCustomParametersToXml (XmlElement* xml)
{
    LOGD ("Saving OECommutatorEditor settings.");

    xml->setAttribute ("COM_PORT", serialSelection->getText());
    xml->setAttribute ("OVERRIDE_STATUS", axisOverride->getToggleState());
    xml->setAttribute ("OVERRIDE_AXIS", axisSelection->getText());
}

void OECommutatorEditor::loadCustomParametersFromXml (XmlElement* xml)
{
    LOGD ("Loading OECommutatorEditor settings.");

    if (xml->hasAttribute ("COM_PORT"))
    {
        String comPort = xml->getStringAttribute ("COM_PORT");

        if (comPort.contains ("COM") && ! comPort.isEmpty())
        {
            getProcessor()->getParameter ("serial_name")->setNextValue (comPort);
        }
    }

    if (xml->hasAttribute ("OVERRIDE_STATUS"))
    {
        axisOverride->setToggleState (xml->getBoolAttribute ("OVERRIDE_STATUS"), sendNotification);
    }

    if (xml->hasAttribute ("OVERRIDE_AXIS"))
    {
        int axisIndex = OECommutator::getAxisIndex (xml->getStringAttribute ("OVERRIDE_AXIS").toStdString());

        if (axisIndex >= 0)
            axisSelection->setSelectedItemIndex (axisIndex, dontSendNotification);
    }
}
