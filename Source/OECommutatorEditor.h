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

#ifndef PROCESSORPLUGINEDITOR_H_DEFINED
#define PROCESSORPLUGINEDITOR_H_DEFINED

#include <EditorHeaders.h>
#include <SerialLib.h>

class OECommutatorEditor : public GenericEditor,
                           public ComboBox::Listener,
                           public ArrowButton::Listener
{
public:
    /** Constructor */
    OECommutatorEditor (GenericProcessor* parentNode);

    /** Destructor */
    ~OECommutatorEditor() {}

    void buttonClicked (Button*) override;
    void comboBoxChanged (ComboBox*) override;

    void updateSettings() override;

    void startAcquisition() override;
    void stopAcquisition() override;

private:
    ofSerial serial;

    std::unique_ptr<ComboBoxParameterEditor> axisSelection;

    std::unique_ptr<ComboBox> serialSelection;
    std::unique_ptr<ComboBox> streamSelection;
    std::unique_ptr<Label> serialLabel;
    std::unique_ptr<Label> streamLabel;
    std::unique_ptr<UtilityButton> axisOverride;
    std::unique_ptr<Label> manualTurnLabel;
    std::unique_ptr<ArrowButton> leftButton;
    std::unique_ptr<ArrowButton> rightButton;

    uint16 currentStream = 0;

    /** Generates an assertion if this class leaks */
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (OECommutatorEditor);
};

#endif // PROCESSORPLUGINEDITOR_H_DEFINED
