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
    setLookAndFeel(&modernLookAndFeel);
    setResizable(true, true);
    addAndMakeVisible(wavFileList);
    wavFileList.setModel(this);

    addAndMakeVisible(loadLibraryButton);
    loadLibraryButton.setButtonText("Load Library");
    loadLibraryButton.onClick = [this] { chooseLibraryFolder(); };

    addAndMakeVisible(searchBar);
    searchBar.setTextToShowWhenEmpty("Search sounds...", juce::Colour(0xff5AC8AA).withAlpha(0.5f));
    searchBar.onTextChange = [this](){filterFileList(searchBar.getText());};

    filteredWavFileNames = audioProcessor.getWavFileNames();
    
    //Record Button
    addAndMakeVisible(recordButton);
    recordButton.setButtonText("REC");
    recordButton.setClickingTogglesState(true);
    recordButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff500000));
    recordButton.setColour(juce::TextButton::buttonOnColourId, juce::Colour(0xff005000));

    recordButton.onClick = [this]
        {
            if (audioProcessor.getIsRecording())
            {
                audioProcessor.stopRecording();
                recordButton.setToggleState(false, juce::dontSendNotification);
            }
            else
            {
                audioProcessor.startRecording();
                recordButton.setToggleState(true, juce::dontSendNotification);
            }
        };
    
    //Images:
    
    Logo = juce::ImageCache::getFromMemory (BinaryData::QuAPLogo_png, BinaryData::QuAPLogo_pngSize);
    assistantPixels = juce::ImageCache::getFromMemory(BinaryData::QuAPAssistant_png, BinaryData::QuAPAssistant_pngSize);
    assistantImage.setImage(assistantPixels, juce::RectanglePlacement::centred);


    addAndMakeVisible(assistantImage);
    addAndMakeVisible(assistantLabel);
    assistantLabel.setColour(juce::Label::textColourId, juce::Colours::black);
    assistantLabel.setFont(juce::Font(18.0f));
    assistantLabel.setJustificationType(juce::Justification::centredLeft);
    assistantLabel.setInterceptsMouseClicks(false, false); // Important: so you can still click the list behind it
    assistantLabel.setColour(juce::Label::backgroundColourId, juce::Colours::white);
    assistantLabel.setColour(juce::Label::outlineColourId, juce::Colour(0xff5AC8AA));
    

    assistantImage.setVisible(false);
    assistantLabel.setVisible(false);
    
    // Procedural Audio Models.
    setupExplosionUI();
    
    setupFireUI();
    
    setupGunUI();
    
    setupJetUI();
    
    setupHelicopterUI();
    
    setupRocketUI();
    
    recordButton.toFront(false);
    setResizeLimits(600, 400, 2000, 1300);
    setSize(700, 700); // Set the overall size of your plugin editor
}

QAPAudioProcessorEditor::~QAPAudioProcessorEditor()
{
    loadLibraryButton.setLookAndFeel(nullptr);
    wavFileList.setLookAndFeel(nullptr);
    searchBar.setLookAndFeel(nullptr);
    recordButton.setLookAndFeel(nullptr);
    triggerButton.setLookAndFeel(nullptr);
    //Explosion
    rumbleSlider.setLookAndFeel(nullptr);
    RumbleLabel.setLookAndFeel(nullptr);
    rumbleDecaySlider.setLookAndFeel(nullptr);
    RumbleDecayLabel.setLookAndFeel(nullptr);
    AirSlider.setLookAndFeel(nullptr);
    AirLabel.setLookAndFeel(nullptr);
    AirDecaySlider.setLookAndFeel(nullptr);
    AirDecayLabel.setLookAndFeel(nullptr);
    DustSlider.setLookAndFeel(nullptr);
    DustDecaySlider.setLookAndFeel(nullptr);
    DustDecaySlider.setLookAndFeel(nullptr);
    DustLabel.setLookAndFeel(nullptr);
    GritAmountSlider.setLookAndFeel(nullptr);
    GritAmountLabel.setLookAndFeel(nullptr);
    //fire
    FireButton.setLookAndFeel(nullptr);
    HissingSlider.setLookAndFeel(nullptr);
    HissingLabel.setLookAndFeel(nullptr);
    CracklingSlider.setLookAndFeel(nullptr);
    CracklingLabel.setLookAndFeel(nullptr);
    IntensitySlider.setLookAndFeel(nullptr);
    IntensityLabel.setLookAndFeel(nullptr);
    LappingSlider.setLookAndFeel(nullptr);
    LappingLabel.setLookAndFeel(nullptr);
   //Rocket
    RocketButton.setLookAndFeel(nullptr);
    DurationSlider.setLookAndFeel(nullptr);
    DurationLabel.setLookAndFeel(nullptr);
    ChamberResonanceSlider.setLookAndFeel(nullptr);
    ChamberResonanceLabel.setLookAndFeel(nullptr);
    FlutterSlider.setLookAndFeel(nullptr);
    FlutterLabel.setLookAndFeel(nullptr);
    //Helicopter
    HelicopterButton.setLookAndFeel(nullptr);
    RotorPeriodSlider.setLookAndFeel(nullptr);
    RotorPeriodLabel.setLookAndFeel(nullptr);
    PeriodSlider.setLookAndFeel(nullptr);
    PeriodLabel.setLookAndFeel(nullptr);
    TailMixSlider.setLookAndFeel(nullptr);
    TailMixLabel.setLookAndFeel(nullptr);
    BaseFreqSlider.setLookAndFeel(nullptr);
    BaseFreqLabel.setLookAndFeel(nullptr);
    RotorMixSlider.setLookAndFeel(nullptr);
    RotorMixLabel.setLookAndFeel(nullptr);
    EngineMixSlider.setLookAndFeel(nullptr);
    EngineMixLabel.setLookAndFeel(nullptr);
    BladeNoiseSlider.setLookAndFeel(nullptr);
    BladeNoiseLabel.setLookAndFeel(nullptr);
    EngineSpeedSlider.setLookAndFeel(nullptr);
    EngineSpeedLabel.setLookAndFeel(nullptr);
    
    //Jet
    JetPanel.setLookAndFeel(nullptr);
    JetButton.setLookAndFeel(nullptr);
    SpeedSlider.setLookAndFeel(nullptr);
    SpeedLabel.setLookAndFeel(nullptr);
    TurbineSlider.setLookAndFeel(nullptr);
    TurbineLabel.setLookAndFeel(nullptr);
    BurnSlider.setLookAndFeel(nullptr);
    BurnLabel.setLookAndFeel(nullptr);
    
   //Gun
    GunButton.setLookAndFeel(nullptr);
    ShellFreqSlider.setLookAndFeel(nullptr);
    ShellFrequecyLabel.setLookAndFeel(nullptr);
    ShellFreqDecaySlider.setLookAndFeel(nullptr);
    ShellFrequencyDecayLabel.setLookAndFeel(nullptr);
}


//DRAG AND DROP FUNCTIONS
bool QAPAudioProcessorEditor::isInterestedInFileDrag (const juce::StringArray& files)
{
    for (auto file : files)
        if (file.endsWith(".wav") || file.endsWith(".aif") || file.endsWith(".mp3"))
            return true;
            
    return false;
}
void QAPAudioProcessorEditor::fileDragEnter (const juce::StringArray& files, int x, int y)
{
    isDraggingFile = true;
    repaint();
}
void QAPAudioProcessorEditor::fileDragExit (const juce::StringArray& files)
{
    isDraggingFile = false;
    repaint();
}

void QAPAudioProcessorEditor::filesDropped (const juce::StringArray& files, int x, int y)
{
    isDraggingFile = false;
    juce::File droppedFile (files[0]);

    if (droppedFile.existsAsFile())
        {
            audioProcessor.PlayAssistantSound.store(true);
            thumbnail.setSource (new juce::FileInputSource (droppedFile));
            audioProcessor.LoadDroppedFile(droppedFile);
            SimilaritySearch(droppedFile);
        } // Ensure this brace exists!
    repaint();
}
void QAPAudioProcessorEditor::SimilaritySearch(const juce::File& droppedFile)
{
  
        // 1. Tell the brain to search
        audioProcessor.performSimilaritySearch(droppedFile);
        
        // 2. Get the results
        auto results = audioProcessor.getSimilarFiles();
        DBG("AI Search results found: " << results.size());

        // 3. Clear and update the UI list
        filteredWavFileNames.clear();
        for (auto& f : results)
        {
            filteredWavFileNames.add(f.getFileName());
        }

        // 4. Refresh the display
        wavFileList.updateContent();
        wavFileList.repaint();
    
}

//==============================================================================
void QAPAudioProcessorEditor::paint (juce::Graphics& g)
{
    auto area = getLocalBounds();
    
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
    
    int headerHeight = 100;
    auto headerArea = area.removeFromTop(headerHeight);
    
    // Header Background (Darker + Bottom Border)
    g.setColour(juce::Colour(0xff1a1e22)); // Very dark header
    g.fillRect(0, 108, getWidth(), 2);;
    
    // Header Bottom Border (Accent Line)
    g.setColour(juce::Colour(0xff5AC8AA).withAlpha(0.3f)); // Glow
    g.fillRect(0, headerHeight - 3, getWidth(), 5);
    g.setColour(juce::Colour(0xff5AC8AA)); // Solid Teal Line
    g.fillRect(0, headerHeight - 2, getWidth(), 2);
    
    // Header Title / Logo
    juce::Font titleFont (juce::Font::getDefaultSansSerifFontName(), 28.0f, juce::Font::bold);
    g.setFont(titleFont);
    g.setColour(juce::Colours::white.withAlpha(0.9f));
    g.drawText("Quick Audio Prototyping", 0, 0, getWidth(), 50, juce::Justification::centred);

    if (Logo.isValid())
    {
        g.drawImageWithin (Logo,
                           20, 10,        // X and Y position
                           120, 100,       // Maximum Width and Height
                           juce::RectanglePlacement::centred);
    }
    //Search or Drag
    juce::Rectangle<int> instructionArea (20, 100, 300, 200);
    auto searchBounds = searchBar.getBounds().toFloat();
    g.setColour(juce::Colour(0xff5AC8AA));
    g.fillRoundedRectangle(searchBounds, 4.0f);
    g.setColour(juce::Colours::black.withAlpha(0.4f));
    g.drawHorizontalLine(searchBounds.getY(), searchBounds.getX(), searchBounds.getRight());
    g.drawVerticalLine(searchBounds.getX(), searchBounds.getY(), searchBounds.getBottom());
    
    
    g.setColour(juce::Colours::white.withAlpha(0.1f));
    g.drawHorizontalLine(searchBounds.getBottom(), searchBounds.getX(), searchBounds.getRight());
    g.drawVerticalLine(searchBounds.getRight(), searchBounds.getY(), searchBounds.getBottom());
    auto listBounds = wavFileList.getBounds().toFloat();
    
    juce::ColourGradient panelGrad(juce::Colour(40, 42, 45), listBounds.getTopLeft(),
                                   juce::Colour(30, 32, 35), listBounds.getBottomLeft(), false);
    g.setGradientFill(panelGrad);
    g.fillRoundedRectangle(listBounds, 6.0f);
    
    
    g.setColour(juce::Colours::black.withAlpha(0.4f));
    g.drawRoundedRectangle(listBounds.translated(0, 2), 6.0f, 2.0f);
    
    g.setColour(juce::Colours::white.withAlpha(0.15f));
    g.drawRoundedRectangle(listBounds, 6.0f, 1.0f);
    
    
    if (thumbnail.getTotalLength() > 0.0 && !waveformBounds.isEmpty())
    {
        g.setColour(juce::Colour(0xff1B2127)); // Dark background for waveform container
        g.fillRoundedRectangle(waveformBounds, 3.0f);
        
        auto thumbArea = waveformBounds.reduced(2.0f); // Margin inside the box
        
        g.setColour(juce::Colour(0xff5AC8AA)); // The Waveform Color
        
        juce::Path wavePath;
        wavePath.preallocateSpace(thumbArea.getWidth() * 2);
        
        auto numSamples = thumbnail.getNumChannels() * thumbnail.getTotalLength() * audioProcessor.getSampleRate();
        auto samplesPerPixel = numSamples / thumbArea.getWidth();
        
        // Start the path
        wavePath.startNewSubPath(thumbArea.getX(), thumbArea.getCentreY());
        
        // Loop through every pixel width of the area
        for (int x = 0; x < thumbArea.getWidth(); ++x)
        {
            auto drawX = thumbArea.getX() + x;
            
            // Map pixel X to time
            auto time = (x / (double)thumbArea.getWidth()) * thumbnail.getTotalLength();
            
            // Get Min/Max for this time slice
            float min = 0.0f, max = 0.0f;
            thumbnail.getApproximateMinMax(time, time + (thumbnail.getTotalLength() / thumbArea.getWidth()), 0, min, max);
            
            auto magnitude = juce::jmax(std::abs(min), std::abs(max)) * 1.5f;
            magnitude = juce::jmin(magnitude, 1.0f); // Clip at 1.0
            
            
            auto height = magnitude * (thumbArea.getHeight() * 0.5f);
            
            wavePath.lineTo(drawX, thumbArea.getCentreY() - height);
        }
        
        
        for (int x = thumbArea.getWidth() - 1; x >= 0; --x)
        {
            auto drawX = thumbArea.getX() + x;
            auto time = (x / (double)thumbArea.getWidth()) * thumbnail.getTotalLength();
            float min = 0.0f, max = 0.0f;
            thumbnail.getApproximateMinMax(time, time + (thumbnail.getTotalLength() / thumbArea.getWidth()), 0, min, max);
            auto magnitude = juce::jmax(std::abs(min), std::abs(max)) * 1.5f;
            magnitude = juce::jmin(magnitude, 1.0f);
            
            auto height = magnitude * (thumbArea.getHeight() * 0.5f);
            wavePath.lineTo(drawX, thumbArea.getCentreY() + height);
        }
        
        wavePath.closeSubPath();
        
        juce::ColourGradient gradient(juce::Colour(0xff5AC8AA), thumbArea.getCentreX(), thumbArea.getY(),
                                      juce::Colour(0xff5AC8AA), thumbArea.getCentreX(), thumbArea.getBottom(), false);
        g.setGradientFill(gradient);
        g.fillPath(wavePath);
    }
    else
    {
        // "Empty State" text
        g.setColour(juce::Colours::grey.withAlpha(0.5f));
        g.setFont(15.0f);
        if (!waveformBounds.isEmpty())
            g.drawText("No waveform loaded", waveformBounds.toNearestInt(), juce::Justification::centred);
    }
    
    if (searchBar.getText().isEmpty() && thumbnail.getTotalLength() <= 0.0)
    {
        if (!waveformBounds.isEmpty())
        {
            
            g.setColour(juce::Colour(0xff5AC8AA).withAlpha(0.3f));
            float dashLengths[] = { 5.0f, 5.0f };
            
            juce::Path dashedBorder;
            dashedBorder.addRoundedRectangle(waveformBounds.reduced(2.0f), 3.0f);
            juce::PathStrokeType stroke (1.0f, juce::PathStrokeType::curved, juce::PathStrokeType::butt);

            stroke.createDashedStroke (dashedBorder, dashedBorder, dashLengths, 2);

            g.strokePath (dashedBorder, stroke);
            
            g.setColour(juce::Colours::white.withAlpha(0.6f));
            g.setFont(juce::Font(16.0f, juce::Font::italic));
            g.drawFittedText("Type in search or Drag a sound here\nto find something similar",
                             waveformBounds.toNearestInt(),
                             juce::Justification::centred,
                             2);
        }
        
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
        label.setColour(juce::Label::textColourId, juce::Colours::white);
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
        label.setColour(juce::Label::textColourId, juce::Colours::white);
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
        label.setColour(juce::Label::textColourId, juce::Colours::white);
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
        label.setFont(juce::Font(15.0f, juce::Font::bold));
        label.setJustificationType(juce::Justification::centred);
        label.setColour(juce::Label::textColourId, juce::Colours::white);
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

    addAndMakeVisible(FireButton);
    FireButton.setButtonText("Start Fire");
    FireButton.onClick = [this]() { audioProcessor.triggerFire(); };
    
    auto setupSliderAndLabel = [&](juce::Slider& slider, const char* paramID, juce::Label& label, const char* labelText)
    {
        // 1. Style
        slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20); // Slightly wider text box
        
        addAndMakeVisible(slider);
        label.setText(labelText, juce::dontSendNotification);
        label.setFont(juce::Font(14.0f)); // Non-bold looks cleaner with this style
        label.setJustificationType(juce::Justification::centred);
        label.setColour(juce::Label::textColourId, juce::Colours::white.withAlpha(0.8f));
        addAndMakeVisible(label);
        
        slider.setVisible(false);
        label.setVisible(false);
    };

    // ... The rest of your attachments remain the same ...
    setupSliderAndLabel(LappingSlider, "lapping", LappingLabel, "Lapping");
    lappingAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters, "lapping", LappingSlider);
    LappingSlider.onDragStart = [this]() { updateAssistantForParameter("lapping"); };
    
    // ... Repeat for Hissing, Crackling, Intensity ...
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
        

        addAndMakeVisible(slider);
        
        // Label Setup: Centered, bold text, and orange color
        label.setText(labelText, juce::dontSendNotification);
        label.setFont(juce::Font(12.0f, juce::Font::bold));
        label.setJustificationType(juce::Justification::centred);
        label.setColour(juce::Label::textColourId, juce::Colours::white);
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
        g.fillAll(juce::Colour(0xff5AC8AA).withAlpha(0.5f));

    if (rowNumber >= 0 && rowNumber < filteredWavFileNames.size())
    {
        g.setColour(juce::Colours::grey);
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
    auto area = getLocalBounds();
    
    int headerHeight = 110;
    auto headerArea = area.removeFromTop(headerHeight);

    
    auto titleArea = headerArea.removeFromTop(50);
    
    auto buttonRow = headerArea.reduced(20, 5); // Add side margins
    int buttonWidth = 120;
    int buttonHeight = 35;
    int gap = 15;
    
    int totalButtonGroupWidth = (buttonWidth * 2) + gap;
    int startX = buttonRow.getCentreX() - (totalButtonGroupWidth / 2);
    
    loadLibraryButton.setBounds(startX, buttonRow.getY(), buttonWidth, buttonHeight);
    recordButton.setBounds(startX + buttonWidth + gap, buttonRow.getY(), buttonWidth, buttonHeight);

    // --- BODY OF THE PLUGIN ---
    area.reduce(20, 10); // Margins for the rest of the UI
    
    int proceduralWidth = 250;
    bool proceduralOpen = explosionMode || FireMode || GunMode || JetMode || HelicopterMode || RocketMode;
    int rightPanelReserve = proceduralOpen ? proceduralWidth + 20 : 0;
    auto assistantRow = area.removeFromBottom(40);
    // Waveform at the bottom
    const int waveformHeight = 80;
    auto waveformArea = area.removeFromBottom(waveformHeight);
    waveformBounds = waveformArea.toFloat().withWidth(waveformArea.getWidth() - rightPanelReserve);

    area.removeFromBottom(20);

    // SEARCH BAR (The dividing line should be drawn above this in paint)
    auto topStrip = area.removeFromTop(35);
    searchBar.setBounds(topStrip.withWidth(topStrip.getWidth() - rightPanelReserve));
    
    area.removeFromTop(10);
    wavFileList.setBounds(area.withWidth(area.getWidth() - rightPanelReserve));
    
    if (assistantImage.isVisible())
        {
            // Force these to the top of the "Z-order" so they aren't hidden by the list
            assistantImage.toFront(false);
            assistantLabel.toFront(false);

            int secondQuarterX = getWidth() * 0.25f;
            
            // Let's lift him up slightly so he's clearly visible over the list
            int assistantY = wavFileList.getBottom() - 100;
            int charSize = 80;

            assistantImage.setBounds(secondQuarterX, assistantY, charSize, charSize);

            // Position bubble to the right
            assistantLabel.setBounds(secondQuarterX + charSize + 5,
                                     assistantY + 20,
                                     200, 40);
        }

      
    //Sub lay-outs.
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

    // --- Dimensions ---
    const int panelWidth    = 250;
    const int knobSize      = 80;
    const int labelHeight   = 20; // Increased to prevent name clipping
    const int controlMargin = 20;
    const int buttonHeight  = 40;
    
    // Space between the rectangle's top edge and the first button
    const int internalPadding = 15;
    
    // Total vertical space needed for one control unit (Label + Knob + Textbox)
    const int controlUnitHeight = labelHeight + knobSize + 35;
    
    // 1. POSITIONING THE RECTANGLE
    const int margin  = 20;
    const int panelX  = getWidth() - panelWidth - margin;
    
    // Align the rectangle top with the search bar area
    const int panelTop = searchBar.getBottom() + 5;

    // 2. CALCULATE TOTAL HEIGHT
    // (Padding + Button + Margin + 3 Controls + Bottom Padding)
    const int totalHeight = internalPadding + buttonHeight + controlMargin +
                            (controlUnitHeight * 3) + (controlMargin * 2);

    // Set the Background Rectangle Bounds
    JetPanel.setBounds(panelX, panelTop, panelWidth, totalHeight);
    
    // 3. POSITIONING COMPONENTS
    // currentY starts relative to the panelTop
    int currentY = panelTop + internalPadding;
    int centerX = panelX + (panelWidth / 2);

    // Place the Jet Button
    JetButton.setBounds(centerX - 60, currentY, 120, buttonHeight);
    currentY += buttonHeight + controlMargin;

    // Components arrays
    juce::Component* sliders[] = { &SpeedSlider, &TurbineSlider, &BurnSlider };
    juce::Component* labels[]  = { &SpeedLabel,  &TurbineLabel,  &BurnLabel  };
    
    // Lay out the 3 Sliders and Labels
    for (int i = 0; i < 3; ++i)
    {
        // Use full panelWidth for labels to prevent "..." clipping
        labels[i]->setBounds(panelX, currentY, panelWidth, labelHeight);
        
        // Center the slider
        sliders[i]->setBounds(centerX - (knobSize / 2), currentY + labelHeight, knobSize, knobSize + 25);
        
        currentY += controlUnitHeight + controlMargin;
    }
}
//GunUI
void QAPAudioProcessorEditor::layoutGunUI()
{
    if(!GunMode) return;

    const int panelWidth    = 250;
    const int knobSize      = 80;
    const int labelHeight   = 20;
    const int controlMargin = 20;
    const int buttonHeight  = 40;
    
    const int internalPadding = 15;

    const int controlUnitHeight = labelHeight + knobSize + 25;
    

    const int margin = 20;
    const int panelX      = getWidth() - panelWidth - margin;
        
        // Aligned with the bottom of the search bar area
        const int panelTop    = searchBar.getBottom() + 5;

        // Height for 2 rows of controls
        const int totalHeight = internalPadding + buttonHeight + controlMargin +
                                (2 * controlUnitHeight) + (controlMargin * 2);

        GunPanel.setBounds(panelX, panelTop, panelWidth, totalHeight);

        // --- 3. POSITIONING COMPONENTS ---
        int currentY = panelTop + internalPadding;
        int centerX  = panelX + (panelWidth / 2);

    // 2. Gun Button
    GunButton.setBounds(centerX - 60, currentY, 120, buttonHeight);
    currentY += buttonHeight + controlMargin;

    // 3. Shell Frequency (Label width increased to panelWidth to prevent clipping)
    ShellFrequecyLabel.setBounds(panelX, currentY, panelWidth, labelHeight);
    currentY += labelHeight;
    ShellFreqSlider.setBounds(centerX - (knobSize / 2), currentY, knobSize, knobSize + 20);
    currentY += (knobSize + 25) + controlMargin;

    // 4. Shell Frequency Decay
    ShellFrequencyDecayLabel.setBounds(panelX, currentY, panelWidth, labelHeight);
    currentY += labelHeight;
    ShellFreqDecaySlider.setBounds(centerX - (knobSize / 2), currentY, knobSize, knobSize + 20);
}

//Fire UI
void QAPAudioProcessorEditor::layoutFireUI()
{
    if (!FireMode) return;

    // DIMENSIONS
    const int panelWidth    = 250;
    const int knobSize      = 80;
    const int labelHeight   = 20;
    const int controlMargin = 15;
    const int buttonHeight  = 40;
    const int internalPadding = 15;

    const int controlUnitHeight = labelHeight + knobSize + 25;

    // --- 2. POSITIONING THE RECTANGLE ---
    const int margin      = 20;
    const int panelX      = getWidth() - panelWidth - margin;
    
    // Aligned with the bottom of the search bar area
    const int panelTop    = searchBar.getBottom() + 5;

    // Height for 2 rows of controls
    const int totalHeight = internalPadding + buttonHeight + controlMargin +
                            (2 * controlUnitHeight) + (controlMargin * 2);

    firePanel.setBounds(panelX, panelTop, panelWidth, totalHeight);

    // --- 3. POSITIONING COMPONENTS ---
    int currentY = panelTop + internalPadding;
    int centerX  = panelX + (panelWidth / 2);

    // Center the Start Fire Button
    FireButton.setBounds(centerX - 60, currentY, 120, buttonHeight);
    currentY += buttonHeight + controlMargin;

    juce::Component* sliders[] = { &LappingSlider, &HissingSlider, &CracklingSlider, &IntensitySlider };
    juce::Component* labels[]  = { &LappingLabel,  &HissingLabel,  &CracklingLabel,  &IntensityLabel  };

    // Calculate horizontal spacing for 2 columns inside the 250px width
    // Total knobs width = 160. Remaining space = 90.
    // We divide remaining space into 3 gaps (left, middle, right).
    int sideGap = 30;
    int columnGap = panelWidth - (2 * knobSize) - (2 * sideGap);

    for (int i = 0; i < 4; ++i)
    {
        int col = i % 2;
        int row = i / 2;
        
        int x = panelX + sideGap + (col * (knobSize + columnGap));
        int y = currentY + (row * (controlUnitHeight + controlMargin));

        labels[i]->setBounds(x, y, knobSize, labelHeight);
        sliders[i]->setBounds(x, y + labelHeight, knobSize, knobSize + 25);
    }
}
//EXPLOSION UI
void QAPAudioProcessorEditor::layoutExplosionUI()
{
    if (!explosionMode) return;

    const int numSliders = 7;
    const int numColumns = 3;   
    
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
    // Store the old text so we can check if it actually changed
    juce::String oldText = assistantLabel.getText();
    juce::String newText = "";

    if (paramID == "rumble")             newText = "This controls the deep low-end rumble of the explosion.";
    else if (paramID == "rumbleDecay")    newText = "How fast or slow the rumble will dissapear.";
    else if (paramID == "air")           newText = "Controls the high frequency shockwave.>0.6";
    else if (paramID == "airDecay")      newText = "How fast the air blast fades.";
    else if (paramID == "dust")          newText = "Mid-frequency layer, for debris after the explosion.>0.6";
    else if (paramID == "dustDecay")     newText = "Sets how long the dust and debris lasts.";
    else if (paramID == "gritAmount")    newText = "Adds roughness.";
    else if (paramID == "lapping")       newText = "Controls the motion of the flames. Try <0.70.";
    else if (paramID == "hissing")       newText = "Is the steam escaping from the burning material. Try <0.50.";
    else if (paramID == "crackling")     newText = "Controls the sharp pops from the material.Try <0.50.";
    else if (paramID == "intensity")     newText = "Overall energy of the fire.";
    else if (paramID == "shellfreq")     newText = "Higher values create denser impacts.";
    else if (paramID == "shellfreqdecay")newText = "How quickly the shell impacts fade away.";
    else if (paramID == "speed")         newText = "Controls speed and movement, intensity of engine.";
    else if (paramID == "turbine")       newText = "High-pitched mechanical sound from the jet engines.";
    else if (paramID == "burn")          newText = "Controls the combustion roar. Adds heat and intensity.";
    else if (paramID == "rotorPeriod")   newText = "Controls the timing between each blade pass.";
    else if (paramID == "period")        newText = "Acts like a timing reference for the rotor. Start with 149.";
    else if (paramID == "tailMix")       newText = "Higher values emphasize the high-pitched buzzing.";
    else if (paramID == "baseFreq")      newText = "Controls the sound of the helicopters main sound. Lower=heavy.";
    else if (paramID == "rotorMix")      newText = "Makes the signature 'whup-whup' sound more pronounced or not.";
    else if (paramID == "engineMix")     newText = "Balances the engine noise relative to the rotor sound.";
    else if (paramID == "bladeNoise")    newText = "Controls the air rush and 'slap' from rotor tips.";
    else if (paramID == "engineSpeed")   newText = "Higher values increase overall intensity and pitch.";
    else if (paramID == "duration")      newText = "How long the Rocket will sound.";
    else if (paramID == "flutter")       newText = "Adjusts the flutter or instability in the rocket exhaust.";
    else if (paramID == "chamberresonance") newText = "It gives a reverb sensation.";

    // Apply the text
    assistantLabel.setText(newText, juce::dontSendNotification);

    // ONLY Play sound if the text changed and isn't empty
    if (newText != oldText && newText.isNotEmpty())
    {
        audioProcessor.PlayAssistantSound = true;
    }

    assistantImage.setVisible(newText.isNotEmpty());
    assistantLabel.setVisible(newText.isNotEmpty());
}
//Virtual Friend.
void QAPAudioProcessorEditor::updateAssistant(const juce::String& searchText)
{
    // Check if it was already visible before we do anything
    bool wasVisible = assistantImage.isVisible();
    bool shouldBeVisible = false;

    if (searchText.containsIgnoreCase("explosion"))
    {
        assistantLabel.setText("Hi. An explosion", juce::dontSendNotification);
        setExplosionMode(true); setFireMode(false); setRocketMode(false); setGunMode(false);setJetMode(false);setHelicopterMode(false);
        shouldBeVisible = true;
    }
    else if (searchText.containsIgnoreCase("fire"))
    {
        assistantLabel.setText("Hi. Create your own fire.", juce::dontSendNotification);
        setExplosionMode(false); setFireMode(true); setRocketMode(false); setGunMode(false);setJetMode(false);setHelicopterMode(false);
        shouldBeVisible = true;
    }
    else if (searchText.containsIgnoreCase("gun"))
    {
        assistantLabel.setText("Hi. Gunshots", juce::dontSendNotification);
        setExplosionMode(false); setFireMode(false); setRocketMode(false); setGunMode(true);setJetMode(false);setHelicopterMode(false);
        shouldBeVisible = true;
    }
    else if (searchText.containsIgnoreCase("jet"))
    {
        assistantLabel.setText("Hi. Jet sounds", juce::dontSendNotification);
        setExplosionMode(false); setFireMode(false); setGunMode(false); setRocketMode(false); setJetMode(true);setHelicopterMode(false);
        shouldBeVisible = true;
    }
    else if (searchText.containsIgnoreCase("helicopter"))
    {
        assistantLabel.setText("Hi. Helicopters", juce::dontSendNotification);
        setExplosionMode(false); setFireMode(false); setGunMode(false); setJetMode(false); setRocketMode(false); setHelicopterMode(true);
        shouldBeVisible = true;
    }
    else if (searchText.containsIgnoreCase("rocket"))
    {
        assistantLabel.setText("Hi. Rockets", juce::dontSendNotification);
        setExplosionMode(false); setFireMode(false); setGunMode(false); setJetMode(false); setHelicopterMode(false); setRocketMode(true);
        shouldBeVisible = true;
    }
    else
    {
        assistantImage.setVisible(false);
        assistantLabel.setVisible(false);
        setExplosionMode(false); setFireMode(false); setJetMode(false); setHelicopterMode(false); setGunMode(false);setRocketMode(false);
    }


    if (shouldBeVisible && !wasVisible)
    {
        audioProcessor.PlayAssistantSound = true;
    }

    if (shouldBeVisible)
    {
        assistantImage.setVisible(true);
        assistantLabel.setVisible(true);
    }

    resized();
}
