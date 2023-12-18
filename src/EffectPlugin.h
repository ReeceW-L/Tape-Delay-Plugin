//
//  EffectPlugin.h
//  MyEffect Plugin Header File
//
//  Used to declare objects and data structures used by the plugin.
//

#pragma once

#include "apdi/Plugin.h"
#include "apdi/Helpers.h"
using namespace APDI;

#include "EffectExtra.h"

class MyEffect : public APDI::Effect
{
public:
    MyEffect(const Parameters& parameters, const Presets& presets); // constructor (initialise variables, etc.)
    ~MyEffect();                                                    // destructor (clean up, free memory, etc.)

    void setSampleRate(float sampleRate){ stk::Stk::setSampleRate(sampleRate); }
    float getSampleRate() const { return stk::Stk::sampleRate(); };
    
    void process(const float** inputBuffers, float** outputBuffers, int numSamples);
    
    void presetLoaded(int iPresetNum, const char *sPresetName);
    void optionChanged(int iOptionMenu, int iItem);
    void buttonPressed(int iButton);
    
private:
    // Declare shared member variables here
    int TapPos(float fTime);
    
    float Trioderizer (float input);
    float Pentoderizer (float input);
    
    int iBufferSize, iBufferWritePos;
    int fSR = 44100;
    
    int iBufferPos;
    
    float *pfCircularBuffer;
    
    float fDelaySig = 0.0;
    
    Noise hiss;
};
