#include "Player.hpp"
#define MINIAUDIO_IMPLEMENTATION
#include <miniaudio.h>
Player::Player()
{
    Configure(0,0);
}
void Player::Configure( ma_uint32 OutputChannels, ma_uint32 OutputSampleRate)
{
   Config = ma_engine_config_init();
   Config.channels =  OutputChannels;
   Config.sampleRate = OutputSampleRate;

}
void Player::InitEngine()
{
    Engine.Init = ma_engine_init(&Config,&Engine) == MA_SUCCESS;
}
void Player::Reload(std::string Path)
{
    float time;
    bool is_playing = false;
    decltype((&Sound)->endCallback) callback = nullptr;
    decltype((&Sound)->pEndCallbackUserData) data = nullptr;
    if(Sound.Init)
    {
        ma_sound_get_cursor_in_seconds(&Sound,&time);
        is_playing = ma_sound_is_playing(&Sound);
        if(is_playing)
        {
            ma_sound_stop(&Sound);
        }
        callback = (&Sound)->endCallback;
        data = (&Sound)->pEndCallbackUserData;
        ma_sound_uninit(&Sound);
        Sound.Init = false;
    }
    
    InitSound(Path,callback,data);
    if(is_playing)
    {
        ma_sound_seek_to_second(&Sound,time);
        ma_sound_start(&Sound);
    }

}
bool Player::InitSound(std::string Path,void(*Callback)(void *pUserData, ma_sound *pSound) = nullptr,void* CustomData = nullptr)
{
    if(Sound)
    {
        ma_sound_uninit(&Sound);
        Sound.Init = false;
    }
    if(!Engine)
    {
        InitEngine();
    }
    Sound.Init = ma_sound_init_from_file(
        &Engine, 
        Path.c_str(), 
        MA_SOUND_FLAG_STREAM | MA_SOUND_FLAG_ASYNC, 
        nullptr, 
        nullptr, 
        &Sound) == MA_SUCCESS;
    if(!Sound)
        return false;
    ma_sound_set_end_callback(&Sound,Callback,CustomData);
    return true;
}

void Player::SetVolume(int Volume)
{
    if(Sound)
        ma_sound_set_volume(&Sound,Volume / 100.);
}

void Player::Play()
{
    if(Sound)
        ma_sound_start(&Sound);
}

void Player::Stop()
{
    if(Sound)
        ma_sound_stop(&Sound);
}

void Player::PlayStop()
{
    if(Sound)
    {
        if(ma_sound_is_playing(&Sound))
        {
            ma_sound_stop(&Sound);
        }
        else
        {
            ma_sound_start(&Sound);
        }
    }
}

bool Player::IsPlaying()
{
    if(Sound)
        return ma_sound_is_playing(&Sound);
    return false;
}

void Player::ReinitEngine()
{
    if(Sound)
    {
        ma_sound_uninit(&Sound);
        Sound.Init = false;
    }
    if(Engine)
    {
        ma_engine_uninit(&Engine);
        Engine.Init = false;
    }
    InitEngine();
}

void Player::DeinintAudio()
{
    if(Sound)
    {
        ma_sound_uninit(&Sound);
        Sound.Init = false;
    }
}

void Player::SetTime(float Time)
{
    if(Sound)
        ma_sound_seek_to_second(&Sound,Time);
}

void Player::AddTime(float Time)
{
    if(Sound)
    {
    seconds_t CurrentTime = GetPlayTimeInSeconds();
    ma_sound_seek_to_second(&Sound,Time + CurrentTime);
    }
}


seconds_t Player::GetPlayTimeInSeconds()
{
    if(Sound)
    {
    float CurrentTime;
    ma_sound_get_cursor_in_seconds(&Sound,&CurrentTime);
    return CurrentTime;
    }
    return 0;
}

seconds_t Player::GetLengthInSeconds()
{
    if(Sound)
    {
    seconds_t length;
    ma_sound_get_length_in_seconds(&Sound,&length);
    return length;
    }
    return 0;
}

Player::~Player()
{
    if (Sound.Init) { ma_sound_uninit(&Sound); Sound.Init = false; }
    if (Engine.Init) { ma_engine_uninit(&Engine); Engine.Init = false; }
    
}

