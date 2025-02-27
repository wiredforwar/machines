/*
 * C T X S T A T S . C P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

//  Definitions of non-inline non-template methods and global functions

#include "machgui/ctxstats.hpp"
#include <string.h>
#include "system/pathname.hpp"
#include "device/time.hpp"
#include "device/cd.hpp"
#include "render/device.hpp"
#include "render/display.hpp"
#include "ani/AniSmacker.hpp"
#include "ani/AniSmackerRegular.hpp"
#include "gui/gui.hpp"
#include "gui/font.hpp"
#include "gui/restring.hpp"
#include "machphys/machphys.hpp"
#include "machlog/score.hpp"
#include "machlog/races.hpp"
#include "machgui/ui/MenuButton.hpp"
#include "machgui/startup.hpp"
#include "machgui/strtdata.hpp"
#include "machgui/internal/strings.hpp"
#include "machgui/statistics.hpp"
#include "machgui/statdisp.hpp"
#include "machgui/genstats.hpp"
#include "machgui/ui/MenuStyle.hpp"
#include "machgui/ui/MenuText.hpp"
#include "machgui/database.hpp"
#include "machgui/dbplayer.hpp"
#include "machgui/gui.hpp"
#include "machgui/internal/mgsndman.hpp"
#include "machgui/internal/playeri.hpp"
#include "machgui/internal/playscri.hpp"

MachGuiCtxStatistics::MachGuiCtxStatistics(MachGuiStartupScreens* pStartupScreens)
    : MachGui::GameMenuContext("sq", pStartupScreens, LoadAnimations::No)
    , delayBarsGrowth_(2)
{
    MachGuiMenuButton* pOkBtn = new MachGuiMenuButton(
        pStartupScreens,
        pStartupScreens,
        Gui::Box(425, 406, 576, 439) * MachGui::menuScaleFactor(),
        IDS_MENUBTN_OK,
        MachGui::ButtonEvent::EXIT);
    pOkBtn->escapeControl(true);

    changeBackdrop("gui/menu/sq.bmp");

    pStartupScreens->cursorOn(true);
    pStartupScreens->desiredCdTrack(MachGuiStartupScreens::MENU_MUSIC);

    SysPathName statAnim("flics/gui/statist.smk");

    // Get flic off hard-disk or CD-Rom
    if (! statAnim.existsAsFile())
    {
        // Make sure the cd is stopped before accessing files on it.
        if (DevCD::instance().isPlayingAudioCd())
        {
            DevCD::instance().stopPlaying();
        }

        std::string cdRomDrive;

        if (MachGui::getCDRomDriveContainingFile(cdRomDrive, "flics/gui/statist.smk"))
        {
            statAnim = SysPathName(cdRomDrive + "flics/gui/statist.smk");

            // Can't play music and smacker anim off CD at same time
            if (statAnim.existsAsFile())
            {
                pStartupScreens_->desiredCdTrack(MachGuiStartupScreens::DONT_PLAY_CD);
            }
        }
    }

    if (statAnim.existsAsFile())
    {
        // Construct a smacker player
        const auto& topLeft = getBackdropTopLeft();
        const Gui::Coord animationCoord = Gui::Coord(55, 142) * MachGui::menuScaleFactor();
        AniSmacker* pSmackerAnimation
            = new AniSmackerRegular(statAnim, animationCoord.x() + topLeft.second, animationCoord.y() + topLeft.first);
        pSmackerAnimation->setScaleFactor(MachGui::menuScaleFactor());
        pStartupScreens->addSmackerAnimation(pSmackerAnimation);
    }

    // Heading texts
    GuiBmpFont font = GuiBmpFont::getFont(SysPathName(MachGui::Menu::smallFontWhite()));
    GuiResourceString generalHeading(IDS_MENU_STSGENERAL);

    new MachGuiMenuText(
        pStartupScreens,
        Gui::Box(
            Gui::Coord(196, 41) * MachGui::menuScaleFactor(),
            font.textWidth(generalHeading.asString()),
            font.charHeight() + 2),
        IDS_MENU_STSGENERAL,
        MachGui::Menu::smallFontWhite(),
        Gui::AlignLeft);

    GuiResourceString BARHeading(IDS_MENU_STSBAR);

    new MachGuiMenuText(
        pStartupScreens,
        Gui::Box(
            Gui::Coord(196, 132) * MachGui::menuScaleFactor(),
            font.textWidth(BARHeading.asString()),
            font.charHeight() + 2),
        IDS_MENU_STSBAR,
        MachGui::Menu::smallFontWhite(),
        Gui::AlignLeft);

    GuiResourceString killsHeading(IDS_MENU_STSKILLS);

    new MachGuiMenuText(
        pStartupScreens,
        Gui::Box(
            Gui::Coord(196, 232) * MachGui::menuScaleFactor(),
            font.textWidth(killsHeading.asString()),
            font.charHeight() + 2),
        IDS_MENU_STSKILLS,
        MachGui::Menu::smallFontWhite(),
        Gui::AlignLeft);

    GuiBmpFont largeFont = GuiBmpFont::getFont(SysPathName(MachGui::Menu::largeFontLight()));

    GuiResourceString statsHeading(IDS_MENU_STATISTICS);

    new MachGuiMenuText(
        pStartupScreens,
        Gui::Box(
            Gui::Coord(53, 31) * MachGui::menuScaleFactor(),
            largeFont.textWidth(statsHeading.asString()),
            largeFont.charHeight()),
        IDS_MENU_STATISTICS,
        MachGui::Menu::largeFontLight(),
        Gui::AlignLeft);

    // Get handle to score object for each race
    MachLogRaces::instance().removeNotDisplayedScores();
    const MachLogScore& redScore = MachLogRaces::instance().score(MachPhys::RED);
    const MachLogScore& blueScore = MachLogRaces::instance().score(MachPhys::BLUE);
    const MachLogScore& greenScore = MachLogRaces::instance().score(MachPhys::GREEN);
    const MachLogScore& yellowScore = MachLogRaces::instance().score(MachPhys::YELLOW);

    // Determine type of game
    const MachGuiStartupScreens::Context context = pStartupScreens->currentContext();
    std::string redName, blueName, greenName, yellowName;
    redName = blueName = greenName = yellowName = GuiResourceString(IDS_MENU_STSNA).asString();

    MachPhys::Race playerRace;
    std::string playerName;

    if ((context == MachGuiStartupScreens::CTX_CASTATISTICS) || (context == MachGuiStartupScreens::CTX_SKSTATISTICS))
    {
        playerRace = MachLogRaces::instance().playerRace();
        if (context == MachGuiStartupScreens::CTX_CASTATISTICS)
        {
            playerName = MachGuiDatabase::instance().currentPlayer().name();
        }
        else
        {
            playerName = GuiResourceString(IDS_MENU_STSPLAYER).asString();
        }

        for (MachPhys::Race currentRace : MachPhys::AllRaces)
        {
            // Display string 'Computer' for every player in the game who is not user controlled
            if (MachLogRaces::instance().raceInGame(currentRace)
                && (currentRace != MachLogRaces::instance().playerRace())
                && MachLogRaces::instance().score(currentRace).scoreShouldBeDisplayed())
            {
                std::string computerString = GuiResourceString(IDS_MENU_STSCOMPUTER).asString();

                switch (currentRace)
                {
                    case MachPhys::RED:
                        redName = computerString;
                        break;
                    case MachPhys::BLUE:
                        blueName = computerString;
                        break;
                    case MachPhys::GREEN:
                        greenName = computerString;
                        break;
                    case MachPhys::YELLOW:
                        yellowName = computerString;
                        break;
                    case MachPhys::N_RACES:
                    case MachPhys::NORACE:
                        // Invalid
                        break;
                }
            }
        }
        switch (playerRace)
        {
            // Assign player to appropriate colour
            case MachPhys::RED:
                redName = playerName;
                break;
            case MachPhys::BLUE:
                blueName = playerName;
                break;
            case MachPhys::GREEN:
                greenName = playerName;
                break;
            case MachPhys::YELLOW:
                yellowName = playerName;
                break;
            case MachPhys::N_RACES:
            case MachPhys::NORACE:
                // Invalid
                break;
        }
    }
    else // Multi player mode
    {
        const MachGuiStartupData::PlayerInfo* playerInfo = pStartupScreens->startupData()->players();
        for (uint i = 0; i < 4; ++i)
        {
            std::string* name = nullptr;
            switch (playerInfo[i].race_)
            {
                case MachPhys::RED:
                    name = &redName;
                    break;
                case MachPhys::BLUE:
                    name = &blueName;
                    break;
                case MachPhys::GREEN:
                    name = &greenName;
                    break;
                case MachPhys::YELLOW:
                    name = &yellowName;
                    break;
                case MachPhys::N_RACES:
                case MachPhys::NORACE:
                    // Invalid
                    break;
            }

            if (playerInfo[i].status_ == MachGuiStartupData::PlayerInfo::HUMAN
                || playerInfo[i].status_ == MachGuiStartupData::PlayerInfo::CONNECTION_LOST)
            {
                *name = playerInfo[i].playerName_;
            }
            else if (MachLogRaces::instance().raceInGame(
                         playerInfo[i].race_)) // race in game but not player therefore computer
            {
                *name = GuiResourceString(IDS_MENU_STSCOMPUTER).asString();
            }
            // Assert??
        }
    }
    //
    // General statistics
    //
    // Machine kills
    const int FirstRowY = 50;
    int RowY = FirstRowY;
    int TableContentX = 196;
    MachGuiGeneralStatistics* machineKills = new MachGuiGeneralStatistics(
        pStartupScreens,
        Gui::Coord(TableContentX, RowY) * MachGui::menuScaleFactor(),
        IDS_MENU_MACHINEKILLS,
        MachPhys::N_RACES);
    uint width1 = machineKills->width() - 1 * MachGui::menuScaleFactor();
    statistics_.push_back(machineKills);
    statistics_[0]->setStatistics(
        redScore.otherMachinesDestroyed(),
        blueScore.otherMachinesDestroyed(),
        greenScore.otherMachinesDestroyed(),
        yellowScore.otherMachinesDestroyed());
    // Construction Kills
    statistics_.push_back(new MachGuiGeneralStatistics(
        pStartupScreens,
        Gui::Coord(TableContentX, RowY) * MachGui::menuScaleFactor() + Gui::Coord(width1 * 1, 0),
        IDS_MENU_CONSTRUCTKILLS,
        MachPhys::N_RACES));
    statistics_[1]->setStatistics(
        redScore.otherConstructionsDestroyed(),
        blueScore.otherConstructionsDestroyed(),
        greenScore.otherConstructionsDestroyed(),
        yellowScore.otherConstructionsDestroyed());
    // Machine Losses
    statistics_.push_back(new MachGuiGeneralStatistics(
        pStartupScreens,
        Gui::Coord(TableContentX, RowY) * MachGui::menuScaleFactor() + Gui::Coord(width1 * 2, 0),
        IDS_MENU_MACHINELOSSES,
        MachPhys::N_RACES));
    statistics_[2]->setStatistics(
        redScore.myMachinesDestroyed(),
        blueScore.myMachinesDestroyed(),
        greenScore.myMachinesDestroyed(),
        yellowScore.myMachinesDestroyed());
    // Construction Losses
    statistics_.push_back(new MachGuiGeneralStatistics(
        pStartupScreens,
        Gui::Coord(TableContentX, RowY) * MachGui::menuScaleFactor() + Gui::Coord(width1 * 3, 0),
        IDS_MENU_CONSTRUCTLOSSES,
        MachPhys::N_RACES));
    statistics_[3]->setStatistics(
        redScore.myConstructionsDestroyed(),
        blueScore.myConstructionsDestroyed(),
        greenScore.myConstructionsDestroyed(),
        yellowScore.myConstructionsDestroyed());
    //
    // Building and Research
    //
    // BMU's Mined
    statistics_.push_back(new MachGuiGeneralStatistics(
        pStartupScreens,
        Gui::Coord(TableContentX, RowY) * MachGui::menuScaleFactor() + Gui::Coord(width1 * 4, 0),
        IDS_MENU_BMUSMINED,
        MachPhys::N_RACES));
    statistics_[4]
        ->setStatistics(redScore.BMUsMined(), blueScore.BMUsMined(), greenScore.BMUsMined(), yellowScore.BMUsMined());
    // Machines Built
    RowY = 141; // was 50, +91

    statistics_.push_back(new MachGuiGeneralStatistics(
        pStartupScreens,
        Gui::Coord(TableContentX, RowY) * MachGui::menuScaleFactor(),
        IDS_MENU_MACHINESBUILT,
        MachPhys::N_RACES));
    statistics_[5]->setStatistics(
        redScore.machinesBuilt(),
        blueScore.machinesBuilt(),
        greenScore.machinesBuilt(),
        yellowScore.machinesBuilt());
    // Military Machines Built
    statistics_.push_back(new MachGuiGeneralStatistics(
        pStartupScreens,
        Gui::Coord(TableContentX, RowY) * MachGui::menuScaleFactor() + Gui::Coord(width1 * 1, 0),
        IDS_MENU_MLTMACHINESBUILT,
        MachPhys::N_RACES));
    statistics_[6]->setStatistics(
        redScore.militaryMachinesBuilt(),
        blueScore.militaryMachinesBuilt(),
        greenScore.militaryMachinesBuilt(),
        yellowScore.militaryMachinesBuilt());
    // Constructions Built
    statistics_.push_back(new MachGuiGeneralStatistics(
        pStartupScreens,
        Gui::Coord(TableContentX, RowY) * MachGui::menuScaleFactor() + Gui::Coord(width1 * 2, 0),
        IDS_MENU_CONSTRUCTSBUILT,
        MachPhys::N_RACES));
    statistics_[7]->setStatistics(
        redScore.constructionsBuilt(),
        blueScore.constructionsBuilt(),
        greenScore.constructionsBuilt(),
        yellowScore.constructionsBuilt());
    // Items Researched
    statistics_.push_back(new MachGuiGeneralStatistics(
        pStartupScreens,
        Gui::Coord(TableContentX, RowY) * MachGui::menuScaleFactor() + Gui::Coord(width1 * 3, 0),
        IDS_MENU_ITEMSRESEARCHED,
        MachPhys::N_RACES));
    statistics_[8]->setStatistics(
        redScore.itemsResearched(),
        blueScore.itemsResearched(),
        greenScore.itemsResearched(),
        yellowScore.itemsResearched());
    // Researce points
    statistics_.push_back(new MachGuiGeneralStatistics(
        pStartupScreens,
        Gui::Coord(TableContentX, RowY) * MachGui::menuScaleFactor() + Gui::Coord(width1 * 4, 0),
        IDS_MENU_RESEARCHPOINTS,
        MachPhys::N_RACES));
    statistics_[9]->setStatistics(
        redScore.totalResearchCost(),
        blueScore.totalResearchCost(),
        greenScore.totalResearchCost(),
        yellowScore.totalResearchCost());
    //
    // Kills
    RowY = 241;
    //
    // Player name list
    MachGuiPlayerNameList* names1 = new MachGuiPlayerNameList(
        pStartupScreens,
        Gui::Coord(TableContentX, RowY) * MachGui::menuScaleFactor(),
        MachGui::Menu::smallFontLight(),
        IDS_MENU_STSTPLAYER);
    names1->names(redName, blueName, greenName, yellowName);
    // Destroyed Machines belonging to player 1
    statistics_.push_back(new MachGuiGeneralStatistics(
        pStartupScreens,
        Gui::Coord(TableContentX, RowY) * MachGui::menuScaleFactor() + Gui::Coord(width1 * 1, 0),
        IDS_MENU_DESTROYEDMACH,
        MachPhys::RED,
        redName));
    statistics_[10]->setStatistics(
        redScore.raceMachinesDestroyed(MachPhys::RED),
        blueScore.raceMachinesDestroyed(MachPhys::RED),
        greenScore.raceMachinesDestroyed(MachPhys::RED),
        yellowScore.raceMachinesDestroyed(MachPhys::RED));
    // Destroyed Machines belonging to player 2
    statistics_.push_back(new MachGuiGeneralStatistics(
        pStartupScreens,
        Gui::Coord(TableContentX, RowY) * MachGui::menuScaleFactor() + Gui::Coord(width1 * 2, 0),
        IDS_MENU_DESTROYEDMACH,
        MachPhys::BLUE,
        blueName));
    statistics_[11]->setStatistics(
        redScore.raceMachinesDestroyed(MachPhys::BLUE),
        blueScore.raceMachinesDestroyed(MachPhys::BLUE),
        greenScore.raceMachinesDestroyed(MachPhys::BLUE),
        yellowScore.raceMachinesDestroyed(MachPhys::BLUE));
    // Destroyed Machines belonging to player 3
    statistics_.push_back(new MachGuiGeneralStatistics(
        pStartupScreens,
        Gui::Coord(TableContentX, RowY) * MachGui::menuScaleFactor() + Gui::Coord(width1 * 3, 0),
        IDS_MENU_DESTROYEDMACH,
        MachPhys::GREEN,
        greenName));
    statistics_[12]->setStatistics(
        redScore.raceMachinesDestroyed(MachPhys::GREEN),
        blueScore.raceMachinesDestroyed(MachPhys::GREEN),
        greenScore.raceMachinesDestroyed(MachPhys::GREEN),
        yellowScore.raceMachinesDestroyed(MachPhys::GREEN));
    // Destroyed Machines belonging to player 4
    statistics_.push_back(new MachGuiGeneralStatistics(
        pStartupScreens,
        Gui::Coord(TableContentX, RowY) * MachGui::menuScaleFactor() + Gui::Coord(width1 * 4, 0),
        IDS_MENU_DESTROYEDMACH,
        MachPhys::YELLOW,
        yellowName));
    statistics_[13]->setStatistics(
        redScore.raceMachinesDestroyed(MachPhys::YELLOW),
        blueScore.raceMachinesDestroyed(MachPhys::YELLOW),
        greenScore.raceMachinesDestroyed(MachPhys::YELLOW),
        yellowScore.raceMachinesDestroyed(MachPhys::YELLOW));
    // Player name list
    RowY = 322;
    MachGuiPlayerNameList* names2 = new MachGuiPlayerNameList(
        pStartupScreens,
        Gui::Coord(TableContentX, RowY) * MachGui::menuScaleFactor(),
        MachGui::Menu::smallFontLight(),
        IDS_MENU_STSTPLAYER);
    names2->names(redName, blueName, greenName, yellowName);
    // Destroyed constructions belonging to player 1
    statistics_.push_back(new MachGuiGeneralStatistics(
        pStartupScreens,
        Gui::Coord(TableContentX, RowY) * MachGui::menuScaleFactor() + Gui::Coord(width1 * 1, 0),
        IDS_MENU_DESTROYEDCONST,
        MachPhys::RED,
        redName));
    statistics_[14]->setStatistics(
        redScore.raceConstructionsDestroyed(MachPhys::RED),
        blueScore.raceConstructionsDestroyed(MachPhys::RED),
        greenScore.raceConstructionsDestroyed(MachPhys::RED),
        yellowScore.raceConstructionsDestroyed(MachPhys::RED));
    // Destroyed constructions belonging to player 2
    statistics_.push_back(new MachGuiGeneralStatistics(
        pStartupScreens,
        Gui::Coord(TableContentX, RowY) * MachGui::menuScaleFactor() + Gui::Coord(width1 * 2, 0),
        IDS_MENU_DESTROYEDCONST,
        MachPhys::BLUE,
        blueName));
    statistics_[15]->setStatistics(
        redScore.raceConstructionsDestroyed(MachPhys::BLUE),
        blueScore.raceConstructionsDestroyed(MachPhys::BLUE),
        greenScore.raceConstructionsDestroyed(MachPhys::BLUE),
        yellowScore.raceConstructionsDestroyed(MachPhys::BLUE));
    // Destroyed constructions belonging to player 3
    statistics_.push_back(new MachGuiGeneralStatistics(
        pStartupScreens,
        Gui::Coord(TableContentX, RowY) * MachGui::menuScaleFactor() + Gui::Coord(width1 * 3, 0),
        IDS_MENU_DESTROYEDCONST,
        MachPhys::GREEN,
        greenName));
    statistics_[16]->setStatistics(
        redScore.raceConstructionsDestroyed(MachPhys::GREEN),
        blueScore.raceConstructionsDestroyed(MachPhys::GREEN),
        greenScore.raceConstructionsDestroyed(MachPhys::GREEN),
        yellowScore.raceConstructionsDestroyed(MachPhys::GREEN));
    // Destroyed constructions belonging to player 4
    statistics_.push_back(new MachGuiGeneralStatistics(
        pStartupScreens,
        Gui::Coord(TableContentX, RowY) * MachGui::menuScaleFactor() + Gui::Coord(width1 * 4, 0),
        IDS_MENU_DESTROYEDCONST,
        MachPhys::YELLOW,
        yellowName));
    statistics_[17]->setStatistics(
        redScore.raceConstructionsDestroyed(MachPhys::YELLOW),
        blueScore.raceConstructionsDestroyed(MachPhys::YELLOW),
        greenScore.raceConstructionsDestroyed(MachPhys::YELLOW),
        yellowScore.raceConstructionsDestroyed(MachPhys::YELLOW));
    //
    // Overall Scores
    //
    MachGuiPlayerScore* names3 = new MachGuiPlayerScore(pStartupScreens, Gui::Coord(53, FirstRowY) * MachGui::menuScaleFactor());
    names3->names(redName, blueName, greenName, yellowName);
    statistics_.push_back(names3);
    statistics_[18]->setStatistics(
        redScore.grossScore(),
        blueScore.grossScore(),
        greenScore.grossScore(),
        yellowScore.grossScore());

    // Set starting time to coordinate stat display updates
    MachGuiStatisticsDisplay::setStartTime(DevTime::instance().time());

    TEST_INVARIANT;
}

MachGuiCtxStatistics::~MachGuiCtxStatistics()
{
    TEST_INVARIANT;
}

// virtual
void MachGuiCtxStatistics::update()
{
    if (delayBarsGrowth_ == 2)
    {
        // Set starting time to coordinate stat display updates
        MachGuiStatisticsDisplay::setStartTime(DevTime::instance().time());
        --delayBarsGrowth_;
    }
    else if (delayBarsGrowth_ == 1)
    {
        // Set starting time to coordinate stat display updates
        MachGuiStatisticsDisplay::setStartTime(DevTime::instance().time());
        MachGuiSoundManager::instance().playSound("gui/sounds/stats.wav");
        --delayBarsGrowth_;
    }

    // Set new time for stats displays to calculate amount of each bar to display
    MachGuiStatisticsDisplay::setTime(DevTime::instance().time());

    for (Statistics::iterator i = statistics_.begin(); i != statistics_.end(); i++)
    {
        (*i)->update();
    }
}

void MachGuiCtxStatistics::CLASS_INVARIANT
{
    INVARIANT(this != nullptr);
}

std::ostream& operator<<(std::ostream& o, const MachGuiCtxStatistics& t)
{

    o << "MachGuiCtxStatistics " << static_cast<const void*>(&t) << " start" << std::endl;
    o << "MachGuiCtxStatistics " << static_cast<const void*>(&t) << " end" << std::endl;

    return o;
}

/* End CTXSTATS.CPP *************************************************/
