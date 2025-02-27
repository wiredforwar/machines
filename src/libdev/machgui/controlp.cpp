/*
 * C O N T R O L P . C P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

//  Definitions of non-inline non-template methods and global functions

#include "machgui/controlp.hpp"
#include "machgui/internal/controli.hpp"
#include "stdlib/string.hpp"
#include "system/pathname.hpp"
#include "world4d/manager.hpp"
#include "world4d/scenemgr.hpp"
#include "render/device.hpp"
#include "gui/painter.hpp"
#include "gui/button.hpp"
#include "machgui/gui.hpp"
#include "machgui/machnav.hpp"
#include "machgui/ingame.hpp"
#include "gui/restring.hpp"
#include "machgui/internal/strings.hpp"
#include "machgui/internal/mgsndman.hpp"
#include "machlog/races.hpp"

///////////////////////////////////////////////////////////////////////////////

class MachGuiControlPanelOnOffBtn : public GuiButton
// cannonical from revoked
{
public:
    MachGuiControlPanelOnOffBtn(GuiDisplayable* pParent, const Gui::Coord& rel, MachInGameScreen* pInGameScreen)
        : GuiButton(pParent, Gui::Box(rel, reqWidth(), reqHeight()))
        , pInGameScreen_(pInGameScreen)
        , slideHoleBmp_(MachGui::getScaledImage("gui/misc/slidhole.bmp"))
        , sliderBmp_(MachGui::getScaledImage("gui/misc/slider.bmp"))
    {
    }

    static int reqWidth() { return 6 * Gui::uiScaleFactor(); }

    static int reqHeight() { return 49 * Gui::uiScaleFactor(); }

protected:
    void doBeReleased(const GuiMouseEvent&) override
    {
        pInGameScreen_->controlPanelOn(! pInGameScreen_->controlPanelOn());
    }

    void doBeDepressed(const GuiMouseEvent&) override
    {
        MachGuiSoundManager::instance().playSound("gui/sounds/igclick.wav");
    }

    void doHandleMouseExitEvent(const GuiMouseEvent& rel) override
    {
        GuiButton::doHandleMouseExitEvent(rel);

        // Clear the cursor prompt string
        pInGameScreen_->clearCursorPromptText();
    }

    void doHandleContainsMouseEvent(const GuiMouseEvent& rel) override
    {
        GuiButton::doHandleContainsMouseEvent(rel);

        if (pInGameScreen_->controlPanelOn())
        {
            // Load the resource string
            GuiResourceString prompt(IDS_CONTROLPANELOFF);

            // Set the cursor prompt
            pInGameScreen_->setCursorPromptText(prompt.asString());
        }
        else
        {
            // Load the resource string
            GuiResourceString prompt(IDS_CONTROLPANELON);

            // Set the cursor prompt
            pInGameScreen_->setCursorPromptText(prompt.asString());
        }
    }

    void doDisplayDepressedEnabled() override
    {
        GuiPainter::instance().blit(slideHoleBmp_, absoluteBoundary().minCorner());
        GuiPainter::instance().blit(
            sliderBmp_,
            Gui::Coord(
                absoluteBoundary().minCorner().x(),
                absoluteBoundary().minCorner().y() + 12 * Gui::uiScaleFactor()));
    }

    void doDisplayReleasedEnabled() override
    {
        GuiPainter::instance().blit(slideHoleBmp_, absoluteBoundary().minCorner());

        if (pInGameScreen_->controlPanelOn())
            GuiPainter::instance().blit(sliderBmp_, absoluteBoundary().minCorner());
        else
            GuiPainter::instance().blit(
                sliderBmp_,
                Gui::Coord(
                    absoluteBoundary().minCorner().x(),
                    absoluteBoundary().minCorner().y() + 24 * Gui::uiScaleFactor()));
    }

private:
    MachGuiControlPanelOnOffBtn(const MachGuiControlPanelOnOffBtn&);
    MachGuiControlPanelOnOffBtn& operator=(const MachGuiControlPanelOnOffBtn&);
    bool operator==(const MachGuiControlPanelOnOffBtn&) const;

    // Data members...
    MachInGameScreen* pInGameScreen_;
    GuiBitmap slideHoleBmp_;
    GuiBitmap sliderBmp_;
};

MachGuiControlPanel::MachGuiControlPanel(GuiDisplayable* pParent, const Gui::Box& box, MachInGameScreen* pInGameScreen)
    : GuiDisplayable(pParent, box, GuiDisplayable::LAYER3)
    , pImpl_(new MachGuiControlPanelImpl())
{
    CB_DEPIMPL_ARRAY(GuiBitmap, decals_);
    CB_DEPIMPL(MachGuiControlPanelImpl::RedrawAreas, redrawAreas_);

    redrawAreas_.reserve(64);

    setupDecalBitmaps();
    setupDecalCoords();

    new MachGuiControlPanelOnOffBtn(
        this,
        Gui::Coord(
            MachGui::controlPanelOutXPos() - MachGuiControlPanelOnOffBtn::reqWidth(),
            8 * Gui::uiScaleFactor()),
        pInGameScreen);

    useFastSecondDisplay(false);

    TEST_INVARIANT;
}

MachGuiControlPanel::~MachGuiControlPanel()
{
    TEST_INVARIANT;

    delete pImpl_;
}

void MachGuiControlPanel::CLASS_INVARIANT
{
    INVARIANT(this != nullptr);
}

std::ostream& operator<<(std::ostream& o, const MachGuiControlPanel& t)
{

    o << "MachGuiControlPanel " << static_cast<const void*>(&t) << " start" << std::endl;
    o << "MachGuiControlPanel " << static_cast<const void*>(&t) << " end" << std::endl;

    return o;
}

// virtual
void MachGuiControlPanel::doDisplay()
{
    redrawAreaImmediate(Gui::Box(0, 0, width(), height()));
}

void MachGuiControlPanel::redrawArea(const GuiDisplayable& displayable)
{
    redrawArea(displayable.relativeBoundary(*this));
}

void MachGuiControlPanel::redrawArea(const Gui::Box& area)
{
    NEIL_STREAM("***MachGuiControlPanel::redrawArea() " << area << std::endl);

    CB_DEPIMPL(MachGuiControlPanelImpl::RedrawAreas, redrawAreas_);

    MachGuiControlPanelImpl::RedrawArea* pNewRedrawArea = new MachGuiControlPanelImpl::RedrawArea();
    pNewRedrawArea->area_ = area;

    if (GuiDisplayable::useFourTimesRender())
    {
        pNewRedrawArea->count_ = 4;
    }
    else
    {
        pNewRedrawArea->count_ = 2;
    }

    redrawAreas_.push_back(pNewRedrawArea);
}

void MachGuiControlPanel::redrawAreaImmediate(const GuiDisplayable& displayable)
{
    redrawAreaImmediate(displayable.relativeBoundary(*this));
}

void MachGuiControlPanel::redrawAreaImmediate(const Gui::Box& area)
{
    CB_DEPIMPL_ARRAY(GuiBitmap, decals_);
    CB_DEPIMPL_ARRAY(Gui::Coord, decalCoord_);

    int y = 3;
    int maxY = getVisibleHeight();
    int yStep = MachGui::controlPanelBmp().height();

    int areaYMax = area.maxCorner().y();
    int areaXMax = area.maxCorner().x();
    int areaYMin = area.minCorner().y();
    int areaXMin = area.minCorner().x();

    // DEBUG_STREAM( DIAG_NEIL, "*** redraw area " << areaXMin << "," << areaYMin << "," << areaXMax << "," << areaYMax
    // << std::endl );

    while (y < maxY && y < areaYMax)
    {
        if (areaYMin <= y + yStep)
        {
            Gui::Box drawArea(
                std::max(0, areaXMin),
                std::max(y, areaYMin),
                std::min((int)MachGui::controlPanelBmp().width(), areaXMax),
                std::min(y + yStep, areaYMax));

            int drawAreaYMax = drawArea.maxCorner().y();
            int drawAreaXMax = drawArea.maxCorner().x();
            int drawAreaYMin = drawArea.minCorner().y();
            int drawAreaXMin = drawArea.minCorner().x();

            // DEBUG_STREAM( DIAG_NEIL, "     drawArea " << drawAreaXMin << "," << drawAreaYMin << "," << drawAreaXMax
            // << "," << drawAreaYMax << std::endl );

            Gui::Coord screenCoords(
                drawAreaXMin + absoluteBoundary().minCorner().x(),
                drawAreaYMin + absoluteBoundary().minCorner().y());

            GuiPainter::instance().blit(
                MachGui::controlPanelBmp(),
                Gui::Box(
                    Gui::Coord(drawAreaXMin, drawAreaYMin - y),
                    drawAreaXMax - drawAreaXMin, /* width */
                    drawAreaYMax - drawAreaYMin /* height*/),
                screenCoords);

            // DEBUG_STREAM( DIAG_NEIL, "     blitFrom " << drawAreaXMin << "," <<  drawAreaYMin - y << "," <<
            // drawAreaXMax - drawAreaXMin << "," << drawAreaYMax - drawAreaYMin << std::endl );
        }

        y += MachGui::controlPanelBmp().height();
    }

    // Render decals
    for (int loop = 0; loop < 8; ++loop)
    {
        // Move decal co-ord if screen has been scrolled to the left/right.
        Gui::Coord screenDecalCoord(decalCoord_[loop].x(), decalCoord_[loop].y());
        Gui::Box decalBox(screenDecalCoord, decals_[loop].width(), decals_[loop].height());

        if (decalBox.intersects(area))
        {
            Gui::Box decalIntersect = decalBox;
            decalIntersect.intersectWith(area);

            GuiPainter::instance().blit(
                decals_[loop],
                Gui::Box(
                    Gui::Coord(
                        decalIntersect.minCorner().x() - decalBox.minCorner().x(),
                        decalIntersect.minCorner().y() - decalBox.minCorner().y()),
                    decalIntersect.maxCorner().x() - decalIntersect.minCorner().x(), // width
                    decalIntersect.maxCorner().y() - decalIntersect.minCorner().y()), // height
                Gui::Coord(
                    decalIntersect.minCorner().x() + absoluteBoundary().minCorner().x(),
                    decalIntersect.minCorner().y() + absoluteBoundary().minCorner().y()));
        }
    }

    // TEMP
    // hollowRectangle( area, Gui::BLACK(), 1 );
}

void MachGuiControlPanel::redrawAllAreas()
{
    CB_DEPIMPL(MachGuiControlPanelImpl::RedrawAreas, redrawAreas_);

    size_t n = redrawAreas_.size();
    for (size_t i = n; i-- != 0;)
    {
        MachGuiControlPanelImpl::RedrawArea* pRedrawArea = redrawAreas_[i];

        NEIL_STREAM("***MachGuiControlPanel::redrawAllAreas() " << pRedrawArea->area_ << std::endl);

        redrawAreaImmediate(pRedrawArea->area_);
        --(pRedrawArea->count_);

        // Clean up any redraw areas that have been used twice
        if (pRedrawArea->count_ == 0)
        {
            delete pRedrawArea;
            redrawAreas_.erase(redrawAreas_.begin() + i);
        }
    }
}

int MachGuiControlPanel::getVisibleHeight() const
{
    int screenHeight = W4dManager::instance().sceneManager()->pDevice()->windowHeight();
    int controlPanelY = absoluteBoundary().minCorner().y();

    return screenHeight - controlPanelY;
}

void MachGuiControlPanel::setupDecalCoords()
{
    CB_DEPIMPL_ARRAY(Gui::Coord, decalCoord_);
    CB_DEPIMPL_ARRAY(GuiBitmap, decals_);

    decalCoord_[0] = Gui::Coord(0, 78) * Gui::uiScaleFactor();
    decalCoord_[1] = Gui::Coord(1, 175) * Gui::uiScaleFactor();
    decalCoord_[2] = Gui::Coord(127, 60) * Gui::uiScaleFactor();
    decalCoord_[3] = Gui::Coord(79, 155) * Gui::uiScaleFactor();
    decalCoord_[4] = Gui::Coord(0, 0) * Gui::uiScaleFactor();

    decalCoord_[5] = Gui::Coord(
        60 * Gui::uiScaleFactor(),
        getVisibleHeight() - (decals_[5].height() + 5 * Gui::uiScaleFactor()));
    decalCoord_[6] = Gui::Coord(
        5 * Gui::uiScaleFactor(),
        getVisibleHeight() - (decals_[6].height() + 3 * Gui::uiScaleFactor()));
    decalCoord_[7] = Gui::Coord(
        136 * Gui::uiScaleFactor(),
        getVisibleHeight() - (decals_[7].height() + 3 * Gui::uiScaleFactor()));
}

void MachGuiControlPanel::repositionNavigators(
    GuiDisplayable* pMachineNav,
    GuiDisplayable* pConstructionNav,
    GuiDisplayable* pSquadNav)
{
    Gui::Coord navTopLeft(1, getVisibleHeight() - MachGuiNavigatorBase::reqHeight());

    positionChildRelative(pMachineNav, navTopLeft);
    positionChildRelative(pConstructionNav, navTopLeft);
    positionChildRelative(pSquadNav, navTopLeft);
}

std::string MachGuiControlPanel::getDecalRootDirectory() const
{
    std::string retValue = "gui/misc/red/";

    if (MachLogRaces::instance().hasPCRace())
    {
        switch (MachLogRaces::instance().playerRace())
        {
            case MachPhys::RED:
                break;
            case MachPhys::GREEN:
                retValue = "gui/misc/green/";
                break;
            case MachPhys::BLUE:
                retValue = "gui/misc/blue/";
                break;
            case MachPhys::YELLOW:
                retValue = "gui/misc/yellow/";
                break;
                DEFAULT_ASSERT_BAD_CASE(MachLogRaces::instance().playerRace());
        }
    }

    return retValue;
}

void MachGuiControlPanel::setupDecalBitmaps()
{
    CB_DEPIMPL_ARRAY(GuiBitmap, decals_);

    // Setup decal bitmaps
    decals_[0] = MachGui::getScaledImage(getDecalRootDirectory() + "decal1.bmp");
    decals_[1] = MachGui::getScaledImage(getDecalRootDirectory() + "decal2.bmp");
    decals_[2] = MachGui::getScaledImage(getDecalRootDirectory() + "decal3.bmp");
    decals_[3] = MachGui::getScaledImage(getDecalRootDirectory() + "decal4.bmp");
    decals_[4] = MachGui::getScaledImage("gui/misc/decal5.bmp");
    decals_[5] = MachGui::getScaledImage("gui/misc/decal6.bmp");
    decals_[6] = MachGui::getScaledImage("gui/misc/decal7.bmp");
    decals_[7] = MachGui::getScaledImage("gui/misc/decal7.bmp");

    for (int loop = 0; loop < 8; ++loop)
    {
        decals_[loop].enableColourKeying();
    }
}

/* End CONTROLP.CPP *************************************************/
