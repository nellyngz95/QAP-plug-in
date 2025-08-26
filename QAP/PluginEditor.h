/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "ExplosionImpl.h"

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
    void updateAssistant(const juce::String& searchText);//check for the assistant

    
    void paint (juce::Graphics&) override;
    void resized() override;
    void chooseLibraryFolder();
    void selectedRowsChanged(int lastRowSelected) override; //Check the changes
    
    //Procedural UI
    void setupExplosionUI();       
    void layoutExplosionUI();
    void setExplosionMode (bool shouldShow);
   

    

    

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
    
    //IREDOKI Assistant
    juce::ImageComponent assistantImage;
    juce::Label assistantLabel;
    
    //Panels for the procedural Audio Models
    bool explosionMode=false;
    juce::GroupComponent explosionPanel { "explosionPanel", "Procedural Explosion" };

    juce::TextButton triggerButton;
    
    juce::Slider rumbleSlider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> rumbleAttachment;

    juce::Slider airSlider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> airAttachment;
    

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (QAPAudioProcessorEditor)
};
