#pragma once
#include <miniaudio.h>
#include <string>

using seconds_t = float;
class Player{
    template<typename T>
    class PrevInit
    {
        T Data;
        public:
        bool Init = false;
        operator T()
        {
            return Data;
        }
        operator bool()
        {
            return Init;
        }
        T* operator &()
        {
            return &Data;
        }
    };
    PrevInit<ma_sound> Sound;
    ma_engine_config Config;
    PrevInit<ma_engine> Engine;
    std::string CurrentPlaying;
    public:
    Player();
    Player(const Player& Another) = delete;
    Player& operator=(const Player& Another) = delete;
    void Configure( ma_uint32 OutputChannels, ma_uint32 OutputSampleRate);
    void InitEngine();
    void Reload(std::string Path);
    bool InitSound(std::string Path,void(*Callback)(void *pUserData, ma_sound *pSound),void* CustomData);
    void SetVolume(int Volume);
    void SetTime(seconds_t Time);
    void AddTime(seconds_t Time);
    void Play();
    void Stop();
    void PlayStop();
    bool IsPlaying();
    void ReinitEngine();
    void DeinintAudio();
    seconds_t GetPlayTimeInSeconds();
    seconds_t GetLengthInSeconds();
    ~Player();

};
