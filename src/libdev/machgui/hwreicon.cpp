/*
 * (c) Charybdis Limited, 1997. All Rights Reserved
 */

//  Definitions of non-inline non-template methods and global functions

#include "stdlib/string.hpp"
#include "ctl/vector.hpp"
#include "machgui/hwreicon.hpp"
#include "machgui/hwrebank.hpp"
#include "machgui/actbmpnm.hpp"
#include "machgui/ingame.hpp"
#include "machgui/actnamid.hpp"
#include "machlog/actor.hpp"
#include "machlog/hwlab.hpp"
#include "machlog/resitem.hpp"
#include "machlog/races.hpp"
#include "gui/restring.hpp"
#include "machgui/internal/strings.hpp"
#include "machgui/internal/mgsndman.hpp"

/* ////////////////////////////////////////////// constructor /////////////////////////////////////////////////// */

MachHWResearchIcon::MachHWResearchIcon(
    GuiDisplayable* pParent,
    MachInGameScreen* pInGameScreen,
    MachHWResearchBank* pHWResearchBank,
    MachLogHardwareLab* pHardwareLab,
    const MachLogResearchItem* pResearchItem)
    : GuiIcon(
        pParent,
        Gui::Coord(0, 0), // Will be relocated by icon sequence parent
        MachGui::getScaledImage(MachActorBitmaps::name(
            pResearchItem->objectType(),
            pResearchItem->subType(),
            pResearchItem->hwLevel(),
            pResearchItem->weaponCombo(),
            MachLogRaces::instance().playerRace())))
    , pInGameScreen_(pInGameScreen)
    , pHWResearchBank_(pHWResearchBank)
    , pHardwareLab_(pHardwareLab)
    , pResearchItem_(pResearchItem)
{

    TEST_INVARIANT;
}

/* /////////////////////////////////////////////// destructor /////////////////////////////////////////////////// */

MachHWResearchIcon::~MachHWResearchIcon()
{
    TEST_INVARIANT;
}

std::string MachHWResearchIcon::getPromptText() const
{
    GuiString prompt = MachLogActorStringIdRestorer::getActorPromptText(
        pResearchItem_->objectType(),
        pResearchItem_->subType(),
        pResearchItem_->weaponCombo(),
        pResearchItem_->hwLevel(),
        IDS_RESEARCH_PROMPT,
        IDS_RESEARCH_WITH_WEAPON_PROMPT);

    // Add bmu cost and rp cost to end of prompt text
    char bmuBuffer[20];
    char rpBuffer[20];
    //  itoa( pResearchItem_->buildingCost(), bmuBuffer, 10 );
    //  itoa( pResearchItem_->researchCost(), rpBuffer, 10 );
    sprintf(bmuBuffer, "%d", pResearchItem_->buildingCost());
    sprintf(rpBuffer, "%d", pResearchItem_->researchCost());

    if (pResearchItem_->buildingCost() != 0)
    {
        GuiStrings strings;
        strings.push_back(GuiString(bmuBuffer));
        strings.push_back(GuiString(rpBuffer));
        GuiResourceString costText(IDS_COST_WITH_RP, strings);
        prompt += "\n" + costText.asString();
    }
    else
    {
        GuiResourceString costText(IDS_COST_RP, GuiString(rpBuffer));
        prompt += "\n" + costText.asString();
    }

    return prompt;
}

/* ////////////////////////////////////////////////////////////////////////////////////////////////////////////// */

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void MachHWResearchIcon::CLASS_INVARIANT
{
    INVARIANT(this != nullptr);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::ostream& operator<<(std::ostream& o, const MachHWResearchIcon& t)
{

    o << "MachHWResearchIcon " << static_cast<const void*>(&t) << " start" << std::endl;
    o << "MachHWResearchIcon " << static_cast<const void*>(&t) << " end" << std::endl;

    return o;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// virtual
void MachHWResearchIcon::doBeDepressed(const GuiMouseEvent&)
{
    MachGuiSoundManager::instance().playSound("gui/sounds/igclick.wav");
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// virtual
void MachHWResearchIcon::doBeReleased(const GuiMouseEvent&)
{
    // Add the item to the queue
    if (pHardwareLab_->addResearchItem(*pResearchItem_))
    {
        // Wasn't in the queue previously, so update the bank icons
        pHWResearchBank_->updateQueueIcons();
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// static
size_t MachHWResearchIcon::reqWidth()
{
    return 42 * Gui::uiScaleFactor(); // Todo : remove hardcoding
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// static
size_t MachHWResearchIcon::reqHeight()
{
    return 42 * Gui::uiScaleFactor(); // todo : remove hardcoding
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// virtual
void MachHWResearchIcon::doHandleMouseEnterEvent(const GuiMouseEvent& mouseEvent)
{
    GuiIcon::doHandleMouseEnterEvent(mouseEvent);

    pInGameScreen_->setCursorPromptText(getPromptText());
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// virtual
void MachHWResearchIcon::doHandleMouseExitEvent(const GuiMouseEvent& mouseEvent)
{
    // Clear the cursor prompt string
    pInGameScreen_->clearCursorPromptText();

    GuiIcon::doHandleMouseExitEvent(mouseEvent);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* End *************************************************/
