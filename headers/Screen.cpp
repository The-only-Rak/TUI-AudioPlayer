#include "Screen.hpp"
#include "Player.hpp"
#include <algorithm>
#include <cstdlib>
#include <ftxui/component/component.hpp>
#include <ftxui/component/event.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>
#include <filesystem>
#include <ftxui/screen/color.hpp>
#include <vector>

enum EMenuSelect
{
    PLAYER, LIST, SETTINGS
};

ScreenUI::ScreenUI()
{
    
    using namespace ftxui;
    CurrectDir = std::filesystem::current_path().string();
    const std::vector<std::string> menu_selective = {"Player","List","Settings"};
    auto options = MenuOption::Horizontal();
    auto settingsMenu = Menu(menu_selective, &SelectedTab, options);
    auto setting = BuildSettingsTab();
    auto player = BuildPlayerTab();
    auto list = BuildListTab();
    auto tabs = Container::Tab({player,list,setting},&SelectedTab);
    auto all_in = Container::Vertical({settingsMenu,tabs});
    MainComponent = Renderer(all_in,[&, settingsMenu = settingsMenu, tabs = tabs](){
        return vbox({
            settingsMenu->Render() | center,
            separator(),
            tabs->Render() | flex,
            separator(),
            text(std::format("Current playing : {} ", Playlist.empty() || MusicIndex < 0 || MusicIndex >= Playlist.size()? "None" : Playlist[MusicIndex].c_str())) | center
        }) | flex;
    });
    AudioPlayer.InitEngine();
}

void ScreenUI::Render()
{
    using namespace ftxui;
    auto screen = ScreenInteractive::Fullscreen();
    screen.Loop(MainComponent);
}



ftxui::Component ScreenUI::BuildSettingsTab()
{
    using namespace ftxui;
    using namespace std::string_literals;
    auto autoplay_toggle = Toggle(std::vector{"Yes"s,"No"s}, &(int&)AutoPlay);
    auto playmode_toggle = Toggle(std::vector{"Normal"s,"Loop playlist"s,"Shuffle"s, "Looptrack"s}, &(int&)PlayMode);
    auto sample_rate_toggle = Toggle(std::vector{"Default"s,"8000"s,"16000"s,"22050"s,"32000"s,"44100"s,"48000"s},&(int&)SampleRate );
    auto chanels_toggle = Toggle(std::vector{"Default"s,"1"s,"2"s,"3"s,"4"s,"5"s,"6"s,"7"s,"8"s},&(int&)Channels);
    auto button = Button("Ok", [this](){
        ApplySettings();
    });
    auto vertical = Container::Vertical({autoplay_toggle,playmode_toggle,sample_rate_toggle,chanels_toggle,button});
    return Renderer(vertical,[=](){
            return vbox({
                hbox({text("AutoPlay"),filler(),separator(),autoplay_toggle->Render()}) | border,
                hbox({text("Playmode"),filler(),separator(),playmode_toggle->Render()}) | border,
                hbox({text("Sample Rate"),filler(),separator(),sample_rate_toggle->Render()})| border,
                hbox({text("Chanels"),filler(),separator(),chanels_toggle->Render()})| border,
                button->Render()
            });

    });
}

ftxui::Component ScreenUI::BuildPlayerTab()
{
    using namespace ftxui;
    auto slider = Slider("🔊",&Volume,0,100,1);
    auto ctch = CatchEvent(slider,[this, last_volume = Volume](Event e) mutable {
        
        if (Volume != last_volume) {
            AudioPlayer.SetVolume(Volume); 
            last_volume = Volume;          
        }
        return false;
    });
    auto next_music_button = Button(" ⏭ ",[this](){
        NextMusic();
    });
    auto prev_music_button = Button(" ⏮ ",[this](){
        PrevMusic();
    });
    auto next_fragment_button = Button(" ⏩ ",[&AudioPlayer = AudioPlayer,&SkipTime = SkipTime](){
        AudioPlayer.AddTime(SkipTime);
    });
    auto prev_fragment_button = Button(" ⏪ ",[&AudioPlayer = AudioPlayer, &SkipTime = SkipTime](){
        AudioPlayer.AddTime(-SkipTime);
    });
    auto play_stop_button = Button(" ▶/⏸ ",[&AudioPlayer = AudioPlayer](){
        AudioPlayer.PlayStop();
    });
    auto play_box = Container::Vertical({
        ctch,
        Container::Horizontal({
            prev_music_button | flex  | center, prev_fragment_button | flex  | center, 
            play_stop_button | flex | center,
            next_fragment_button | flex | center,next_music_button | flex  | center
        }) | center
    });
    auto info_box = Renderer(play_box,[play_box,&AudioPlayer = AudioPlayer](){
        auto playtime = AudioPlayer.GetPlayTimeInSeconds(), length = AudioPlayer.GetLengthInSeconds();
        if(AudioPlayer.IsPlaying())
        {
            animation::RequestAnimationFrame();
        }
        return vbox({
        hbox({ gauge(playtime / (length + 1e-6)) | flex, text(std::format("{}:{}/{}:{}",(int)playtime / 60,(int)playtime % 60,(int)length / 60, (int)length % 60)) }),
        separatorLight(),
        play_box->Render()
    });
    });
    return info_box;
}


ftxui::Component ScreenUI::BuildListTab()
{
    using namespace ftxui;
    ftxui::MenuOption options = MenuOption::Vertical();
    static std::vector<std::string> files;
    static std::vector<bool> is_folder;
    options.on_enter = [this]()
    {
        namespace fs = std::filesystem;
        auto path = fs::path(CurrectDir);
        if(InTab1 == 0)
        {
            CurrectDir = path.parent_path().string();
        }
        else if(is_folder[InTab1])
        {
            CurrectDir = (path / files[InTab1]).string();
        }
        else
        {
            auto file_path = (path / files[InTab1]).string();
            if(IsValid(file_path))
            {
                Playlist.push_back(file_path);
                if(Playlist.size() == 1)
                {
                    auto init = AudioPlayer.InitSound(file_path,&ScreenUI::AudioCallback,this);
                    AudioPlayer.SetVolume(Volume);
                    if(init == false)
                        Playlist.pop_back();
                    if(AutoPlay == Yes)
                    {
                        if(init == true)
                        {
                            AudioPlayer.Play();
                        }
                    }
                }
                return;
            }
        }
        files.clear();
        is_folder.clear();
        files.push_back("...");
        is_folder.push_back(true);
        InTab1 = 0;
        ScreenUI::UpdateFilesInDir(CurrectDir,files,is_folder);
    };

    if(files.empty())
    {
        files.push_back("...");
        is_folder.push_back(true);
        ScreenUI::UpdateFilesInDir(CurrectDir,files,is_folder);
    }
    auto playlist_opts = options;
    playlist_opts.on_enter = [this](){
        DeleteMusicFromArray(InTab2);
    };
    playlist_opts.entries_option.transform = [&MusicIndex = MusicIndex](const EntryState& state)
    {
        auto label = state.label;
        if(state.active)
        {
            label = "> " + label;
        }
        else 
        {
            label = "  " + label;
        }
        if(state.index == MusicIndex)
        {
            label += " 🎵 ";
        }
        auto txt = text(label);
        return state.focused? txt | bgcolor(Color::Red): txt;
    };
    auto files_menu = Menu(&files, &InTab1,  options) | frame;
    auto playlist_menu = Menu(&Playlist, &InTab2,  playlist_opts) | frame;
    auto h_box_menus = Container::Horizontal({files_menu,playlist_menu});

    return Renderer(h_box_menus,[files_menu = files_menu,playlist_menu = playlist_menu](){
            return hbox({ 
                files_menu->Render(),
                separator(),
                playlist_menu->Render()

            });
    });
}

bool ScreenUI::IsValid(const std::string& Path)
{
    return std::filesystem::exists(Path);
}

void ScreenUI::AudioCallback(void *pUserData, ma_sound *pSound)
{
        auto screen = static_cast<ScreenUI*>(pUserData);
        ftxui::ScreenInteractive::Active()->Post([screen](){
            screen->NextMusic();
        });
}

void ScreenUI::NextMusic()
{
    if (Playlist.empty() ) return;
    if(PlayMode == LoopTrack)
    {
        AudioPlayer.SetTime(0);
        AudioPlayer.Play();
        return;
    }
    if(AddToMusicIndex(1))
    {
        auto init = AudioPlayer.InitSound(Playlist[MusicIndex],&ScreenUI::AudioCallback,this);
        if(!init)
        {
            DeleteMusicFromArray(MusicIndex);
        }
        AudioPlayer.SetVolume(Volume);
        AudioPlayer.Play();
        
    }
    else {
        AudioPlayer.DeinintAudio();
    }
    ftxui::ScreenInteractive::Active()->PostEvent(ftxui::Event::Custom);
}

void ScreenUI::PrevMusic()
{
    if(AudioPlayer.GetPlayTimeInSeconds() > 5)
    {
        AudioPlayer.SetTime(0);
        return;
    }
    if (Playlist.empty()) return;
    if(PlayMode == LoopTrack)
    {
        AudioPlayer.SetTime(0);
        AudioPlayer.Play();
        return;
    }
    if(AddToMusicIndex(-1))
    {
        auto init = AudioPlayer.InitSound(Playlist[MusicIndex],&ScreenUI::AudioCallback,this);
        if(!init)
        {
            DeleteMusicFromArray(MusicIndex);
        }
        AudioPlayer.SetVolume(Volume);
        AudioPlayer.Play();
        
    }
    else {
        AudioPlayer.DeinintAudio();
    }
    ftxui::ScreenInteractive::Active()->PostEvent(ftxui::Event::Custom);
}

void ScreenUI::DeleteMusicFromArray(int Index)
{
    if(Playlist.empty()) return;
    if(Index < 0 || Index >= (int)Playlist.size()) return;
    Playlist.erase(Playlist.begin() + Index);
    if (Playlist.empty())
    {
        MusicIndex = 0;
        AudioPlayer.Stop();
        return; 
    }
    if(MusicIndex > Index)
    {

        MusicIndex = std::clamp(MusicIndex -1, 0, (int)Playlist.size() -1 );
        return;
    }
    if(MusicIndex == Index)
    {
        if(MusicIndex >= Playlist.size())
            MusicIndex = Playlist.size() - 1;
        if(MusicIndex >= 0 && MusicIndex < Playlist.size())
        {
        bool is_playing = AudioPlayer.IsPlaying();
        if(AudioPlayer.InitSound(Playlist[MusicIndex],&ScreenUI::AudioCallback,this) && is_playing)
            AudioPlayer.Play();
        }
    }
}

bool ScreenUI::AddToMusicIndex(int Add)
{
    switch (PlayMode) {
        case LoopPlaylist:
            MusicIndex += Add;
            if(MusicIndex >= Playlist.size())
            {
                MusicIndex = 0;
            }
            if(MusicIndex < 0)
            {
                MusicIndex =  Playlist.size() - 1;
            }
            return true;
        break;
        case LoopTrack:
            return true;
        break;
        case Shuffle:
        {
            if(Playlist.empty()) return false;
            auto random = std::rand() % Playlist.size();
            MusicIndex = random;
            return true;
        }
        break;
        case Normal:
            MusicIndex += Add;
            if(MusicIndex >= (int)Playlist.size())
            {
                MusicIndex = std::clamp(MusicIndex, 0, (int)Playlist.size());
                return false;
            }
            MusicIndex = std::clamp(MusicIndex, 0, (int)Playlist.size());
            return true;
        break;
    }
    return false;
}

void ScreenUI::UpdateFilesInDir(std::string_view Dir,std::vector<std::string>& FilesOut,std::vector<bool>& IsFolderOut)
{
    std::error_code ec;
    for(auto i : std::filesystem::directory_iterator(Dir,ec))
    {
        if(ec) continue;
        FilesOut.push_back(i.path().filename().string());
        IsFolderOut.push_back(i.is_directory(ec));
    }
}

void ScreenUI::ApplySettings()
{
    int sample_rate = 0;
    switch (SampleRate) {
        case R8000:
        sample_rate = 8000;
        break;
        case R16000:
        sample_rate = 16000;
        break;
        case R22050:
        sample_rate = 22050;
        break;
        case R32000:
        sample_rate = 32000;
        break;
        case R44100:
        sample_rate = 44100;
        break;
        case R48000:
        sample_rate = 48000;
        break;
    }
    AudioPlayer.Configure(Channels, sample_rate);
    float time = AudioPlayer.GetPlayTimeInSeconds();
    bool is_playing = AudioPlayer.IsPlaying();
    AudioPlayer.ReinitEngine();
    if(!Playlist.empty() && MusicIndex >= 0 && MusicIndex < Playlist.size())
    {
        AudioPlayer.InitSound(Playlist[MusicIndex],&ScreenUI::AudioCallback,this);
        AudioPlayer.SetVolume(Volume);
        if(is_playing)
            AudioPlayer.Play();
        AudioPlayer.SetTime(time);

    }

    
}

