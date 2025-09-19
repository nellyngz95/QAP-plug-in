/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "ExplosionImpl.h"

QAPAudioProcessor::QAPAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
    : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                      ), parameters(*this, nullptr, "parameters", createParameterLayout()),explosionModel(std::make_unique<nemisindo::Explosion>())

{

      formatManager.registerBasicFormats();
}

#endif
QAPAudioProcessor::~QAPAudioProcessor()
{

}


//==============================================================================
const juce::String QAPAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool QAPAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool QAPAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool QAPAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double QAPAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int QAPAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int QAPAudioProcessor::getCurrentProgram()
{
    return 0;
}

void QAPAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String QAPAudioProcessor::getProgramName (int index)
{
    return {};
}

void QAPAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================


#ifndef JucePlugin_PreferredChannelConfigurations
bool QAPAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif


void QAPAudioProcessor::triggerExplosion()
{
    float newRumble=0.0f;
    float newRumbleDecay=0.0f;
    float newAir=0.0f;
    float newAirDecay=0.0f;
    float newDust=0.0f;
    float newDustDecay=0.0f;
    float newGritAmount=0.0f;
    if (explosionModel)
       {
           auto* rumbleParam = parameters.getRawParameterValue("rumble");
           newRumble=rumbleParam->load();
           explosionModel->setRumble(newRumble);
           //explosionModel->setRumble(0.6f);
           auto* rumbleDecayParam=parameters.getRawParameterValue("rumbleDecay");
           newRumbleDecay=rumbleDecayParam->load();
           explosionModel->setRumbleDecay(newRumbleDecay);
           //explosionModel->setRumbleDecay(4.0f);
           auto* airParam=parameters.getRawParameterValue("air");
           newAir=airParam->load();
           explosionModel->setAir(newAir);
           auto* airDecayParam=parameters.getRawParameterValue("airDecay");
           newAirDecay=airDecayParam->load();
            explosionModel->setAirDecay(newAirDecay);
           //explosionModel->setAir(0.8f);
           //explosionModel->setAirDecay(2.0f);
           auto* dustParam=parameters.getRawParameterValue("dust");
           newDust=dustParam->load();
           explosionModel->setDust(newDust);
           auto* dustDecayParam=parameters.getRawParameterValue("dustDecay");
           newDustDecay=dustDecayParam->load();
           explosionModel->setDustDecay(newDustDecay);
                   
           explosionModel->setTimeSeparation(0.0f);
           explosionModel->setGrit(true);
           auto* gritAmountParam=parameters.getRawParameterValue("gritAmount");
           newGritAmount=gritAmountParam->load();
           explosionModel->setGritAmount(newGritAmount);
           explosionModel->setOverTheTop(true);
           explosionModel->trigger();
       }
}

void QAPAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    transportSource.prepareToPlay(samplesPerBlock, sampleRate);
    if (explosionModel)
        {
            explosionModel->initialize((float) sampleRate);
           // explosionModel->updateParameterInterval(samplesPerBlock);

        }
}

void QAPAudioProcessor::releaseResources()
{
    transportSource.releaseResources();
}

void QAPAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();
    buffer.clear();
    

    juce::AudioSourceChannelInfo info(buffer);
    transportSource.getNextAudioBlock(info);
    float samples=buffer.getNumSamples();
     
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());
    


    if (explosionModel->isActive())
        {
            explosionModel->fillBuffer(const_cast<float**>(buffer.getArrayOfWritePointers()), buffer.getNumSamples());
        }
    
}


void QAPAudioProcessorEditor::chooseLibraryFolder()
{
    folderChooser = std::make_unique<juce::FileChooser>(
        "Select your sound library folder...",
        juce::File::getSpecialLocation(juce::File::userHomeDirectory),
        ""
    );

    const int flags = juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectDirectories;

    folderChooser->launchAsync(flags, [this](const juce::FileChooser& fc) {
        juce::File selectedFolder = fc.getResult();

        if (selectedFolder.isDirectory())
        {
            audioProcessor.loadAllWavFilesFromFolder(selectedFolder);
        }

        // Clear the pointer to destroy FileChooser after use
        folderChooser = nullptr;
    });
}


void QAPAudioProcessor::loadAllWavFilesFromFolder(const juce::File &folder)
{
    juce::Array<juce::File> wavFiles;
    folder.findChildFiles(wavFiles, juce::File::findFiles, true, "*.wav");
    wavFileNames.clear();
    wavFilePaths.clear();
    

    for (auto& file : wavFiles)
    {
        DBG("Found WAV: " + file.getFileName());
        wavFilePaths.add(file);
        wavFileNames.add(file.getFileName());
    }
    if (auto* editor = dynamic_cast<QAPAudioProcessorEditor*>(getActiveEditor()))
    {
        editor->refreshWavFileList();
    }
}

juce::File QAPAudioProcessor::getWavFileByName(const juce::String& name) const
{
    for (int i = 0; i < wavFilePaths.size(); ++i)
    {
        if (wavFilePaths[i].getFileName() == name)
            return wavFilePaths[i];
    }
    return {};
}

void QAPAudioProcessor::playWavFileByName(const juce::String& name)
{
    auto file = getWavFileByName(name);

    if (!file.existsAsFile())
        return;

    auto* reader = formatManager.createReaderFor(file);

    if (reader != nullptr)
    {
        std::unique_ptr<juce::AudioFormatReaderSource> newSource(new juce::AudioFormatReaderSource(reader, true));
        transportSource.stop();
        transportSource.setSource(newSource.get(), 0, nullptr, reader->sampleRate);
        readerSource = std::move(newSource);
        transportSource.start();
    }
}


bool QAPAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* QAPAudioProcessor::createEditor()
{
    return new QAPAudioProcessorEditor (*this);
}


//==============================================================================
void QAPAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void QAPAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new QAPAudioProcessor();
}


juce::AudioProcessorValueTreeState::ParameterLayout QAPAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> parameters;

    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"rumble", 1},"Rumble", 0.1f, 1.0f, 0.5f));
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"rumbleDecay",1},"Rumble Decay", 0.5f, 4.0f, 4.0f));
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"dust",1}, "Dust", 0.0f, 1.0f, 0.5f));
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"dustDecay",1}, "Dust Decay", 0.0f, 5.0f, 1.0f));
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"air",1}, "Air", 0.0f, 1.0f, 0.5f));
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"airDecay",1}, "Air Decay", 1.0f, 5.0f, 1.0f));
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"gritAmount",1}, "Grit Amount", 0.0f, 1.0f, 0.5f));
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"timeSeparation",1}, "Time Separation", 0.0f, 1.0f, 0.5f));

    return { parameters.begin(), parameters.end() };
}
