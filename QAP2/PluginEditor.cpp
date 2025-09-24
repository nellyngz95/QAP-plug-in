/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "ExplosionImpl.h"
#include "FireImpl.h"
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

void QAPAudioProcessorEditor::setupFireUI()
{
    addAndMakeVisible(firePanel);
    firePanel.setVisible(false);

    // === Trigger Button ===
    addAndMakeVisible(FireButton);
    FireButton.setButtonText("Start Fire");
    FireButton.onClick = [this]() { audioProcessor.triggerFire(); };
    
    addAndMakeVisible(LappingSlider);
    LappingSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    LappingSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 60, 20);
    lappingAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters, "lapping", LappingSlider);
    addAndMakeVisible(HissingSlider);
    HissingSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    HissingSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 60, 20);
    hissingAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters, "hissing", HissingSlider);
    addAndMakeVisible(CracklingSlider);
    CracklingSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    CracklingSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 60, 20);
    cracklingAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters, "crackling", CracklingSlider);
    addAndMakeVisible(IntensitySlider);
    IntensitySlider.setSliderStyle(juce::Slider::LinearHorizontal);
    IntensitySlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 60, 20);
    intensityAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters, "intensity", IntensitySlider);
    
    
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


    // Explosion Parameters
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
        
    // Corrected line: Check for either explosionMode or FireMode.
    int rightPanelWidth = (explosionMode || FireMode) ? 250 : 0; // Reserve space for either panel
        
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
    layoutFireUI();
}

//Fire UI
void QAPAudioProcessorEditor::layoutFireUI()
{
    if(!FireMode)return;
    const int panelWidth  = 250;
    const int margin      = 20;
    const int panelX      = getWidth() - panelWidth - margin;
    const int top         = 60;
    const int sliderH     = 40;
    const int spacing     = 16;

    firePanel.setBounds(panelX, top, panelWidth, getHeight() - top - 20);

    int y = top + 30;
    auto line = [&](juce::Component& c)
    {
        c.setBounds(panelX + 10, y, panelWidth - 20, sliderH);
        y += sliderH + spacing;
    };
line(FireButton);
line(LappingSlider);
line(HissingSlider);
line(CracklingSlider);
line(IntensitySlider);

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


// in PluginEditor.cpp
void QAPAudioProcessorEditor::setExplosionMode(bool shouldShow)
{
    if (explosionMode == shouldShow)
        return;

    explosionMode = shouldShow;

    // Show/hide all Explosion UI elements
    explosionPanel.setVisible(shouldShow);
    triggerButton.setVisible(shouldShow);
    rumbleSlider.setVisible(shouldShow);
    rumbleDecaySlider.setVisible(shouldShow);
    AirSlider.setVisible(shouldShow);
    AirDecaySlider.setVisible(shouldShow);
    DustSlider.setVisible(shouldShow);
    DustDecaySlider.setVisible(shouldShow);
    GritAmountSlider.setVisible(shouldShow);

    // Explicitly hide all Fire UI elements
    firePanel.setVisible(false);
    FireButton.setVisible(false);
    HissingSlider.setVisible(false);
    LappingSlider.setVisible(false);
    CracklingSlider.setVisible(false);
    IntensitySlider.setVisible(false);

    if (shouldShow)
    {
        setSize(800, getHeight());
    }
    else
    {
        setSize(500, getHeight());
    }

    resized();
}


void QAPAudioProcessorEditor::setFireMode(bool shouldShow)
{
    if (FireMode == shouldShow)
        return;

    FireMode = shouldShow;

    // Show/hide all Fire UI elements
    firePanel.setVisible(shouldShow);
    FireButton.setVisible(shouldShow);
    LappingSlider.setVisible(shouldShow);
    HissingSlider.setVisible(shouldShow);
    CracklingSlider.setVisible(shouldShow);
    IntensitySlider.setVisible(shouldShow);

    // Explicitly hide all Explosion UI elements
    explosionPanel.setVisible(false);
    triggerButton.setVisible(false);
    rumbleSlider.setVisible(false);
    rumbleDecaySlider.setVisible(false);
    AirSlider.setVisible(false);
    AirDecaySlider.setVisible(false);
    DustSlider.setVisible(false);
    DustDecaySlider.setVisible(false);
    GritAmountSlider.setVisible(false);

    if (shouldShow)
    {
        setSize(800, getHeight());
    }
    else
    {
        setSize(500, getHeight());
    }

    resized();
}

//Virtual Friend.
void QAPAudioProcessorEditor::updateAssistant(const juce::String& searchText)
{
    if (searchText.containsIgnoreCase("explosion"))
    {
        assistantLabel.setText(
            "Hi.An explosion",
            juce::dontSendNotification);
        assistantImage.setVisible(true);
        assistantLabel.setVisible(true);
        setFireMode(false);
        setExplosionMode(true);
    }
    
    else if  (searchText.containsIgnoreCase("fire"))
    {
        assistantLabel.setText(
            "Hi.Fire sounds",
            juce::dontSendNotification);
        assistantImage.setVisible(true);
        assistantLabel.setVisible(true);
        setExplosionMode(false);
        setFireMode(true);
    }
    
    else
    {
        assistantImage.setVisible(false);
        assistantLabel.setVisible(false);
        // Corrected logic: Hide all panels when search text is not a match.
        setExplosionMode (false);
        setFireMode (false);
    }
    resized();
}

