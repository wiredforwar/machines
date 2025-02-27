/*
 * D I S P N O T I . H P P
 * (c) Charybdis Limited, 1999. All Rights Reserved
 */

/*
    MachGuiDispositionChangeNotifiable

    A brief description of the class should go in here
*/

#ifndef _MACHGUI_DISPNOTI_HPP
#define _MACHGUI_DISPNOTI_HPP

#include "base/base.hpp"
#include "gui/restring.hpp"
#include "machlog/dispnoti.hpp"

class MachGuiStartupScreens;

class MachGuiDispositionChangeNotifiable : public MachLogDispositionChangeNotifiable
// Canonical form revoked
{
public:
    MachGuiDispositionChangeNotifiable(MachGuiStartupScreens*);

    ~MachGuiDispositionChangeNotifiable() override;

    void notifyGeneralDispositionChange() override;
    // one of these is generated for the actual specifics of a race
    void notifySpecificDispositionChange(MachPhys::Race, MachPhys::Race) override;
    // This is called when a class changes disposition to move to enemy or neutral, if they started as ally
    void notifyDispositionChangeToNoneAlly(MachPhys::Race, MachPhys::Race) override;
    // This is called when a class changes disposition to move to ally, if they started as non-ally
    void notifyDispositionChangeToAlly(MachPhys::Race, MachPhys::Race) override;

    void CLASS_INVARIANT;

private:
    friend std::ostream& operator<<(std::ostream& o, const MachGuiDispositionChangeNotifiable& t);

    void doDisplay(GuiResourceString::Id id, MachPhys::Race race1, MachPhys::Race race2);
    std::string getDisplayName(MachPhys::Race race);

    MachGuiDispositionChangeNotifiable(const MachGuiDispositionChangeNotifiable&);
    MachGuiDispositionChangeNotifiable& operator=(const MachGuiDispositionChangeNotifiable&);

    MachGuiStartupScreens* pStartupScreens_;
};

#endif

/* End DISPNOTI.HPP *************************************************/
