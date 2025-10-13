/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "ExplosionImpl.h"
#include "FireImpl.h"
#include "GunImpl.h"
#include "JetImpl.h"
#include "HelicopterImpl.h"
#include "RocketImpl.h"
//==============================================================================
/**
*/
class QAPAudioProcessorEditor  : public juce::AudioProcessorEditor,
public juce::ListBoxModel
{
public:
    QAPAudioProcessorEditor (QAPAudioProcessor&);
    ~QAPAudioProcessorEditor() override;

    //==============================================================================
    int getNumRows() override;
    void paintListBoxItem(int rowNumber, juce::Graphics& g, int width, int height, bool rowIsSelected) override;
    void refreshWavFileList();
    void filterFileList(const juce::String& searchText);
    void updateAssistant(const juce::String& searchText);
    void updateAssistantForParameter(const juce::String& paramID);


    
    void paint (juce::Graphics&) override;
    void resized() override;
    void chooseLibraryFolder();
    void selectedRowsChanged(int lastRowSelected) override; //Check the changes
    
    //Procedural UI
    void setupExplosionUI();
    void layoutExplosionUI();
    void setExplosionMode (bool shouldShow);
    void setupFireUI();
    void layoutFireUI();
    void setFireMode (bool shouldShow);
    void setupGunUI();
    void layoutGunUI();
    void setGunMode (bool shouldShow);
    
    void setupJetUI();
    void layoutJetUI();
    void setJetMode (bool shouldShow);
   
    void setupHelicopterUI();
    void layoutHelicopterUI();
    void setHelicopterMode (bool shouldShow);
    
    void setupRocketUI();
    void layoutRocketUI();
    void setRocketMode (bool shouldShow);


    

private:
    juce::TextButton loadLibraryButton {"Load Library"};
    juce::ListBox wavFileList; //Total wav files
    juce::TextEditor searchBar;
    juce::StringArray filteredWavFileNames; //Filtered wav files
    
    std::unique_ptr<juce::FileChooser> folderChooser;
    QAPAudioProcessor& audioProcessor;
    juce::AudioThumbnailCache thumbnailCache {10}; // Cache up to 5 thumbnails
    juce::AudioThumbnail thumbnail;
    juce::Rectangle<float> waveformBounds;
    
    juce::TextButton recordButton{"Record Sound"};
    
    //IREDOKI Assistant
    juce::ImageComponent assistantImage;
    juce::Label assistantLabel;
    
    //Panels for the procedural Audio Models
    bool explosionMode=false;
    juce::GroupComponent explosionPanel { "explosionPanel", "Procedural Explosion" };
    juce::TextButton triggerButton;
    juce::Label RumbleLabel;
    juce::Label RumbleDecayLabel;
    juce::Label DustLabel;
    juce::Label DustDecayLabel;
    juce::Label AirLabel;
    juce::Label AirDecayLabel;
    juce::Label GritAmountLabel;
    
    bool FireMode=false;
    juce::GroupComponent firePanel { "firePanel", "Procedural Fire" };
    juce::TextButton FireButton;
    //Explosion
    juce::Slider rumbleSlider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> rumbleAttachment;
    juce::Slider rumbleDecaySlider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> rumbleDecayAttachment;
    juce::Slider AirSlider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> airAttachment;
    juce::Slider AirDecaySlider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> airDecayAttachment;
    juce::Slider DustSlider;
    juce::Slider DustDecaySlider;
    juce::Slider GritAmountSlider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> dustAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> dustDecayAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> gritAmountAttachment;
    
    //Fire
    juce::Slider LappingSlider;
    juce::Slider HissingSlider;
    juce::Slider CracklingSlider;
    juce::Slider IntensitySlider;
    juce::Label LappingLabel;
    juce::Label HissingLabel;
    juce::Label IntensityLabel;
    juce::Label CracklingLabel;
    
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> lappingAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> hissingAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> cracklingAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> intensityAttachment;
    
    //Gun
    bool GunMode=false;
    juce::GroupComponent GunPanel { "GunPanel", "Procedural Gun" };
    juce::TextButton GunButton;
    
    juce::Slider ShellFreqSlider;
    juce::Slider ShellFreqDecaySlider;
    juce::Label ShellFrequecyLabel;
    juce::Label ShellFrequencyDecayLabel;
    
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> ShellFreqAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> ShellFreqDecayAttachment;
    
    //Jet
    bool JetMode=false;
    juce::GroupComponent JetPanel { "JetPanel", "Procedural Jet" };
    juce::TextButton JetButton;
    
    juce::Slider SpeedSlider;
    juce::Slider TurbineSlider;
    juce::Slider BurnSlider;
    
    juce::Label SpeedLabel;
    juce::Label TurbineLabel;
    juce::Label BurnLabel;
    
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> SpeedAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> TurbineAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> BurnAttachment;
    
    //Helicopter
    bool HelicopterMode=false;
    juce::GroupComponent helicopterPanel { "HelicopterPanel", "Procedural Helicopter" };
    juce::TextButton HelicopterButton;
    
    juce::Slider RotorPeriodSlider;
    juce::Slider PeriodSlider;
    juce::Slider TailMixSlider;
    juce::Slider BaseFreqSlider;
    juce::Slider RotorMixSlider;
    juce::Slider EngineMixSlider;
    juce::Slider BladeNoiseSlider;
    juce::Slider EngineSpeedSlider;
    
    juce::Label RotorPeriodLabel;
     juce::Label PeriodLabel;
     juce::Label TailMixLabel;
     juce::Label BaseFreqLabel;
     juce::Label RotorMixLabel;
     juce::Label EngineMixLabel;
     juce::Label BladeNoiseLabel;
     juce::Label EngineSpeedLabel;
    
    
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> RotorPeriodAttachment;
     std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> PeriodAttachment;
     std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> TailMixAttachment;
     std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> BaseFreqAttachment;
     std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> RotorMixAttachment;
     std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> EngineMixAttachment;
     std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> BladeNoiseAttachment;
     std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> EngineSpeedAttachment;
    
    //Rocket Mode
    bool RocketMode=false;
    juce::GroupComponent RocketPanel { "RocketPanel", "Procedural Rocket" };
    juce::TextButton RocketButton;
    
    juce::Slider DurationSlider;
    juce::Slider ChamberResonanceSlider;
    juce::Slider FlutterSlider;
 
    juce::Label DurationLabel;
    juce::Label ChamberResonanceLabel;
    juce::Label FlutterLabel;
    
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> durationAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> chamberResonanceDecayAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> flutterAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (QAPAudioProcessorEditor)
};
