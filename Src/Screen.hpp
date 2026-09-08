#pragma once
#include <cstdint>
#include <ftxui/component/component.hpp>
#include "Player.hpp"
class ScreenUI
{
    public:
    ScreenUI();
    void Render();
    private:

    enum : int
    {
        Yes,No
    } AutoPlay = No;
    enum : int{
        Normal,
        LoopPlaylist,
        Shuffle,
        LoopTrack
    } PlayMode = Normal;
    enum : int{
        CDefault,
        C1,
        C2,
        C3,
        C4,
        C5,
        C6,
        C7,
        С8
    } Channels;
    enum : int{
        RDefault,
        R8000,
        R16000,
        R22050,
        R32000,
        R44100,
        R48000,
    }SampleRate;
    int SkipTime = 15;
    
    int SelectedTab = 0;
    int InTab1 = 0;
    int InTab2 = 0;
    int Volume = 100;
    int MusicIndex = 0;
    Player AudioPlayer;
    ftxui::Component MainComponent;
    std::string CurrectDir;
    std::vector<std::string> Playlist;
    ftxui::Component BuildSettingsTab();
    ftxui::Component BuildPlayerTab();
    ftxui::Component BuildListTab();
    bool IsValid(const std::string& Path);
    static void AudioCallback(void *pUserData, ma_sound *pSound);
    void NextMusic();
    void PrevMusic();
    void DeleteMusicFromArray(int Index);
    bool AddToMusicIndex(int Add);
    static void UpdateFilesInDir(std::string_view Dir,std::vector<std::string>& FilesOut,std::vector<bool>& IsFolderOut);
    void ApplySettings();
};

