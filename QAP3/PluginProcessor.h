/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include <atomic>
#include "ExplosionImpl.h"
#include "FireImpl.h"
#include "GunImpl.h"
#include "JetImpl.h"
#include "HelicopterImpl.h"
#include "RocketImpl.h"
class QAPAudioProcessor  : public juce::AudioProcessor
                          
{
public:
    //==============================================================================
    QAPAudioProcessor();
    ~QAPAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;
    void loadAllWavFilesFromFolder(const juce::File& folder);
    void refreshWavFileList();          // Refresh list display (called from processor)
    void playWavFileByName(const juce::String& name);


    // Variables
    juce::StringArray wavFileNames;
    juce::Array<juce::File> wavFilePaths;
    
    std::unique_ptr<juce::AudioFormatReaderSource> readerSource;
    juce::AudioTransportSource transportSource;
    juce::AudioFormatManager formatManager;
    
    juce::StringArray getWavFileNames() const { return wavFileNames; }
    juce::File getWavFileByName(const juce::String& name) const;
    
    
    //ParameterValueTreeState
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    juce::AudioProcessorValueTreeState parameters;
    
    //Procedural Audio
    void triggerExplosion();
    std::unique_ptr<nemisindo::Explosion> explosionModel;
    void triggerFire();
    std::unique_ptr<nemisindo::Fire> fireModel;
    void triggerGun();
    std::unique_ptr<nemisindo::Gun> gunModel;
    void triggerJet();
    std::unique_ptr<nemisindo::Jet::impl> JetModel;
    
    void triggerHelicopter();
    std::unique_ptr<nemisindo::Helicopter::impl> HelicopterModel;
    
    void triggerRocket();
    std::unique_ptr<nemisindo::Rocket> RocketModel;
    


private:
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (QAPAudioProcessor)
    //void parameterChanged (const juce::String& parameterID, float newValue) override;
    
    
};
