//
//  EffectPlugin.cpp
//  MyEffect Plugin Source Code
//
//  Used to define the bodies of functions used by the plugin, as declared in EffectPlugin.h.
//

#include "EffectPlugin.h"
#include<cmath>

////////////////////////////////////////////////////////////////////////////
// EFFECT - represents the whole effect plugin
////////////////////////////////////////////////////////////////////////////
///


// Called to create the effect (used to add your effect to the host plugin)
extern "C" {
    CREATE_FUNCTION createEffect(float sampleRate) {
        ::stk::Stk::setSampleRate(sampleRate);
        
        //==========================================================================
        // CONTROLS - Use this array to completely specify your UI
        // - tells the system what parameters you want, and how they are controlled
        // - add or remove parameters by adding or removing entries from the list
        // - each control should have an expressive label / caption
        // - controls can be of different types: ROTARY, BUTTON, TOGGLE, SLIDER, or MENU (see definitions)
        // - for rotary and linear sliders, you can set the range of values (make sure the initial value is inside the range)
        // - for menus, replace the three numeric values with a single array of option strings: e.g. { "one", "two", "three" }
        // - by default, the controls are laid out in a grid, but you can also move and size them manually
        //   i.e. replace AUTO_SIZE with { 50,50,100,100 } to place a 100x100 control at (50,50)
        
        const Parameters CONTROLS = {
            //  name,       type,              min, max, initial, size
            {   "Delay time",  Parameter::ROTARY, 0.0, 1.0, 0.0, AUTO_SIZE  },
            {   "Feedback Gain",  Parameter::ROTARY, 0.0, 1.0, 0.0, AUTO_SIZE  },
            {   "Distortion",  Parameter::ROTARY, 0.0, 1.0, 0.0, AUTO_SIZE  },
            {   "Hiss",  Parameter::SLIDER, 0.0, 1.0, 0.0, AUTO_SIZE  },
            {   "Distort Type",  Parameter::MENU, {"Triode", "Pentode"}, AUTO_SIZE  },
            {   "Param 5",  Parameter::ROTARY, 0.0, 1.0, 0.0, AUTO_SIZE  },
            {   "Param 6",  Parameter::ROTARY, 0.0, 1.0, 0.0, AUTO_SIZE  },
            {   "Param 7",  Parameter::ROTARY, 0.0, 1.0, 0.0, AUTO_SIZE  },
            {   "Param 8",  Parameter::ROTARY, 0.0, 1.0, 0.0, AUTO_SIZE  },
            {   "Param 9",  Parameter::ROTARY, 0.0, 1.0, 0.0, AUTO_SIZE  },
        };

        const Presets PRESETS = {
            { "Preset 1", { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
            { "Preset 2", { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
            { "Preset 3", { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
        };

        return (APDI::Effect*)new MyEffect(CONTROLS, PRESETS);
    }
}

// Constructor: called when the effect is first created / loaded
MyEffect::MyEffect(const Parameters& parameters, const Presets& presets)
: Effect(parameters, presets)
{
    // Initialise member variables, etc.
    iBufferSize = 2 * 44100;
    
    pfCircularBuffer = new float[iBufferSize];
    for (int i = 0; i < iBufferSize; i++) pfCircularBuffer[i] = 0;
    
    iBufferWritePos = 0;
    
}

// Destructor: called when the effect is terminated / unloaded
MyEffect::~MyEffect()
{
    // Put your own additional clean up code here (e.g. free memory)
    delete[] pfCircularBuffer;
}

// EVENT HANDLERS: handle different user input (button presses, preset selection, drop menus)

void MyEffect::presetLoaded(int iPresetNum, const char *sPresetName)
{
    // A preset has been loaded, so you could perform setup, such as retrieving parameter values
    // using getParameter and use them to set state variables in the plugin
}

void MyEffect::optionChanged(int iOptionMenu, int iItem)
{
    // An option menu, with index iOptionMenu, has been changed to the entry, iItem
}

void MyEffect::buttonPressed(int iButton)
{
    // A button, with index iButton, has been pressed
}

int MyEffect::TapPos(float fTime)
{
    int iBufferReadPos = iBufferWritePos - (fTime * fSR);
    
    if(iBufferReadPos < 0)
        iBufferReadPos += iBufferSize;
    
    return iBufferReadPos;
}

float MyEffect::Trioderizer (float input)
{
    if( input > 0 ) return input*input;
    else return -(input*(0.9 * input));
}

float MyEffect::Pentoderizer(float input)
{
    float out = 0.0;
    if ( input > 0 ) out = 1 - pow(10, (-1 * input));
    else out = -1 + pow(9, input);
        
    return out * 1.111;
}

// Applies audio processing to a buffer of audio
// (inputBuffer contains the input audio, and processed samples should be stored in outputBuffer)
void MyEffect::process(const float** inputBuffers, float** outputBuffers, int numSamples)
{
    float fIn0, fIn1, fOut0 = 0, fOut1 = 0;
    const float *pfInBuffer0 = inputBuffers[0], *pfInBuffer1 = inputBuffers[1];
    float *pfOutBuffer0 = outputBuffers[0], *pfOutBuffer1 = outputBuffers[1];
    
    //Param Controls
    float fTime = (parameters[0] * 0.3) + 0.1;
    float fFeedbackGain = (parameters[1] * parameters[1]);
    float fDistortionAmount = parameters[2];
    float fHissAmount = parameters[3];
    int iDistortType = parameters[4];
    
    float fDistortSig = 0.0;

    while(numSamples--)
    {
        // Get sample from input
        fIn0 = *pfInBuffer0++;
        fIn1 = *pfInBuffer1++;
        
        //Mix inputs to mono
        float fMix = (fIn0 + fIn1);
        pfCircularBuffer[iBufferWritePos] = fMix;
        
        //Increment buffer position and check for falling off
        iBufferWritePos++;
        if (iBufferWritePos > iBufferSize) iBufferWritePos -= iBufferSize;
        
        //Set read position and check for falling off
        iBufferPos = TapPos(fTime);
        fDelaySig = pfCircularBuffer[iBufferPos];
        
        //Distort Signal
        switch (iDistortType) {
            case 0:
                fDistortSig = 1.2 * Trioderizer(fMix + (fDelaySig * fFeedbackGain));
                break;
                
            case 1:
                fDistortSig = Pentoderizer(fMix + (fDelaySig * fFeedbackGain));
                break;
                
            default:
                break;
        }
        
        //Apply Hiss
        float fHiss = (hiss.tick() * fHissAmount) * 0.015;
        
        float fOutTotal =  (fMix * (1 - fDistortionAmount));
              fOutTotal += (fDelaySig * fFeedbackGain) * (1 - fDistortionAmount);
              fOutTotal += (fDistortSig * fDistortionAmount);
              fOutTotal += fHiss;
        
        //Output
        fOut0 = fOutTotal;
        fOut1 = fOutTotal;
        
        // Copy result to output
        *pfOutBuffer0++ = fOut0;
        *pfOutBuffer1++ = fOut1;
    }
}
