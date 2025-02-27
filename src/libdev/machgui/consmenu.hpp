/*
 * C O N S M E N U . H P P
 * (c) Charybdis Limited, 1997. All Rights Reserved
 */

/*
    MachConstructMenu

    Parent of the construct menu items. This menu is displayed during interaction
    of the Construct command, allowing selection of the construction to be built.
*/

#ifndef _MACHGUI_CONSMENU_HPP
#define _MACHGUI_CONSMENU_HPP

#include "base/base.hpp"
#include "machgui/ui/ScrollArea.hpp"

// forward refs
class MachInGameScreen;

// orthodox canonical revoked
class MachConstructMenu : public MachGuiScrollArea
{
public:
    // ctor. Parent is pInGameScreen.
    MachConstructMenu(GuiDisplayable* pParent, const Gui::Coord& relCoord, MachInGameScreen* pInGameScreen);

    // dtor.
    ~MachConstructMenu() override;

    void CLASS_INVARIANT;

    static size_t reqWidth();
    static size_t reqHeight(MachInGameScreen* pInGameScreen);

protected:
    GuiSimpleScrollableList* createList(GuiDisplayable* pParent, const Gui::Coord& rel, MachInGameScreen*) override;

private:
    // Operations deliberately revoked
    MachConstructMenu(const MachConstructMenu&);
    MachConstructMenu& operator=(const MachConstructMenu&);
    bool operator==(const MachConstructMenu&);

    friend std::ostream& operator<<(std::ostream& o, const MachConstructMenu& t);
};

#endif

/* End CONSMENU.HPP *************************************************/
