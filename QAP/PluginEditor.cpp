/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "ExplosionImpl.h"

//==============================================================================
QAPAudioProcessorEditor::QAPAudioProcessorEditor (QAPAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p),thumbnail(512, audioProcessor.formatManager, thumbnailCache)
{
    addAndMakeVisible(wavFileList);
    wavFileList.setModel(this);

    addAndMakeVisible(loadLibraryButton);
    loadLibraryButton.setButtonText("Load Library");
    loadLibraryButton.onClick = [this] { chooseLibraryFolder(); };

    addAndMakeVisible(searchBar);
    searchBar.setTextToShowWhenEmpty("Search sounds...", juce::Colours::grey);
    searchBar.onTextChange = [this]()
        {
        filterFileList(searchBar.getText());
        };

    filteredWavFileNames = audioProcessor.getWavFileNames();

    juce::Image loadedAssistantImage;
    juce::File imageFile("/Users/nellygarcia/Downloads/Irhedoki.png");

    DBG("Attempting to load assistant image from: " + imageFile.getFullPathName());

    if (imageFile.existsAsFile())
    {
        DBG("✅ Image file exists.");
        auto inputStream = imageFile.createInputStream();
        if (inputStream != nullptr)
        {
            DBG("✅ Input stream created successfully.");
            loadedAssistantImage = juce::ImageFileFormat::loadFrom(*inputStream); // Load image data from stream

            if (loadedAssistantImage.isValid()) // Check if the loaded image data is valid
            {
                DBG("✅ Image data is valid. Setting image for component.");
                assistantImage.setImage(loadedAssistantImage); // Assign the valid image to the component
            }
            else
            {
                DBG("❌ Error: Loaded image data is NOT valid. File might be corrupted or not a valid PNG/image format. Path: " + imageFile.getFullPathName());
                assistantImage.setImage({}); // Fallback: Set an empty (valid) image to prevent crashes
            }
        }
        else
        {
            DBG("❌ Error: Failed to create input stream for image file. Check file permissions or if file is in use by another app. Path: " + imageFile.getFullPathName());
            assistantImage.setImage({}); // Fallback: Set an empty (valid) image
        }
    }
    else // THIS IS THE CRITICAL MISSING ELSE BLOCK FROM YOUR PREVIOUS CODE
    {
        DBG("❌ Error: Image file DOES NOT exist at the specified path. Double-check case, spelling, and actual location. Path: " + imageFile.getFullPathName());
        assistantImage.setImage({}); // Fallback: Set an empty (valid) image to prevent crashes
    }

    addAndMakeVisible(assistantImage);
    addAndMakeVisible(assistantLabel);

    assistantLabel.setColour(juce::Label::textColourId, juce::Colours::orange);
    assistantLabel.setFont(juce::Font(14.0f));
    assistantLabel.setJustificationType(juce::Justification::centredLeft);

    assistantImage.setVisible(false);
    assistantLabel.setVisible(false);
    
    // Procedural Audio Models.
    setupExplosionUI();
    setupFireUI();
    

    setSize(700, 700); // Set the overall size of your plugin editor
}

QAPAudioProcessorEditor::~QAPAudioProcessorEditor()
{
}

//==============================================================================
void QAPAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    g.setColour (juce::Colours::white);
    g.setFont (15.0f);

    // waveformBounds is Rectangle<float> - perfect for layout with floats

    if (thumbnail.getTotalLength() > 0.0)
    {
        g.setColour(juce::Colours::lightblue);
        // Convert to Rectangle<int> *only here* for drawChannels, which expects ints:
        thumbnail.drawChannels(g,
                               waveformBounds.toNearestInt(),
                               0.0,
                               thumbnail.getTotalLength(),
                               1.0f);
    }
    else
    {
        g.setColour(juce::Colours::grey);
        // drawText needs Rectangle<int>, so convert here too
        g.drawText("No waveform loaded", waveformBounds.toNearestInt(), juce::Justification::centred);
    }
}


void QAPAudioProcessorEditor::setupExplosionUI()
{
    // === Explosion Panel ===
    // Add the panel first
    addAndMakeVisible(explosionPanel);
    explosionPanel.setVisible(false);

    // === Trigger Button ===
    addAndMakeVisible(triggerButton);
    triggerButton.setButtonText("Trigger Explosion");
    triggerButton.onClick = [this]() { audioProcessor.triggerExplosion(); };


    // Rumble Slider
    addAndMakeVisible(rumbleSlider);
    rumbleSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    rumbleSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 60, 20);
    rumbleAttachment.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(audioProcessor.parameters, "rumble", rumbleSlider));
    
    addAndMakeVisible(rumbleDecaySlider);
    rumbleDecaySlider.setSliderStyle(juce::Slider::LinearHorizontal);
    rumbleDecaySlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 60, 20);
    rumbleDecayAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters, "rumbleDecay", rumbleDecaySlider);
    
    addAndMakeVisible(AirSlider);
    AirSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    airAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters, "air", AirSlider);
        
    addAndMakeVisible(AirDecaySlider);
    AirDecaySlider.setSliderStyle(juce::Slider::LinearHorizontal);
    airDecayAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters, "airDecay", AirDecaySlider);
    
    addAndMakeVisible(DustSlider);
    DustSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    dustAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters, "dust", DustSlider);

    addAndMakeVisible(DustDecaySlider);
    DustDecaySlider.setSliderStyle(juce::Slider::LinearHorizontal);
    dustDecayAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters, "dustDecay", DustDecaySlider);
        
    addAndMakeVisible(GritAmountSlider);
    GritAmountSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    gritAmountAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters, "gritAmount", GritAmountSlider);
}


int QAPAudioProcessorEditor::getNumRows()
{
    //return audioProcessor.wavFileNames.size();
    return filteredWavFileNames.size();
}

void QAPAudioProcessorEditor::paintListBoxItem(int rowNumber, juce::Graphics& g,
                                               int width, int height, bool rowIsSelected)
{
    if (rowIsSelected)
        g.fillAll(juce::Colours::lightblue);

    if (rowNumber >= 0 && rowNumber < filteredWavFileNames.size())
    {
        g.setColour(juce::Colours::black);
        g.drawText(filteredWavFileNames[rowNumber], 5, 0, width, height, juce::Justification::centredLeft);
    }
}


void QAPAudioProcessorEditor::refreshWavFileList()
{
    filteredWavFileNames = audioProcessor.getWavFileNames();
    wavFileList.updateContent();
    wavFileList.repaint();
}

void QAPAudioProcessorEditor::resized()
{
    int y = 20;

    loadLibraryButton.setBounds(20, y, 150, 30);
    y = loadLibraryButton.getBottom() + 10;

    int rightPanelWidth = explosionMode ? 250 : 0; // Reserve space for explosion panel

    searchBar.setBounds(20, y, getWidth() - 40 - rightPanelWidth, 24);
    y = searchBar.getBottom() + 10;

    // Leave 120 pixels at the bottom for the waveform
    int waveformHeight = 100;
    int waveformMargin = 20;
    int waveformY = getHeight() - waveformHeight - waveformMargin;

    wavFileList.setBounds(10, y, getWidth() - 20 - rightPanelWidth, waveformY - y);

    waveformBounds = juce::Rectangle<float>(10.0f, (float)waveformY,
                                            getWidth() - 20.0f - rightPanelWidth,
                                            (float)waveformHeight);

    assistantImage.setBounds(20, getHeight() - 210, 80, 80); // Adjust size as needed
    assistantLabel.setBounds(110, getHeight() - 210, getWidth() - 130 - rightPanelWidth, 80);

    layoutExplosionUI();
}


//EXPLOSION UI
void QAPAudioProcessorEditor::layoutExplosionUI()
{
    if (!explosionMode) return;

        const int panelWidth  = 250;
        const int margin      = 20;
        const int panelX      = getWidth() - panelWidth - margin;
        const int top         = 60;
        const int sliderH     = 40;
        const int spacing     = 16;

        explosionPanel.setBounds(panelX, top, panelWidth, getHeight() - top - 20);

        int y = top + 30;
        auto line = [&](juce::Component& c)
        {
            c.setBounds(panelX + 10, y, panelWidth - 20, sliderH);
            y += sliderH + spacing;
        };
    line(triggerButton);
    line(rumbleSlider);
    line(rumbleDecaySlider);
    line(AirDecaySlider);
    line(AirSlider);
    line(DustSlider);
    line(DustDecaySlider);
    line(GritAmountSlider);
}

void QAPAudioProcessorEditor::filterFileList(const juce::String& searchText)
{
    filteredWavFileNames.clear();

    if (searchText.isEmpty())
    {
        filteredWavFileNames = audioProcessor.getWavFileNames();
    }
    else
    {
        for (auto& name : audioProcessor.getWavFileNames())
        {
            if (name.containsIgnoreCase(searchText))
                filteredWavFileNames.add(name);
        }
    }

    wavFileList.updateContent();
    wavFileList.repaint();
    updateAssistant(searchText); //Check if we're searching for the top 20 sound categories
}

void QAPAudioProcessorEditor::selectedRowsChanged(int lastRowSelected)
{
    if (lastRowSelected >= 0 && lastRowSelected < filteredWavFileNames.size())
    {
        auto fileName = filteredWavFileNames[lastRowSelected];
        audioProcessor.playWavFileByName(fileName); // play the file

        auto file = audioProcessor.getWavFileByName(fileName); // get the actual juce::File
        if (file.existsAsFile())
        {
            thumbnail.setSource(new juce::FileInputSource(file));

            repaint(); // Redraw the editor to show the new waveform
        }
    }
}


void QAPAudioProcessorEditor::setExplosionMode (bool shouldShow)
{
    if (explosionMode == shouldShow)
        return;

    explosionMode = shouldShow;

    
    explosionPanel.setVisible (shouldShow);

    if (shouldShow)
    {
        
        setSize (800, getHeight());
    }
    else
    {
        // shrink back
        setSize (500, getHeight());

    }

    resized();
}

//Virtual Friend.
void QAPAudioProcessorEditor::updateAssistant(const juce::String& searchText)
{
    if (searchText.containsIgnoreCase("explosion"))
    {
        assistantLabel.setText(
            "Hi.",
            juce::dontSendNotification);
        assistantImage.setVisible(true);
        assistantLabel.setVisible(true);
        setExplosionMode (true);


    }
    
    else
        {
            assistantImage.setVisible(false);
            assistantLabel.setVisible(false);
            setExplosionMode (false);
        }
    
}

