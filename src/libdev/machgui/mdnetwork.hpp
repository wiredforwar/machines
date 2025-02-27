/*
 * N E T W M O D E . H P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

/*
    MachGuiNetworkProtocolMode

    An abc for network details
*/

#ifndef _MACHGUI_NETWMODE_HPP
#define _MACHGUI_NETWMODE_HPP

#include "base/base.hpp"

#include "network/netnet.hpp"

class GuiDisplayable;
class MachGuiStartupScreens;

class MachGuiNetworkProtocolMode
// Canonical form revoked
{
public:
    MachGuiNetworkProtocolMode(GuiDisplayable*, MachGuiStartupScreens*);
    virtual ~MachGuiNetworkProtocolMode();

    void CLASS_INVARIANT;

    virtual void setNetworkDetails() = 0;
    virtual void readNetworkDetails() = 0;

    virtual bool validNetworkDetails(bool isHost) = 0;

    virtual void updateGUI() { }

    virtual void charFocus() { }

    bool connectionSet();
    //  Use the no record version in assertions
    bool connectionSetNoRecord();

protected:
    GuiDisplayable& parent();
    MachGuiStartupScreens& startupScreens();

private:
    friend std::ostream& operator<<(std::ostream& o, const MachGuiNetworkProtocolMode& t);

    MachGuiNetworkProtocolMode(const MachGuiNetworkProtocolMode&);
    MachGuiNetworkProtocolMode& operator=(const MachGuiNetworkProtocolMode&);

    bool connectionSetHelper(NetNetwork::NetNetworkStatus status);

    GuiDisplayable* pParent_;
    MachGuiStartupScreens* pStartupScreens_;
};

#endif

/* End NETWMODE.HPP *************************************************/
