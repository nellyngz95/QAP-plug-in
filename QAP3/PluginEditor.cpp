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
    
    setupGunUI();
    
    setupJetUI();
    
    setupHelicopterUI();
    
    setupRocketUI();
    

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

void QAPAudioProcessorEditor::setupRocketUI()
{
    addAndMakeVisible(RocketPanel);
    RocketPanel.setVisible(false);
    
    // === Trigger Button ===
    addAndMakeVisible(RocketButton);
    RocketButton.setButtonText("Start Rocket");
    RocketButton.onClick = [this]() { audioProcessor.triggerRocket(); };
    auto setupRotarySlider = [](juce::Slider& s, juce::Label& l, const juce::String& text)
            {
                s.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
                s.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
                l.setText(text, juce::dontSendNotification);
                l.setJustificationType(juce::Justification::centred);
                // We will no longer attach the label here, as the flexbox will handle it
            };
                    

    setupRotarySlider(DurationSlider, DurationLabel, "Duration");
    addAndMakeVisible(DurationSlider);
    addAndMakeVisible(DurationLabel);

    setupRotarySlider(ChamberResonanceSlider, ChamberResonanceLabel, "Chamber Resonance");
    addAndMakeVisible(ChamberResonanceSlider);
    addAndMakeVisible(ChamberResonanceLabel);

    setupRotarySlider(FlutterSlider, FlutterLabel, "Flutter");
    addAndMakeVisible(FlutterSlider);
    addAndMakeVisible(FlutterLabel);

    durationAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters, "duration", DurationSlider);
  
    chamberResonanceDecayAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters, "chamberresonance", ChamberResonanceSlider);
 
    flutterAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters, "flutter", FlutterSlider);
    
    setSize (400, 300);
}

void QAPAudioProcessorEditor::setupHelicopterUI()
{
    addAndMakeVisible(helicopterPanel);
    helicopterPanel.setVisible(false);
    
    // === Trigger Button ===
    addAndMakeVisible(HelicopterButton);
    HelicopterButton.setButtonText("Start Helicopter");
    HelicopterButton.onClick = [this]() { audioProcessor.triggerHelicopter(); };
    
    auto setupRotarySlider = [&](juce::Slider& s, juce::Label& l, const juce::String& text)
       {
           addAndMakeVisible(s);
           addAndMakeVisible(l);
           s.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
           s.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
           l.setText(text, juce::dontSendNotification);
           l.setJustificationType(juce::Justification::centred);
           l.attachToComponent(&s, false); // Label is above the slider
       };

       // Set up all the sliders and their labels using the lambda
       setupRotarySlider(RotorPeriodSlider, RotorPeriodLabel, "Rotor Period");
       setupRotarySlider(PeriodSlider, PeriodLabel, "Period");
       setupRotarySlider(TailMixSlider, TailMixLabel, "Tail Mix");
       setupRotarySlider(BaseFreqSlider, BaseFreqLabel, "Base Freq");
       setupRotarySlider(RotorMixSlider, RotorMixLabel, "Rotor Mix");
       setupRotarySlider(EngineMixSlider, EngineMixLabel, "Engine Mix");
       setupRotarySlider(BladeNoiseSlider, BladeNoiseLabel, "Blade Noise");
       setupRotarySlider(EngineSpeedSlider, EngineSpeedLabel, "Engine Speed");
    
    
    RotorPeriodAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters, "rotorPeriod", RotorPeriodSlider);
    PeriodAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters, "period", PeriodSlider);
    TailMixAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters, "tailMix", TailMixSlider);
    BaseFreqAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters, "baseFreq", BaseFreqSlider);
    RotorMixAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters, "rotorMix", RotorMixSlider);
    EngineMixAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters, "engineMix", EngineMixSlider);
    BladeNoiseAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters, "bladeNoise", BladeNoiseSlider);
    EngineSpeedAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters, "engineSpeed", EngineSpeedSlider);

    
    setSize (400, 300);
    
}


void QAPAudioProcessorEditor::setupJetUI()
{
    addAndMakeVisible(JetPanel);
    JetPanel.setVisible(false);
    // === Trigger Button ===
    addAndMakeVisible(JetButton);
    JetButton.setButtonText("Start Jet");
    JetButton.onClick = [this]() { audioProcessor.triggerJet(); };
    
    addAndMakeVisible(SpeedSlider);
    SpeedSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    SpeedAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters, "speed", SpeedSlider);
    addAndMakeVisible(TurbineSlider);
    TurbineSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    TurbineAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters, "turbine", TurbineSlider);
    addAndMakeVisible(BurnSlider);
    BurnSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    TurbineAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters, "burn", BurnSlider);
}

void QAPAudioProcessorEditor::setupGunUI()
{
    addAndMakeVisible(GunPanel);
    GunPanel.setVisible(false);
    
    // === Trigger Button ===
    addAndMakeVisible(GunButton);
    GunButton.setButtonText("Shot Gun");
    GunButton.onClick = [this]() { audioProcessor.triggerGun(); };
   
    addAndMakeVisible(ShellFreqSlider);
    ShellFreqSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    ShellFreqAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters, "shellfreq", ShellFreqSlider);
    addAndMakeVisible(ShellFreqDecaySlider);
    ShellFreqDecaySlider.setSliderStyle(juce::Slider::LinearHorizontal);
    ShellFreqDecayAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters, "shellfreqdecay", ShellFreqDecaySlider);
    setSize (400, 300);
    
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
    layoutGunUI();
    layoutJetUI();
    layoutHelicopterUI();
    layoutRocketUI();
}

//Rocket UI
void QAPAudioProcessorEditor::layoutRocketUI()
{
    if (!RocketMode) return;

    const int panelWidth  = 250;
    const int margin      = 20;
    const int panelX      = getWidth() - panelWidth - margin;
    const int top         = 60;
    const int sliderH     = 100;
    const int spacing     = 20;

    RocketPanel.setBounds(panelX, top, panelWidth, getHeight() - top - 20);

    int y = top + 30;
    auto line = [&](juce::Component& c)
    {
        c.setBounds(panelX + 10, y, panelWidth - 20, sliderH);
        y += sliderH + spacing;
    };

    line(RocketButton);
    line(DurationSlider);
    line(ChamberResonanceSlider);
    line(FlutterSlider);
}

//HelicopterUI
void QAPAudioProcessorEditor::layoutHelicopterUI()
{
    if (!HelicopterMode) return;

    const int panelWidth  = 250;
    const int margin      = 20;
    const int panelX      = getWidth() - panelWidth - margin;
    const int top         = 60;
    const int sliderH     = 100;
    const int spacing     = 20;

    helicopterPanel.setBounds(panelX, top, panelWidth, getHeight() - top - 20);

    int y = top + 30;
    auto line = [&](juce::Component& c)
    {
        c.setBounds(panelX + 10, y, panelWidth - 20, sliderH);
        y += sliderH + spacing;
    };

    line(HelicopterButton);
    line(RotorPeriodSlider);  // ✅ was missing
    line(PeriodSlider);
    line(TailMixSlider);
    line(BaseFreqSlider);
    line(RotorMixSlider);     // only once now
    line(EngineMixSlider);
    line(BladeNoiseSlider);
    line(EngineSpeedSlider);
}

//JetUI
void QAPAudioProcessorEditor::layoutJetUI()
{
    if(!JetMode)return;
    const int panelWidth  = 250;
    const int margin      = 20;
    const int panelX      = getWidth() - panelWidth - margin;
    const int top         = 60;
    const int sliderH     = 40;
    const int spacing     = 16;
    
    JetPanel.setBounds(panelX, top, panelWidth, getHeight() - top - 20);
    
    int y = top + 30;
    auto line = [&](juce::Component& c)
    {
        c.setBounds(panelX + 10, y, panelWidth - 20, sliderH);
        y += sliderH + spacing;
    };
    line(JetButton);
    line(SpeedSlider);
    line(TurbineSlider);
    line(BurnSlider);
}

//GunUI
void QAPAudioProcessorEditor::layoutGunUI()
{
    if(!GunMode)return;
    const int panelWidth  = 250;
    const int margin      = 20;
    const int panelX      = getWidth() - panelWidth - margin;
    const int top         = 60;
    const int sliderH     = 40;
    const int spacing     = 16;
    
    GunPanel.setBounds(panelX, top, panelWidth, getHeight() - top - 20);
    
    int y = top + 30;
    auto line = [&](juce::Component& c)
    {
        c.setBounds(panelX + 10, y, panelWidth - 20, sliderH);
        y += sliderH + spacing;
    };
    line(GunButton);
    line(ShellFreqSlider);
    line(ShellFreqDecaySlider);
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

//Rocket SetUp
void QAPAudioProcessorEditor::setRocketMode(bool shouldShow)
{
    if (RocketMode == shouldShow)
        return;
    //Rocket
    RocketMode = shouldShow;
    RocketPanel.setVisible(shouldShow);
    RocketButton.setVisible(shouldShow);
    DurationSlider.setVisible(shouldShow);
    ChamberResonanceSlider.setVisible(shouldShow);
    FlutterSlider.setVisible(shouldShow);
    
    //Helicopter
    HelicopterMode = false;
    helicopterPanel.setVisible(false);
    HelicopterButton.setVisible(false);
    RotorPeriodSlider.setVisible(false);
    PeriodSlider.setVisible(false);
    TailMixSlider.setVisible(false);
    BaseFreqSlider.setVisible(false);
    RotorMixSlider.setVisible(false);
    EngineMixSlider.setVisible(false);
    BladeNoiseSlider.setVisible(false);
    EngineSpeedSlider.setVisible(false);
    //Hide Jet
    JetPanel.setVisible(false);
    JetButton.setVisible(false);
    SpeedSlider.setVisible(false);
    TurbineSlider.setVisible(false);
    BurnSlider.setVisible(false);
    
    //Hide GUN GUI
    GunPanel.setVisible(false);
    GunButton.setVisible(false);
    ShellFreqSlider.setVisible(false);
    ShellFreqDecaySlider.setVisible(false);
    
    // hide all Fire UI elements
    firePanel.setVisible(false);
    FireButton.setVisible(false);
    LappingSlider.setVisible(false);
    HissingSlider.setVisible(false);
    CracklingSlider.setVisible(false);
    IntensitySlider.setVisible(false);

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

//HELICOPTER STUP

void QAPAudioProcessorEditor::setHelicopterMode(bool shouldShow)
{
    if (HelicopterMode == shouldShow)
        return;
    HelicopterMode = shouldShow;
    helicopterPanel.setVisible(shouldShow);
    HelicopterButton.setVisible(shouldShow);
    RotorPeriodSlider.setVisible(shouldShow);
    PeriodSlider.setVisible(shouldShow);
    TailMixSlider.setVisible(shouldShow);
    BaseFreqSlider.setVisible(shouldShow);
    RotorMixSlider.setVisible(shouldShow);
    EngineMixSlider.setVisible(shouldShow);
    BladeNoiseSlider.setVisible(shouldShow);
    EngineSpeedSlider.setVisible(shouldShow);
    
    //Hide Rocket
    RocketPanel.setVisible(false);
    RocketButton.setVisible(false);
    RocketPanel.setVisible(false);
    RocketButton.setVisible(false);
    DurationSlider.setVisible(false);
    ChamberResonanceSlider.setVisible(false);
    FlutterSlider.setVisible(false);
    //Hide Jet
    JetPanel.setVisible(false);
    JetButton.setVisible(false);
    SpeedSlider.setVisible(false);
    TurbineSlider.setVisible(false);
    BurnSlider.setVisible(false);
    
    //Hide GUN GUI
    GunPanel.setVisible(false);
    GunButton.setVisible(false);
    ShellFreqSlider.setVisible(false);
    ShellFreqDecaySlider.setVisible(false);
    
    // hide all Fire UI elements
    firePanel.setVisible(false);
    FireButton.setVisible(false);
    LappingSlider.setVisible(false);
    HissingSlider.setVisible(false);
    CracklingSlider.setVisible(false);
    IntensitySlider.setVisible(false);

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

void QAPAudioProcessorEditor::setJetMode(bool shouldShow)
{
    if (JetMode == shouldShow)
        return;

    JetMode = shouldShow;
    JetPanel.setVisible(shouldShow);
    JetButton.setVisible(shouldShow);
    SpeedSlider.setVisible(shouldShow);
    TurbineSlider.setVisible(shouldShow);
    BurnSlider.setVisible(shouldShow);
    //ROCKET
    RocketPanel.setVisible(false);
    RocketButton.setVisible(false);
    RocketPanel.setVisible(false);
    RocketButton.setVisible(false);
    DurationSlider.setVisible(false);
    ChamberResonanceSlider.setVisible(false);
    FlutterSlider.setVisible(false);
    //Hide Helicopter
    helicopterPanel.setVisible(false);
    HelicopterButton.setVisible(false);
    RotorPeriodSlider.setVisible(false);
    PeriodSlider.setVisible(false);
    TailMixSlider.setVisible(false);
    BaseFreqSlider.setVisible(false);
    RotorMixSlider.setVisible(false);
    EngineMixSlider.setVisible(false);
    BladeNoiseSlider.setVisible(false);
    EngineSpeedSlider.setVisible(false);
    
    //Hide JEt GUI
    GunPanel.setVisible(false);
    GunButton.setVisible(false);
    ShellFreqSlider.setVisible(false);
    ShellFreqDecaySlider.setVisible(false);
    
    // hide all Fire UI elements
    firePanel.setVisible(false);
    FireButton.setVisible(false);
    LappingSlider.setVisible(false);
    HissingSlider.setVisible(false);
    CracklingSlider.setVisible(false);
    IntensitySlider.setVisible(false);

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

void QAPAudioProcessorEditor::setGunMode(bool shouldShow)
{
    if (GunMode == shouldShow)
        return;

    GunMode = shouldShow;
    GunPanel.setVisible(shouldShow);
    GunButton.setVisible(shouldShow);
    ShellFreqSlider.setVisible(shouldShow);
    ShellFreqDecaySlider.setVisible(shouldShow);
    //ROCKET
    RocketPanel.setVisible(false);
    RocketButton.setVisible(false);
    RocketPanel.setVisible(false);
    RocketButton.setVisible(false);
    DurationSlider.setVisible(false);
    ChamberResonanceSlider.setVisible(false);
    FlutterSlider.setVisible(false);
    //Hide Helicopter GUI
    helicopterPanel.setVisible(false);
    HelicopterButton.setVisible(false);
    RotorPeriodSlider.setVisible(false);
    PeriodSlider.setVisible(false);
    TailMixSlider.setVisible(false);
    BaseFreqSlider.setVisible(false);
    RotorMixSlider.setVisible(false);
    EngineMixSlider.setVisible(false);
    BladeNoiseSlider.setVisible(false);
    EngineSpeedSlider.setVisible(false);
    
    //Hide JEt GUI
    JetPanel.setVisible(false);
    JetButton.setVisible(false);
    SpeedSlider.setVisible(false);
    TurbineSlider.setVisible(false);
    BurnSlider.setVisible(false);
    
    // hide all Fire UI elements
    firePanel.setVisible(false);
    FireButton.setVisible(false);
    LappingSlider.setVisible(false);
    HissingSlider.setVisible(false);
    CracklingSlider.setVisible(false);
    IntensitySlider.setVisible(false);

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
    //Rocket
    RocketPanel.setVisible(false);
    RocketButton.setVisible(false);
    RocketPanel.setVisible(false);
    RocketButton.setVisible(false);
    DurationSlider.setVisible(false);
    ChamberResonanceSlider.setVisible(false);
    FlutterSlider.setVisible(false);
    //Fire
    firePanel.setVisible(false);
    FireButton.setVisible(false);
    HissingSlider.setVisible(false);
    LappingSlider.setVisible(false);
    CracklingSlider.setVisible(false);
    IntensitySlider.setVisible(false);
    
    //GUN
    GunPanel.setVisible(false);
    GunButton.setVisible(false);
    ShellFreqSlider.setVisible(false);
    ShellFreqDecaySlider.setVisible(false);
    //JET
    JetPanel.setVisible(false);
    JetButton.setVisible(false);
    SpeedSlider.setVisible(false);
    TurbineSlider.setVisible(false);
    BurnSlider.setVisible(false);
    //Helicopter
    helicopterPanel.setVisible(false);
    HelicopterButton.setVisible(false);
    RotorPeriodSlider.setVisible(false);
    PeriodSlider.setVisible(false);
    TailMixSlider.setVisible(false);
    BaseFreqSlider.setVisible(false);
    RotorMixSlider.setVisible(false);
    EngineMixSlider.setVisible(false);
    BladeNoiseSlider.setVisible(false);
    EngineSpeedSlider.setVisible(false);
    
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
    //Rocket
    RocketPanel.setVisible(false);
    RocketButton.setVisible(false);
    RocketPanel.setVisible(false);
    RocketButton.setVisible(false);
    DurationSlider.setVisible(false);
    ChamberResonanceSlider.setVisible(false);
    FlutterSlider.setVisible(false);
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
    
    //Gun
    GunPanel.setVisible(false);
    GunButton.setVisible(false);
    ShellFreqSlider.setVisible(false);
    ShellFreqDecaySlider.setVisible(false);
    //Jet
    JetPanel.setVisible(false);
    JetButton.setVisible(false);
    SpeedSlider.setVisible(false);
    TurbineSlider.setVisible(false);
    BurnSlider.setVisible(false);

    //helicopter
    helicopterPanel.setVisible(false);
    HelicopterButton.setVisible(false);
    RotorPeriodSlider.setVisible(false);
    PeriodSlider.setVisible(false);
    TailMixSlider.setVisible(false);
    BaseFreqSlider.setVisible(false);
    RotorMixSlider.setVisible(false);
    EngineMixSlider.setVisible(false);
    BladeNoiseSlider.setVisible(false);
    EngineSpeedSlider.setVisible(false);
    
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
        setRocketMode(false);
        setExplosionMode(true);
        setGunMode(false);
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
        setRocketMode(false);
        setGunMode(false);
    }
    else if  (searchText.containsIgnoreCase("gun"))
    {
        assistantLabel.setText(
            "Hi.Gunshots",
            juce::dontSendNotification);
        assistantImage.setVisible(true);
        assistantLabel.setVisible(true);
        setExplosionMode(false);
        setFireMode(false);
        setRocketMode(false);
        setGunMode(true);
    }
    else if  (searchText.containsIgnoreCase("jet"))
    {
        assistantLabel.setText(
            "Hi.Jet sounds",
            juce::dontSendNotification);
        assistantImage.setVisible(true);
        assistantLabel.setVisible(true);
        setExplosionMode(false);
        setFireMode(false);
        setGunMode(false);
        setRocketMode(false);
        setJetMode(true);
    }
    else if  (searchText.containsIgnoreCase("helicopter"))
    {
        assistantLabel.setText(
            "Hi.Helicopters",
            juce::dontSendNotification);
        assistantImage.setVisible(true);
        assistantLabel.setVisible(true);
        setExplosionMode(false);
        setFireMode(false);
        setGunMode(false);
        setJetMode(false);
        setRocketMode(false);
        setHelicopterMode(true);
    }
    else if  (searchText.containsIgnoreCase("rocket"))
    {
        assistantLabel.setText(
            "Hi.Rockets",
            juce::dontSendNotification);
        assistantImage.setVisible(true);
        assistantLabel.setVisible(true);
        setExplosionMode(false);
        setFireMode(false);
        setGunMode(false);
        setJetMode(false);
        setHelicopterMode(false);
        setRocketMode(true);
    }
    
    else
    {
        assistantImage.setVisible(false);
        assistantLabel.setVisible(false);
        setExplosionMode (false);
        setFireMode (false);
        setJetMode(false);
        setHelicopterMode(false);
        setGunMode(false);
    }
    resized();
}

