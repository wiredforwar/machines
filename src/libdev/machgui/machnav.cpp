/*
 * M A C H N A V . C P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

//  Definitions of non-inline non-template methods and global functions

#include "machgui/machnav.hpp"
#include "machgui/ingame.hpp"
#include "machgui/cameras.hpp"
#include "machgui/gui.hpp"
#include "gui/icon.hpp"

#include "gui/painter.hpp"
#include "gui/event.hpp"
#include "gui/restring.hpp"
#include "render/TextOptions.hpp"
#include "system/pathname.hpp"
#include "machlog/machlog.hpp"
#include "machlog/races.hpp"
#include "machlog/machine.hpp"
#include "machlog/apc.hpp"
#include "machphys/machphys.hpp"
#include "device/keyboard.hpp"
#include "device/keytrans.hpp"
#include "machgui/internal/strings.hpp"
#include "ctl/list.hpp"
#include "ctl/pvector.hpp"
#include "machgui/internal/mgsndman.hpp"

//////
// ******************************************* Code for MachGuiDismissNavIcon *************************************
//////

class MachGuiNavRow;

class MachGuiDismissNavIcon : public GuiIcon
{
public:
    MachGuiDismissNavIcon(GuiDisplayable* pParent, const Gui::Coord& rel, MachInGameScreen* pInGameScreen)
        : GuiIcon(pParent, rel, MachGui::getScaledImage("gui/navigate/nonav.bmp"))
        , pInGameScreen_(pInGameScreen)
    {
    }

    ~MachGuiDismissNavIcon() override
    {
        // Intentionally Empty
    }

    void doBeDepressed(const GuiMouseEvent&) override
    {
        MachGuiSoundManager::instance().playSound("gui/sounds/igclick.wav");
    }

    void doBeReleased(const GuiMouseEvent&) override { pInGameScreen_->mainMenuOrSingleFactoryContext(); }

    void doHandleMouseEnterEvent(const GuiMouseEvent& mouseEvent) override
    {
        GuiIcon::doHandleMouseEnterEvent(mouseEvent);

        // Load the string for the command
        GuiResourceString prompt(IDS_CLOSE_NAV);

        // Set the cursor prompt
        pInGameScreen_->setCursorPromptText(prompt.asString());
    }

    void doHandleMouseExitEvent(const GuiMouseEvent& mouseEvent) override
    {
        // Clear the cursor prompt string
        pInGameScreen_->clearCursorPromptText();

        GuiIcon::doHandleMouseExitEvent(mouseEvent);
    }

private:
    MachInGameScreen* pInGameScreen_;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class MachGuiNavButtonWithCounter : public GuiBitmapButtonWithFilledBorder
// Canonical form revoked
{
public:
    MachGuiNavButtonWithCounter(
        GuiDisplayable* pParent,
        const Gui::Coord& rel,
        const SysPathName& bitmapFilename,
        GuiResourceString::Id stringId)
        : GuiBitmapButtonWithFilledBorder(
            pParent,
            rel,
            GuiBorderMetrics(1, 1, 1),
            GuiFilledBorderColours(Gui::BLACK(), Gui::LIGHTGREY(), Gui::DARKGREY(), Gui::GREEN()),
            MachGui::getScaledImage(bitmapFilename.pathname()),
            Gui::Coord(1, 1))
        , stringId_(stringId)
    {
    }

    ~MachGuiNavButtonWithCounter() override = default;

    void buttonNumber(size_t num)
    {
        if (numThisType_ == num)
        {
            return;
        }

        numThisType_ = num;

        if (numThisType_ == 0 && isNavButton() && ! isDisabled())
            disable();
        else if (isDisabled())
            enable();

        changed();
    }

    size_t buttonNumber() const { return numThisType_; }

    virtual bool isNavButton() { return false; }

    bool isEligableForVisibility() const override { return isSwitchedOn(); }

    bool isSwitchedOn() const { return isSwitchedOn_; }

    void isSwitchedOn(bool switchedOnStatus)
    {
        if (isSwitchedOn_ != switchedOnStatus)
        {
            isSwitchedOn_ = switchedOnStatus;

            if (isSwitchedOn_ == false)
                buttonNumber(0);
        }
    }

protected:
    void doDisplayInteriorEnabled(const Gui::Coord& abs) override
    {
        GuiBitmapButtonWithFilledBorder::doDisplayInteriorEnabled(abs);

        if (numThisType_ != 0)
        {
            Gui::Coord absCopy(abs);
            absCopy.y(absCopy.y() + 19 * Gui::uiScaleFactor());
            absCopy.x(absCopy.x() + width() - 2 * Gui::uiScaleFactor());

            char buffer[8];
            snprintf(buffer, sizeof(buffer), "%zu", numThisType_);

            // Render the text
            Gui::TextOptions options;
            options.setAlignment(Gui::AlignRight);
            constexpr Gui::Colour shadowColor(74.0 / 256.0, 74.0 / 256.0, 82.0 / 256.0);

            options.setColor(Gui::WHITE());
            options.setShadow(1, 1, shadowColor);

            GuiPainter::instance().drawText(absCopy, buffer, options, MachGui::navigationButtonNumberFontSize());
        }
    }

    GuiResourceString::Id stringId_;

private:
    MachGuiNavButtonWithCounter(const MachGuiNavButtonWithCounter&);
    MachGuiNavButtonWithCounter& operator=(const MachGuiNavButtonWithCounter&);
    bool operator==(const MachGuiNavButtonWithCounter&);

    void CLASS_INVARIANT;

    friend std::ostream& operator<<(std::ostream& o, const MachGuiNavButtonWithCounter& t);

    // Data members...
    size_t numThisType_ = 0;
    bool isSwitchedOn_ = false;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class MachGuiNavButton : public MachGuiNavButtonWithCounter
// Canonical form revoked
{
public:
    MachGuiNavButton(
        GuiDisplayable* pParent,
        MachLog::ObjectType ot,
        size_t subType,
        MachInGameScreen* pInGameScreen,
        MachGuiNavigatorBase* pNavigator,
        GuiResourceString::Id stringId);
    ~MachGuiNavButton() override { }

    static size_t reqWidth()
    {
        return 28 * Gui::uiScaleFactor(); // TODO : remove hardcoded value
    }
    static size_t reqHeight()
    {
        return 33 * Gui::uiScaleFactor(); // TODO : remove hardcoded value
    }

    void refresh()
    {
        if (selected_)
        {
            selected_ = false;
            border().isSelected(false);
            changed();
        }
    }
    void selected()
    {
        if (! selected_ && ! isDisabled())
        {
            selected_ = true;
            border().isSelected(true);
            changed();
        }
    }

    bool isNavButton() override { return true; }

    const GuiBitmap& getBitmap() const override
    {
        if (buttonNumber() > 0)
        {
            return MachGuiNavButtonWithCounter::getBitmap();
        }
        return greyBitmap_;
    }

    void updateBitmap() { bitmap(MachGui::getScaledImage(getBitmapFilename(objType_, subType_, false))); }

protected:
    void cycleThroughActors()
    {
        // Cycle to machine of the sub type indicated on button
        size_t cycleIndex = ++cycleIndex_;
        const MachActor* pSwitchToActor = nullptr;
        const MachActor* pFirstActor = nullptr;

        MachLogRaces& races = MachLogRaces::instance();
        MachPhys::Race playerRace = races.playerRace();

        MachLogRaces::Objects& allObjects = races.objects();

        MachInGameScreen::Actors actors; // List of actors being selected/deselected

        for (MachLogRaces::Objects::iterator itter = allObjects.begin();
             itter != allObjects.end() && ! pSwitchToActor;
             ++itter)
        {
            const MachActor* pActor = *itter;
            if (pActor->race() == playerRace && pActor->objectType() == objType_ && pActor->subType() == subType_)
            {
                if (pActor->objectIsMachine() && pActor->asMachine().insideAPC())
                {
                    pActor = &pActor->asMachine().APCImInside();
                }

                if (! pFirstActor)
                    pFirstActor = pActor;

                --cycleIndex;
                if (! cycleIndex)
                {
                    pSwitchToActor = pActor;
                }
            }
        }

        if (! pSwitchToActor)
        {
            cycleIndex_ = 1;
            if (pFirstActor)
                pInGameScreen_->cameras()->lookAt(*pFirstActor);
        }
        else
        {
            pInGameScreen_->cameras()->lookAt(*pSwitchToActor);
        }
    }

    void selectActors(const GuiMouseEvent& event)
    {
        // Select logic
        bool ctrlPressed = event.isCtrlPressed();

        // You must press ctrl to keep current selected items in corral and navigator
        if (! ctrlPressed)
        {
            pInGameScreen_->deselectAll();
            pNavigator_->deselectAll();
        }

        selected_ = ! selected_;
        border().isSelected(selected_);

        MachLogRaces& races = MachLogRaces::instance();
        MachPhys::Race playerRace = races.playerRace();

        MachLogRaces::Objects& allObjects = races.objects();

        MachInGameScreen::Actors actors; // List of actors being selected/deselected
        actors.reserve(64);

        for (MachLogRaces::Objects::iterator itter = allObjects.begin(); itter != allObjects.end(); ++itter)
        {
            MachActor* pActor = *itter;
            if (pActor->race() == playerRace && pActor->objectType() == objType_ && pActor->subType() == subType_)
            {
                if (selected_ || ! ctrlPressed)
                {
                    if (! pInGameScreen_->isSelected(*pActor)) // Make sure actor isn't already selected
                        actors.push_back(pActor);
                }
                else
                {
                    if (pInGameScreen_->isSelected(*pActor)) // Make sure actor is selected
                        actors.push_back(pActor);
                }
            }
        }

        // Indicates to the ingame screen whether the navigator should remain visible after
        // this operation.
        pNavigator_->remainVisible(ctrlPressed);

        if (selected_ || ! ctrlPressed) // Select actors if button is being selected
        {
            pInGameScreen_->select(actors);
        }
        else // Deselect actors if ctrl wasn't pressed or button is being deselected
        {
            pInGameScreen_->deselect(actors);
        }
    }

    void doBeDepressed(const GuiMouseEvent&) override
    {
        MachGuiSoundManager::instance().playSound("gui/sounds/igclick.wav");
    }

    void doBeReleased(const GuiMouseEvent& event) override
    {
        if (event.isAltPressed())
        {
            cycleThroughActors();
        }
        else
        {
            selectActors(event);
        }
    }

    void doHandleMouseEnterEvent(const GuiMouseEvent& mouseEvent) override
    {
        MachGuiNavButtonWithCounter::doHandleMouseEnterEvent(mouseEvent);

        // Load the string for the command
        GuiResourceString prompt(stringId_);
        GuiResourceString selectAll(IDS_SELECT_ALL, prompt.asString());

        // Set the cursor prompt
        pInGameScreen_->setCursorPromptText(selectAll.asString());
    }

    void doHandleMouseExitEvent(const GuiMouseEvent& mouseEvent) override
    {
        // Clear the cursor prompt string
        pInGameScreen_->clearCursorPromptText();

        MachGuiNavButtonWithCounter::doHandleMouseExitEvent(mouseEvent);
    }

    static std::string getBitmapFilename(MachLog::ObjectType objType, size_t subType, bool grey)
    {
        std::string retValue;

        switch (objType)
        {
            // Machines
            case MachLog::ADMINISTRATOR:
            case MachLog::AGGRESSOR:
            case MachLog::CONSTRUCTOR:
            case MachLog::GEO_LOCATOR:
            case MachLog::SPY_LOCATOR:
            case MachLog::TECHNICIAN:
            case MachLog::RESOURCE_CARRIER:
            case MachLog::APC:
                retValue = "gui/navigate/machines/";
                break;
            // Constructions
            case MachLog::POD:
            case MachLog::MISSILE_EMPLACEMENT:
            case MachLog::FACTORY:
            case MachLog::HARDWARE_LAB:
            case MachLog::SMELTER:
            case MachLog::MINE:
            case MachLog::GARRISON:
            case MachLog::BEACON:
                retValue = "gui/navigate/construc/";
                break;
        }

        if (grey || ! MachLogRaces::instance().hasPCRace())
        {
            retValue += "grey/";
        }
        else
        {
            switch (MachLogRaces::instance().playerRace())
            {
                case MachPhys::RED:
                    retValue += "red/";
                    break;
                case MachPhys::GREEN:
                    retValue += "green/";
                    break;
                case MachPhys::BLUE:
                    retValue += "blue/";
                    break;
                case MachPhys::YELLOW:
                    retValue += "yellow/";
                    break;
                    DEFAULT_ASSERT_BAD_CASE(MachLogRaces::instance().playerRace());
            }
        }

        switch (objType)
        {
            // Machines
            case MachLog::ADMINISTRATOR:
                switch ((MachPhys::AdministratorSubType)subType)
                {
                    case MachPhys::BOSS:
                        retValue += "nadb.bmp";
                        break;
                    case MachPhys::OVERSEER:
                        retValue += "nado.bmp";
                        break;
                    case MachPhys::COMMANDER:
                        retValue += "nadc.bmp";
                        break;
                        DEFAULT_ASSERT_BAD_CASE(subType);
                }
                break;
            case MachLog::AGGRESSOR:
                switch ((MachPhys::AggressorSubType)subType)
                {
                    case MachPhys::GRUNT:
                        retValue += "nagg.bmp";
                        break;
                    case MachPhys::ASSASSIN:
                        retValue += "naga.bmp";
                        break;
                    case MachPhys::BALLISTA:
                        retValue += "nagb.bmp";
                        break;
                    case MachPhys::KNIGHT:
                        retValue += "nagk.bmp";
                        break;
                    case MachPhys::NINJA:
                        retValue += "nagm.bmp";
                        break;
                        DEFAULT_ASSERT_BAD_CASE(subType);
                }
                break;
            case MachLog::CONSTRUCTOR:
                switch ((MachPhys::ConstructorSubType)subType)
                {
                    case MachPhys::DOZER:
                        retValue += "ncod.bmp";
                        break;
                    case MachPhys::BUILDER:
                        retValue += "ncob.bmp";
                        break;
                    case MachPhys::BEHEMOTH:
                        retValue += "ncom.bmp";
                        break;
                        DEFAULT_ASSERT_BAD_CASE(subType);
                }
                break;
            case MachLog::GEO_LOCATOR:
                retValue += "nlog.bmp";
                break;
            case MachLog::SPY_LOCATOR:
                retValue += "nlos.bmp";
                break;
            case MachLog::TECHNICIAN:
                switch ((MachPhys::TechnicianSubType)subType)
                {
                    case MachPhys::LAB_TECH:
                        retValue += "ntel.bmp";
                        break;
                    case MachPhys::TECH_BOY:
                        retValue += "ntet.bmp";
                        break;
                    case MachPhys::BRAIN_BOX:
                        retValue += "nteb.bmp";
                        break;
                        DEFAULT_ASSERT_BAD_CASE(subType);
                }
                break;
            case MachLog::RESOURCE_CARRIER:
                retValue += "ntrr.bmp";
                break;
            case MachLog::APC:
                retValue += "ntrp.bmp";
                break;

            // Constructions
            case MachLog::POD:
                retValue += "pod.bmp";
                break;
            case MachLog::MISSILE_EMPLACEMENT:
                switch ((MachPhys::MissileEmplacementSubType)subType)
                {
                    case MachPhys::TURRET:
                        retValue += "turret.bmp";
                        break;
                    case MachPhys::SENTRY:
                        retValue += "sentry.bmp";
                        break;
                    case MachPhys::LAUNCHER:
                        retValue += "launche.bmp";
                        break;
                    case MachPhys::ICBM:
                        retValue += "icbm.bmp";
                        break;
                        DEFAULT_ASSERT_BAD_CASE(subType);
                }
                break;
            case MachLog::FACTORY:
                switch ((MachPhys::FactorySubType)subType)
                {
                    case MachPhys::CIVILIAN:
                        retValue += "civfac.bmp";
                        break;
                    case MachPhys::MILITARY:
                        retValue += "milfac.bmp";
                        break;
                        DEFAULT_ASSERT_BAD_CASE(subType);
                }
                break;
            case MachLog::HARDWARE_LAB:
                switch ((MachPhys::HardwareLabSubType)subType)
                {
                    case MachPhys::LAB_CIVILIAN:
                        retValue += "civlab.bmp";
                        break;
                    case MachPhys::LAB_MILITARY:
                        retValue += "millab.bmp";
                        break;
                        DEFAULT_ASSERT_BAD_CASE(subType);
                }
                break;
            case MachLog::SMELTER:
                retValue += "smelt.bmp";
                break;
            case MachLog::MINE:
                retValue += "mine.bmp";
                break;
            case MachLog::GARRISON:
                retValue += "garris.bmp";
                break;
            case MachLog::BEACON:
                retValue += "beacon.bmp";
                break;
        }
        return retValue;
    }

#ifndef PRODUCTION
    const char* description() const override { return "MachGuiNavButton"; }
#endif

private:
    MachGuiNavButton(const MachGuiNavButton&);
    MachGuiNavButton& operator=(const MachGuiNavButton&);
    bool operator==(const MachGuiNavButton&);

    void CLASS_INVARIANT;

    friend std::ostream& operator<<(std::ostream& o, const MachGuiNavButton& t);

    // Data members...
    MachLog::ObjectType objType_;
    size_t subType_;
    MachInGameScreen* pInGameScreen_;
    MachGuiNavigatorBase* pNavigator_;
    bool selected_;
    GuiBitmap greyBitmap_;
    size_t cycleIndex_;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class MachGuiSelectAllNavButton : public MachGuiNavButtonWithCounter
// Canonical form revoked
{
public:
    MachGuiSelectAllNavButton(
        GuiDisplayable* pParent,
        MachLog::ObjectType ot,
        MachInGameScreen* pInGameScreen,
        MachGuiNavigatorBase* pNavigator,
        MachGuiNavRow* pNavRow,
        GuiResourceString::Id stringId)
        : MachGuiNavButtonWithCounter(pParent, Gui::Coord(0, 0), SysPathName("gui/navigate/all.bmp"), stringId)
        , objectType_(ot)
        , pInGameScreen_(pInGameScreen)
        , pNavigator_(pNavigator)
        , pNavRow_(pNavRow)
        , cycleIndex_(0)
    {
        isSwitchedOn(true);
    }

    ~MachGuiSelectAllNavButton() override { }

    static size_t reqWidth()
    {
        return 20 * Gui::uiScaleFactor(); // TODO : remove hardcoded value
    }
    static size_t reqHeight()
    {
        return 33 * Gui::uiScaleFactor(); // TODO : remove hardcoded value
    }

protected:
    void cycleThroughActors()
    {
        // Cycle to machine of the sub type indicated on button
        size_t cycleIndex = ++cycleIndex_;
        MachActor* pSwitchToActor = nullptr;
        MachActor* pFirstActor = nullptr;

        MachLogRaces& races = MachLogRaces::instance();
        MachPhys::Race playerRace = races.playerRace();

        MachLogRaces::Objects& allObjects = races.objects();

        MachInGameScreen::Actors actors; // List of actors being selected/deselected

        for (MachLogRaces::Objects::iterator itter = allObjects.begin();
             itter != allObjects.end() && ! pSwitchToActor;
             ++itter)
        {
            MachActor* pActor = *itter;
            if (pActor->race() == playerRace)
            {
                if ((pActor->objectType() == objectType_)
                    || (pActor->objectType() == MachLog::SPY_LOCATOR && objectType_ == MachLog::GEO_LOCATOR)
                    || // GEO and SPY share select all button
                    (pActor->objectType() == MachLog::APC && objectType_ == MachLog::RESOURCE_CARRIER)
                    || // APC and RES CARRIER share select all button
                    (pActor->objectType() == MachLog::MINE && objectType_ == MachLog::SMELTER)
                    || // MINE and SMELTER share select all button
                    (pActor->objectType() == MachLog::BEACON
                     && objectType_ == MachLog::POD)) // POD and BEACON share select all button
                {
                    if (! pFirstActor)
                        pFirstActor = pActor;

                    --cycleIndex;
                    if (! cycleIndex)
                    {
                        pSwitchToActor = pActor;
                    }
                }
            }
        }

        if (! pSwitchToActor)
        {
            cycleIndex_ = 1;
            if (pFirstActor)
                pInGameScreen_->cameras()->lookAt(*pFirstActor);
        }
        else
        {
            pInGameScreen_->cameras()->lookAt(*pSwitchToActor);
        }
    }

    void doBeDepressed(const GuiMouseEvent&) override
    {
        MachGuiSoundManager::instance().playSound("gui/sounds/igclick.wav");
    }

    void doBeReleased(const GuiMouseEvent&) override;

    void doHandleMouseEnterEvent(const GuiMouseEvent& mouseEvent) override
    {
        MachGuiNavButtonWithCounter::doHandleMouseEnterEvent(mouseEvent);

        // Load the string for the command
        GuiResourceString prompt(stringId_);
        GuiResourceString selectAll(IDS_SELECT_ALL, prompt.asString());

        // Set the cursor prompt
        pInGameScreen_->setCursorPromptText(selectAll.asString());
    }

    void doHandleMouseExitEvent(const GuiMouseEvent& mouseEvent) override
    {
        // Clear the cursor prompt string
        pInGameScreen_->clearCursorPromptText();

        MachGuiNavButtonWithCounter::doHandleMouseExitEvent(mouseEvent);
    }

#ifndef PRODUCTION
    const char* description() const override { return "MachGuiSelectAllNavButton"; }
#endif

private:
    MachGuiSelectAllNavButton(const MachGuiSelectAllNavButton&);
    MachGuiSelectAllNavButton& operator=(const MachGuiSelectAllNavButton&);
    bool operator==(const MachGuiSelectAllNavButton&);

    void CLASS_INVARIANT;

    friend std::ostream& operator<<(std::ostream& o, const MachGuiSelectAllNavButton& t);

    // Data members...
    MachLog::ObjectType objectType_;
    MachInGameScreen* pInGameScreen_;
    MachGuiNavigatorBase* pNavigator_;
    MachGuiNavRow* pNavRow_;
    size_t cycleIndex_;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//////
// ******************************************* Code for MachGuiNavButton *************************************
//////

#include "gui/iconseq.hpp"

class MachGuiNavRow : public GuiIconSequence
// Canonical form revoked
{
public:
    MachGuiNavRow(GuiDisplayable* pParent, const Gui::Coord& rel, size_t nIcons)
        : GuiIconSequence(
            pParent,
            Gui::Box(
                rel.x(),
                rel.y(),
                rel.x() + MachGuiSelectAllNavButton::reqWidth() + (nIcons * MachGuiNavButton::reqWidth()),
                rel.y() + MachGuiSelectAllNavButton::reqHeight()),
            MachGuiNavRow::nCoordsSideBySide(nIcons))

    {
    }

    ~MachGuiNavRow() override { }

    void selectAll()
    {
        // Highlight all children with a green surround ( except select all buttons ).
        for (Children::iterator itter = children().begin(); itter != children().end(); ++itter)
        {
            GuiDisplayable* pChild = *itter;
            MachGuiNavButtonWithCounter* pNavButtonCount = _REINTERPRET_CAST(MachGuiNavButtonWithCounter*, pChild);

            if (pNavButtonCount->isNavButton())
            {
                MachGuiNavButton* pNavButton = _REINTERPRET_CAST(MachGuiNavButton*, pNavButtonCount);
                pNavButton->selected();
            }
        }
    }

    static GuiIconSequence::Coords nCoordsSideBySide(size_t nIcons)
    {
        GuiIconSequence::Coords coords;
        coords.reserve(nIcons + 1);

        Gui::Coord selectAllCoord(0, 0);
        coords.push_back(selectAllCoord);

        for (size_t n = 0; n < nIcons; ++n)
        {
            Gui::Coord nextCoord(MachGuiSelectAllNavButton::reqWidth() + (n * MachGuiNavButton::reqWidth()), 0);
            coords.push_back(nextCoord);
        }

        return coords;
    }

    void update()
    {
        // call protected method from base class
        GuiIconSequence::update();
        changed();
    }

protected:
#ifndef PRODUCTION
    const char* description() const override { return "MachGuiNavRow"; }
#endif

private:
    MachGuiNavRow(const MachGuiNavRow&);
    MachGuiNavRow& operator=(const MachGuiNavRow&);
    bool operator==(const MachGuiNavRow&);

    void CLASS_INVARIANT;

    friend std::ostream& operator<<(std::ostream& o, const MachGuiNavRow& t);
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//////
// ******************************************* Code for MachGuiNavButton *************************************
//////

MachGuiNavButton::MachGuiNavButton(
    GuiDisplayable* pParent,
    MachLog::ObjectType ot,
    size_t subType,
    MachInGameScreen* pInGameScreen,
    MachGuiNavigatorBase* pNavigator,
    GuiResourceString::Id stringId)
    : MachGuiNavButtonWithCounter(pParent, Gui::Coord(0, 0), getBitmapFilename(ot, subType, false), stringId)
    , objType_(ot)
    , subType_(subType)
    , pInGameScreen_(pInGameScreen)
    , pNavigator_(pNavigator)
    , selected_(false)
    , cycleIndex_(0)
{
    greyBitmap_ = MachGui::getScaledImage(getBitmapFilename(ot, subType, true));
    isVisible(false);
    disable();
}

//////
// ******************************************* Code for MachGuiSelectAllNavButton *************************************
//////

// virtual
void MachGuiSelectAllNavButton::doBeReleased(const GuiMouseEvent& event)
{
    if (event.isAltPressed())
    {
        cycleThroughActors();
    }
    else
    {
        // Select logic
        bool ctrlPressed = event.isCtrlPressed();

        // You must press ctrl to keep current selected items in corral and navigator
        if (! ctrlPressed)
        {
            pInGameScreen_->deselectAll();
            pNavigator_->deselectAll();
        }

        // Highlight all navigator buttons in this row
        pNavRow_->selectAll();

        MachLogRaces& races = MachLogRaces::instance();
        MachPhys::Race playerRace = races.playerRace();

        MachLogRaces::Objects& allObjects = races.objects();

        MachInGameScreen::Actors actors; // List of actors to be selected

        // Iterate through machines selecting one in players race that match the correct object type
        for (MachLogRaces::Objects::iterator itter = allObjects.begin(); itter != allObjects.end(); ++itter)
        {
            MachActor* pActor = *itter;
            if (pActor->race() == playerRace)
            {
                if ((pActor->objectType() == objectType_)
                    || (pActor->objectType() == MachLog::SPY_LOCATOR && objectType_ == MachLog::GEO_LOCATOR)
                    || // GEO and SPY share select all button
                    (pActor->objectType() == MachLog::APC && objectType_ == MachLog::RESOURCE_CARRIER)
                    || // APC and RES CARRIER share select all button
                    (pActor->objectType() == MachLog::MINE && objectType_ == MachLog::SMELTER)
                    || // MINE and SMELTER share select all button
                    (pActor->objectType() == MachLog::BEACON
                     && objectType_ == MachLog::POD)) // POD and BEACON share select all button
                {
                    if (! pInGameScreen_->isSelected(*pActor))
                        actors.push_back(pActor);
                }
            }
        }

        // Indicates to the ingame screen whether the navigator should remain visible after
        // this operation.
        pNavigator_->remainVisible(true);
        pInGameScreen_->select(actors);
    }
}

/////////////////////////////////////////////////////////////////////////////

//////
// ******************************************* Code for MachGuiConstructionNavigator
// *************************************
//////

#include "machlog/constree.hpp"

MachGuiConstructionNavigator::MachGuiConstructionNavigator(
    GuiDisplayable* pParent,
    const Gui::Coord& rel,
    MachInGameScreen* pInGameScreen)
    : MachGuiNavigatorBase(
        pParent,
        Gui::Box(rel.x(), rel.y(), rel.x() + reqWidth(), rel.y() + reqHeight()),
        pInGameScreen)
    , MachLogNotifiable(MachPhys::RED)
    , // What race is the PC controller controlling?
    pConstructionTree_(nullptr)
{
    pKeyTranslator_->addTranslation(DevKeyToCommand(
        Device::KeyCode::KEY_S,
        SELECT_ALL,
        DevKeyToCommand::RELEASED,
        DevKeyToCommand::PRESSED,
        DevKeyToCommand::RELEASED));
    pKeyTranslator_->addTranslation(DevKeyToCommand(
        Device::KeyCode::KEY_S,
        SELECT_ALL_KEEP_SELECTION,
        DevKeyToCommand::PRESSED,
        DevKeyToCommand::PRESSED,
        DevKeyToCommand::RELEASED));
    pKeyTranslator_->addTranslation(DevKeyToCommand(
        Device::KeyCode::KEY_S,
        VIEW_NEXT,
        DevKeyToCommand::RELEASED,
        DevKeyToCommand::RELEASED,
        DevKeyToCommand::PRESSED));
    pKeyTranslator_->initEventQueue();

    buildConstructIcons();

    TEST_INVARIANT;
}

/////////////////////////////////////////////////////////////////////////////

MachGuiConstructionNavigator::~MachGuiConstructionNavigator()
{
    TEST_INVARIANT;
}

/////////////////////////////////////////////////////////////////////////////

void MachGuiConstructionNavigator::loadGame()
{
    PRE(pConstructionTree_ == nullptr);

    changeRace(MachLogRaces::instance().playerRace());

    pConstructionTree_ = &MachLogRaces::instance().constructionTree();
    pConstructionTree_->addMe(this);
    pPodButton_->isSwitchedOn(true);
    updateConstructionIconBitmaps();
    refreshConstructIcons();
}

void MachGuiConstructionNavigator::unloadGame()
{
    PRE(pConstructionTree_ != nullptr);

    pConstructionTree_->removeMe(this);
    pConstructionTree_ = nullptr;
    removeAllIcons();
}

void MachGuiConstructionNavigator::buildConstructIcons()
{
    pRow1_ = new MachGuiNavRow(this, Gui::Coord(0, 0), 2);

    pPodSelectAll_
        = new MachGuiSelectAllNavButton(pRow1_, MachLog::POD, pInGameScreen_, this, pRow1_, IDS_PODSBEACONS);
    pPodButton_ = new MachGuiNavButton(pRow1_, MachLog::POD, 0, pInGameScreen_, this, IDS_PODS);
    pBeaconButton_ = new MachGuiNavButton(pRow1_, MachLog::BEACON, 0, pInGameScreen_, this, IDS_BEACONS);

    pRow2_ = new MachGuiNavRow(this, Gui::Coord(0, MachGuiNavButton::reqHeight()), 4);

    pMissileSelectAll_ = new MachGuiSelectAllNavButton(
        pRow2_,
        MachLog::MISSILE_EMPLACEMENT,
        pInGameScreen_,
        this,
        pRow2_,
        IDS_MISSILE_EMPLACEMENTS);
    pTurretButton_ = new 
        MachGuiNavButton(pRow2_, MachLog::MISSILE_EMPLACEMENT, MachPhys::TURRET, pInGameScreen_, this, IDS_TURRETS);
    pSentryButton_ = new 
        MachGuiNavButton(pRow2_, MachLog::MISSILE_EMPLACEMENT, MachPhys::SENTRY, pInGameScreen_, this, IDS_SENTRIES);
    pLauncherButton_ = new MachGuiNavButton(
        pRow2_,
        MachLog::MISSILE_EMPLACEMENT,
        MachPhys::LAUNCHER,
        pInGameScreen_,
        this,
        IDS_LAUNCHERS);
    pICBMButton_
        = new MachGuiNavButton(pRow2_, MachLog::MISSILE_EMPLACEMENT, MachPhys::ICBM, pInGameScreen_, this, IDS_ICBMS);

    pRow3_ = new MachGuiNavRow(this, Gui::Coord(0, MachGuiNavButton::reqHeight() * 2), 2);

    pFactorySelectAll_
        = new MachGuiSelectAllNavButton(pRow3_, MachLog::FACTORY, pInGameScreen_, this, pRow3_, IDS_FACTORIES);
    pCivFacButton_
        = new MachGuiNavButton(pRow3_, MachLog::FACTORY, MachPhys::CIVILIAN, pInGameScreen_, this, IDS_FACS_CIVILIAN);
    pMilFacButton_
        = new MachGuiNavButton(pRow3_, MachLog::FACTORY, MachPhys::MILITARY, pInGameScreen_, this, IDS_FACS_MILITARY);

    pRow4_ = new MachGuiNavRow(this, Gui::Coord(0, MachGuiNavButton::reqHeight() * 3), 2);

    pLabSelectAll_
        = new MachGuiSelectAllNavButton(pRow4_, MachLog::HARDWARE_LAB, pInGameScreen_, this, pRow4_, IDS_LABS);
    pCivLabButton_ = new MachGuiNavButton(
        pRow4_,
        MachLog::HARDWARE_LAB,
        MachPhys::LAB_CIVILIAN,
        pInGameScreen_,
        this,
        IDS_LABS_CIVILIAN);
    pMilLabButton_ = new MachGuiNavButton(
        pRow4_,
        MachLog::HARDWARE_LAB,
        MachPhys::LAB_MILITARY,
        pInGameScreen_,
        this,
        IDS_LABS_MILITARY);

    pRow5_ = new MachGuiNavRow(this, Gui::Coord(0, MachGuiNavButton::reqHeight() * 4), 2);

    pSmelterSelectAll_
        = new MachGuiSelectAllNavButton(pRow5_, MachLog::SMELTER, pInGameScreen_, this, pRow5_, IDS_SMELTERSMINES);
    pSmelterButton_ = new MachGuiNavButton(pRow5_, MachLog::SMELTER, 0, pInGameScreen_, this, IDS_SMELTERS);
    pMineButton_ = new MachGuiNavButton(pRow5_, MachLog::MINE, 0, pInGameScreen_, this, IDS_MINES);

    pRow6_ = new MachGuiNavRow(this, Gui::Coord(0, MachGuiNavButton::reqHeight() * 5), 1);

    pGarrisonSelectAll_
        = new MachGuiSelectAllNavButton(pRow6_, MachLog::GARRISON, pInGameScreen_, this, pRow6_, IDS_GARRISONS);
    pGarrisonButton_ = new MachGuiNavButton(pRow6_, MachLog::GARRISON, 0, pInGameScreen_, this, IDS_GARRISONS);

    new MachGuiDismissNavIcon(
        this,
        Gui::Coord(MachGuiNavButton::reqWidth() * 5, MachGuiNavButton::reqHeight() * 5),
        pInGameScreen_);

    TEST_INVARIANT;
}

/////////////////////////////////////////////////////////////////////////////

void MachGuiConstructionNavigator::refreshConstructIcons()
{
    PRE(pConstructionTree_ != nullptr);

    if (pConstructionTree_->activated(race(), MachLog::MISSILE_EMPLACEMENT, MachPhys::TURRET))
        pTurretButton_->isSwitchedOn(true);

    if (pConstructionTree_->activated(race(), MachLog::MISSILE_EMPLACEMENT, MachPhys::SENTRY))
        pSentryButton_->isSwitchedOn(true);

    if (pConstructionTree_->activated(race(), MachLog::MISSILE_EMPLACEMENT, MachPhys::LAUNCHER))
        pLauncherButton_->isSwitchedOn(true);

    if (pConstructionTree_->activated(race(), MachLog::MISSILE_EMPLACEMENT, MachPhys::ICBM))
        pICBMButton_->isSwitchedOn(true);

    if (pConstructionTree_->activated(race(), MachLog::FACTORY, MachPhys::CIVILIAN))
        pCivFacButton_->isSwitchedOn(true);

    if (pConstructionTree_->activated(race(), MachLog::FACTORY, MachPhys::MILITARY))
        pMilFacButton_->isSwitchedOn(true);

    if (pConstructionTree_->activated(race(), MachLog::HARDWARE_LAB, MachPhys::LAB_CIVILIAN))
        pCivLabButton_->isSwitchedOn(true);

    if (pConstructionTree_->activated(race(), MachLog::HARDWARE_LAB, MachPhys::LAB_MILITARY))
        pMilLabButton_->isSwitchedOn(true);

    if (pConstructionTree_->activated(race(), MachLog::SMELTER))
        pSmelterButton_->isSwitchedOn(true);

    if (pConstructionTree_->activated(race(), MachLog::MINE))
        pMineButton_->isSwitchedOn(true);

    if (pConstructionTree_->activated(race(), MachLog::GARRISON))
        pGarrisonButton_->isSwitchedOn(true);

    if (pConstructionTree_->activated(race(), MachLog::BEACON))
        pBeaconButton_->isSwitchedOn(true);

    pRow1_->update();
    pRow2_->update();
    pRow3_->update();
    pRow4_->update();
    pRow5_->update();
    pRow6_->update();

    update();

    TEST_INVARIANT;
}

/////////////////////////////////////////////////////////////////////////////

void MachGuiConstructionNavigator::removeAllIcons()
{
    pTurretButton_->isSwitchedOn(false);
    pSentryButton_->isSwitchedOn(false);
    pLauncherButton_->isSwitchedOn(false);
    pICBMButton_->isSwitchedOn(false);
    pCivFacButton_->isSwitchedOn(false);
    pMilFacButton_->isSwitchedOn(false);
    pCivLabButton_->isSwitchedOn(false);
    pMilLabButton_->isSwitchedOn(false);
    pSmelterButton_->isSwitchedOn(false);
    pMineButton_->isSwitchedOn(false);
    pGarrisonButton_->isSwitchedOn(false);
    pBeaconButton_->isSwitchedOn(false);

    TEST_INVARIANT;
}

/////////////////////////////////////////////////////////////////////////////

void MachGuiConstructionNavigator::update()
{
    if (! isVisible())
    {
        updateReq_ = true;
        return;
    }

    updateReq_ = false;

    // Put numbers on buttons
    size_t nPodCount = 0;
    size_t nTurretCount = 0;
    size_t nSentryCount = 0;
    size_t nLauncherCount = 0;
    size_t nICBMCount = 0;
    size_t nCivFacCount = 0;
    size_t nMilFacCount = 0;
    size_t nCivLabCount = 0;
    size_t nMilLabCount = 0;
    size_t nSmelterCount = 0;
    size_t nMineCount = 0;
    size_t nGarrisonCount = 0;
    size_t nBeaconCount = 0;

    // Iterate through all the machines counting the different types
    MachLogRaces& races = MachLogRaces::instance();
    MachPhys::Race playerRace = races.playerRace();

    MachLogRaces::Objects& allObjects = races.objects();

    for (MachLogRaces::Objects::iterator itter = allObjects.begin(); itter != allObjects.end(); ++itter)
    {
        MachActor* pActor = *itter;
        if (pActor->race() == playerRace)
        {
            switch (pActor->objectType())
            {
                case MachLog::POD:
                    nPodCount++;
                    break;
                case MachLog::MISSILE_EMPLACEMENT:
                    switch ((MachPhys::MissileEmplacementSubType)pActor->subType())
                    {
                        case MachPhys::TURRET:
                            nTurretCount++;
                            break;
                        case MachPhys::SENTRY:
                            nSentryCount++;
                            break;
                        case MachPhys::LAUNCHER:
                            nLauncherCount++;
                            break;
                        case MachPhys::ICBM:
                            nICBMCount++;
                            break;
                            DEFAULT_ASSERT_BAD_CASE(pActor->subType());
                    }
                    break;
                case MachLog::FACTORY:
                    switch ((MachPhys::FactorySubType)pActor->subType())
                    {
                        case MachPhys::CIVILIAN:
                            nCivFacCount++;
                            break;
                        case MachPhys::MILITARY:
                            nMilFacCount++;
                            break;
                            DEFAULT_ASSERT_BAD_CASE(pActor->subType());
                    }
                    break;
                case MachLog::HARDWARE_LAB:
                    switch ((MachPhys::HardwareLabSubType)pActor->subType())
                    {
                        case MachPhys::LAB_CIVILIAN:
                            nCivLabCount++;
                            break;
                        case MachPhys::LAB_MILITARY:
                            nMilLabCount++;
                            break;
                            DEFAULT_ASSERT_BAD_CASE(pActor->subType());
                    }
                    break;
                case MachLog::SMELTER:
                    nSmelterCount++;
                    break;
                case MachLog::MINE:
                    nMineCount++;
                    break;
                case MachLog::GARRISON:
                    nGarrisonCount++;
                    break;
                case MachLog::BEACON:
                    nBeaconCount++;
                    break;
            }
        }
    }

    // Update numbers on buttons
    updateButtonNumber(pPodButton_, nPodCount, pRow1_);
    updateButtonNumber(pBeaconButton_, nBeaconCount, pRow1_);
    updateButtonNumber(pTurretButton_, nTurretCount, pRow2_);
    updateButtonNumber(pSentryButton_, nSentryCount, pRow2_);
    updateButtonNumber(pLauncherButton_, nLauncherCount, pRow2_);
    updateButtonNumber(pICBMButton_, nICBMCount, pRow2_);
    updateButtonNumber(pCivFacButton_, nCivFacCount, pRow3_);
    updateButtonNumber(pMilFacButton_, nMilFacCount, pRow3_);
    updateButtonNumber(pCivLabButton_, nCivLabCount, pRow4_);
    updateButtonNumber(pMilLabButton_, nMilLabCount, pRow4_);
    updateButtonNumber(pSmelterButton_, nSmelterCount, pRow5_);
    updateButtonNumber(pMineButton_, nMineCount, pRow5_);
    updateButtonNumber(pGarrisonButton_, nGarrisonCount, pRow6_);

    // Update select all buttons
    pPodSelectAll_->buttonNumber(nPodCount + nBeaconCount);
    pMissileSelectAll_->buttonNumber(nTurretCount + nSentryCount + nLauncherCount + nICBMCount);
    pFactorySelectAll_->buttonNumber(nCivFacCount + nMilFacCount);
    pLabSelectAll_->buttonNumber(nCivLabCount + nMilLabCount);
    pSmelterSelectAll_->buttonNumber(nSmelterCount + nMineCount);
    pGarrisonSelectAll_->buttonNumber(nGarrisonCount);

    TEST_INVARIANT;
}

/////////////////////////////////////////////////////////////////////////////

// virtual
void MachGuiConstructionNavigator::notifiableBeNotified()
{
    refreshConstructIcons();
}

/////////////////////////////////////////////////////////////////////////////

// virtual
void MachGuiConstructionNavigator::deselectAll()
{
    pPodButton_->refresh();
    pTurretButton_->refresh();
    pSentryButton_->refresh();
    pLauncherButton_->refresh();
    pICBMButton_->refresh();
    pCivFacButton_->refresh();
    pMilFacButton_->refresh();
    pCivLabButton_->refresh();
    pMilLabButton_->refresh();
    pSmelterButton_->refresh();
    pMineButton_->refresh();
    pGarrisonButton_->refresh();
    pBeaconButton_->refresh();

    TEST_INVARIANT;
}

/////////////////////////////////////////////////////////////////////////////

void MachGuiConstructionNavigator::CLASS_INVARIANT
{
    INVARIANT(this != nullptr);
}

/////////////////////////////////////////////////////////////////////////////

std::ostream& operator<<(std::ostream& o, const MachGuiConstructionNavigator& t)
{

    o << "MachGuiConstructionNavigator " << static_cast<const void*>(&t) << " start" << std::endl;
    o << "MachGuiConstructionNavigator " << static_cast<const void*>(&t) << " end" << std::endl;

    return o;
}

/////////////////////////////////////////////////////////////////////////////

//////
// ******************************************* Code for MachGuiMachineNavigator *************************************
//////

#include "machlog/restree.hpp"

MachGuiMachineNavigator::MachGuiMachineNavigator(
    GuiDisplayable* pParent,
    const Gui::Coord& rel,
    MachInGameScreen* pInGameScreen)
    : MachGuiNavigatorBase(
        pParent,
        Gui::Box(rel.x(), rel.y(), rel.x() + reqWidth(), rel.y() + reqHeight()),
        pInGameScreen)
    , MachLogNotifiable(MachPhys::RED)
    , // What race is the PC controller controlling?
    pResearchTree_(nullptr)
{
    pKeyTranslator_->addTranslation(DevKeyToCommand(
        Device::KeyCode::KEY_A,
        SELECT_ALL,
        DevKeyToCommand::RELEASED,
        DevKeyToCommand::PRESSED,
        DevKeyToCommand::RELEASED));
    pKeyTranslator_->addTranslation(DevKeyToCommand(
        Device::KeyCode::KEY_A,
        SELECT_ALL_KEEP_SELECTION,
        DevKeyToCommand::PRESSED,
        DevKeyToCommand::PRESSED,
        DevKeyToCommand::RELEASED));
    pKeyTranslator_->addTranslation(DevKeyToCommand(
        Device::KeyCode::KEY_A,
        VIEW_NEXT,
        DevKeyToCommand::RELEASED,
        DevKeyToCommand::RELEASED,
        DevKeyToCommand::PRESSED));
    pKeyTranslator_->initEventQueue();

    buildMachineIcons();

    TEST_INVARIANT;
}

/////////////////////////////////////////////////////////////////////////////

MachGuiMachineNavigator::~MachGuiMachineNavigator()
{
    TEST_INVARIANT;
}

/////////////////////////////////////////////////////////////////////////////

void MachGuiMachineNavigator::loadGame()
{
    PRE(pResearchTree_ == nullptr);

    changeRace(MachLogRaces::instance().playerRace());

    pResearchTree_ = &MachLogRaces::instance().researchTree();
    pResearchTree_->addMe(this);
    updateMachineIconBitmaps();
    refreshMachineIcons();
}

void MachGuiMachineNavigator::unloadGame()
{
    PRE(pResearchTree_ != nullptr);

    pResearchTree_->removeMe(this);
    pResearchTree_ = nullptr;
    removeAllIcons();
}

void MachGuiMachineNavigator::buildMachineIcons()
{

    pRow1_ = new MachGuiNavRow(this, Gui::Coord(0, 0), 5);

    pAggressorSelectAll_
        = new MachGuiSelectAllNavButton(pRow1_, MachLog::AGGRESSOR, pInGameScreen_, this, pRow1_, IDS_AGGRESSORS);
    pGruntButton_
        = new MachGuiNavButton(pRow1_, MachLog::AGGRESSOR, MachPhys::GRUNT, pInGameScreen_, this, IDS_GRUNTS);
    pAssassinButton_
        = new MachGuiNavButton(pRow1_, MachLog::AGGRESSOR, MachPhys::ASSASSIN, pInGameScreen_, this, IDS_ASSASSINS);
    pBallistaButton_
        = new MachGuiNavButton(pRow1_, MachLog::AGGRESSOR, MachPhys::BALLISTA, pInGameScreen_, this, IDS_BALLISTAS);
    pKnightButton_
        = new MachGuiNavButton(pRow1_, MachLog::AGGRESSOR, MachPhys::KNIGHT, pInGameScreen_, this, IDS_KNIGHTS);
    pNinjaButton_
        = new MachGuiNavButton(pRow1_, MachLog::AGGRESSOR, MachPhys::NINJA, pInGameScreen_, this, IDS_NINJAS);

    pRow2_ = new MachGuiNavRow(this, Gui::Coord(0, MachGuiNavButton::reqHeight()), 3);

    pAdministratorSelectAll_ = new 
        MachGuiSelectAllNavButton(pRow2_, MachLog::ADMINISTRATOR, pInGameScreen_, this, pRow2_, IDS_ADMINISTRATORS);
    pBossButton_
        = new MachGuiNavButton(pRow2_, MachLog::ADMINISTRATOR, MachPhys::BOSS, pInGameScreen_, this, IDS_BOSSES);
    pOverseerButton_ = new 
        MachGuiNavButton(pRow2_, MachLog::ADMINISTRATOR, MachPhys::OVERSEER, pInGameScreen_, this, IDS_OVERSEERS);
    pCommanderButton_ = new 
        MachGuiNavButton(pRow2_, MachLog::ADMINISTRATOR, MachPhys::COMMANDER, pInGameScreen_, this, IDS_COMMANDERS);

    pRow3_ = new MachGuiNavRow(this, Gui::Coord(0, MachGuiNavButton::reqHeight() * 2), 3);

    pTechSelectAll_
        = new MachGuiSelectAllNavButton(pRow3_, MachLog::TECHNICIAN, pInGameScreen_, this, pRow3_, IDS_TECHNICIANS);
    pLabTechButton_
        = new MachGuiNavButton(pRow3_, MachLog::TECHNICIAN, MachPhys::LAB_TECH, pInGameScreen_, this, IDS_LAB_TECHS);
    pTechBoyButton_
        = new MachGuiNavButton(pRow3_, MachLog::TECHNICIAN, MachPhys::TECH_BOY, pInGameScreen_, this, IDS_TECH_BOYS);
    pBrainBoxButton_ = new 
        MachGuiNavButton(pRow3_, MachLog::TECHNICIAN, MachPhys::BRAIN_BOX, pInGameScreen_, this, IDS_BRAIN_BOXES);

    pRow4_ = new MachGuiNavRow(this, Gui::Coord(0, MachGuiNavButton::reqHeight() * 3), 3);

    pConstructorSelectAll_
        = new MachGuiSelectAllNavButton(pRow4_, MachLog::CONSTRUCTOR, pInGameScreen_, this, pRow4_, IDS_CONSTRUCTORS);
    pDozerButton_
        = new MachGuiNavButton(pRow4_, MachLog::CONSTRUCTOR, MachPhys::DOZER, pInGameScreen_, this, IDS_DOZERS);
    pBuilderButton_
        = new MachGuiNavButton(pRow4_, MachLog::CONSTRUCTOR, MachPhys::BUILDER, pInGameScreen_, this, IDS_BUILDERS);
    pBehemothButton_
        = new MachGuiNavButton(pRow4_, MachLog::CONSTRUCTOR, MachPhys::BEHEMOTH, pInGameScreen_, this, IDS_BEHEMOTHS);

    pRow5_ = new MachGuiNavRow(this, Gui::Coord(0, MachGuiNavButton::reqHeight() * 4), 2);

    pLocatorSelectAll_
        = new MachGuiSelectAllNavButton(pRow5_, MachLog::GEO_LOCATOR, pInGameScreen_, this, pRow5_, IDS_LOCATORS);
    pGeoLocatorButton_
        = new MachGuiNavButton(pRow5_, MachLog::GEO_LOCATOR, 0, pInGameScreen_, this, IDS_GEO_LOCATORS);
    pSpyLocatorButton_
        = new MachGuiNavButton(pRow5_, MachLog::SPY_LOCATOR, 0, pInGameScreen_, this, IDS_SPY_LOCATORS);

    pRow6_ = new MachGuiNavRow(this, Gui::Coord(0, MachGuiNavButton::reqHeight() * 5), 2);

    pTransportSelectAll_ = new 
        MachGuiSelectAllNavButton(pRow6_, MachLog::RESOURCE_CARRIER, pInGameScreen_, this, pRow6_, IDS_CARRIERS);
    pResourceCarrierButton_
        = new MachGuiNavButton(pRow6_, MachLog::RESOURCE_CARRIER, 0, pInGameScreen_, this, IDS_RESOURCE_CARRIERS);
    pAPCButton_ = new MachGuiNavButton(pRow6_, MachLog::APC, 0, pInGameScreen_, this, IDS_APCS);

    new MachGuiDismissNavIcon(
        this,
        Gui::Coord(MachGuiNavButton::reqWidth() * 5, MachGuiNavButton::reqHeight() * 5),
        pInGameScreen_);
}

/////////////////////////////////////////////////////////////////////////////

// virtual
void MachGuiMachineNavigator::notifiableBeNotified()
{
    refreshMachineIcons();
}

/////////////////////////////////////////////////////////////////////////////

void MachGuiMachineNavigator::refreshMachineIcons()
{
    if (pResearchTree_->activated(race(), MachLog::AGGRESSOR, MachPhys::GRUNT))
        pGruntButton_->isSwitchedOn(true);

    if (pResearchTree_->activated(race(), MachLog::AGGRESSOR, MachPhys::ASSASSIN))
        pAssassinButton_->isSwitchedOn(true);

    if (pResearchTree_->activated(race(), MachLog::AGGRESSOR, MachPhys::BALLISTA))
        pBallistaButton_->isSwitchedOn(true);

    if (pResearchTree_->activated(race(), MachLog::AGGRESSOR, MachPhys::KNIGHT))
        pKnightButton_->isSwitchedOn(true);

    if (pResearchTree_->activated(race(), MachLog::AGGRESSOR, MachPhys::NINJA))
        pNinjaButton_->isSwitchedOn(true);

    if (pResearchTree_->activated(race(), MachLog::ADMINISTRATOR, MachPhys::BOSS))
        pBossButton_->isSwitchedOn(true);

    if (pResearchTree_->activated(race(), MachLog::ADMINISTRATOR, MachPhys::OVERSEER))
        pOverseerButton_->isSwitchedOn(true);

    if (pResearchTree_->activated(race(), MachLog::ADMINISTRATOR, MachPhys::COMMANDER))
        pCommanderButton_->isSwitchedOn(true);

    if (pResearchTree_->activated(race(), MachLog::TECHNICIAN, MachPhys::LAB_TECH))
        pLabTechButton_->isSwitchedOn(true);

    if (pResearchTree_->activated(race(), MachLog::TECHNICIAN, MachPhys::TECH_BOY))
        pTechBoyButton_->isSwitchedOn(true);

    if (pResearchTree_->activated(race(), MachLog::TECHNICIAN, MachPhys::BRAIN_BOX))
        pBrainBoxButton_->isSwitchedOn(true);

    if (pResearchTree_->activated(race(), MachLog::CONSTRUCTOR, MachPhys::DOZER))
        pDozerButton_->isSwitchedOn(true);

    if (pResearchTree_->activated(race(), MachLog::CONSTRUCTOR, MachPhys::BUILDER))
        pBuilderButton_->isSwitchedOn(true);

    if (pResearchTree_->activated(race(), MachLog::CONSTRUCTOR, MachPhys::BEHEMOTH))
        pBehemothButton_->isSwitchedOn(true);

    if (pResearchTree_->activated(race(), MachLog::GEO_LOCATOR))
        pGeoLocatorButton_->isSwitchedOn(true);

    if (pResearchTree_->activated(race(), MachLog::SPY_LOCATOR))
        pSpyLocatorButton_->isSwitchedOn(true);

    if (pResearchTree_->activated(race(), MachLog::RESOURCE_CARRIER))
        pResourceCarrierButton_->isSwitchedOn(true);

    if (pResearchTree_->activated(race(), MachLog::APC))
        pAPCButton_->isSwitchedOn(true);

    pRow1_->update();
    pRow2_->update();
    pRow3_->update();
    pRow4_->update();
    pRow5_->update();
    pRow6_->update();

    update();
}

/////////////////////////////////////////////////////////////////////////////

void MachGuiMachineNavigator::removeAllIcons()
{
    pGruntButton_->isSwitchedOn(false);
    pAssassinButton_->isSwitchedOn(false);
    pBallistaButton_->isSwitchedOn(false);
    pKnightButton_->isSwitchedOn(false);
    pNinjaButton_->isSwitchedOn(false);
    pBossButton_->isSwitchedOn(false);
    pOverseerButton_->isSwitchedOn(false);
    pCommanderButton_->isSwitchedOn(false);
    pLabTechButton_->isSwitchedOn(false);
    pTechBoyButton_->isSwitchedOn(false);
    pBrainBoxButton_->isSwitchedOn(false);
    pDozerButton_->isSwitchedOn(false);
    pBuilderButton_->isSwitchedOn(false);
    pBehemothButton_->isSwitchedOn(false);
    pGeoLocatorButton_->isSwitchedOn(false);
    pSpyLocatorButton_->isSwitchedOn(false);
    pResourceCarrierButton_->isSwitchedOn(false);
    pAPCButton_->isSwitchedOn(false);

    TEST_INVARIANT;
}

/////////////////////////////////////////////////////////////////////////////

void MachGuiMachineNavigator::update()
{
    if (! isVisible())
    {
        updateReq_ = true;
        return;
    }

    updateReq_ = false;

    // Put numbers on buttons
    size_t nGruntCount = 0;
    size_t nAssassinCount = 0;
    size_t nBallistaCount = 0;
    size_t nKnightCount = 0;
    size_t nNinjaCount = 0;
    size_t nBossCount = 0;
    size_t nOverseerCount = 0;
    size_t nCommanderCount = 0;
    size_t nLabTechCount = 0;
    size_t nTechBoyCount = 0;
    size_t nBrainBoxCount = 0;
    size_t nDozerCount = 0;
    size_t nBuilderCount = 0;
    size_t nBehemothCount = 0;
    size_t nGeoLocatorCount = 0;
    size_t nSpyLocatorCount = 0;
    size_t nResourceCarrierCount = 0;
    size_t nAPCCount = 0;

    // Itterate through all the machines counting the different types
    MachLogRaces& races = MachLogRaces::instance();
    MachPhys::Race playerRace = races.playerRace();

    MachLogRaces::Machines& allMachines = races.machines();

    for (MachLogRaces::Machines::iterator itter = allMachines.begin(); itter != allMachines.end(); ++itter)
    {
        MachLogMachine* pMachine = *itter;
        if (pMachine->race() == playerRace)
        {
            switch (pMachine->objectType())
            {
                case MachLog::ADMINISTRATOR:
                    switch ((MachPhys::AdministratorSubType)pMachine->subType())
                    {
                        case MachPhys::BOSS:
                            nBossCount++;
                            break;
                        case MachPhys::OVERSEER:
                            nOverseerCount++;
                            break;
                        case MachPhys::COMMANDER:
                            nCommanderCount++;
                            break;
                            DEFAULT_ASSERT_BAD_CASE(pMachine->subType());
                    }
                    break;
                case MachLog::AGGRESSOR:
                    switch ((MachPhys::AggressorSubType)pMachine->subType())
                    {
                        case MachPhys::GRUNT:
                            nGruntCount++;
                            break;
                        case MachPhys::ASSASSIN:
                            nAssassinCount++;
                            break;
                        case MachPhys::BALLISTA:
                            nBallistaCount++;
                            break;
                        case MachPhys::KNIGHT:
                            nKnightCount++;
                            break;
                        case MachPhys::NINJA:
                            nNinjaCount++;
                            break;
                            DEFAULT_ASSERT_BAD_CASE(pMachine->subType());
                    }
                    break;
                case MachLog::CONSTRUCTOR:
                    switch ((MachPhys::ConstructorSubType)pMachine->subType())
                    {
                        case MachPhys::DOZER:
                            nDozerCount++;
                            break;
                        case MachPhys::BUILDER:
                            nBuilderCount++;
                            break;
                        case MachPhys::BEHEMOTH:
                            nBehemothCount++;
                            break;
                            DEFAULT_ASSERT_BAD_CASE(pMachine->subType());
                    }
                    break;
                case MachLog::GEO_LOCATOR:
                    nGeoLocatorCount++;
                    break;
                case MachLog::SPY_LOCATOR:
                    nSpyLocatorCount++;
                    break;
                case MachLog::TECHNICIAN:
                    switch ((MachPhys::TechnicianSubType)pMachine->subType())
                    {
                        case MachPhys::LAB_TECH:
                            nLabTechCount++;
                            break;
                        case MachPhys::TECH_BOY:
                            nTechBoyCount++;
                            break;
                        case MachPhys::BRAIN_BOX:
                            nBrainBoxCount++;
                            break;
                            DEFAULT_ASSERT_BAD_CASE(pMachine->subType());
                    }
                    break;
                case MachLog::RESOURCE_CARRIER:
                    nResourceCarrierCount++;
                    break;
                case MachLog::APC:
                    nAPCCount++;
                    break;
            }
        }
    }

    // Update numbers on buttons
    updateButtonNumber(pGruntButton_, nGruntCount, pRow1_);
    updateButtonNumber(pAssassinButton_, nAssassinCount, pRow1_);
    updateButtonNumber(pBallistaButton_, nBallistaCount, pRow1_);
    updateButtonNumber(pKnightButton_, nKnightCount, pRow1_);
    updateButtonNumber(pNinjaButton_, nNinjaCount, pRow1_);
    updateButtonNumber(pBossButton_, nBossCount, pRow2_);
    updateButtonNumber(pOverseerButton_, nOverseerCount, pRow2_);
    updateButtonNumber(pCommanderButton_, nCommanderCount, pRow2_);
    updateButtonNumber(pLabTechButton_, nLabTechCount, pRow3_);
    updateButtonNumber(pTechBoyButton_, nTechBoyCount, pRow3_);
    updateButtonNumber(pBrainBoxButton_, nBrainBoxCount, pRow3_);
    updateButtonNumber(pDozerButton_, nDozerCount, pRow4_);
    updateButtonNumber(pBuilderButton_, nBuilderCount, pRow4_);
    updateButtonNumber(pBehemothButton_, nBehemothCount, pRow4_);
    updateButtonNumber(pGeoLocatorButton_, nGeoLocatorCount, pRow5_);
    updateButtonNumber(pSpyLocatorButton_, nSpyLocatorCount, pRow5_);
    updateButtonNumber(pResourceCarrierButton_, nResourceCarrierCount, pRow6_);
    updateButtonNumber(pAPCButton_, nAPCCount, pRow6_);

    // Update select all buttons
    pAggressorSelectAll_->buttonNumber(nGruntCount + nAssassinCount + nBallistaCount + nKnightCount + nNinjaCount);
    pAdministratorSelectAll_->buttonNumber(nBossCount + nOverseerCount + nCommanderCount);
    pConstructorSelectAll_->buttonNumber(nDozerCount + nBuilderCount + nBehemothCount);
    pTechSelectAll_->buttonNumber(nLabTechCount + nTechBoyCount + nBrainBoxCount);
    pLocatorSelectAll_->buttonNumber(nGeoLocatorCount + nSpyLocatorCount);
    pTransportSelectAll_->buttonNumber(nResourceCarrierCount + nAPCCount);
}

/////////////////////////////////////////////////////////////////////////////

// virtual
void MachGuiMachineNavigator::deselectAll()
{
    pGruntButton_->refresh();
    pAssassinButton_->refresh();
    pBallistaButton_->refresh();
    pKnightButton_->refresh();
    pNinjaButton_->refresh();
    pBossButton_->refresh();
    pOverseerButton_->refresh();
    pCommanderButton_->refresh();
    pLabTechButton_->refresh();
    pTechBoyButton_->refresh();
    pBrainBoxButton_->refresh();
    pDozerButton_->refresh();
    pBuilderButton_->refresh();
    pBehemothButton_->refresh();
    pGeoLocatorButton_->refresh();
    pSpyLocatorButton_->refresh();
    pResourceCarrierButton_->refresh();
    pAPCButton_->refresh();
}

/////////////////////////////////////////////////////////////////////////////

void MachGuiMachineNavigator::CLASS_INVARIANT
{
    INVARIANT(this != nullptr);
}

/////////////////////////////////////////////////////////////////////////////

std::ostream& operator<<(std::ostream& o, const MachGuiMachineNavigator& t)
{

    o << "MachGuiMachineNavigator " << static_cast<const void*>(&t) << " start" << std::endl;
    o << "MachGuiMachineNavigator " << static_cast<const void*>(&t) << " end" << std::endl;

    return o;
}

//////
// ******************************************* Code for MachGuiNavigatorBase *************************************
//////

MachGuiNavigatorBase::MachGuiNavigatorBase(
    GuiDisplayable* pParent,
    const Gui::Box& box,
    MachInGameScreen* pInGameScreen)
    : GuiDisplayable(pParent, box)
    , updateReq_(false)
    , cycleIndex_(0)
    , pInGameScreen_(pInGameScreen)
{
    pKeyTranslator_ = new DevKeyToCommandTranslator();
}

/////////////////////////////////////////////////////////////////////////////

MachGuiNavigatorBase::~MachGuiNavigatorBase()
{
    delete pKeyTranslator_;
}

/////////////////////////////////////////////////////////////////////////////

// static
size_t MachGuiNavigatorBase::reqWidth()
{
    return (MachGuiSelectAllNavButton::reqWidth() + (5 * MachGuiNavButton::reqWidth()));
}

/////////////////////////////////////////////////////////////////////////////

// static
size_t MachGuiNavigatorBase::reqHeight()
{
    return (6 * MachGuiSelectAllNavButton::reqHeight());
}

/////////////////////////////////////////////////////////////////////////////

// virtual
void MachGuiNavigatorBase::doDisplay()
{
}

/////////////////////////////////////////////////////////////////////////////

void MachGuiNavigatorBase::makeVisible(bool visible)
{
    isVisible(visible);

    if (visible && updateReq_)
    {
        update();
    }
    else if (! visible)
    {
        deselectAll();
    }
}

/////////////////////////////////////////////////////////////////////////////

void MachGuiNavigatorBase::remainVisible(bool remainVis)
{
    remainVisible_ = remainVis;
}

/////////////////////////////////////////////////////////////////////////////

bool MachGuiNavigatorBase::remainVisible()
{
    return remainVisible_;
}

/////////////////////////////////////////////////////////////////////////////

// static
void MachGuiNavigatorBase::updateButtonNumber(MachGuiNavButtonWithCounter* pButton, size_t value, MachGuiNavRow* pRow)
{
    pButton->buttonNumber(value);
    if (value != 0 && ! pButton->isSwitchedOn())
    {
        // if value is not zero then we must be able to navigate to
        // an actor of this type, regardless of whether we have
        // researched it. We may have captured a building!!
        pButton->isSwitchedOn(true);
        pRow->update();
    }
}

/////////////////////////////////////////////////////////////////////////////

// virtual
bool MachGuiNavigatorBase::machineNav()
{
    return false;
}

// virtual
bool MachGuiMachineNavigator::machineNav()
{
    return true;
}

bool MachGuiNavigatorBase::processButtonEvent(const DevButtonEvent& buttonEvent)
{
    if (buttonEvent.action() == DevButtonEvent::RELEASE)
        return false;

    typedef DevKeyToCommand::CommandId CommandId;

    CommandId commandId;
    bool processed = pKeyTranslator_->translate(buttonEvent, &commandId);

    if (processed)
    {
        switch (commandId)
        {
            case SELECT_ALL:
                selectAll(false);
                break;
            case SELECT_ALL_KEEP_SELECTION:
                selectAll(true);
                break;
            case VIEW_NEXT:
                viewNext();
                break;
        }
    }

    return processed;
}

void MachGuiNavigatorBase::selectAll(bool keepSelection)
{
    if (! keepSelection)
    {
        remainVisible(isVisible());
        pInGameScreen_->deselectAll();
        deselectAll();
    }

    MachLogRaces& races = MachLogRaces::instance();
    MachPhys::Race playerRace = races.playerRace();

    MachLogRaces::Objects& allObjects = races.raceObjects(playerRace);

    MachInGameScreen::Actors actors; // List of actors being selected/deselected
    MachInGameScreen::Actors
        canAttackActors; // List of can attack actors being selected/deselected ( these take priority in corral )
    actors.reserve(20);
    canAttackActors.reserve(20);

    for (MachLogRaces::Objects::iterator itter = allObjects.begin(); itter != allObjects.end(); ++itter)
    {
        MachActor* pActor = *itter;
        if ((machineNav() && pActor->objectIsMachine()) || (! machineNav() && pActor->objectIsConstruction()))
        {
            // Make sure the actor hasn't already been selected
            if (pActor->selectionState() != MachLog::SELECTED)
            {
                if (pActor->objectIsCanAttack())
                {
                    canAttackActors.push_back(pActor);
                }
                else
                {
                    actors.push_back(pActor);
                }
            }
        }
    }

    // If the navigator is visible then make all the buttons become selected.
    if (isVisible())
    {
        selectAll();
    }

    // Indicates to the ingame screen whether the navigator should remain visible after
    // this operation.
    remainVisible(isVisible());
    pInGameScreen_->select(canAttackActors);

    remainVisible(isVisible());
    pInGameScreen_->select(actors);
}

void MachGuiMachineNavigator::selectAll()
{
    pRow1_->selectAll();
    pRow2_->selectAll();
    pRow3_->selectAll();
    pRow4_->selectAll();
    pRow5_->selectAll();
    pRow6_->selectAll();
}

void MachGuiConstructionNavigator::selectAll()
{
    pRow1_->selectAll();
    pRow2_->selectAll();
    pRow3_->selectAll();
    pRow4_->selectAll();
    pRow5_->selectAll();
    pRow6_->selectAll();
}

void MachGuiNavigatorBase::viewNext()
{
    size_t cycleIndex = ++cycleIndex_;
    MachActor* pSwitchToActor = nullptr;
    MachActor* pFirstActor = nullptr;

    MachLogRaces& races = MachLogRaces::instance();
    MachPhys::Race playerRace = races.playerRace();

    MachLogRaces::Objects& allObjects = races.raceObjects(playerRace);

    for (MachLogRaces::Objects::iterator itter = allObjects.begin(); itter != allObjects.end() && ! pSwitchToActor;
         ++itter)
    {
        MachActor* pActor = *itter;
        if ((machineNav() && pActor->objectIsMachine()) || (! machineNav() && pActor->objectIsConstruction()))
        {
            if (! pFirstActor)
                pFirstActor = pActor;

            --cycleIndex;
            if (! cycleIndex)
            {
                pSwitchToActor = pActor;
            }
        }
    }

    if (! pSwitchToActor)
    {
        cycleIndex_ = 1;
        if (pFirstActor)
            pInGameScreen_->cameras()->lookAt(*pFirstActor);
    }
    else
    {
        pInGameScreen_->cameras()->lookAt(*pSwitchToActor);
    }
}

void MachGuiConstructionNavigator::updateConstructionIconBitmaps()
{
    pTurretButton_->updateBitmap();
    pSentryButton_->updateBitmap();
    pLauncherButton_->updateBitmap();
    pICBMButton_->updateBitmap();
    pCivFacButton_->updateBitmap();
    pMilFacButton_->updateBitmap();
    pCivLabButton_->updateBitmap();
    pMilLabButton_->updateBitmap();
    pSmelterButton_->updateBitmap();
    pMineButton_->updateBitmap();
    pGarrisonButton_->updateBitmap();
    pBeaconButton_->updateBitmap();
    pPodButton_->updateBitmap();

    TEST_INVARIANT;
}

void MachGuiMachineNavigator::updateMachineIconBitmaps()
{
    pGruntButton_->updateBitmap();
    pAssassinButton_->updateBitmap();
    pBallistaButton_->updateBitmap();
    pKnightButton_->updateBitmap();
    pNinjaButton_->updateBitmap();
    pBossButton_->updateBitmap();
    pOverseerButton_->updateBitmap();
    pCommanderButton_->updateBitmap();
    pLabTechButton_->updateBitmap();
    pTechBoyButton_->updateBitmap();
    pBrainBoxButton_->updateBitmap();
    pDozerButton_->updateBitmap();
    pBuilderButton_->updateBitmap();
    pBehemothButton_->updateBitmap();
    pGeoLocatorButton_->updateBitmap();
    pSpyLocatorButton_->updateBitmap();
    pResourceCarrierButton_->updateBitmap();
    pAPCButton_->updateBitmap();

    TEST_INVARIANT;
}

/* End MACHNAV.CPP **************************************************/
