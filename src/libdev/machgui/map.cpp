/*
 * M A P . C P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

//  Definitions of non-inline non-template methods and global functions

#include "stdlib/string.hpp"
#include "ctl/list.hpp"
#include "mathex/transf3d.hpp"
#include "mathex/line3d.hpp"
#include "device/time.hpp"
#include "system/pathname.hpp"
#include "gui/painter.hpp"
#include "gui/event.hpp"
#include "gui/restring.hpp"
#include "render/device.hpp"
#include "render/camera.hpp"
#include "world4d/manager.hpp"
#include "world4d/scenemgr.hpp"
#include "world4d/soundman.hpp"
#include "machlog/planet.hpp"
#include "machlog/actor.hpp"
#include "machlog/races.hpp"
#include "machlog/camera.hpp"
#include "machlog/machine.hpp"
#include "machlog/constron.hpp"
#include "machphys/plansurf.hpp"
#include "machphys/machdata.hpp"
#include "machgui/worldvie.hpp"
#include "machgui/map.hpp"
#include "machgui/ingame.hpp"
#include "machgui/bmutext.hpp"
#include "machgui/cameras.hpp"
#include "machgui/commands/command.hpp"
#include "machgui/internal/mgsndman.hpp"
#include "machgui/internal/strings.hpp"
#include "machgui/internal/mgsndman.hpp"
#include "sim/manager.hpp"

using SysPathNames = std::pair<SysPathName, SysPathName>;

const size_t NUM_FRAMES = 12;

const size_t SCANNER_UPPERLIMIT[5] = { 20, 50, 90, 130, 10000 };

const size_t SCANNER_ACTUALSIZE[5] = { 20, 40, 80, 130, 200 };

const size_t BEENHERE_ARRAYWIDTH = 70;
const size_t BEENHERE_ARRAYHEIGHT = 70;

class MachGuiTerrainOnOffButton : public GuiBitmapButtonWithFilledBorder
{
public:
    MachGuiTerrainOnOffButton(
        GuiDisplayable* pParent,
        const Gui::Coord& rel,
        MachContinentMap* pMap,
        MachInGameScreen* pInGameScreen)
        : GuiBitmapButtonWithFilledBorder(
            pParent,
            rel,
            GuiBorderMetrics(1, 1, 1),
            GuiFilledBorderColours(MachGui::VERYDARKGREY(), Gui::LIGHTGREY(), Gui::DARKGREY(), Gui::GREEN()),
            MachGui::getScaledImage("gui/map/mapt.bmp"),
            Gui::Coord(1, 1))
        , pMap_(nullptr)
        , pInGameScreen_(pInGameScreen)
    {
        popupButton(false);
        setDepressed(true);
        pMap_ = pMap; // Must come after line above
    }

    bool mapOn() { return isDepressed(); }

    static size_t reqWidth() { return 14 * Gui::uiScaleFactor(); }

    static size_t reqHeight() { return 11 * Gui::uiScaleFactor(); }

protected:
    void doBeDepressed(const GuiMouseEvent&) override
    {
        MachGuiSoundManager::instance().playSound("gui/sounds/igclick.wav");

        if (pMap_)
            pMap_->forceUpdate();
    }
    void doBeReleased(const GuiMouseEvent&) override
    {
        MachGuiSoundManager::instance().playSound("gui/sounds/igclick.wav");

        if (pMap_)
            pMap_->forceUpdate();
    }
    void doHandleMouseEnterEvent(const GuiMouseEvent& mouseEvent) override
    {
        GuiBitmapButtonWithFilledBorder::doHandleMouseEnterEvent(mouseEvent);

        // Load the resource string
        GuiResourceString prompt(IDS_TERRAINONOFF);

        // Set the cursor prompt
        pInGameScreen_->setCursorPromptText(prompt.asString());
    }
    void doHandleMouseExitEvent(const GuiMouseEvent& mouseEvent) override
    {
        // Clear the cursor prompt string
        pInGameScreen_->clearCursorPromptText();

        GuiBitmapButtonWithFilledBorder::doHandleMouseExitEvent(mouseEvent);
    }

    MachContinentMap* pMap_;
    MachInGameScreen* pInGameScreen_;
};

class MachGuiMapModeButton : public GuiBitmapButtonWithFilledBorder
{
public:
    MachGuiMapModeButton(
        GuiDisplayable* pParent,
        const Gui::Coord& rel,
        MachContinentMap* pMap,
        MachInGameScreen* pInGameScreen)
        : GuiBitmapButtonWithFilledBorder(
            pParent,
            rel,
            GuiBorderMetrics(1, 1, 1),
            GuiFilledBorderColours(MachGui::VERYDARKGREY(), Gui::LIGHTGREY(), Gui::DARKGREY(), Gui::GREEN()),
            MachGui::getScaledImage("gui/map/mapm.bmp"),
            Gui::Coord(1, 1))
        , pMap_(nullptr)
        , pInGameScreen_(pInGameScreen)
    {
        pMap_ = pMap;
    }

    static size_t reqWidth() { return 14 * Gui::uiScaleFactor(); }

    static size_t reqHeight() { return 11 * Gui::uiScaleFactor(); }

protected:
    void doBeDepressed(const GuiMouseEvent&) override { }

    void doBeReleased(const GuiMouseEvent&) override
    {
        if (pMap_)
        {
            switch (pMap_->mapMode())
            {
                case MachContinentMap::UNITS_ONLY:
                    pMap_->mapMode(MachContinentMap::RESOURCES_ONLY);
                    break;
                case MachContinentMap::RESOURCES_ONLY:
                    pMap_->mapMode(MachContinentMap::ALL);
                    break;
                case MachContinentMap::ALL:
                    pMap_->mapMode(MachContinentMap::UNITS_ONLY);
                    break;
                    DEFAULT_ASSERT_BAD_CASE(pMap_->mapMode());
            }
        }

        updatePromptText();
    }
    void updatePromptText()
    {
        PRE(pMap_);

        std::string promptStr;

        switch (pMap_->mapMode())
        {
            case MachContinentMap::UNITS_ONLY:
                {
                    GuiResourceString prompt(IDS_MAPMODERESOURCE);
                    promptStr = prompt.asString();
                    break;
                }
            case MachContinentMap::RESOURCES_ONLY:
                {
                    GuiResourceString prompt(IDS_MAPMODEALL);
                    promptStr = prompt.asString();
                    break;
                }
            case MachContinentMap::ALL:
                {
                    GuiResourceString prompt(IDS_MAPMODEUNITS);
                    promptStr = prompt.asString();
                    break;
                }
                DEFAULT_ASSERT_BAD_CASE(pMap_->mapMode());
        }

        // Set the cursor prompt
        pInGameScreen_->setCursorPromptText(promptStr);
    }

    void doHandleMouseEnterEvent(const GuiMouseEvent& mouseEvent) override
    {
        GuiBitmapButtonWithFilledBorder::doHandleMouseEnterEvent(mouseEvent);

        updatePromptText();
    }
    void doHandleMouseExitEvent(const GuiMouseEvent& mouseEvent) override
    {
        // Clear the cursor prompt string
        pInGameScreen_->clearCursorPromptText();

        GuiBitmapButtonWithFilledBorder::doHandleMouseExitEvent(mouseEvent);
    }

    MachContinentMap* pMap_;
    MachInGameScreen* pInGameScreen_;
};

MachContinentMap::MachContinentMap(
    GuiDisplayable* pParent,
    const Gui::Coord& rel,
    MachCameras* pCameras,
    MachInGameScreen* pInGameScreen)
    : GuiDisplayable(
        pParent,
        Gui::translateBitmapDimensions(MachGui::getScaledImagePath("gui/map/rlogo.bmp"), rel),
        GuiDisplayable::LAYER2)
    , pCameras_(pCameras)
    , firstDraw_(false)
    , pInGameScreen_(pInGameScreen)
    , mapBackground_(logoImage(MachPhys::RED))
    , fogOfWarOn_(true)
    , mapMode_(UNITS_ONLY)
    , currentBeacon_(MachLog::NO_BEACON)
{
    actorPositions_.reserve(512);

    RenColour black(0, 0, 0);
    cameraPosImage_ = MachGui::getScaledImage("gui/map/campos.bmp");
    cameraPosImage_.colourKey(black); // Black is colourKey
    cameraPosImage_.enableColourKeying();

    // Switch on colour keying for map images
    for (MachPhys::Race race : MachPhys::AllRaces)
    {
        machineImage(race).colourKey(black);
        podImage(race).colourKey(black);
        selectedMachineImage(race).colourKey(black);
        selectedPodImage(race).colourKey(black);
        machineImage(race).enableColourKeying();
        podImage(race).enableColourKeying();
        selectedMachineImage(race).enableColourKeying();
        selectedPodImage(race).enableColourKeying();
    }

    // Turn on colour keying for various map images
    oreImage().colourKey(black);
    attackedMachineImage().colourKey(black);
    attackedPodImage().colourKey(black);
    selectedAttackedMachineImage().colourKey(black);
    selectedAttackedPodImage().colourKey(black);
    oreImage().enableColourKeying();
    attackedMachineImage().enableColourKeying();
    attackedPodImage().enableColourKeying();
    selectedAttackedMachineImage().enableColourKeying();
    selectedAttackedPodImage().enableColourKeying();

    pBmuText_ = new MachGuiBmuText(this, Gui::Coord(1, 1) * Gui::uiScaleFactor());

    pTerrainOnOffButton_ = new MachGuiTerrainOnOffButton(
        this,
        Gui::Coord(width() - MachGuiTerrainOnOffButton::reqWidth(), height() - MachGuiTerrainOnOffButton::reqHeight()),
        this,
        pInGameScreen_);
    pMapModeButton_ = new MachGuiMapModeButton(
        this,
        Gui::Coord(
            width() - MachGuiTerrainOnOffButton::reqWidth() - MachGuiMapModeButton::reqWidth()
                + 1 /* 1 pixel overlap */,
            height() - MachGuiMapModeButton::reqHeight()),
        this,
        pInGameScreen_);

    useFastSecondDisplay(false);
}

void MachContinentMap::loadGame(const std::string& planet)
{
    std::string mapPath = "models/planet/";
    mapPath += planet;
    std::string mapBmp = mapPath + "/map.bmp";

    ASSERT_FILE_EXISTS(mapBmp.c_str());

    // Store map directory.
    mapPath_ = mapBmp;

    DEBUG_STREAM(DIAG_NEIL, "Changing map to " << mapBmp << std::endl);

    if (Gui::uiScaleFactor() == 1)
    {
        mapBackground_ = Gui::bitmap(mapBmp);
    }
    else
    {
        GuiBitmap mapBitmap = Gui::bitmap(mapBmp);
        mapBackground_ = RenSurface::createAnonymousSurface(
            mapBitmap.width() * Gui::uiScaleFactor(),
            mapBitmap.height() * Gui::uiScaleFactor(),
            mapBitmap);
        mapBackground_.stretchBlit(mapBitmap);
    }
    mapFrameOne_ = RenSurface::createAnonymousSurface(mapBackground_.width(), mapBackground_.height(), mapBackground_);
    mapFrameTwo_ = RenSurface::createAnonymousSurface(mapFrameOne_.width(), mapFrameOne_.height(), mapFrameOne_);

    // Set up visible area. This is used for "fog of war"
    mapVisibleArea_
        = RenSurface::createAnonymousSurface(mapBackground_.width(), mapBackground_.height(), mapBackground_);
    // Initialise to nothing visible
    mapVisibleArea_.filledRectangle(
        RenSurface::Rect(0, 0, mapBackground_.width(), mapBackground_.height()),
        Gui::BLACK());
    // TODO: check this, not sure what exactly it does but creates a line of pixels without FOW on map
    //  mapVisibleArea_.hollowRectangle( RenSurface::Rect( 0, 0, mapBackground_.width() - 1, mapBackground_.height() - 1
    //  ), Gui::MAGENTA(), 1 );
    // Setup magenta (default) colour keying for visible area bitmap
    mapVisibleArea_.enableColourKeying();

    updateMapToWorldMetrics();

    // Unfog areas of Map which are unusable, e.g. if the Map is thin and long (not square)
    if (xOffset() != 0)
    {
        mapVisibleArea_.filledRectangle(RenSurface::Rect(0, 0, xOffset(), mapBackground_.height()), Gui::MAGENTA());
        mapVisibleArea_.filledRectangle(
            RenSurface::Rect(mapBackground_.width() - xOffset(), 0, xOffset(), mapBackground_.height()),
            Gui::MAGENTA());
    }
    else if (yOffset() != 0)
    {
        mapVisibleArea_.filledRectangle(RenSurface::Rect(0, 0, mapBackground_.width(), yOffset()), Gui::MAGENTA());
        mapVisibleArea_.filledRectangle(
            RenSurface::Rect(0, mapBackground_.height() - yOffset(), mapBackground_.width(), yOffset()),
            Gui::MAGENTA());
    }

    // Create different scanner ranges ( used for unfogging ( note : fog of war NOT atmospheric fog ) ).
    for (int i = 0; i < 5; ++i)
    {
        size_t scannerPixelDiameter = SCANNER_ACTUALSIZE[i] / mapToWorldRatio_;
        if (scannerPixelDiameter < 6)
            scannerPixelDiameter = 6;

        scannerRangeImage_[i]
            = RenSurface::createAnonymousSurface(scannerPixelDiameter, scannerPixelDiameter, mapBackground_);
        //      scannerRangeImage_[i].filledRectangle( RenSurface::Rect( 0, 0, scannerPixelDiameter,
        //      scannerPixelDiameter ), Gui::BLACK() );
        scannerRangeImage_[i].ellipse(
            RenSurface::Rect(0, 0, scannerPixelDiameter, scannerPixelDiameter),
            Gui::MAGENTA(),
            Gui::MAGENTA());

        // Setup black colour keying for the unfog bitmap.
        scannerRangeImage_[i].colourKey(RenColour(0, 0, 0));
        scannerRangeImage_[i].enableColourKeying();
        scannerRange_[i] = scannerRangeImage_[i].width() / 2.0;
    }

    playerRace_ = MachLogRaces::instance().playerRace();

    updateBeacon(true);

    // Sort out the level of FOW.
    fogOfWarLightLevel_ = 1.0;
    fogOfWarLevel_ = NO_FOW;
    noFastChangeInLightLevel_ = true;
    fogOfWarFrameTimer_ = DevTime::instance().time();

    forceUpdate();

    update3dFogOfWarLightLevel();
}

void MachContinentMap::unloadGame()
{
    pBeenHere_.clear();
}

void MachContinentMap::doDisplay()
{
    // doDisplay gets called twice in a row. We can use
    // this to produce simple flashing of colours ( e.g. when machine is selected )
    firstDraw_ = ! firstDraw_;

    /*  if ( firstDraw_ ) // This causes onmap units to blink
        GuiPainter::instance().blit( mapFrameOne_, absoluteCoord() );
    else*/
    GuiPainter::instance().blit(mapFrameTwo_, absoluteCoord());
}

// static
GuiBitmap& MachContinentMap::logoImage(MachPhys::Race race)
{
    static GuiBitmap logoPixel[MachPhys::N_RACES] = {
        MachGui::getScaledImage("gui/map/rlogo.bmp"),
        MachGui::getScaledImage("gui/map/blogo.bmp"),
        MachGui::getScaledImage("gui/map/glogo.bmp"),
        MachGui::getScaledImage("gui/map/ylogo.bmp"),
    };

    return logoPixel[race];
}

// static
GuiBitmap& MachContinentMap::machineImage(MachPhys::Race race)
{
    static GuiBitmap machinePixel[MachPhys::N_RACES] = {
        MachGui::getScaledImage("gui/map/rmachpix.bmp"),
        MachGui::getScaledImage("gui/map/bmachpix.bmp"),
        MachGui::getScaledImage("gui/map/gmachpix.bmp"),
        MachGui::getScaledImage("gui/map/ymachpix.bmp"),
    };

    return machinePixel[race];
}

// static
GuiBitmap& MachContinentMap::selectedMachineImage(MachPhys::Race race)
{
    static GuiBitmap machinePixel[MachPhys::N_RACES] = {
        MachGui::getScaledImage("gui/map/rmachpxs.bmp"),
        MachGui::getScaledImage("gui/map/bmachpxs.bmp"),
        MachGui::getScaledImage("gui/map/gmachpxs.bmp"),
        MachGui::getScaledImage("gui/map/ymachpxs.bmp"),
    };

    return machinePixel[race];
}

GuiBitmap& MachContinentMap::constructionImage(MachPhys::Race race)
{
    static GuiBitmap constructionPixel[MachPhys::N_RACES] = {
        MachGui::getScaledImage("gui/map/rconspix.bmp"),
        MachGui::getScaledImage("gui/map/bconspix.bmp"),
        MachGui::getScaledImage("gui/map/gconspix.bmp"),
        MachGui::getScaledImage("gui/map/yconspix.bmp"),
    };

    return constructionPixel[race];
}

GuiBitmap& MachContinentMap::selectedConstructionImage(MachPhys::Race race)
{
    static GuiBitmap constructionPixel[MachPhys::N_RACES] = {
        MachGui::getScaledImage("gui/map/rconspxs.bmp"),
        MachGui::getScaledImage("gui/map/bconspxs.bmp"),
        MachGui::getScaledImage("gui/map/gconspxs.bmp"),
        MachGui::getScaledImage("gui/map/yconspxs.bmp"),
    };

    return constructionPixel[race];
}

GuiBitmap& MachContinentMap::podImage(MachPhys::Race race)
{
    static GuiBitmap podPixel[MachPhys::N_RACES] = {
        MachGui::getScaledImage("gui/map/rpodpix.bmp"),
        MachGui::getScaledImage("gui/map/bpodpix.bmp"),
        MachGui::getScaledImage("gui/map/gpodpix.bmp"),
        MachGui::getScaledImage("gui/map/ypodpix.bmp"),
    };

    return podPixel[race];
}

GuiBitmap& MachContinentMap::selectedPodImage(MachPhys::Race race)
{
    static GuiBitmap podPixel[MachPhys::N_RACES] = {
        MachGui::getScaledImage("gui/map/rpodpxs.bmp"),
        MachGui::getScaledImage("gui/map/bpodpxs.bmp"),
        MachGui::getScaledImage("gui/map/gpodpxs.bmp"),
        MachGui::getScaledImage("gui/map/ypodpxs.bmp"),
    };

    return podPixel[race];
}

// static
GuiBitmap& MachContinentMap::debrisImage()
{
    static GuiBitmap debrisPixel = MachGui::getScaledImage("gui/map/debris.bmp");
    return debrisPixel;
}

// static
GuiBitmap& MachContinentMap::artifactImage()
{
    static GuiBitmap artifactPixel = MachGui::getScaledImage("gui/map/artifact.bmp");
    return artifactPixel;
}

// static
GuiBitmap& MachContinentMap::oreImage()
{
    static GuiBitmap orePixel = MachGui::getScaledImage("gui/map/ore.bmp");
    return orePixel;
}

void MachContinentMap::updateMapBackground()
{
    PRE(! pInGameScreen_->inFirstPerson());

    // Blit map
    if (pTerrainOnOffButton_->mapOn() || currentBeacon_ == MachLog::NO_BEACON)
    {
        mapFrameOne_.simpleBlit(mapBackground_);
    }
    else
    {
        // No map terrain
        mapFrameOne_.filledRectangle(RenSurface::Rect(0, 0, width() - 1, height() - 1), MachGui::VERYDARKGREY());
    }

    if (currentBeacon_ != MachLog::NO_BEACON)
    {
        // Draw black vertical or horizontal lines to show edge of playable area
        if (xOffset() != 0)
        {
            // Draw dark rectangles to show unused area of map
            mapFrameOne_.filledRectangle(RenSurface::Rect(0, 0, xOffset(), mapBackground_.height()), Gui::DARKGREY());
            mapFrameOne_.filledRectangle(
                RenSurface::Rect(mapBackground_.width() - xOffset(), 0, xOffset(), mapBackground_.height()),
                Gui::DARKGREY());

            // Draw lines to separate playable area from unplayable area of map
            GuiBitmap::Points points;
            points.reserve(2);

            points.push_back(MexPoint2d(xOffset(), 0));
            points.push_back(MexPoint2d(xOffset(), mapBackground_.height() - 1));
            mapFrameOne_.polyLine(points, Gui::BLACK(), 1);

            points.erase(points.begin(), points.end());

            points.push_back(MexPoint2d(mapBackground_.width() - xOffset(), 0));
            points.push_back(MexPoint2d(mapBackground_.width() - xOffset(), mapBackground_.height() - 1));
            mapFrameOne_.polyLine(points, Gui::BLACK(), 1);
        }
        else if (yOffset() != 0)
        {
            // Draw dark rectangles to show unused area of map
            mapFrameOne_.filledRectangle(RenSurface::Rect(0, 0, mapBackground_.width(), yOffset()), Gui::DARKGREY());
            mapFrameOne_.filledRectangle(
                RenSurface::Rect(0, mapBackground_.height() - yOffset(), mapBackground_.width(), yOffset()),
                Gui::DARKGREY());

            // Draw lines to separate playable area from unplayable area of map
            GuiBitmap::Points points;
            points.reserve(2);

            points.push_back(MexPoint2d(0, yOffset()));
            points.push_back(MexPoint2d(mapBackground_.width() - 1, yOffset()));
            mapFrameOne_.polyLine(points, Gui::BLACK(), 1);

            points.erase(points.begin(), points.end());

            points.push_back(MexPoint2d(0, mapBackground_.height() - yOffset()));
            points.push_back(MexPoint2d(mapBackground_.width() - 1, mapBackground_.height() - yOffset()));
            mapFrameOne_.polyLine(points, Gui::BLACK(), 1);
        }

        // Draw black rectangle surrounding map
        GuiBitmap::Points points;
        points.reserve(5);

        points.push_back(MexPoint2d(0, 0));
        points.push_back(MexPoint2d(0, mapFrameOne_.height() - 1));
        points.push_back(MexPoint2d(mapFrameOne_.width() - 1, mapFrameOne_.height() - 1));
        points.push_back(MexPoint2d(mapFrameOne_.width() - 1, 0));
        points.push_back(MexPoint2d(0, 0));
        mapFrameOne_.polyLine(points, Gui::BLACK(), 1);
    }
}

void MachContinentMap::updateMapBackground2()
{
    PRE(! pInGameScreen_->inFirstPerson());

    mapFrameTwo_.simpleBlit(mapFrameOne_);
}

void MachContinentMap::updateMapFrameOne(size_t loop)
{
    PRE(! pInGameScreen_->inFirstPerson());
    PRE(! oreImage().empty());
    PRE(! debrisImage().empty());
    PRE(! artifactImage().empty());
    PRE(! mapFrameOne_.empty());

    // Prepare actor for drawing on second frame
    if (actorPositions_[loop].actorState_ & ATTACKED)
        secondFrameActorPositions_.push_back(actorPositions_[loop]);

    const Gui::Coord& pos = actorPositions_[loop].drawPos_;
    MachPhys::Race race = actorPositions_[loop].displayAsRace_;

    // Check for existance of beacon. No point drawing stuff if there is no beacon present.
    if (currentBeacon_ != MachLog::NO_BEACON)
    {
        if (actorPositions_[loop].type_ == POD && mapMode_ != RESOURCES_ONLY)
        {
            // Draw pod...
            if (actorPositions_[loop].actorState_ & SELECTED)
                mapFrameOne_.simpleBlit(selectedPodImage(race), pos.x(), pos.y());
            else
                mapFrameOne_.simpleBlit(podImage(race), pos.x(), pos.y());
        }
        else if (actorPositions_[loop].type_ == MACHINE && mapMode_ != RESOURCES_ONLY)
        {
            // Draw machine...
            if (actorPositions_[loop].actorState_ & SELECTED)
                mapFrameOne_.simpleBlit(selectedMachineImage(race), pos.x(), pos.y());
            else
                mapFrameOne_.simpleBlit(machineImage(race), pos.x(), pos.y());
        }
        else if (actorPositions_[loop].type_ == CAMOUFLAGEDMACHINE && mapMode_ != RESOURCES_ONLY)
        {
            // Draw machine...
            if (actorPositions_[loop].actorState_ & SELECTED)
                mapFrameOne_.simpleBlit(selectedMachineImage(playerRace_), pos.x(), pos.y());
            else
                mapFrameOne_.simpleBlit(machineImage(playerRace_), pos.x(), pos.y());
        }
        else if (actorPositions_[loop].type_ == CONSTRUCTION && mapMode_ != RESOURCES_ONLY)
        {
            // Draw construction...
            if (actorPositions_[loop].actorState_ & SELECTED)
                mapFrameOne_.simpleBlit(selectedConstructionImage(race), pos.x(), pos.y());
            else
                mapFrameOne_.simpleBlit(constructionImage(race), pos.x(), pos.y());
        }
        else if (actorPositions_[loop].type_ == DEBRIS)
        {
            // Draw debris...
            mapFrameOne_.simpleBlit(debrisImage(), pos.x(), pos.y());
        }
        else if (actorPositions_[loop].type_ == ORE)
        {
            // Draw ore...
            mapFrameOne_.simpleBlit(oreImage(), pos.x(), pos.y());
        }
        else if (actorPositions_[loop].type_ == ARTIFACT)
        {
            // Draw artifact...
            mapFrameOne_.simpleBlit(artifactImage(), pos.x(), pos.y());
        }
    }

    // Unfog new areas of map
    updateVisibleAreas(loop);
}

void MachContinentMap::updateVisibleAreas(size_t loop)
{
    if (actorPositions_[loop].scanner_ < 1)
        return; // No visible areas update

    MachPhys::Race race = actorPositions_[loop].race_;
    // Make sure that artifacts,ore and debris are not associated with any race by
    // setting race_ to N_RACES ( one after end of race enum )
    switch (actorPositions_[loop].type_)
    {
        case DEBRIS:
        case ARTIFACT:
        case ORE:
            race = MachPhys::N_RACES;
            break;
    }

    if (race == playerRace_)
    {
        Gui::Coord& scannerPos = actorPositions_[loop].worldPos_;
        size_t beenHereCheckX = scannerPos.x() / beenHereXRatio_;
        size_t beenHereCheckY = scannerPos.y() / beenHereYRatio_;

        // Check to see if an actor with a particular scanner range has already visited
        // the area of the map.
        short beenHereScannerSize = pBeenHere_[(beenHereCheckY * BEENHERE_ARRAYWIDTH) + beenHereCheckX];

        ASSERT(actorPositions_[loop].scanner_ < 6, "scanner size is invalid");

        if (beenHereScannerSize < actorPositions_[loop].scanner_)
        {
            // Update the size of scanner that has visited a particular cell
            pBeenHere_[(beenHereCheckY * BEENHERE_ARRAYWIDTH) + beenHereCheckX] = actorPositions_[loop].scanner_;

            Gui::Coord scannerDrawPos = actorPositions_[loop].actualPos_;

            // Show a bit more of the map
            scannerDrawPos.x(scannerDrawPos.x() - scannerRange_[actorPositions_[loop].scanner_ - 1]);
            scannerDrawPos.y(scannerDrawPos.y() - scannerRange_[actorPositions_[loop].scanner_ - 1]);

            GLint blendSrc, blendDst; // TODO move this GL code from here!
            glGetIntegerv(GL_BLEND_SRC_ALPHA, &blendSrc);
            glGetIntegerv(GL_BLEND_DST_ALPHA, &blendDst);
            glBlendFunc(GL_ZERO, GL_ONE_MINUS_SRC_ALPHA);

            mapVisibleArea_.simpleBlit(
                scannerRangeImage_[actorPositions_[loop].scanner_ - 1],
                scannerDrawPos.x(),
                scannerDrawPos.y());

            glBlendFunc(blendSrc, blendDst);
        }
    }
}

void MachContinentMap::drawCameraPos(GuiBitmap* pMapFrame)
{
    PRE(! pInGameScreen_->inFirstPerson());

    if (currentBeacon_ != MachLog::NO_BEACON)
    {
        Gui::Coord cameraImageOffset = Gui::Coord(-2, -2) * Gui::uiScaleFactor();
        int cameraPolygonThickness = 1;
        if (zenithCamera_)
        {
            // Draw box representing visible terrain area.
            RenSurface::Points cameraFovPoints;
            cameraFovPoints.reserve(5);
            cameraFovPoints.push_back(cameraFov_[0]);
            cameraFovPoints.push_back(cameraFov_[1]);
            cameraFovPoints.push_back(cameraFov_[2]);
            cameraFovPoints.push_back(cameraFov_[3]);
            cameraFovPoints.push_back(cameraFov_[0]);
            pMapFrame->polyLine(cameraFovPoints, Gui::LIGHTGREY(), cameraPolygonThickness);

            // Draw camera pos
            pMapFrame->simpleBlit(
                cameraPosImage_,
                cameraPos_.x() + cameraImageOffset.x(),
                cameraPos_.y() + cameraImageOffset.y());
        }
        else
        {
            // Draw camera pos
            pMapFrame->simpleBlit(
                cameraPosImage_,
                cameraPos_.x() + cameraImageOffset.x(),
                cameraPos_.y() + cameraImageOffset.y());

            Gui::Vec vecToEndPos(cameraPos_, cameraEndPos_);
            vecToEndPos *= Gui::uiScaleFactor();
            Gui::Coord realEndPos = cameraPos_ + vecToEndPos;

            // Draw line of sight
            RenSurface::Points lineOfSight;
            lineOfSight.reserve(2);
            lineOfSight.push_back(cameraPos_);
            lineOfSight.push_back(realEndPos);
            pMapFrame->polyLine(lineOfSight, Gui::LIGHTGREY(), cameraPolygonThickness);
        }
    }
}

// static
GuiBitmap& MachContinentMap::attackedMachineImage()
{
    static GuiBitmap machineAttackedImage = MachGui::getScaledImage("gui/map/amachpix.bmp");
    return machineAttackedImage;
}

// static
GuiBitmap& MachContinentMap::selectedAttackedMachineImage()
{
    static GuiBitmap selectedMachineAttackedImage = MachGui::getScaledImage("gui/map/amachpxs.bmp");
    return selectedMachineAttackedImage;
}

// static
GuiBitmap& MachContinentMap::attackedPodImage()
{
    static GuiBitmap podAttackedImage = MachGui::getScaledImage("gui/map/apodpix.bmp");
    return podAttackedImage;
}

// static
GuiBitmap& MachContinentMap::selectedAttackedPodImage()
{
    static GuiBitmap selectedPodAttackedImage = MachGui::getScaledImage("gui/map/apodpxs.bmp");
    return selectedPodAttackedImage;
}

void MachContinentMap::updateMapFrameTwo(size_t loop)
{
    PRE(! pInGameScreen_->inFirstPerson());

    GuiBitmap& machineAttackedImage = attackedMachineImage();
    GuiBitmap& podAttackedImage = attackedPodImage();
    GuiBitmap& selectedMachineAttackedImage = selectedAttackedMachineImage();
    GuiBitmap& selectedPodAttackedImage = selectedAttackedPodImage();
    static GuiBitmap constructionAttackedImage = MachGui::getScaledImage("gui/map/aconspix.bmp");
    static GuiBitmap selectedConstructionAttackedImage = MachGui::getScaledImage("gui/map/aconspxs.bmp");

    const Gui::Coord& pos = secondFrameActorPositions_[loop].drawPos_;
    MachPhys::Race race = secondFrameActorPositions_[loop].displayAsRace_;
    ActorState actorState = secondFrameActorPositions_[loop].actorState_;

    if (mapMode_ != RESOURCES_ONLY && currentBeacon_ != MachLog::NO_BEACON)
    {
        if (secondFrameActorPositions_[loop].type_ == POD)
        {
            // Draw attacked pods...
            if ((actorState & ATTACKED) && !(actorState & SELECTED))
            {
                mapFrameTwo_.simpleBlit(podAttackedImage, pos.x(), pos.y());
            }
            else if ((actorState & ATTACKED) && (actorState & SELECTED))
            {
                mapFrameTwo_.simpleBlit(selectedPodAttackedImage, pos.x(), pos.y());
            }
        }
        else if (
            secondFrameActorPositions_[loop].type_ == MACHINE
            || secondFrameActorPositions_[loop].type_ == CAMOUFLAGEDMACHINE)
        {
            // Draw attacked machines...
            if ((actorState & ATTACKED) && !(actorState & SELECTED))
            {
                mapFrameTwo_.simpleBlit(machineAttackedImage, pos.x(), pos.y());
            }
            else if ((actorState & ATTACKED) && (actorState & SELECTED))
            {
                mapFrameTwo_.simpleBlit(selectedMachineAttackedImage, pos.x(), pos.y());
            }
        }
        else if (secondFrameActorPositions_[loop].type_ == CONSTRUCTION)
        {
            // Draw attacked constructions...
            if ((actorState & ATTACKED) && !(actorState & SELECTED))
            {
                mapFrameTwo_.simpleBlit(constructionAttackedImage, pos.x(), pos.y());
            }
            else if ((actorState & ATTACKED) && (actorState & SELECTED))
            {
                mapFrameTwo_.simpleBlit(selectedConstructionAttackedImage, pos.x(), pos.y());
            }
        }
    }
}

MachGuiBmuText* MachContinentMap::bmuText()
{
    return pBmuText_;
}

void MachContinentMap::updateMapToWorldMetrics()
{
    MATHEX_SCALAR planetX = MachLogPlanet::instance().surface()->xMax();
    MATHEX_SCALAR planetY = MachLogPlanet::instance().surface()->yMax();
    MATHEX_SCALAR mapX = width() - 2; // -2 to take into account border
    MATHEX_SCALAR mapY = height() - 2;

    MATHEX_SCALAR xRatio = planetX / mapX;
    MATHEX_SCALAR yRatio = planetY / mapY;

    mapToWorldRatio_ = std::max(xRatio, yRatio);

    xOffset_ = (width() - (planetX / mapToWorldRatio())) / 2.0;
    yOffset_ = (height() - (planetY / mapToWorldRatio())) / 2.0;

    // Fog of war stuff...
    beenHereXRatio_ = planetX / BEENHERE_ARRAYWIDTH;
    beenHereYRatio_ = planetY / BEENHERE_ARRAYHEIGHT;
    pBeenHere_.resize(BEENHERE_ARRAYWIDTH * BEENHERE_ARRAYHEIGHT, 0);
}

MATHEX_SCALAR MachContinentMap::mapToWorldRatio() const
{
    return mapToWorldRatio_;
}

size_t MachContinentMap::xOffset() const
{
    return xOffset_;
}

size_t MachContinentMap::yOffset() const
{
    return yOffset_;
}

void MachContinentMap::updateRacePos()
{
    static size_t podImageWidth = podImage(MachPhys::RED).width();
    static size_t podImageHeight = podImage(MachPhys::RED).height();
    static size_t machineImageWidth = machineImage(MachPhys::RED).width();
    static size_t machineImageHeight = machineImage(MachPhys::RED).height();
    static size_t constructionImageWidth = constructionImage(MachPhys::RED).width();
    static size_t constructionImageHeight = constructionImage(MachPhys::RED).height();
    static size_t oreImageWidth = oreImage().width();
    static size_t oreImageHeight = oreImage().height();
    static size_t debrisImageWidth = debrisImage().width();
    static size_t debrisImageHeight = debrisImage().height();
    static size_t artifactImageWidth = artifactImage().width();
    static size_t artifactImageHeight = artifactImage().height();

    updateBeacon();

    firstFrameNumActors_ = 0;
    secondFrameNumActors_ = 0;

    MATHEX_SCALAR ratio = mapToWorldRatio();

    MachLogRaces& races = MachLogRaces::instance();

    // Clear out old info
    actorPositions_.erase(actorPositions_.begin(), actorPositions_.end());
    secondFrameActorPositions_.erase(secondFrameActorPositions_.begin(), secondFrameActorPositions_.end());

    MachLogRaces::Objects& allObjects = races.objects();

    for (MachLogRaces::Objects::iterator iter = allObjects.begin(); iter != allObjects.end(); ++iter)
    {
        MachActor* pActor = *iter;

        ActorPosInfo newPosInfo;
        newPosInfo.race_ = pActor->race();
        newPosInfo.displayAsRace_ = pActor->displayMapAndIconRace();
        newPosInfo.actorId_ = pActor->id();

        MexTransform3d trans = pActor->globalTransform();
        MexPoint3d pos = trans.position();

        newPosInfo.worldPos_ = Gui::Coord(pos.x(), pos.y());

        Gui::Coord absCoord = absoluteCoord();
        absCoord.x(absCoord.x() + (pos.x() / ratio) + xOffset());
        absCoord.y(absCoord.y() + (pos.y() / ratio) + yOffset());

        ActorState actorState = pActor->selectionState() == MachLog::SELECTED ? SELECTED : NORMAL;

        // Has the machine been attacked recently?
        if ((W4dManager::instance().frameNumber() - pActor->lastBeHitFrame()) < 10
            && (W4dManager::instance().frameNumber() > 10))
        {
            // actorState |= ATTACKED;
            actorState = _STATIC_CAST(ActorState, ActorState::ATTACKED | actorState);
        }

        newPosInfo.actorState_ = actorState;

        if (pActor->objectIsMachine())
        {
            if (! pActor->asMachine().insideBuilding() && // Ignore any machines inside buildings
                ! pActor->asMachine().insideAPC()) // Ignore any machines inside APCs
            {
                newPosInfo.actualPos_ = absCoord;

                // Account for machine image being bigger than one pixel
                absCoord.x(absCoord.x() - (machineImageWidth / 2.0));
                absCoord.y(absCoord.y() - (machineImageHeight / 2.0));

                newPosInfo.drawPos_ = absCoord;

                // Make camouflaged machines appear as the players race.
                if (pActor->asMachine().camouflaged())
                {
                    newPosInfo.type_ = CAMOUFLAGEDMACHINE;
                }
                else
                {
                    newPosInfo.type_ = MACHINE;
                }

                ++firstFrameNumActors_;
                if (actorState & ATTACKED)
                    ++secondFrameNumActors_;

                // store actors scanner range ( approximation ). Used for unfogging "fog of war"
                int scannerRange = pActor->asMachine().machineData().scannerRange();
                if (scannerRange <= SCANNER_UPPERLIMIT[0])
                    newPosInfo.scanner_ = 1;
                else if (scannerRange > SCANNER_UPPERLIMIT[0] && scannerRange <= SCANNER_UPPERLIMIT[1])
                    newPosInfo.scanner_ = 2;
                else if (scannerRange > SCANNER_UPPERLIMIT[1] && scannerRange <= SCANNER_UPPERLIMIT[2])
                    newPosInfo.scanner_ = 3;
                else if (scannerRange > SCANNER_UPPERLIMIT[2] && scannerRange <= SCANNER_UPPERLIMIT[3])
                    newPosInfo.scanner_ = 4;
                else
                    newPosInfo.scanner_ = 5;

                // Add to list of actors
                actorPositions_.push_back(newPosInfo);
            }
        }
        else if (pActor->objectIsConstruction())
        {
            ++firstFrameNumActors_;
            if (actorState & ATTACKED)
                ++secondFrameNumActors_;

            if (pActor->objectType() == MachLog::POD)
            {
                // store actors scanner range ( approximation ). Used for unfogging "fog of war".
                // For pods this is currently stored as scanner type 4.
                newPosInfo.scanner_ = 4;

                newPosInfo.actualPos_ = absCoord;

                // Account for pod image being bigger than one pixel
                absCoord.x(absCoord.x() - (podImageWidth / 2.0));
                absCoord.y(absCoord.y() - (podImageHeight / 2.0));

                newPosInfo.drawPos_ = absCoord;
                newPosInfo.type_ = POD;

                actorPositions_.push_back(newPosInfo);
            }
            else
            {
                // store actors scanner range ( approximation ). Used for unfogging "fog of war".
                // For constructions this is currently stored as scanner type 3.
                // Prevent unfogging by placing dummy constructions - fow exploit fix
                if (pActor->asConstruction().percentageComplete() > 0)
                    newPosInfo.scanner_ = 3;
                else
                    newPosInfo.scanner_ = 0;

                newPosInfo.actualPos_ = absCoord;

                // Account for construction image being bigger than one pixel
                absCoord.x(absCoord.x() - (constructionImageWidth / 2.0));
                absCoord.y(absCoord.y() - (constructionImageHeight / 2.0));

                newPosInfo.drawPos_ = absCoord;
                newPosInfo.type_ = CONSTRUCTION;

                actorPositions_.push_back(newPosInfo);
            }
        }
        else if (pActor->objectIsDebris() && mapMode_ != UNITS_ONLY && ! pInGameScreen_->inFirstPerson())
        {
            newPosInfo.actualPos_ = absCoord;

            // Account for debris image being bigger than one pixel
            absCoord.x(absCoord.x() - (debrisImageWidth / 2.0));
            absCoord.y(absCoord.y() - (debrisImageHeight / 2.0));

            newPosInfo.drawPos_ = absCoord;
            newPosInfo.type_ = DEBRIS;
            ++firstFrameNumActors_;

            actorPositions_.push_back(newPosInfo);
        }
        else if (pActor->objectIsOreHolograph() && mapMode_ != UNITS_ONLY && ! pInGameScreen_->inFirstPerson())
        {
            newPosInfo.actualPos_ = absCoord;

            // Account for ore image being bigger than one pixel
            absCoord.x(absCoord.x() - (oreImageWidth / 2.0));
            absCoord.y(absCoord.y() - (oreImageHeight / 2.0));

            newPosInfo.drawPos_ = absCoord;
            newPosInfo.type_ = ORE;
            ++firstFrameNumActors_;

            actorPositions_.push_back(newPosInfo);
        }
        else if (pActor->objectIsArtefact() && mapMode_ != UNITS_ONLY && ! pInGameScreen_->inFirstPerson())
        {
            newPosInfo.actualPos_ = absCoord;

            // Account for artifact image being bigger than one pixel
            absCoord.x(absCoord.x() - (artifactImageWidth / 2.0));
            absCoord.y(absCoord.y() - (artifactImageHeight / 2.0));

            newPosInfo.drawPos_ = absCoord;
            newPosInfo.type_ = ARTIFACT;
            ++firstFrameNumActors_;

            actorPositions_.push_back(newPosInfo);
        }
    }

    // Update camera position
    if (! pInGameScreen_->inFirstPerson())
        updateMapCameraRepresentation();

    // Update info for refresh function
    size_t numActors = firstFrameNumActors_ + secondFrameNumActors_;
    numActorsPerFrame_ = numActors / (NUM_FRAMES - 2);
    if (numActorsPerFrame_ == 0) // Ensure that if there is a low number of actors they still get drawn.
        numActorsPerFrame_ = 1;
}

void MachContinentMap::forceUpdate()
{
    PRE(! pInGameScreen_->inFirstPerson());

    // This will force the map to be refreshed
    updateRacePos();

    updateMapBackground();

    while (firstFrameNumActors_--)
        updateMapFrameOne(firstFrameNumActors_);

    // Show only unfogged areas
    if (currentBeacon_ == MachLog::LEVEL_1_BEACON)
        mapFrameOne_.simpleBlit(mapVisibleArea_);

    drawCameraPos(&mapFrameOne_);
    updateMapBackground2();

    while (secondFrameNumActors_--)
        updateMapFrameTwo(secondFrameNumActors_);

    // Show only unfogged areas
    if (currentBeacon_ == MachLog::LEVEL_1_BEACON)
        mapFrameTwo_.simpleBlit(mapVisibleArea_);

    drawCameraPos(&mapFrameTwo_);
    updateScreen();
    mapRefreshCounter_ = NUM_FRAMES - 1;
}

void MachContinentMap::updateScreen()
{
    // Indicate that map needs redrawing
    changed();
    pBmuText_->refresh();
    firstDraw_ = false;
}

// virtual
void MachContinentMap::doHandleMouseClickEvent(const GuiMouseEvent& rel)
{
    if (rel.leftButton() == Gui::RELEASED)
    {
        handleIntelligentCursor(rel, true);
    }
}

// virtual
void MachContinentMap::doHandleContainsMouseEvent(const GuiMouseEvent& rel)
{
    if (rel.rightButton() == Gui::PRESSED)
    {
        positionCamera(rel);
    }

    handleIntelligentCursor(rel, false);
}

// virtual
void MachContinentMap::doHandleMouseExitEvent(const GuiMouseEvent&)
{
    // Switch to menu cursor
    pInGameScreen_->cursor2d(MachGui::MENU_CURSOR);
}

bool MachContinentMap::findActor(const GuiMouseEvent& rel, MachActor** ppActor)
{
    PRE(ppActor);

    bool returnVal = false;

    for (ctl_vector<ActorPosInfo>::iterator iter = actorPositions_.begin(); iter != actorPositions_.end(); ++iter)
    {
        ActorPosInfo& actorPosInfo = (*iter);

        // Find actor that mouse cursor is pointing at. Allow a certain amount of
        // tolerance so that user doesn't have to be pixel perfect
        if (actorPosInfo.actualPos_.x() >= rel.coord().x() - 2 && actorPosInfo.actualPos_.x() <= rel.coord().x() + 2
            && actorPosInfo.actualPos_.y() >= rel.coord().y() - 2
            && actorPosInfo.actualPos_.y() <= rel.coord().y() + 2
            && MachLogRaces::instance().actorExists(actorPosInfo.actorId_))
        {
            MachActor* pActor = &MachLogRaces::instance().actor(actorPosInfo.actorId_);

            // Is valid actor ( i.e. does current map mode show actor ). Debris/Ore/Artefacts only
            // shown on RESOURCE map mode etc.
            bool validActor = false;
            if (pActor->objectIsDebris() || pActor->objectIsArtefact() || pActor->objectIsOreHolograph())
            {
                if (mapMode_ != MachContinentMap::UNITS_ONLY)
                {
                    validActor = true;
                }
            }
            else if (pActor->objectIsMachine() || pActor->objectIsConstruction())
            {
                if (mapMode_ != MachContinentMap::RESOURCES_ONLY)
                {
                    validActor = true;
                }
            }

            if (validActor)
            {
                returnVal = true;
                if (*ppActor) // Have we already found an actor?
                {
                    // Is the new actor a closer match?
                    if (actorPosInfo.actualPos_.x() >= rel.coord().x() - 1
                        && actorPosInfo.actualPos_.x() <= rel.coord().x() + 1
                        && actorPosInfo.actualPos_.y() >= rel.coord().y() - 1
                        && actorPosInfo.actualPos_.y() <= rel.coord().y() + 1)
                    {
                        *ppActor = pActor;
                        break;
                    }
                }
                else
                {
                    // Store pointer to first actor found. We will continue searching to find a closer match ( i.e.
                    // an actor closer to the cursor ).
                    *ppActor = pActor;
                }
            }
        }
    }

    return returnVal;
}

void MachContinentMap::handleIntelligentCursor(const GuiMouseEvent& rel, bool buttonClicked)
{
    // Default 2d cursor is arrow
    MachGui::Cursor2dType cursor2d = MachGui::MENU_CURSOR;

    // Must have at least a level 1 beacon to do intelligent cursor actions on map
    if (currentBeacon_ != MachLog::NO_BEACON)
    {
        // Indicate if commands are going to be executed in FOW
        MachGuiCommand::cursorInFogOfWar(
            currentBeacon_ != MachLog::LEVEL_3_BEACON && getFogOfWarLevel(rel.coord()) == FULL_FOW);

        // Pick on ACTOR
        MachActor* pActorUnderCursor = nullptr;
        // Don't do actor picks in a FOW situation ( drop through and do a terrain pick instead )
        if (! MachGuiCommand::cursorInFogOfWar() && findActor(rel, &pActorUnderCursor))
        {
            ASSERT(pActorUnderCursor != nullptr, "Found actor but pointer to actor left as NULL");

            // Show info about actor in corral and on prompt text
            pInGameScreen_->displayActorPromptText(pActorUnderCursor);
            if (pActorUnderCursor->selectableType() == MachLog::FULLY_SELECTABLE)
            {
                pInGameScreen_->highlightActor(pActorUnderCursor);
            }

            if (buttonClicked)
            {
                // Can't dispatch commands if game is paused or network is busy
                if (SimManager::instance().isSuspended() || pInGameScreen_->isNetworkStuffed())
                {
                    // Only allow selecting when paused
                    if (pInGameScreen_->activeCommand().cursorOnActor(
                            pActorUnderCursor,
                            rel.isCtrlPressed(),
                            rel.isShiftPressed(),
                            rel.isAltPressed())
                        == MachGui::SELECT_CURSOR)
                    {
                        pInGameScreen_->activeCommand().pickOnActor(
                            pActorUnderCursor,
                            rel.isCtrlPressed(),
                            rel.isShiftPressed(),
                            rel.isAltPressed());
                    }
                    else
                    {
                        MachGuiSoundManager::instance().playSound("gui/sounds/clickbad.wav");
                    }
                }
                else
                {
                    pInGameScreen_->activeCommand()
                        .pickOnActor(pActorUnderCursor, rel.isCtrlPressed(), rel.isShiftPressed(), rel.isAltPressed());
                }
            }
            else
            {
                cursor2d = pInGameScreen_->activeCommand().cursorOnActor(
                    pActorUnderCursor,
                    rel.isCtrlPressed(),
                    rel.isShiftPressed(),
                    rel.isAltPressed());
            }
        }
        // Pick on TERRAIN
        else
        {
            // Clear out single icon corral
            pInGameScreen_->removeHighlightedActor();

            // Convert cursor position to position on planet
            MATHEX_SCALAR x = rel.coord().x();
            MATHEX_SCALAR y = rel.coord().y();
            x -= xOffset_;
            y -= yOffset_;

            x *= mapToWorldRatio_;
            y *= mapToWorldRatio_;

            MachPhysPlanetSurface* pSurface = MachLogPlanet::instance().surface();

            // Make sure x,y lies inside planet surface boundary
            if (x >= pSurface->xMin() && x <= pSurface->xMax() && y >= pSurface->yMin() && y <= pSurface->yMax())
            {
                // Get terrain height
                MATHEX_SCALAR z = pSurface->terrainHeight(x, y);

                if (buttonClicked)
                {
                    // Can't issue commands if game is paused or network is busy
                    if (SimManager::instance().isSuspended() || pInGameScreen_->isNetworkStuffed())
                    {
                        MachGuiSoundManager::instance().playSound("gui/sounds/clickbad.wav");
                    }
                    else
                    {
                        pInGameScreen_->activeCommand().pickOnTerrain(
                            MexPoint3d(x, y, z),
                            rel.isCtrlPressed(),
                            rel.isShiftPressed(),
                            rel.isAltPressed());
                    }
                }
                else
                {
                    cursor2d = pInGameScreen_->activeCommand().cursorOnTerrain(
                        MexPoint3d(x, y, z),
                        rel.isCtrlPressed(),
                        rel.isShiftPressed(),
                        rel.isAltPressed());
                }
            }
        }
    }

    // Change cursor
    pInGameScreen_->cursor2d(cursor2d, MachInGameCursors2d::SMALLCURSORS);
}

void MachContinentMap::positionCamera(const GuiMouseEvent& rel)
{
    // Only allow position camera to work if a rubber band selection is not happening otherwise
    // the camera starts jumping all over the place and it gets very confusing. Also, you can only
    // position the camera if the map is active (i.e. beacon1 or beacon3 exists ).
    if (rel.rightButton() == Gui::PRESSED && ! pInGameScreen_->rubberBandSelectionHappening()
        && currentBeacon_ != MachLog::NO_BEACON)
    {
        // Don't allow gradual change in FOW lighting if the camera is positioned because the
        // player might be trying to cheat by having a very fast glimpse of what the enemy is
        // doing.
        noFastChangeInLightLevel_ = true;

        MATHEX_SCALAR ratio = mapToWorldRatio();

        MATHEX_SCALAR maxPlanetX = MachLogPlanet::instance().surface()->xMax();
        MATHEX_SCALAR maxPlanetY = MachLogPlanet::instance().surface()->yMax();
        MATHEX_SCALAR minPlanetX = MachLogPlanet::instance().surface()->xMin();
        MATHEX_SCALAR minPlanetY = MachLogPlanet::instance().surface()->yMin();
        MATHEX_SCALAR moveToPlanetX = (rel.coord().x() - xOffset()) * ratio;
        MATHEX_SCALAR moveToPlanetY = (rel.coord().y() - yOffset()) * ratio;

        // Check that we are moving the camera to a valid location
        if (moveToPlanetX > minPlanetX && moveToPlanetX < maxPlanetX && moveToPlanetY > minPlanetY
            && moveToPlanetY < maxPlanetY)
        {
            // Move camera and force map to be re-drawn.
            pCameras_->lookAt(MexPoint3d(
                moveToPlanetX,
                moveToPlanetY,
                MachLogPlanet::instance().surface()->terrainHeight(moveToPlanetX, moveToPlanetY)));
            forceUpdate();
        }
    }
}

void MachContinentMap::refresh()
{
    // Refresh the continent map every 12 frames.
    // Spread to work over a number of frames so there is no visible hit
    // on frame rate as the map is refreshed. Note that spreading the work over an
    // odd number of frames would cause attacked machines/constructions to flash
    // irregularly.
    mapRefreshCounter_ = (++mapRefreshCounter_) % NUM_FRAMES;

    switch (mapRefreshCounter_)
    {
        case 0:
            // Refresh race positions
            updateRacePos();
            break;
        case NUM_FRAMES - 1:
            // Last frame. Do any clean up
            refreshLastFrame();
            break;
        case 1:
            if (! pInGameScreen_->inFirstPerson())
            {
                updateMapBackground();
            }
            // Special case when there are no actors to display
            if (firstFrameNumActors_ == 0)
            {
                // Show only unfogged areas
                if (currentBeacon_ == MachLog::LEVEL_1_BEACON)
                    mapFrameOne_.simpleBlit(mapVisibleArea_);
                // Show camera
                drawCameraPos(&mapFrameOne_);
                // Copy from frame 1 to 2
                updateMapBackground2();

                break;
            }
            // deliberate drop-through
        default:
            refreshFrame();
            break;
    }

    if (! pInGameScreen_->inFirstPerson())
    {
        update3dFogOfWarLightLevel();
    }
}

void MachContinentMap::refreshLastFrame()
{
    while (firstFrameNumActors_ || secondFrameNumActors_)
    {
        if (firstFrameNumActors_)
        {
            --firstFrameNumActors_;

            if (pInGameScreen_->inFirstPerson())
            {
                updateVisibleAreas(firstFrameNumActors_);
            }
            else
            {
                updateMapFrameOne(firstFrameNumActors_);
            }

            // have we finished frame one?
            if (firstFrameNumActors_ == 0 && ! pInGameScreen_->inFirstPerson())
            {
                // If so then prepare second frame
                // Show only unfogged areas
                if (currentBeacon_ == MachLog::LEVEL_1_BEACON)
                    mapFrameOne_.simpleBlit(mapVisibleArea_);

                drawCameraPos(&mapFrameOne_);

                updateMapBackground2();
            }
        }
        else if (secondFrameNumActors_)
        {
            --secondFrameNumActors_;

            if (! pInGameScreen_->inFirstPerson())
            {
                updateMapFrameTwo(secondFrameNumActors_);
            }
        }
    }

    if (! pInGameScreen_->inFirstPerson())
    {
        // Finish off frame two
        // Show only unfogged areas
        if (currentBeacon_ == MachLog::LEVEL_1_BEACON)
            mapFrameTwo_.simpleBlit(mapVisibleArea_);

        drawCameraPos(&mapFrameTwo_);
        // Instruct Gui to display map
        updateScreen();
    }
}

void MachContinentMap::refreshFrame()
{
    for (int loop = 0; loop < numActorsPerFrame_ && (firstFrameNumActors_ || secondFrameNumActors_); ++loop)
    {
        if (firstFrameNumActors_)
        {
            --firstFrameNumActors_;

            if (pInGameScreen_->inFirstPerson())
            {
                updateVisibleAreas(firstFrameNumActors_);
            }
            else
            {
                updateMapFrameOne(firstFrameNumActors_);
            }

            // have we finished frame one?
            if (firstFrameNumActors_ == 0 && ! pInGameScreen_->inFirstPerson())
            {
                // If so then prepare second frame after final touches are added to frame one
                // Show only unfogged areas
                if (currentBeacon_ == MachLog::LEVEL_1_BEACON)
                    mapFrameOne_.simpleBlit(mapVisibleArea_);

                drawCameraPos(&mapFrameOne_);

                updateMapBackground2();
            }
        }
        else if (secondFrameNumActors_)
        {
            --secondFrameNumActors_;
            if (! pInGameScreen_->inFirstPerson())
            {
                updateMapFrameTwo(secondFrameNumActors_);
            }
        }
    }
}

MexLine3d MachContinentMap::fromCameraToScreenLine(const Gui::Coord& screenPoint) const
{
    // Get the scene manager
    W4dSceneManager& sceneManager = pInGameScreen_->sceneManager();

    // The viewport must be correctly set to correspond to this window before
    // we can use the RenDevice methods.
    pInGameScreen_->setWorldViewViewport();

    // Hence get corresponding world position
    MexPoint3d cursorWorldPos = sceneManager.pDevice()->screenToCamera(screenPoint);

    // Reset the viewport correctly for GUI drawing.  TBD: a save/restore or
    // push/pop idiom would be much more robust.
    pInGameScreen_->setGuiViewport();

    // Construct a line from the camera origin through the cursor point in world coordinates
    const MexTransform3d& cameraTransform = sceneManager.currentCamera()->globalTransform();
    MexPoint3d cameraOrigin(cameraTransform.position());
    cameraTransform.transform(&cursorWorldPos);
    MexLine3d theLine(cameraOrigin, cursorWorldPos);

    return theLine;
}

Gui::Coord MachContinentMap::translate3DScreenCoordToMapCoord(const Gui::Coord& screenCoord) const
{
    MexLine3d cursorLine = fromCameraToScreenLine(screenCoord);
    // Work out the 3D position when the cursor line hits z = 0
    MexVec3 cursorLineUnitVec = cursorLine.unitDirectionVector();
    MATHEX_SCALAR ratio = cursorLine.end1().z() / cursorLineUnitVec.z();
    MATHEX_SCALAR x2 = cursorLine.end1().x() - (ratio * cursorLineUnitVec.x());
    MATHEX_SCALAR y2 = cursorLine.end1().y() - (ratio * cursorLineUnitVec.y());

    return Gui::Coord((x2 / mapToWorldRatio_) + xOffset_, (y2 / mapToWorldRatio_) + yOffset_);
}

void MachContinentMap::updateMapCameraRepresentation()
{
    PRE(! pInGameScreen_->inFirstPerson());

    MATHEX_SCALAR ratio = mapToWorldRatio();

    // Update camera position
    zenithCamera_ = pCameras_->isZenithCameraActive();

    // Update cameras actual position
    MachLogCamera* pCamera = pCameras_->currentCamera();
    MexTransform3d cameraTrans = pCamera->globalTransform();
    MexPoint3d cameraPos = cameraTrans.position();
    cameraPos_.x((cameraPos.x() / ratio) + xOffset());
    cameraPos_.y((cameraPos.y() / ratio) + yOffset());

    // If were in zenith then calculate the area of the terrain that is visible
    if (zenithCamera_)
    {
        W4dSceneManager& sceneManager = pInGameScreen_->sceneManager();

        Gui::Coord topLeft = pInGameScreen_->worldViewWindow().absoluteBoundary().minCorner();
        Gui::Coord bottomRight
            = Gui::Coord(sceneManager.pDevice()->windowWidth(), sceneManager.pDevice()->windowHeight());
        Gui::Coord bottomLeft = Gui::Coord(topLeft.x(), bottomRight.y());
        Gui::Coord topRight = Gui::Coord(bottomRight.x(), topLeft.y());

        cameraFov_[0] = translate3DScreenCoordToMapCoord(topLeft);
        cameraFov_[1] = translate3DScreenCoordToMapCoord(topRight);
        cameraFov_[2] = translate3DScreenCoordToMapCoord(bottomRight);
        cameraFov_[3] = translate3DScreenCoordToMapCoord(bottomLeft);
    }
    else
    {
        // Update camera end pos ( basically a line from camera pos to show the line of sight )
        MexVec3 xBasis = cameraTrans.xBasis();
        xBasis.z(0);
        xBasis.makeUnitVector();
        MexPoint3d cameraEndPos = cameraPos;
        cameraEndPos += xBasis;
        MexLine3d line(cameraPos, cameraEndPos);
        cameraEndPos = line.pointAtDistance(10 * ratio);
        cameraEndPos_.x((cameraEndPos.x() / ratio) + xOffset());
        cameraEndPos_.y((cameraEndPos.y() / ratio) + yOffset());
    }
}

void MachContinentMap::fogOfWarOn(bool fog)
{
    fogOfWarOn_ = fog;
    forceUpdate();
}

void MachContinentMap::mapMode(MapMode mapMode)
{
    mapMode_ = mapMode;
    forceUpdate();
}

MachContinentMap::MapMode MachContinentMap::mapMode() const
{
    return mapMode_;
}

void MachContinentMap::updateBeacon(bool forceBeaconUpdate /* = false */)
{
    MachLog::BeaconType newBeaconSetting = MachLogRaces::instance().beaconType(playerRace_);

    // If fog of war isn't being used then pretend we have a level 3 beacon.
    if (! fogOfWarOn_)
    {
        newBeaconSetting = MachLog::LEVEL_3_BEACON;
    }

    if (newBeaconSetting != currentBeacon_ || forceBeaconUpdate)
    {
        currentBeacon_ = newBeaconSetting;

        // If we don't have any form of beacon then the map just shows the race logo
        if (currentBeacon_ == MachLog::NO_BEACON)
        {
            mapBackground_ = logoImage(playerRace_);
        }
        else
        {
            // Load the actual map terrain
            if (Gui::uiScaleFactor() == 1)
            {
                mapBackground_ = Gui::bitmap(mapPath_);
            }
            else
            {
                GuiBitmap mapBitmap = Gui::bitmap(mapPath_);
                mapBackground_ = RenSurface::createAnonymousSurface(
                    mapBitmap.width() * Gui::uiScaleFactor(),
                    mapBitmap.height() * Gui::uiScaleFactor(),
                    mapBitmap);
                mapBackground_.stretchBlit(mapBitmap);
            }
        }
    }
}

void MachContinentMap::update3dFogOfWarLightLevel()
{
    PRE(! pInGameScreen_->inFirstPerson());

    // A level 3 beacon removes all fog-of-war. Might as well terminate early.
    if (currentBeacon_ == MachLog::LEVEL_3_BEACON)
    {
        fogOfWarLightLevel_ = 1.0;
        pInGameScreen_->sceneManager().pDevice()->interferenceOff();
        pCameras_->currentCamera()->pRenCamera()->colourFilter(RenColour::white());
        noFastChangeInLightLevel_ = false;
        MachGuiCommand::cursorInFogOfWar(false);
        // Make sure sounds are on
        if (! W4dSoundManager::instance().isPlayingSounds())
        {
            W4dSoundManager::instance().setPlayingSoundsStatus(true);
        }
        return;
    }

    MATHEX_SCALAR ratio = mapToWorldRatio();

    bool zenithCamera = pCameras_->isZenithCameraActive();

    // Point on map where we are going to check if the camera is in an unfogged area or not.
    Gui::Coord cameraCheckPos;

    if (zenithCamera)
    {
        // Old code. Projects line of sight through centre of screen to z = 0. This resulted
        // in some undesirable FOW effects when viewing things on hills. NA 4/12/98
        // W4dSceneManager& sceneManager = pInGameScreen_->sceneManager();
        //
        // Gui::Coord topLeft =  pInGameScreen_->worldViewWindow().absoluteBoundary().minCorner() ;
        // Gui::Coord bottomRight =  Gui::Coord( sceneManager.pDevice()->windowWidth(),
        // sceneManager.pDevice()->windowHeight() ); Gui::XCoord x = topLeft.x() + ( ( bottomRight.x() - topLeft.x() )
        // / 2.0 ); Gui::XCoord y = topLeft.y() + ( ( bottomRight.y() - topLeft.y() ) / 2.0 );

        // cameraCheckPos = translate3DScreenCoordToMapCoord( Gui::Coord(x,y) );

        cameraCheckPos = pCameras_->positionOnTerrainThatZenithCameraIsLookingAt();
        cameraCheckPos.x((cameraCheckPos.x() / mapToWorldRatio_) + xOffset_);
        cameraCheckPos.y((cameraCheckPos.y() / mapToWorldRatio_) + yOffset_);
    }
    else
    {
        // Update cameras actual position
        const MachLogCamera* pCamera = pCameras_->currentCamera();
        MexTransform3d cameraTrans = pCamera->globalTransform();
        MexPoint3d cameraPos = cameraTrans.position();
        cameraCheckPos.x((cameraPos.x() / ratio) + xOffset());
        cameraCheckPos.y((cameraPos.y() / ratio) + yOffset());
    }

    const double fogOfWarNewFrameTimer = DevTime::instance().time();

    const double slowChangeInLightLevel = (fogOfWarNewFrameTimer - fogOfWarFrameTimer_) * 0.2;
    const double fastChangeInLightLevel = (fogOfWarNewFrameTimer - fogOfWarFrameTimer_) * 0.8;
    const double veryFastChangeInLightLevel = (fogOfWarNewFrameTimer - fogOfWarFrameTimer_) * 2.0;

    fogOfWarLevel_ = getFogOfWarLevel(cameraCheckPos);

    // Control speed at which light levels change...
    if (fogOfWarLevel_ == NO_FOW)
    {
        fogOfWarLightLevel_ += fastChangeInLightLevel;

        if (fogOfWarLightLevel_ > 1.0)
        {
            fogOfWarLightLevel_ = 1.0;
        }
    }
    else if (fogOfWarLevel_ == TEND_TO_FIFTY_PERCENT_FOW)
    {
        if (fogOfWarLightLevel_ > 0.5)
        {
            fogOfWarLightLevel_ -= slowChangeInLightLevel;

            if (fogOfWarLightLevel_ < 0.5)
            {
                fogOfWarLightLevel_ = 0.5;
            }
        }
        else if (fogOfWarLightLevel_ < 0.5)
        {
            fogOfWarLightLevel_ += slowChangeInLightLevel;

            if (fogOfWarLightLevel_ > 0.5)
            {
                fogOfWarLightLevel_ = 0.5;
            }
        }
    }
    else if (fogOfWarLevel_ == TEND_TO_FULL_FOW)
    {
        fogOfWarLightLevel_ -= slowChangeInLightLevel;

        if (fogOfWarLightLevel_ < 0.0)
        {
            fogOfWarLightLevel_ = 0.0;
        }
    }
    else
    {
        if (noFastChangeInLightLevel_)
            fogOfWarLightLevel_ = 0.0;
        else
        {
            fogOfWarLightLevel_ -= veryFastChangeInLightLevel;

            if (fogOfWarLightLevel_ < 0.0)
            {
                fogOfWarLightLevel_ = 0.0;
            }
        }
    }

    ASSERT_INFO(fogOfWarLightLevel_);
    ASSERT(fogOfWarLightLevel_ >= 0 && fogOfWarLightLevel_ <= 1, "Expected FOW level between 0 and 1");
    const double fogOfWarIntensity = 1 - fogOfWarLightLevel_;

    // Indicate to commands that they will have to execute in FOW if light levels have
    // dropped to blackness...
    MachGuiCommand::cursorInFogOfWar(MexEpsilon::isWithinEpsilonOf(fogOfWarLightLevel_, 0.0));

    // Update sound ( off if in FOW )
    if (MexEpsilon::isWithinEpsilonOf(fogOfWarLightLevel_, 0.0))
    {
        if (W4dSoundManager::instance().isPlayingSounds())
        {
            W4dSoundManager::instance().setPlayingSoundsStatus(false);
        }
    }
    else
    {
        if (! W4dSoundManager::instance().isPlayingSounds())
        {
            W4dSoundManager::instance().setPlayingSoundsStatus(true);
        }
    }

    // Above 0.5, the filter darkness increases at twice the rate of FoW.
    if (fogOfWarLightLevel_ < 0.5)
    {
        const double filterColour = 2 * fogOfWarLightLevel_;
        pCameras_->currentCamera()->pRenCamera()->colourFilter(RenColour(filterColour));
    }
    else
    {
        pCameras_->currentCamera()->pRenCamera()->colourFilter(RenColour::white());
    }

    // Below 0.7 we just use the FoW intensity to determine the amount of interference.
    if (fogOfWarIntensity < 0.7)
        pInGameScreen_->sceneManager().pDevice()->interferenceOn(fogOfWarIntensity);
    else
    {
        // Above 0.7, the interference rapidly drops off, leaving us with a black screen.
        double interferenceLevel = (0.7 / 0.3) * (1.0 - fogOfWarIntensity);
        pInGameScreen_->sceneManager().pDevice()->interferenceOn(interferenceLevel);
    }

    noFastChangeInLightLevel_ = false;

    fogOfWarFrameTimer_ = fogOfWarNewFrameTimer;
}

MachContinentMap::FogOfWarLevel MachContinentMap::getFogOfWarLevel(const Gui::Coord& checkPos)
{
    RenColour fogColour;
    mapVisibleArea_.getPixel(checkPos.x(), checkPos.y(), &fogColour);

    // Check for no fow
    //  if ( fogColour == Gui::MAGENTA() )
    if (fogColour.a() == 0)
    {
        return NO_FOW;
    }

    // Check for no fow at second level of pixels
    for (int x = -1; x <= 1; ++x)
    {
        for (int y = -1; y <= 1; ++y)
        {
            if (!(y == 0 && x == 0))
            {
                mapVisibleArea_.getPixel(checkPos.x() + x, checkPos.y() + y, &fogColour);
                //              if ( fogColour == Gui::MAGENTA() )
                if (fogColour.a() == 0)
                {
                    return NO_FOW;
                }
            }
        }
    }

    // Check for tend to 50% fow
    for (int x = -2; x <= 2; ++x)
    {
        for (int y = -2; y <= 2; ++y)
        {
            if (y == -2 || x == -2 || y == 2 || x == 2)
            {
                mapVisibleArea_.getPixel(checkPos.x() + x, checkPos.y() + y, &fogColour);
                //              if ( fogColour == Gui::MAGENTA() )
                if (fogColour.a() == 0)
                {
                    return TEND_TO_FIFTY_PERCENT_FOW;
                }
            }
        }
    }

    // Check for tend to 100% fow
    for (int x = -3; x <= 3; ++x)
    {
        for (int y = -3; y <= 3; ++y)
        {
            if (y == -3 || x == -3 || y == 3 || x == 3)
            {
                mapVisibleArea_.getPixel(checkPos.x() + x, checkPos.y() + y, &fogColour);
                //              if ( fogColour == Gui::MAGENTA() )
                if (fogColour.a() == 0)
                {
                    return TEND_TO_FULL_FOW;
                }
            }
        }
    }

    return FULL_FOW;
}

void MachContinentMap::saveGame(PerOstream& outStream)
{
    outStream << fogOfWarOn_;

    if (Gui::uiScaleFactor() == 1)
    {
        mapVisibleArea_.write(outStream);
    }
    else
    {
        GuiBitmap visibleArea = RenSurface::createAnonymousSurface(
            mapVisibleArea_.width() / Gui::uiScaleFactor(),
            mapVisibleArea_.height() / Gui::uiScaleFactor(),
            mapVisibleArea_);
        visibleArea.filledRectangle(RenSurface::Rect(0, 0, visibleArea.width(), visibleArea.height()), Gui::BLACK());

        GLint blendSrc, blendDst;
        glGetIntegerv(GL_BLEND_SRC_ALPHA, &blendSrc);
        glGetIntegerv(GL_BLEND_DST_ALPHA, &blendDst);
        glBlendFunc(GL_ONE, GL_ZERO);

        visibleArea.stretchBlit(mapVisibleArea_);
        glBlendFunc(blendSrc, blendDst);
        visibleArea.write(outStream);
    }
}

void MachContinentMap::loadSavedGame(const std::string& planet, PerIstream& inStream)
{
    inStream >> fogOfWarOn_;

    loadGame(planet);

    GuiBitmap loadedVisibleArea = RenSurface::createAnonymousSurface(inStream, Gui::backBuffer());

    if (loadedVisibleArea.width() == mapVisibleArea_.width())
    {
        mapVisibleArea_ = loadedVisibleArea;
    }
    else
    {
        mapVisibleArea_.filledRectangle(
            RenSurface::Rect(0, 0, mapVisibleArea_.width(), mapVisibleArea_.height()),
            Gui::MAGENTA());
        mapVisibleArea_.stretchBlit(loadedVisibleArea);
    }
    mapVisibleArea_.enableColourKeying();
}

/* End MAP.CPP ******************************************************/
