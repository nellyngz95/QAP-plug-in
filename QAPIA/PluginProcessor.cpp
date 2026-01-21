/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "ExplosionImpl.h"
#include "HelicopterImpl.h"
#include "RocketImpl.h"
#include <onnxruntime/onnxruntime_cxx_api.h>
#include <faiss/IndexFlat.h>
QAPAudioProcessor::QAPAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
    : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                      ), parameters(*this, nullptr, "parameters", createParameterLayout()),explosionModel(std::make_unique<nemisindo::Explosion>()),fireModel(std::make_unique<nemisindo::Fire>()),gunModel(std::make_unique<nemisindo::Gun>()),JetModel(std::make_unique<nemisindo::Jet::impl>()),HelicopterModel(std::make_unique<nemisindo::Helicopter::impl>()),RocketModel(std::make_unique<nemisindo::Rocket>()),backgroundThread("Audio Recorder Thread")

{

    formatManager.registerBasicFormats();
    backgroundThread.startThread();
    auto* reader = formatManager.createReaderFor (std::make_unique<juce::MemoryInputStream> (
                                                                                             BinaryData::AssistantSound_wav,
                                                                                             BinaryData::AssistantSound_wavSize,
            false));

        if (reader != nullptr)
        {
            readerSource = std::make_unique<juce::AudioFormatReaderSource> (reader, true);
            transportSource.setSource (readerSource.get());
        }
    try {
            Ort::SessionOptions sessionOptions;
            sessionOptions.SetIntraOpNumThreads(1);

            onnxSession = std::make_unique<Ort::Session>(
                env,
                BinaryData::model_onnx,
                BinaryData::model_onnxSize,
                sessionOptions
            );

            faissIndex = std::make_unique<faiss::IndexFlatL2>(960);

            DBG("ONNX and Faiss initialized!");
        }
        catch (const std::exception& e) {
            DBG("Initialization error: " << e.what());
        }
}


#endif
QAPAudioProcessor::~QAPAudioProcessor()
{
    backgroundThread.stopThread(4000);
}


//=======FAISS==========
void QAPAudioProcessor::indexLibraryFolder(const juce::File& folder)
{
    // 1. Reset everything so we don't have old data
    faissIndex->reset();
    libraryFiles.clear();

    // 2. Find the files
    auto files = folder.findChildFiles(juce::File::findFiles, true, "*.wav;*.mp3");
    
    DBG("--- STARTING INDEXING ---");

    for (auto& file : files)
    {
        // 3. ANALYSIS: Run the file through the ONNX model
        std::vector<float> fingerprint = runInference(file);
        

        // 4. STORAGE: If the AI successfully generated numbers, save them
        if (!fingerprint.empty() && fingerprint.size() == embeddingSize)
        {
            faissIndex->add(1, fingerprint.data()); // Add to math index
            libraryFiles.add(file);                 // Add to file list
            DBG("Indexed: " << file.getFileName());
        }
    }

    DBG("--- INDEXING FINISHED ---");
    DBG("Total files ready for search: " << faissIndex->ntotal);
}

void QAPAudioProcessor::performSimilaritySearch(const juce::File& droppedFile)
{
    if (libraryFiles.isEmpty()) return;

    std::vector<float> query = runInference(droppedFile);
    if (query.empty()) return;

    int k = 10; // Number of results
    std::vector<float> distances(k);
    std::vector<faiss::idx_t> indices(k);

    // FAISS SEARCH
    faissIndex->search(1, query.data(), k, distances.data(), indices.data());

    similarFilesResults.clear();
    for (int i = 0; i < k; ++i)
    {
        int idx = (int)indices[i];
        if (idx >= 0 && idx < libraryFiles.size())
        {
            // Add the actual File object to your results array
            similarFilesResults.add(libraryFiles[idx]);
        }
    }
}
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
// Inference:
std::vector<float> QAPAudioProcessor::runInference(const juce::File& file)
{
    std::unique_ptr<juce::AudioFormatReader> reader(formatManager.createReaderFor(file));
    
    // Si el archivo no existe o está corrupto, avísanos
    if (!reader) {
        DBG("ERROR: No se pudo leer el archivo: " << file.getFileName());
        return {};
    }

    // Forzamos a leer máximo 2 segundos para no saturar la RAM
    int maxSamples = (int)(reader->sampleRate * 2.0);
    int numSamplesToRead = std::min((int)reader->lengthInSamples, maxSamples);
    
    if (numSamplesToRead <= 0) return {};

    juce::AudioBuffer<float> buffer(1, numSamplesToRead);
    // IMPORTANTE: Leemos del canal 0 (Mono)
    reader->read(&buffer, 0, numSamplesToRead, 0, true, true);

    try {
        auto memoryInfo = Ort::MemoryInfo::CreateCpu(OrtDeviceAllocator, OrtMemTypeCPU);
        
        // Cambia esto a { 1, numSamplesToRead } si { 1, 1, numSamplesToRead } falla
        std::vector<int64_t> inputShape = { 1,(int64_t)numSamplesToRead };

        Ort::Value inputTensor = Ort::Value::CreateTensor<float>(
            memoryInfo,
            const_cast<float*>(buffer.getReadPointer(0)),
            numSamplesToRead,
            inputShape.data(),
            inputShape.size());

        // Estos nombres DEBEN coincidir con tu modelo .onnx
        const char* inputNames[] = { "waveform" };
        const char* outputNames[] = { "embedding" };

        auto outputTensors = onnxSession->Run(Ort::RunOptions{nullptr}, inputNames, &inputTensor, 1, outputNames, 1);
        
        float* floatArr = outputTensors[0].GetTensorMutableData<float>();
        return std::vector<float>(floatArr, floatArr + embeddingSize);
    }
    catch (const std::exception& e) {
        // SI LA IA FALLA, ESTO APARECERÁ EN TU CONSOLA:
        DBG("IA ERROR en " << file.getFileName() << ": " << e.what());
        return {};
    }
}
void QAPAudioProcessor::triggerHelicopter()
{
    if (HelicopterModel)
{
    if (HelicopterModel->isActive())
    {
        HelicopterModel->stop();
    }
    else
    {
        HelicopterModel->start();
    }
}
}
void QAPAudioProcessor::triggerJet()
{
    
    if (JetModel)
    {
        if (JetModel->isActive())
        {
            JetModel->stop();
        }
        else
        {
            JetModel->start();
        }
    }
        
}


void QAPAudioProcessor::triggerGun()
{
    float newShellFreq=0.0f;
    float newShellFreqDecay=0.0f;
    auto* ShellFreqParam = parameters.getRawParameterValue("shellfreq");
    newShellFreq=ShellFreqParam->load();
    gunModel->setShellFreq(newShellFreq);
    auto* ShellFreqDecayParam = parameters.getRawParameterValue("shellfreqdecay");
    newShellFreqDecay=ShellFreqDecayParam->load();
    gunModel->setShellDecay(newShellFreqDecay);
    //GunModel->setShellGain(0.5f);
    gunModel->trigger();
    
}


void QAPAudioProcessor::triggerFire()
{
    if (fireModel->isActive()){fireModel->stop();}
      else{fireModel->start();
    }
}

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
           auto* rumbleDecayParam=parameters.getRawParameterValue("rumbleDecay");
           newRumbleDecay=rumbleDecayParam->load();
           explosionModel->setRumbleDecay(newRumbleDecay);
           auto* airParam=parameters.getRawParameterValue("air");
           newAir=airParam->load();
           explosionModel->setAir(newAir);
           auto* airDecayParam=parameters.getRawParameterValue("airDecay");
           newAirDecay=airDecayParam->load();
            explosionModel->setAirDecay(newAirDecay);
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


void QAPAudioProcessor::triggerRocket()
{
    if (RocketModel)
           {
               float newDuration=0.0f;
               float newChamberResonance=0.0f;
               float newFlutter=0.0f;
               
               RocketModel->trigger();
               auto* DurationParam= parameters.getRawParameterValue("duration");
               newDuration=DurationParam->load();
               RocketModel->setDuration(newDuration);
               auto* ChamberResonanceParam=parameters.getRawParameterValue("chamberresonance");
               newChamberResonance=ChamberResonanceParam->load();
               RocketModel->setChamberResonance(newChamberResonance);
               auto* FlutterParam=parameters.getRawParameterValue("flutter");
               newFlutter=FlutterParam->load();
               RocketModel->setFlutter(newFlutter);
               
           }
   
    
    
}

void QAPAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    transportSource.prepareToPlay(samplesPerBlock, sampleRate);

    juce::dsp::ProcessSpec spec;
    spec.sampleRate=sampleRate;
    spec.maximumBlockSize=(juce::uint32)samplesPerBlock;
    spec.numChannels=(juce::uint32)getTotalNumOutputChannels();
    
    juce::dsp::Reverb::Parameters reverbParams;
    reverbParams.roomSize = 0.6f;
    reverbParams.damping = 0.4f;
    reverbParams.wetLevel = 0.3f;
    reverbParams.dryLevel = 0.7f;
    reverb.setParameters(reverbParams);

    // Compressor setup
    compressor.setThreshold(-12.0f);
    compressor.setRatio(3.0f);
    compressor.setAttack(10.0f);
    compressor.setRelease(100.0f);
    
    //Filters
      processors.clear();
      for(int i=0;i<getTotalNumOutputChannels();++i)
      {
          processors.add(new ChannelFilterChain());
          processors.getLast()->prepare(spec);
      }
    
    if (explosionModel)
    {
        explosionModel->initialize((float) sampleRate);
    }
    
    if (fireModel)
    {
        fireModel->initialize((float) sampleRate);
    }
    if (gunModel)
    {
        gunModel->initialize((float) sampleRate);
    }
    if (JetModel)
    {
        JetModel->initialize((float) sampleRate);
    }
    if (HelicopterModel)
    {
        HelicopterModel->initialize((float) sampleRate);
    }
    if (RocketModel)
    {
        RocketModel->initialize((float) sampleRate);
    }
    reverb.prepare(spec);
    compressor.prepare(spec);
    //eqChain.prepare(spec);
}

void QAPAudioProcessor::releaseResources()
{
    transportSource.releaseResources();
    processors.clear();
}

void QAPAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();
    auto numSamples = buffer.getNumSamples();
    float sampleRate = (float)juce::AudioProcessor::getSampleRate();
    buffer.clear();
    
    if (PlayAssistantSound.load())
        {
            transportSource.stop();         // Stop any current ping
            transportSource.setPosition(0); // Rewind immediately
            transportSource.start();        // Start the new ping
            PlayAssistantSound.store(false);
        }
    if (transportSource.isPlaying())
        {
            juce::AudioSourceChannelInfo info(buffer);
            transportSource.getNextAudioBlock(info);
        }
    
    //Filters coefficient
    auto lowCoefficients = juce::dsp::IIR::Coefficients<float>::makeLowShelf(sampleRate, 200.0f, 0.707f, 2.0f);
    auto midCoefficients = juce::dsp::IIR::Coefficients<float>::makePeakFilter(sampleRate, 1000.0f, 0.707f, 2.0f);
    auto highCoefficients = juce::dsp::IIR::Coefficients<float>::makeHighShelf(sampleRate, 3000.0f, 2.0f,2.0f);
        
    
    float newLapping = *parameters.getRawParameterValue("lapping");
    float newHissing = *parameters.getRawParameterValue("hissing");
    float newCrackling = *parameters.getRawParameterValue("crackling");
    float newIntensity = *parameters.getRawParameterValue("intensity");
    //helicopter
    float newRotorPeriod = *parameters.getRawParameterValue("rotorPeriod");
    float newPeriod = *parameters.getRawParameterValue("period");
    float newTailMix = *parameters.getRawParameterValue("tailMix");
    float newBaseFreq = *parameters.getRawParameterValue("baseFreq");
    float newRotorMix = *parameters.getRawParameterValue("rotorMix");
    float newEngineMix = *parameters.getRawParameterValue("engineMix");
    float newBladeNoise = *parameters.getRawParameterValue("bladeNoise");
    float newEngineSpeed = *parameters.getRawParameterValue("engineSpeed");
    
    if (explosionModel->isActive())
        {
            juce::AudioBuffer<float> proceduralBuffer(buffer.getNumChannels(), buffer.getNumSamples());
            proceduralBuffer.clear();
            explosionModel->fillBuffer(const_cast<float**>(proceduralBuffer.getArrayOfWritePointers()), proceduralBuffer.getNumSamples());
            juce::dsp::AudioBlock<float> block(proceduralBuffer);
            juce::dsp::ProcessContextReplacing<float> context(block);
            for (size_t channel = 0; channel < block.getNumChannels(); ++channel)
                    {
                        auto& chain = *processors[(int)channel];
                       

                        chain.get<0>().coefficients=lowCoefficients;
                        chain.get<1>().coefficients=midCoefficients;
                        chain.get<2>().coefficients=highCoefficients;
                        
                        auto channelBlock = block.getSingleChannelBlock ((int)channel);
                        auto context = juce::dsp::ProcessContextReplacing<float>{ channelBlock };
                        chain.process(context);
                        reverb.process(context);
                        compressor.process(context);
                    }
            
            for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
                buffer.addFrom(ch, 0, proceduralBuffer, ch, 0, proceduralBuffer.getNumSamples());


            buffer.addFrom(0, 0, proceduralBuffer, 0, 0, proceduralBuffer.getNumSamples());
        }
        
        fireModel->setLapping(newLapping);
        fireModel->setHissing(newHissing);
        fireModel->setCrackling(newCrackling);
        fireModel->setIntensity(newIntensity);
    
        if (fireModel->isActive())
        {
            juce::AudioBuffer<float> proceduralBuffer(buffer.getNumChannels(), buffer.getNumSamples());
            proceduralBuffer.clear();
            fireModel->fillBuffer(const_cast<float**>(proceduralBuffer.getArrayOfWritePointers()), proceduralBuffer.getNumSamples());
            juce::dsp::AudioBlock<float> block(proceduralBuffer);
            juce::dsp::ProcessContextReplacing<float> context(block);
            for (size_t channel = 0; channel < block.getNumChannels(); ++channel)
                    {
                        auto& chain = *processors[(int)channel];
                       

                        chain.get<0>().coefficients=lowCoefficients;
                        chain.get<1>().coefficients=midCoefficients;
                        chain.get<2>().coefficients=highCoefficients;
                        
                        auto channelBlock = block.getSingleChannelBlock ((int)channel);
                        auto context = juce::dsp::ProcessContextReplacing<float>{ channelBlock };
                        chain.process(context);
                        reverb.process(context);
                        compressor.process(context);
                    }
            
            for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
                buffer.addFrom(ch, 0, proceduralBuffer, ch, 0, proceduralBuffer.getNumSamples());

        }
    
        if (gunModel->isActive())
        {
            juce::AudioBuffer<float> proceduralBuffer(buffer.getNumChannels(), buffer.getNumSamples());
            proceduralBuffer.clear();
            gunModel->fillBuffer(const_cast<float**>(proceduralBuffer.getArrayOfWritePointers()), proceduralBuffer.getNumSamples());
            juce::dsp::AudioBlock<float> block(proceduralBuffer);
            juce::dsp::ProcessContextReplacing<float> context(block);
            for (size_t channel = 0; channel < block.getNumChannels(); ++channel)
                    {
                        auto& chain = *processors[(int)channel];
                       

                        chain.get<0>().coefficients=lowCoefficients;
                        chain.get<1>().coefficients=midCoefficients;
                        chain.get<2>().coefficients=highCoefficients;
                        
                        auto channelBlock = block.getSingleChannelBlock ((int)channel);
                        auto context = juce::dsp::ProcessContextReplacing<float>{ channelBlock };
                        chain.process(context);
                        reverb.process(context);
                        compressor.process(context);
                    }
            
            for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
                buffer.addFrom(ch, 0, proceduralBuffer, ch, 0, proceduralBuffer.getNumSamples());

        }
    float newSpeed= 0.0f;
    float newTurbine=0.0f;
    float newBurn=0.0f;
    
    
    if (JetModel->isActive())
    {
        juce::AudioBuffer<float> proceduralBuffer(buffer.getNumChannels(), buffer.getNumSamples());
        proceduralBuffer.clear();
        auto* SpeedParam = parameters.getRawParameterValue("speed");
        newSpeed=SpeedParam->load();
        JetModel->setSpeed(newSpeed);
       auto* TurbineParam = parameters.getRawParameterValue("turbine");
        newTurbine=TurbineParam->load();
        JetModel->setTurbine(newTurbine);
        auto* BurnParam = parameters.getRawParameterValue("burn");
        newBurn=BurnParam->load();
        JetModel-> setBurn(newBurn);
        JetModel->fillBuffer(const_cast<float**>(proceduralBuffer.getArrayOfWritePointers()), proceduralBuffer.getNumSamples());
        juce::dsp::AudioBlock<float> block(proceduralBuffer);
        juce::dsp::ProcessContextReplacing<float> context(block);
        for (size_t channel = 0; channel < block.getNumChannels(); ++channel)
                {
                    auto& chain = *processors[(int)channel];
                   

                    chain.get<0>().coefficients=lowCoefficients;
                    chain.get<1>().coefficients=midCoefficients;
                    chain.get<2>().coefficients=highCoefficients;
                    
                    auto channelBlock = block.getSingleChannelBlock ((int)channel);
                    auto context = juce::dsp::ProcessContextReplacing<float>{ channelBlock };
                    chain.process(context);
                    reverb.process(context);
                    compressor.process(context);
                }
        
        for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
            buffer.addFrom(ch, 0, proceduralBuffer, ch, 0, proceduralBuffer.getNumSamples());

        buffer.addFrom(0, 0, proceduralBuffer, 0, 0, proceduralBuffer.getNumSamples());
    }
    
    HelicopterModel->setRotorPeriod(newRotorPeriod);
    HelicopterModel->setResonance(newPeriod);
    HelicopterModel->setTailMix(newTailMix);
    HelicopterModel->setBaseFreq(newBaseFreq);
    HelicopterModel->setRotorMix(newRotorMix);
    HelicopterModel->setEngineMix(newEngineMix);
    HelicopterModel->setBladeNoise(newBladeNoise);
    HelicopterModel->setEngineSpeed(newEngineSpeed);

    if (HelicopterModel->isActive())
    {
        juce::AudioBuffer<float> proceduralBuffer(buffer.getNumChannels(), buffer.getNumSamples());
        proceduralBuffer.clear();
        HelicopterModel->fillBuffer(const_cast<float**>(proceduralBuffer.getArrayOfWritePointers()), proceduralBuffer.getNumSamples());
        juce::dsp::AudioBlock<float> block(proceduralBuffer);
        juce::dsp::ProcessContextReplacing<float> context(block);
        for (size_t channel = 0; channel < block.getNumChannels(); ++channel)
                {
                    auto& chain = *processors[(int)channel];
                   

                    chain.get<0>().coefficients=lowCoefficients;
                    chain.get<1>().coefficients=midCoefficients;
                    chain.get<2>().coefficients=highCoefficients;
                    
                    auto channelBlock = block.getSingleChannelBlock ((int)channel);
                    auto context = juce::dsp::ProcessContextReplacing<float>{ channelBlock };
                    chain.process(context);
                    reverb.process(context);
                    compressor.process(context);
                }
        
        for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
            buffer.addFrom(ch, 0, proceduralBuffer, ch, 0, proceduralBuffer.getNumSamples());

    }
    if (RocketModel->isActive())
    {
        juce::AudioBuffer<float> proceduralBuffer(buffer.getNumChannels(), buffer.getNumSamples());
        proceduralBuffer.clear();
        RocketModel->fillBuffer(const_cast<float**>(proceduralBuffer.getArrayOfWritePointers()), proceduralBuffer.getNumSamples());
        juce::dsp::AudioBlock<float> block(proceduralBuffer);
        juce::dsp::ProcessContextReplacing<float> context(block);
        for (size_t channel = 0; channel < block.getNumChannels(); ++channel)
                {
                    auto& chain = *processors[(int)channel];
                   

                    chain.get<0>().coefficients=lowCoefficients;
                    chain.get<1>().coefficients=midCoefficients;
                    chain.get<2>().coefficients=highCoefficients;
                    
                    auto channelBlock = block.getSingleChannelBlock ((int)channel);
                    auto context = juce::dsp::ProcessContextReplacing<float>{ channelBlock };
                    chain.process(context);
                    reverb.process(context);
                    compressor.process(context);
                }
        
        for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
            buffer.addFrom(ch, 0, proceduralBuffer, ch, 0, proceduralBuffer.getNumSamples());

    }
    

    if (auto* writer = activeWriter.load())
    {
        writer->write(buffer.getArrayOfReadPointers(), buffer.getNumSamples());
    }



}


void QAPAudioProcessor::startRecording()
{
    juce::File outputDir = juce::File::getSpecialLocation(juce::File::userDocumentsDirectory);
    juce::File outputFile = outputDir.getChildFile("QAP_recording_" + juce::String(juce::Time::getCurrentTime().toMilliseconds()) + ".wav");

    fileStream = outputFile.createOutputStream();
    if (fileStream == nullptr)
    {
        juce::Logger::writeToLog("ERROR: Couldn't open file for writing.");
        return;
    }

    auto newWriter = juce::WavAudioFormat().createWriterFor(
        fileStream.get(),
        getSampleRate(),
        getTotalNumOutputChannels(),
        16,
        {},
        0
    );

    if (newWriter == nullptr)
    {
        juce::Logger::writeToLog("ERROR: Couldn't create WAV writer.");
        return;
    }

    // Transfer ownership
    fileStream.release();
    threadedWriter = std::make_unique<juce::AudioFormatWriter::ThreadedWriter>(newWriter, backgroundThread, 32768);
    activeWriter.store(threadedWriter.get());
    isRecording = true;
    juce::Logger::writeToLog("Recording started: " + outputFile.getFullPathName());
}




void QAPAudioProcessor::stopRecording()
{
    isRecording = false;
    activeWriter.store(nullptr);
    threadedWriter.reset();
    writer.reset();
    fileStream.reset();

    juce::Logger::writeToLog("Recording stopped.");
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
            auto result = fc.getResult();
            if (result.isDirectory())
            {
                // ESTA LÍNEA ES LA QUE ACTIVA TODO:
                audioProcessor.indexLibraryFolder(result);
                
                // Refrescamos la lista visual
                refreshWavFileList();
            }
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

void QAPAudioProcessor::LoadDroppedFile (const juce::File& file)
{
    transportSource.stop();
    transportSource.setSource(nullptr);

    auto* reader = formatManager.createReaderFor(file);

    if (reader != nullptr)
    {
        
        auto newSource = std::make_unique<juce::AudioFormatReaderSource>(reader, true);
        transportSource.setSource(newSource.get(), 0, nullptr, reader->sampleRate);
        readerSource.reset(newSource.release());
        transportSource.setPosition(0);
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
    //Explosion
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"rumble", 1},"Rumble", 0.1f, 1.0f, 0.5f));
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"rumbleDecay",1},"Rumble Decay", 0.5f, 4.0f, 4.0f));
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"dust",1}, "Dust", 0.0f, 1.0f, 0.5f));
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"dustDecay",1}, "Dust Decay", 0.0f, 5.0f, 1.0f));
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"air",1}, "Air", 0.0f, 1.0f, 0.5f));
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"airDecay",1}, "Air Decay", 1.0f, 5.0f, 1.0f));
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"gritAmount",1}, "Grit Amount", 0.0f, 1.0f, 0.5f));
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"timeSeparation",1}, "Time Separation", 0.0f, 1.0f, 0.5f));
    //Fire
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"lapping", 1},"Lapping", 0.0f, 1.0f, 0.5f));
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"hissing", 1},"Hissing", 0.0f, 1.0f, 0.5f));
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"crackling", 1},"Crackling", 0.0f, 1.0f, 0.5f));
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"intensity", 1},"Intensity", 0.0f, 1.0f, 0.5f));
    //Gun
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"shellfreq", 1},"Shell Frequency", 0.1f,1.0f, 0.5f));
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"shellfreqdecay",1},"Shell Frequency Decay", 0.5f, 1.0f, 0.1f));
    
    //Jet
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"speed", 1},"Speed", 0.1f, 1.0f, 0.5f));
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"turbine",1},"Turbine", 0.5f, 1.0f, 0.1f));
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"burn",1},"Burn", 0.5f, 1.0f, 0.1f));
    
    //Helicopter
    
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"rotorPeriod", 1},"Rotor Period", 10.0f, 210.0f, 77.0f));
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"period",1},"Period", 0.5f, 1.0f, 0.1f));
     parameters.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"tailMix",1},"Tail Mix", 0.0f,1.0f, 0.2f));
     parameters.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"baseFreq",1},"Base Frequency", 0.0f,1.0f, 0.25f));
     parameters.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"rotorMix",1},"Rotor Mix", 0.0f,1.0f, 0.65f));
     parameters.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"engineMix",1},"Engine Mix", 0.0f,1.0f, 0.2f));
     parameters.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"bladeNoise",1},"Blade Noise", 0.0f,1.0f, 0.3f));
     parameters.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"engineSpeed",1},"Engine Speed", 0.0f,1.0f, 0.5f));
    
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"duration", 1},"duration", 1.0f, 10.0f, 5.0f));
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"chamberresonance",1},"Chamber Resonance", 16.0f, 20.0f, 19.0f));
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"flutter",1},"Flutter", 2.0f, 40.0f, 30.0f));
   
    return { parameters.begin(), parameters.end() };
}
