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
    loadLibraryButton.setColour(juce::TextButton::buttonColourId, juce::Colours::darkred);
    loadLibraryButton.setColour(juce::TextButton::buttonOnColourId, juce::Colours::white);
    
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
    
    RocketButton.setColour(juce::TextButton::buttonColourId, juce::Colours::darkred);
    RocketButton.setColour(juce::TextButton::buttonOnColourId, juce::Colours::white);
    addAndMakeVisible(RocketButton);
    RocketButton.setButtonText("Start Rocket");
    RocketButton.onClick = [this]() { audioProcessor.triggerRocket(); };
    
    
    auto setupSliderAndLabel = [&](juce::Slider& slider, const char* paramID, juce::Label& label, const char* labelText)
    {
      
        slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 20);
        
    
        slider.setColour(juce::Slider::thumbColourId, juce::Colours::orange);
        slider.setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colours::darkgrey);
        slider.setColour(juce::Slider::rotarySliderFillColourId, juce::Colours::red.withAlpha(0.6f));
        addAndMakeVisible(slider);
        

        label.setText(labelText, juce::dontSendNotification);
        label.setFont(juce::Font(12.0f, juce::Font::bold));
        label.setJustificationType(juce::Justification::centred);
        label.setColour(juce::Label::textColourId, juce::Colours::orange);
        addAndMakeVisible(label);
        
        
        slider.setVisible(false);
        label.setVisible(false);
    };
    
   
    
    setupSliderAndLabel(DurationSlider, "duration", DurationLabel, "Duration");
    durationAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters, "duration", DurationSlider);
    DurationSlider.onDragStart = [this]() { updateAssistantForParameter("duration"); };
                    
    
    setupSliderAndLabel(ChamberResonanceSlider, "chamberresonance", ChamberResonanceLabel, "Chamber Resonance");
    chamberResonanceDecayAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters, "chamberresonance", ChamberResonanceSlider);
    ChamberResonanceSlider.onDragStart = [this]() { updateAssistantForParameter("chamberresonance"); };
 
    setupSliderAndLabel(FlutterSlider, "flutter", FlutterLabel, "Flutter");
    flutterAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters, "flutter", FlutterSlider);
    FlutterSlider.onDragStart = [this]() { updateAssistantForParameter("flutter"); };
    
    setSize (400, 300);
}

void QAPAudioProcessorEditor::setupHelicopterUI()
{
    addAndMakeVisible(helicopterPanel);
    helicopterPanel.setVisible(false);
    HelicopterButton.setColour(juce::TextButton::buttonColourId, juce::Colours::darkred);
    HelicopterButton.setColour(juce::TextButton::buttonOnColourId, juce::Colours::white);
    addAndMakeVisible(HelicopterButton);
    HelicopterButton.setButtonText("Start Helicopter");
    HelicopterButton.onClick = [this]() { audioProcessor.triggerHelicopter(); };
    
    auto setupSliderAndLabel = [&](juce::Slider& slider, const char* paramID, juce::Label& label, const char* labelText)
    {
      
        slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 20);
        
    
        slider.setColour(juce::Slider::thumbColourId, juce::Colours::orange);
        slider.setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colours::darkgrey);
        slider.setColour(juce::Slider::rotarySliderFillColourId, juce::Colours::red.withAlpha(0.6f));
        addAndMakeVisible(slider);
        

        label.setText(labelText, juce::dontSendNotification);
        label.setFont(juce::Font(12.0f, juce::Font::bold));
        label.setJustificationType(juce::Justification::centred);
        label.setColour(juce::Label::textColourId, juce::Colours::orange);
        addAndMakeVisible(label);
        
        
        slider.setVisible(false);
        label.setVisible(false);
    };
    
   
    
    setupSliderAndLabel(RotorPeriodSlider, "rotorPeriod", RotorPeriodLabel, "RotorPeriod");
    RotorPeriodAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters, "rotorPeriod", RotorPeriodSlider);
    RotorPeriodSlider.onDragStart = [this]() { updateAssistantForParameter("rotorPeriod"); };

    setupSliderAndLabel(PeriodSlider, "period",PeriodLabel, "Period");
    PeriodAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters, "period", PeriodSlider);
   PeriodSlider.onDragStart = [this]() { updateAssistantForParameter("period"); };
    
    setupSliderAndLabel(TailMixSlider, "tailMix",TailMixLabel, "Tail Mix");
    TailMixAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters, "tailMix", TailMixSlider);
    TailMixSlider.onDragStart = [this]() { updateAssistantForParameter("tailMix"); };
    
    setupSliderAndLabel(BaseFreqSlider, "baseFreq",BaseFreqLabel, "Base Frequency");
    BaseFreqAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters, "baseFreq", BaseFreqSlider);
   BaseFreqSlider.onDragStart = [this]() { updateAssistantForParameter("baseFreq"); };
    
    setupSliderAndLabel(RotorMixSlider, "rotorMix",RotorMixLabel, "Rotor Mix");
    RotorMixAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters, "rotorMix", RotorMixSlider);
    RotorMixSlider.onDragStart = [this]() { updateAssistantForParameter("rotorMix"); };
    
    
    setupSliderAndLabel(EngineMixSlider, "engineMix",EngineMixLabel, "Engine Mix ");
    EngineMixAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters, "engineMix", EngineMixSlider);
    EngineMixSlider.onDragStart = [this]() { updateAssistantForParameter("engineMix"); };
    
    setupSliderAndLabel(BladeNoiseSlider, "bladeNoise",BladeNoiseLabel, "Blade Noise");
    BladeNoiseAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters, "bladeNoise", BladeNoiseSlider);
    BladeNoiseSlider.onDragStart = [this]() { updateAssistantForParameter("bladeNoise"); };
    
    setupSliderAndLabel(EngineSpeedSlider, "engineSpeed",EngineSpeedLabel, "Engine Speed");
    EngineSpeedAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters, "engineSpeed", EngineSpeedSlider);
    EngineSpeedSlider.onDragStart = [this]() { updateAssistantForParameter("engineSpeed"); };

    
    setSize (400, 300);
    
}


void QAPAudioProcessorEditor::setupJetUI()
{
    addAndMakeVisible(JetPanel);
    JetPanel.setVisible(false);
    JetPanel.setVisible(false);
    addAndMakeVisible(JetButton);
    JetButton.setColour(juce::TextButton::buttonColourId, juce::Colours::darkred);
    JetButton.setColour(juce::TextButton::buttonOnColourId, juce::Colours::white);
    JetButton.setButtonText("Start Jet");
    JetButton.onClick = [this]() { audioProcessor.triggerJet(); };
    
    auto setupSliderAndLabel = [&](juce::Slider& slider, const char* paramID, juce::Label& label, const char* labelText)
    {
      
        slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 20);
        
    
        slider.setColour(juce::Slider::thumbColourId, juce::Colours::orange);
        slider.setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colours::darkgrey);
        slider.setColour(juce::Slider::rotarySliderFillColourId, juce::Colours::red.withAlpha(0.6f));
        addAndMakeVisible(slider);
        

        label.setText(labelText, juce::dontSendNotification);
        label.setFont(juce::Font(12.0f, juce::Font::bold));
        label.setJustificationType(juce::Justification::centred);
        label.setColour(juce::Label::textColourId, juce::Colours::orange);
        addAndMakeVisible(label);
        
        
        slider.setVisible(false);
        label.setVisible(false);
    };
    
    setupSliderAndLabel(SpeedSlider, "speed", SpeedLabel, "Speed");
    SpeedAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters, "speed", SpeedSlider);
   SpeedSlider.onDragStart = [this]() { updateAssistantForParameter("speed"); };
    

    setupSliderAndLabel(TurbineSlider, "turbine", TurbineLabel, "Turbine");
    TurbineAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters, "turbine", TurbineSlider);
    TurbineSlider.onDragStart = [this]() { updateAssistantForParameter("turbine"); };
    
    setupSliderAndLabel(BurnSlider, "burn", BurnLabel, "Burn");
    BurnAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters, "burn", BurnSlider);
    BurnSlider.onDragStart = [this]() { updateAssistantForParameter("burn"); };
}

void QAPAudioProcessorEditor::setupGunUI()
{
    addAndMakeVisible(GunPanel);
    GunPanel.setVisible(false);
    
    // === Trigger Button ===
    addAndMakeVisible(GunButton);
    GunButton.setColour(juce::TextButton::buttonColourId, juce::Colours::darkred);
    GunButton.setColour(juce::TextButton::buttonOnColourId, juce::Colours::white);
    GunButton.setButtonText("Shot Gun");
    GunButton.onClick = [this]() { audioProcessor.triggerGun(); };
    auto setupSliderAndLabel = [&](juce::Slider& slider, const char* paramID, juce::Label& label, const char* labelText)
    {
      
        slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 20);
        
    
        slider.setColour(juce::Slider::thumbColourId, juce::Colours::orange);
        slider.setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colours::darkgrey);
        slider.setColour(juce::Slider::rotarySliderFillColourId, juce::Colours::red.withAlpha(0.6f));
        addAndMakeVisible(slider);
        

        label.setText(labelText, juce::dontSendNotification);
        label.setFont(juce::Font(12.0f, juce::Font::bold));
        label.setJustificationType(juce::Justification::centred);
        label.setColour(juce::Label::textColourId, juce::Colours::orange);
        addAndMakeVisible(label);
        
        
        slider.setVisible(false);
        label.setVisible(false);
    };
    
    setupSliderAndLabel(ShellFreqSlider, "shellfreq", ShellFrequecyLabel, "Shell Frequency");
    ShellFreqAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters, "shellfreq", ShellFreqSlider);
   ShellFreqSlider.onDragStart = [this]() { updateAssistantForParameter("shellfreq"); };
    
    setupSliderAndLabel(ShellFreqDecaySlider, "shellfreqdecay", ShellFrequencyDecayLabel, "Shell Frequency Decay");
    ShellFreqDecayAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters, "shellfreqdecay", ShellFreqDecaySlider);
    ShellFreqDecaySlider.onDragStart = [this]() { updateAssistantForParameter("shellfreqdecay"); };
    
}

void QAPAudioProcessorEditor::setupFireUI()
{
    addAndMakeVisible(firePanel);
    firePanel.setVisible(false);

    // === Trigger Button ===
    addAndMakeVisible(FireButton);
    FireButton.setColour(juce::TextButton::buttonColourId, juce::Colours::darkred);
    FireButton.setColour(juce::TextButton::buttonOnColourId, juce::Colours::white);
    FireButton.setButtonText("Start Fire");
    FireButton.onClick = [this]() { audioProcessor.triggerFire(); };
    
  
    auto setupSliderAndLabel = [&](juce::Slider& slider, const char* paramID, juce::Label& label, const char* labelText)
    {
      
        slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 20);
        
    
        slider.setColour(juce::Slider::thumbColourId, juce::Colours::orange);
        slider.setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colours::darkgrey);
        slider.setColour(juce::Slider::rotarySliderFillColourId, juce::Colours::red.withAlpha(0.6f));
        addAndMakeVisible(slider);
        

        label.setText(labelText, juce::dontSendNotification);
        label.setFont(juce::Font(12.0f, juce::Font::bold));
        label.setJustificationType(juce::Justification::centred);
        label.setColour(juce::Label::textColourId, juce::Colours::orange);
        addAndMakeVisible(label);
        
        
        slider.setVisible(false);
        label.setVisible(false);
    };
    setupSliderAndLabel(LappingSlider, "lapping", LappingLabel, "Lapping");
    lappingAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters, "lapping", LappingSlider);
    LappingSlider.onDragStart = [this]() { updateAssistantForParameter("lapping"); };
    
    setupSliderAndLabel(HissingSlider, "hissing", HissingLabel, "Hissing");
    hissingAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters, "hissing", HissingSlider);
    HissingSlider.onDragStart = [this]() { updateAssistantForParameter("hissing"); };
    
    setupSliderAndLabel(CracklingSlider, "crackling", CracklingLabel, "Crackling");
    cracklingAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters, "crackling", CracklingSlider);
    CracklingSlider.onDragStart = [this]() { updateAssistantForParameter("crackling"); };
    
    setupSliderAndLabel(IntensitySlider, "intensity", IntensityLabel, "Intensity");
    intensityAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters, "intensity", IntensitySlider);
    IntensitySlider.onDragStart = [this]() { updateAssistantForParameter("intensity"); };
    
    
}

void QAPAudioProcessorEditor::setupExplosionUI()
{
    // === Explosion Panel ===
    addAndMakeVisible(explosionPanel);
    explosionPanel.setVisible(false);


    addAndMakeVisible(triggerButton);
    triggerButton.setButtonText("BOOM!");
    triggerButton.setColour(juce::TextButton::buttonColourId, juce::Colours::darkred);
    triggerButton.setColour(juce::TextButton::buttonOnColourId, juce::Colours::white);
    triggerButton.onClick = [this]() { audioProcessor.triggerExplosion(); };

    auto setupSliderAndLabel = [&](juce::Slider& slider, const char* paramID, juce::Label& label, const char* labelText)
    {
      
        slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 20);
        
    
        slider.setColour(juce::Slider::thumbColourId, juce::Colours::orange);
        slider.setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colours::darkgrey);
        slider.setColour(juce::Slider::rotarySliderFillColourId, juce::Colours::red.withAlpha(0.6f));
        addAndMakeVisible(slider);
        
        // Label Setup: Centered, bold text, and orange color
        label.setText(labelText, juce::dontSendNotification);
        label.setFont(juce::Font(12.0f, juce::Font::bold));
        label.setJustificationType(juce::Justification::centred);
        label.setColour(juce::Label::textColourId, juce::Colours::orange);
        addAndMakeVisible(label);
        
        // Initial visibility
        slider.setVisible(false);
        label.setVisible(false);
    };

  
    setupSliderAndLabel(rumbleSlider, "rumble", RumbleLabel, "Rumble");
    rumbleAttachment.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(audioProcessor.parameters, "rumble", rumbleSlider));
    rumbleSlider.onDragStart = [this]() { updateAssistantForParameter("rumble"); };
    
    setupSliderAndLabel(rumbleDecaySlider, "rumbleDecay", RumbleDecayLabel, "Rumble Decay");
    rumbleDecayAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters, "rumbleDecay", rumbleDecaySlider);
    rumbleDecaySlider.onDragStart = [this]() { updateAssistantForParameter("rumbleDecay"); };
    
    // 3. Air
    setupSliderAndLabel(AirSlider, "air", AirLabel, "Air");
    airAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters, "air", AirSlider);
    AirSlider.onDragStart = [this]() { updateAssistantForParameter("air"); };
    // 4. Air Decay
    setupSliderAndLabel(AirDecaySlider, "airDecay", AirDecayLabel, "Air Decay");
    airDecayAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters, "airDecay", AirDecaySlider);
    AirDecaySlider.onDragStart = [this]() { updateAssistantForParameter("airDecay"); };
    
    // 5. Dust
    setupSliderAndLabel(DustSlider, "dust", DustLabel, "Dust");
    dustAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters, "dust", DustSlider);
    DustSlider.onDragStart = [this]() { updateAssistantForParameter("dust"); };

    // 6. Dust Decay
    setupSliderAndLabel(DustDecaySlider, "dustDecay", DustDecayLabel, "Dust Decay");
    dustDecayAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters, "dustDecay", DustDecaySlider);
    DustDecaySlider.onDragStart = [this]() { updateAssistantForParameter("dustDecay"); };
    // 7. Grit Amount
    setupSliderAndLabel(GritAmountSlider, "gritAmount", GritAmountLabel, "Grit Amount");
    gritAmountAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters, "gritAmount", GritAmountSlider);
    GritAmountSlider.onDragStart = [this]() { updateAssistantForParameter("gritAmount"); };
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
           

    int proceduralPanelWidth = 0;
    
    // Check which mode is active to determine the panel width to reserve
    if (explosionMode) {
        proceduralPanelWidth = 250; // Explosion panel width from previous calculations
    } else if (FireMode) {
        proceduralPanelWidth = 250; // Fire panel width from previous fix
    } else if (GunMode) {
        proceduralPanelWidth = 250; // Gun panel width
    }
    else if(JetMode)
    {
        proceduralPanelWidth=250;
    }
    else if(HelicopterMode)
    {
        proceduralPanelWidth=250;
    }
    else if(RocketMode)
    {
        proceduralPanelWidth=250;
    }
    
    int rightPanelReserve = (proceduralPanelWidth > 0) ? proceduralPanelWidth + 40 : 0;
    
    
    searchBar.setBounds(20, y, getWidth() - 40 - rightPanelReserve, 24);
    y = searchBar.getBottom() + 10;

    
    int waveformHeight = 100;
    int waveformMargin = 20;
    int waveformY = getHeight() - waveformHeight - waveformMargin;

    wavFileList.setBounds(10, y, getWidth() - 20 - rightPanelReserve, waveformY - y);

    waveformBounds = juce::Rectangle<float>(10.0f, (float)waveformY,
                                            getWidth() - 20.0f,
                                            (float)waveformHeight);


    assistantImage.setBounds(20, getHeight() - 210, 80, 80);
    assistantLabel.setBounds(110, getHeight() - 210, getWidth() - 130 - rightPanelReserve, 80);

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

    // --- Control Dimensions ---
    const int panelWidth  = 250;
    const int knobSize    = 80;
    const int labelHeight = 15;
    const int controlMargin = 15; // Vertical spacing between controls
    
    // Total vertical space needed for one control unit (Label + Knob + Textbox + V-Margin)
    const int controlUnitHeight = labelHeight + knobSize + 20 + controlMargin;

    // Panel X/Top Coordinates
    const int margin      = 20;
    const int panelX      = getWidth() - panelWidth - margin;
    
    // FIX 1: Set 'top' to start immediately below the search bar
    const int top         = searchBar.getBottom() + 10;

    // Calculate total panel height (Button + Top Margin + 3 Controls + bottom margin)
    const int totalHeight = 40 /*Button*/ + controlMargin + (controlUnitHeight * 3) + controlMargin;

    // Set Panel Bounds
    RocketPanel.setBounds(panelX, top, panelWidth, totalHeight);

    // FIX 2: Start internal Y position offset
    int currentY = controlMargin; // Start 15px offset INSIDE the panel

    // 1. Place the Rocket Button (Absolute coordinates)
    RocketButton.setBounds(panelX + (panelWidth / 2) - 60, top + currentY, 120, 40);
    currentY += 40 + controlMargin; // Move internal Y down past the button

    juce::Component* sliders[] = { &DurationSlider, &ChamberResonanceSlider, &FlutterSlider };
    juce::Component* labels[] = { &DurationLabel, &ChamberResonanceLabel, &FlutterLabel };
    
    // X position for centered knob (absolute position)
    int sliderX = panelX + (panelWidth / 2) - (knobSize / 2);

    // 2. Lay out the 3 Sliders and Labels in a single column
    for (int i = 0; i < 3; ++i)
    {
        // Y position must be absolute: top + currentY
        int y_abs = top + currentY;

        // Label
        labels[i]->setBounds(sliderX, y_abs, knobSize, labelHeight);
        
        // Slider
        sliders[i]->setBounds(sliderX, y_abs + labelHeight, knobSize, knobSize + 20);
        
        currentY += controlUnitHeight; // Update internal Y offset
    }
}

//HelicopterUI
void QAPAudioProcessorEditor::layoutHelicopterUI()
{
    if (!HelicopterMode) return;

    // --- Control Dimensions ---
    const int numSliders = 8;
    const int numColumns = 2;
    
    const int panelWidth  = 250;
    const int knobSize    = 80;
    const int labelHeight = 15;
    const int controlMargin = 15; // Vertical spacing between controls
    
    // Calculate horizontal spacing to center 2 x 80px knobs in a 250px panel
    const int colSpacing = (panelWidth - (numColumns * knobSize)) / (numColumns + 1); // Approx 30px
    
    // Total vertical space needed for one control unit (Label + Knob + Textbox + V-Margin)
    const int controlUnitHeight = labelHeight + knobSize + 20 + controlMargin;

    // Panel X/Top Coordinates
    const int margin      = 20;
    const int panelX      = getWidth() - panelWidth - margin;
    
    // FIX 1: Start below the Search Bar
    const int top         = searchBar.getBottom() + 10;

    // Calculate total panel height (Button + Top Margin + 4 Rows of Controls)
    const int totalHeight = 40 /*Button*/ + controlMargin + (4 * controlUnitHeight) + controlMargin;

    // Set Panel Bounds (uses absolute coordinates)
    helicopterPanel.setBounds(panelX, top, panelWidth, totalHeight);

    // FIX 2: Start internal Y position relative to the panel's internal space (0)
    int currentY = controlMargin; // Start 15px offset INSIDE the panel

    // 1. Place the Helicopter Button (Absolute coordinates)
    HelicopterButton.setBounds(panelX + (panelWidth / 2) - 60, top + currentY, 120, 40);
    currentY += 40 + controlMargin; // Move internal Y down past the button

    juce::Component* sliders[] = {
        &RotorPeriodSlider, &PeriodSlider, &TailMixSlider, &BaseFreqSlider,
        &RotorMixSlider, &EngineMixSlider, &BladeNoiseSlider, &EngineSpeedSlider
    };

    juce::Component* labels[] = {
        &RotorPeriodLabel, &PeriodLabel, &TailMixLabel, &BaseFreqLabel,
        &RotorMixLabel, &EngineMixLabel, &BladeNoiseLabel, &EngineSpeedLabel
    };

    // 2. Lay out the 8 Sliders and Labels in a 2-column grid
    for (int i = 0; i < numSliders; ++i)
    {
        int col = i % numColumns;
        int row = i / numColumns;

        // X position (absolute)
        int x_abs = panelX + colSpacing + col * (knobSize + colSpacing);
        
        // Y position (absolute)
        int y_abs = top + currentY + row * controlUnitHeight;

        // Label
        labels[i]->setBounds(x_abs, y_abs, knobSize, labelHeight);
        
        // Slider
        sliders[i]->setBounds(x_abs, y_abs + labelHeight, knobSize, knobSize + 20);
    }
}
//JetUI
void QAPAudioProcessorEditor::layoutJetUI()
{
    if(!JetMode) return;

    // --- Rotary Control Dimensions ---
    const int panelWidth  = 250; // Keep the requested panel width
    const int knobSize    = 80;
    const int labelHeight = 15;
    const int controlMargin = 20;
    
    // Total vertical space needed for one control unit (Label + Knob + Textbox)
    const int controlUnitHeight = labelHeight + knobSize + 20; // ~115px
    
    // Panel Y/X coordinates
    const int margin      = 20;
    const int panelX      = getWidth() - panelWidth - margin;
    const int top         = 60;

    // Calculate total panel height (Button + spacing + 3 Controls + bottom margin)
    const int totalHeight = 40 /*Button*/ + controlMargin + (controlUnitHeight * 3) + controlMargin;

    // Set Panel Bounds
    JetPanel.setBounds(panelX, top, panelWidth, totalHeight);
    
    int y = top + 20; // Start position inside the panel (top margin)
    
    // 1. Place the Jet Button
    JetButton.setBounds(panelX + (panelWidth / 2) - 60, y, 120, 40);
    y += 40 + controlMargin; // Move down past the button and add spacing

    // Sliders and Labels arrays
    juce::Component* sliders[] = { &SpeedSlider, &TurbineSlider, &BurnSlider };
    juce::Component* labels[] = { &SpeedLabel, &TurbineLabel, &BurnLabel };
    
    // X position for centered knob in the 250px panel
    int sliderX = panelX + (panelWidth / 2) - (knobSize / 2);

    // 2. Lay out the 3 Sliders and Labels
    for (int i = 0; i < 3; ++i)
    {
        // Label (centered above the knob)
        labels[i]->setBounds(sliderX, y, knobSize, labelHeight);
        
        // Slider (below the label)
        sliders[i]->setBounds(sliderX, y + labelHeight, knobSize, knobSize + 20);
        
        y += controlUnitHeight + controlMargin; // Move down for the next control
    }
}
//GunUI
void QAPAudioProcessorEditor::layoutGunUI()
{
    if(!GunMode) return;

   
    const int panelWidth  = 250;
    const int knobSize    = 80;
    const int labelHeight = 15;
    const int controlMargin = 20; // Spacing around controls
    
    // Height of one rotary control unit (Label + Knob + Textbox)
    const int controlUnitHeight = labelHeight + knobSize + 20;
    
    // Panel Y/X coordinates
    const int margin      = 20;
    const int panelX      = getWidth() - panelWidth - margin;
    const int top         = 60;
    
   
    const int totalHeight = 40 + controlMargin + (controlUnitHeight * 2) + controlMargin;

    GunPanel.setBounds(panelX, top, panelWidth, totalHeight);
    
    int y = top + 20;
    
    // 1. Place the Gun Button
    GunButton.setBounds(panelX + (panelWidth / 2) - 60, y, 120, 40);
    y += 40 + controlMargin; // Move down past the button and add spacing

    // 2. Place the Shell Frequency Label and Slider (Centered in the 250px panel)
    int sliderX = panelX + (panelWidth / 2) - (knobSize / 2);

    
    ShellFrequecyLabel.setBounds(sliderX, y, knobSize, labelHeight);

    ShellFreqSlider.setBounds(sliderX, y + labelHeight, knobSize, knobSize + 20);
    y += controlUnitHeight + controlMargin; // Move down past the first control

    ShellFrequencyDecayLabel.setBounds(sliderX, y, knobSize, labelHeight);
    
    ShellFreqDecaySlider.setBounds(sliderX, y + labelHeight, knobSize, knobSize + 20);
   
}
//Fire UI
void QAPAudioProcessorEditor::layoutFireUI()
{
    if (!FireMode) return;

    // FIX 1: Change to 2 columns for a clean 2x2 grid layout (4 controls)
    const int numColumns  = 2;
    const int knobSize    = 80;
    const int labelHeight = 15;
    const int controlMargin = 15; // Adjusted margin for a snug fit.

    // Calculate panel size based on 2 columns
    const int panelWidth  = numColumns * knobSize + (numColumns + 1) * controlMargin; // 2 * 80 + 3 * 15 = 205
    const int controlUnitHeight = labelHeight + knobSize + 20; // Label + Knob + Textbox height
    const int panelHeight = 2 * controlUnitHeight + 70; // 2 rows of controls + space for button
    
    const int margin      = 20;
    const int panelX      = getWidth() - panelWidth - margin;
    const int top         = 60;

    firePanel.setBounds(panelX, top, panelWidth, panelHeight);

    int currentY = top + 20;

    // Place trigger button at top center
    FireButton.setBounds(panelX + (panelWidth / 2) - 60, currentY, 120, 40);
    currentY += 70;

    juce::Component* sliders[] = {
        &LappingSlider, &HissingSlider,
        &CracklingSlider, &IntensitySlider
    };

    juce::Component* labels[] = {
        // ASSUMES THESE ARE DECLARED IN PluginEditor.h
        &LappingLabel, &HissingLabel,
        &CracklingLabel, &IntensityLabel
    };

    // Grid layout (4 controls in a 2x2 grid)
    for (int i = 0; i < 4; ++i)
    {
        int col = i % numColumns;
        int row = i / numColumns;

        // X position: Panel X + Margin + Column Index * (Knob Size + Margin)
        int x = panelX + controlMargin + col * (knobSize + controlMargin);
        
        // Y position: Current Y (below button) + Row Index * (Control Unit Height)
        int y = currentY + row * controlUnitHeight;

        labels[i]->setBounds(x, y, knobSize, labelHeight);
        
        // The knob/slider uses the rest of the control unit height
        sliders[i]->setBounds(x, y + labelHeight, knobSize, knobSize + 20);
    }
}
//EXPLOSION UI
void QAPAudioProcessorEditor::layoutExplosionUI()
{
    if (!explosionMode) return;

    const int numSliders = 7;   // You have 7 sliders
    const int numColumns = 3;   // Adjust for layout (try 2 or 3)
    
    const int knobSize = 70;
    const int labelHeight = 20;
    const int controlUnitHeight = labelHeight + knobSize + 30; // Label + Knob + Textbox
    const int controlSpacing = 15;

    const int numRows = (numSliders + numColumns - 1) / numColumns;

    const int panelWidth  = numColumns * knobSize + (numColumns + 1) * controlSpacing;
    const int panelHeight = numRows * controlUnitHeight + 100;  // + space for button
    const int margin      = 20;
    const int panelX      = getWidth() - panelWidth - margin;
    const int top         = searchBar.getBottom() + 10;
    
    explosionPanel.setBounds(panelX, top, panelWidth, panelHeight);

    int currentY = top + 20;

    // Centered Trigger Button
    triggerButton.setBounds(panelX + (panelWidth / 2) - 50, currentY, 100, 35);
    currentY += 50;

    juce::Component* sliders[] = {
        &rumbleSlider, &rumbleDecaySlider, &AirSlider, &AirDecaySlider,
        &DustSlider, &DustDecaySlider, &GritAmountSlider
    };

    juce::Component* labels[] = {
        &RumbleLabel, &RumbleDecayLabel, &AirLabel, &AirDecayLabel,
        &DustLabel, &DustDecayLabel, &GritAmountLabel
    };

    for (int i = 0; i < numSliders; ++i)
    {
        int col = i % numColumns;
        int row = i / numColumns;

        int x = panelX + controlSpacing + col * (knobSize + controlSpacing);
        int y = currentY + row * controlUnitHeight;

        labels[i]->setBounds(x, y, knobSize, labelHeight);
        sliders[i]->setBounds(x, y + labelHeight, knobSize, knobSize + 20);
    }
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
    DurationLabel.setVisible(shouldShow);
    ChamberResonanceSlider.setVisible(shouldShow);
    ChamberResonanceLabel.setVisible(shouldShow);
    FlutterSlider.setVisible(shouldShow);
    FlutterLabel.setVisible(shouldShow);
    
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
    RumbleLabel.setVisible(false);
    rumbleDecaySlider.setVisible(false);
    RumbleDecayLabel.setVisible(false);
    AirSlider.setVisible(false);
    AirLabel.setVisible(false);
    AirDecaySlider.setVisible(false);
    AirDecayLabel.setVisible(false);
    DustSlider.setVisible(false);
    DustLabel.setVisible(false);
    DustDecaySlider.setVisible(false);
    DustDecayLabel.setVisible(false);
    GritAmountSlider.setVisible(false);
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
    RotorPeriodLabel.setVisible(shouldShow);
    PeriodSlider.setVisible(shouldShow);
    PeriodLabel.setVisible(shouldShow);
    TailMixSlider.setVisible(shouldShow);
    TailMixLabel.setVisible(shouldShow);
    BaseFreqSlider.setVisible(shouldShow);
    BaseFreqLabel.setVisible(shouldShow);
    RotorMixSlider.setVisible(shouldShow);
    RotorMixLabel.setVisible(shouldShow);
    EngineMixSlider.setVisible(shouldShow);
    EngineMixLabel.setVisible(shouldShow);
    BladeNoiseSlider.setVisible(shouldShow);
    BladeNoiseLabel.setVisible(shouldShow);
    EngineSpeedSlider.setVisible(shouldShow);
    EngineSpeedLabel.setVisible(shouldShow);
    
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
    SpeedLabel.setVisible(shouldShow);
    TurbineSlider.setVisible(shouldShow);
    TurbineLabel.setVisible(shouldShow);
    BurnSlider.setVisible(shouldShow);
    BurnLabel.setVisible(shouldShow);
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
    ShellFrequecyLabel.setVisible(shouldShow);
    ShellFreqDecaySlider.setVisible(shouldShow);
    ShellFrequencyDecayLabel.setVisible(shouldShow);
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
    explosionPanel.setVisible(shouldShow);
    triggerButton.setVisible(shouldShow);
    
    rumbleSlider.setVisible(shouldShow);
    RumbleLabel.setVisible(shouldShow);
    
    rumbleDecaySlider.setVisible(shouldShow);
    RumbleDecayLabel.setVisible(shouldShow);
    
    AirSlider.setVisible(shouldShow);
    AirLabel.setVisible(shouldShow);
    
    AirDecaySlider.setVisible(shouldShow);
    AirDecayLabel.setVisible(shouldShow);
    
    DustSlider.setVisible(shouldShow);
    DustLabel.setVisible(shouldShow);
    
    DustDecaySlider.setVisible(shouldShow);
    DustDecayLabel.setVisible(shouldShow);
    
    GritAmountSlider.setVisible(shouldShow);
    GritAmountLabel.setVisible(shouldShow);
    
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
    LappingLabel.setVisible(shouldShow);
    HissingSlider.setVisible(shouldShow);
    HissingLabel.setVisible(shouldShow);
    CracklingSlider.setVisible(shouldShow);
    CracklingLabel.setVisible(shouldShow);
    IntensitySlider.setVisible(shouldShow);
    IntensityLabel.setVisible(shouldShow);
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
    RumbleLabel.setVisible(false);
    rumbleDecaySlider.setVisible(false);
    RumbleDecayLabel.setVisible(false);
    AirSlider.setVisible(false);
    AirLabel.setVisible(false);
    AirDecaySlider.setVisible(false);
    AirDecayLabel.setVisible(false);
    DustSlider.setVisible(false);
    DustLabel.setVisible(false);
    DustDecaySlider.setVisible(false);
    DustDecayLabel.setVisible(false);
    GritAmountSlider.setVisible(false);
    GritAmountLabel.setVisible(false);
    
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
void QAPAudioProcessorEditor::updateAssistantForParameter(const juce::String& paramID)
{
    if (paramID == "rumble")
    {
        assistantLabel.setText(
            "This controls the deep low-end rumble of the explosion.",
            juce::dontSendNotification);
    }
    else if (paramID == "rumbleDecay")
    {
        assistantLabel.setText(
            "Rumble Decay.",
            juce::dontSendNotification);
    }
    else if (paramID == "air")
    {
        assistantLabel.setText(
            "AIr ....",
            juce::dontSendNotification);
    }
    else if (paramID == "airDecay")
    {
        assistantLabel.setText(
            "AirDecay.",
            juce::dontSendNotification);
    }
    else if (paramID == "dust")
    {
        assistantLabel.setText(
            "Dust",
            juce::dontSendNotification);
    }
    else if (paramID == "dustDecay")
    {
        assistantLabel.setText(
            "Dust Decay.",
            juce::dontSendNotification);
    }
    else if (paramID == "gritAmount")
    {
        assistantLabel.setText(
            "Grit Amount.",
            juce::dontSendNotification);
    }
    //Fire
    else if (paramID == "lapping")
    {
        assistantLabel.setText(
            "Lapping.",
            juce::dontSendNotification);
    }
    else if (paramID == "hissing")
    {
        assistantLabel.setText(
            "Hissing.",
            juce::dontSendNotification);
    }
    else if (paramID == "crackling")
    {
        assistantLabel.setText(
            "Crackling.",
            juce::dontSendNotification);
    }
    else if (paramID == "intensity")
    {
        assistantLabel.setText(
            "Intensity.",
            juce::dontSendNotification);
    }
    else if (paramID == "shellfreq")
    {
        assistantLabel.setText(
            "Shell Frequency.",
            juce::dontSendNotification);
    }
    else if (paramID == "shellfreqdecay")
    {
        assistantLabel.setText(
            "Shell Frequency Decay.",
            juce::dontSendNotification);
    }
    else if (paramID == "speed")
    {
        assistantLabel.setText(
            "Speed.",
            juce::dontSendNotification);
    }
    else if (paramID == "turbine")
    {
        assistantLabel.setText(
            "Turbine.",
            juce::dontSendNotification);
    }
    else if (paramID == "turbine")
    {
        assistantLabel.setText(
            "Turbine.",
            juce::dontSendNotification);
    }
    else if (paramID == "rotorPeriod")
    {
        assistantLabel.setText(
            "RotorPeriod.",
            juce::dontSendNotification);
    }
    else if (paramID == "Period")
    {
        assistantLabel.setText(
            "Period.",
            juce::dontSendNotification);
    }
    else if (paramID == "tailMix")
    {
        assistantLabel.setText(
            "Tail Mix.",
            juce::dontSendNotification);
    }
    else if (paramID == "baseFreq")
    {
        assistantLabel.setText(
            "Base Frequency.",
            juce::dontSendNotification);
    }
    else if (paramID == "rotorMix")
    {
        assistantLabel.setText(
            "Rotor Mix.",
            juce::dontSendNotification);
    }
    else if (paramID == "engineMix")
    {
        assistantLabel.setText(
            "Engine Mix.",
            juce::dontSendNotification);
    }
    else if (paramID == "bladeNoise")
    {
        assistantLabel.setText(
            "Blade Noise.",
            juce::dontSendNotification);
    }
    else if (paramID == "engineSpeed")
    {
        assistantLabel.setText(
            "Engine Speed.",
            juce::dontSendNotification);
    }
    else if (paramID == "duration")
    {
        assistantLabel.setText(
            "Duration.",
            juce::dontSendNotification);
    }
    else if (paramID == "flutter")
    {
        assistantLabel.setText(
            "Flutter.",
            juce::dontSendNotification);
    }
    else if (paramID == "chamberresonance")
    {
        assistantLabel.setText(
            "Chamber Resonance.",
            juce::dontSendNotification);
    }
    else
    {
        assistantLabel.setText("", juce::dontSendNotification);
    }

    assistantImage.setVisible(true);
    assistantLabel.setVisible(true);
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

