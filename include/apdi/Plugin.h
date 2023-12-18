//
//  APDIPlugin.h
//  Effect & Synth Plugin Framework - Plugin Wrappers
//
//  Created by Chris Nash on 02/10/2013.
//  Updated by Chris Nash on 18/01/2016 with support for OS X 10.10 SDK.
//  Updated by Chris Nash on 02/10/2018 with support for TestEffectAU.
//  Updated by Chris Nash on 02/09/2020 with new architecture for MacOS and Windows.
//
//  This file describes a number of abstractions and extensions to STK,
//  to support audio processing and programming teaching at UWE.

#pragma once

#if defined(_WIN32)
#define CREATE_FUNCTION   __declspec(dllexport) void* __stdcall
#define SOUNDS_FUNCTION   __declspec(dllexport) void __stdcall
#else
#define CREATE_FUNCTION   void*
#define SOUNDS_FUNCTION   void
#endif

#include <vector>
#include <string>

namespace APDI
{
    class Wavetable;

    struct Parameter
    {
        enum Type
        {
            ROTARY, // rotary knob (pot)
            BUTTON, // push button (trigger)
            TOGGLE, // on/off switch (toggle)
            SLIDER, // linear slider (fader)
            MENU,   // drop-down list (menu)
            METER,  // level meter (read-only: use setParameter() to set value)
            WHEEL,  // MIDI control (Pitch Bend / Mod Wheel only)
        };
    
        struct Bounds
        {
            Bounds(int x = -1, int y = -1, int width = -1, int height = -1)
            : x(x), y(y), width(width), height(height) { }
        
            int x;
            int y;
            int width;
            int height;

            bool isAuto() const { return x == -1 && y == -1 && width == -1 && height == -1;  }
        
            #define AUTO_SIZE Parameter::Bounds()
        };
    
        Parameter(const char* name, Type type, float min, float max, float initial, Bounds size = AUTO_SIZE)
        : name(name), type(type), min(min), max(max), initial(initial), size(size), value(0.f) { }
    
        Parameter(const char* name, Type type, std::initializer_list<std::string> options, Bounds size = AUTO_SIZE)
        : name(name), type(MENU), min(0.f), max(1.f), initial(0.f), size(size), options(options), value(0.f) { }
    
        std::string name;       // name for control label / saved parameter
        Type type;              // control type (see above)
    
        float min;              // minimum control value (e.g. 0.0)
        float max;              // maximum control value (e.g. 1.0)
        float initial;          // initial value for control (e.g. 0.0)
    
        Bounds size;            // position (x,y) and size (height, width) of the control (use AUTO_SIZE for automatic layout)
    
        std::vector<std::string> options; // text options for menus and group buttons

        float value;            // current control value;
    };

    struct Parameters
    {
        Parameters() { }
        Parameters(const Parameters& parameters) : parameters(parameters.parameters) { }
        Parameters(std::initializer_list<Parameter> parameters) : parameters(parameters) { }

        float& operator[](int index) { return parameters[index].value; }
        float operator[](int index) const { return parameters[index].value; }
        const std::vector<Parameter>& get() const { return parameters; }
    protected:
        std::vector<Parameter> parameters;
    };

    struct Preset
    {
        Preset(const char* name, std::initializer_list<float> values) : name(name), values(values) { }
    
        const std::string name;            // name of preset
        const std::vector<float> values;   // parameter values
    };

    struct Presets {
        Presets(std::initializer_list<Preset> presets) : presets(presets) { }
    
        const std::vector<Preset> presets;
    };
    
    class Effect
    {
    public:
        Effect(const Parameters& parameters, const Presets& presets) : parameters(parameters), presets(presets) { }
        virtual ~Effect() { }
        
        virtual void process(const float** inputBuffers, float** outputBuffers, int numSamples) = 0;
        
        virtual void presetLoaded(int iPresetNum, const char *sPresetName) { };
        virtual void optionChanged(int iOptionMenu, int iItem) { };
        virtual void buttonPressed(int iButton) { };
        
        virtual void setSampleRate(float sr) = 0;
        virtual float getSampleRate() const = 0;
        
        Parameters parameters;
        const Presets presets;
    };

    class Synth
    {
    public:
        class Note
        {
        public:
            Note(Synth* synth) : parameters(synth->parameters), presets(synth->presets), synthesiser(synth) { }
            virtual ~Note() { }
            
            float getSampleRate() const { return synthesiser->getSampleRate(); }
            
            virtual void onStartNote (int pitch, float velocity) = 0;
            virtual bool onStopNote (float velocity) = 0;
            
            virtual void onPitchWheel (const int value) { };
            virtual void onControlChange (const int controller, const int value) { };
            
            virtual bool process (float** outputBuffer, int numChannels, int numSamples) = 0;
                
        protected:
            Parameters& parameters;
            const Presets& presets;
            Synth* synthesiser;
        };
        
        Synth(const Parameters& parameters, const Presets& presets)
        : parameters(parameters), presets(presets), wavetables(nullptr) { }
        
        Synth(const Parameters& parameters, const Presets& presets, const char* resources)
        : parameters(parameters), presets(presets), wavetables(nullptr) {
            try { if(resources) loadWavetables(resources); } catch (...) { };
        }
        
        virtual ~Synth() {
            try { unloadWavetables(); } catch (...) { };
        }
        
        virtual void postProcess(const float** inputBuffers, float** outputBuffers, int numSamples) = 0;
        
        virtual void presetLoaded(int iPresetNum, const char *sPresetName) { };
        virtual void optionChanged(int iOptionMenu, int iItem) { };
        virtual void buttonPressed(int iButton) { };
        
        virtual void setSampleRate(float sr) = 0;
        virtual float getSampleRate() const = 0;
        
        Parameters parameters;
        const Presets presets;
        
        Note* notes[32];
        const Wavetable* const getWavetable(int index) const;
        
    private:
        Wavetable* wavetables;
        void loadWavetables(const char* path);
        void unloadWavetables();
    };

} // namespace APDI
