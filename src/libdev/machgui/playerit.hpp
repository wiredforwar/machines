/*
 * P L A Y E R I T . H P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

/*
    MachGuiPlayerListItem

    A brief description of the class should go in here
*/

#ifndef _MACHGUI_PLAYERIT_HPP
#define _MACHGUI_PLAYERIT_HPP

#include "base/base.hpp"
#include "stdlib/string.hpp"
#include "gui/displaya.hpp"
#include "gui/root.hpp"
#include "machphys/machphys.hpp"
#include "machgui/autodel.hpp"

class MachGuiStartupScreens;
class GuiSimpleScrollableList;
class GuiBmpFont;
class MachGuiPlayerListItem;
class MachGuiDropDownListBox;

struct MachGuiSharedPlayerInfo
{
    std::string name_;
    size_t index_;
    MachPhys::Race race_;
    bool ready_;
    bool host_;
    double ping_;
    bool hasMachinesCD_;
};
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class MachGuiPlayerColour : public GuiDisplayable
{
public:
    MachGuiPlayerColour(MachGuiStartupScreens*, MachGuiPlayerListItem* pParent, MachGuiSharedPlayerInfo*);
    ~MachGuiPlayerColour() override;

    static size_t reqWidth();

    void refresh();

    static void releaseBmpMemory();

protected:
    static Gui::Box getRelPos(MachGuiPlayerListItem*);

    void doDisplay() override;

    void doHandleMouseEnterEvent(const GuiMouseEvent& rel) override;
    void doHandleMouseExitEvent(const GuiMouseEvent& rel) override;
    void doHandleMouseClickEvent(const GuiMouseEvent& rel) override;
    void doHandleContainsMouseEvent(const GuiMouseEvent& rel) override;

    static GuiBitmap*& pRedBmp();
    static GuiBitmap*& pGreenBmp();
    static GuiBitmap*& pBlueBmp();
    static GuiBitmap*& pYellowBmp();
    static GuiBitmap*& pBlackBmp();

    bool canInteract() const;

private:
    MachGuiPlayerColour(const MachGuiPlayerColour&);
    MachGuiPlayerColour& operator=(const MachGuiPlayerColour&);

    bool highlighted_;
    MachGuiStartupScreens* pStartupScreens_;
    MachGuiPlayerListItem* pParent_;
    MachGuiSharedPlayerInfo* pPlayerInfo_;
};
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class MachGuiPlayerListItem : public GuiDisplayable
// Canonical form revoked
{
public:
    // TODO: Removing MGSS. Many usages here. This "list item" updates the game state in startupData and more!
    MachGuiPlayerListItem(
        MachGuiStartupScreens* pStartupScreens,
        GuiSimpleScrollableList* pParentList,
        size_t width,
        const std::string& playerName,
        MachPhys::Race playerRace,
        size_t playerIndex,
        bool playerReady,
        bool playerIsHost,
        double ping,
        bool hasMachinesCD);
    ~MachGuiPlayerListItem() override;

    void CLASS_INVARIANT;

    static size_t reqHeight();

    void updateInfo(
        const std::string& playerName,
        MachPhys::Race playerRace,
        bool playerReady,
        bool playerIsHost,
        double ping,
        bool hasMachinesCD);

    void dropDownListDisplayed(MachGuiDropDownListBox*);

protected:
    static GuiBmpFont getFont();
    static GuiBmpFont getHighlightFont();

    void doDisplay() override;

    void doHandleMouseEnterEvent(const GuiMouseEvent& rel) override;
    void doHandleMouseExitEvent(const GuiMouseEvent& rel) override;
    void doHandleMouseClickEvent(const GuiMouseEvent& rel) override;
    void doHandleContainsMouseEvent(const GuiMouseEvent& rel) override;

    void colourHighlighted(bool);
    bool canInteract() const;
    bool canDisplayDropDownList() const;

private:
    friend class MachGuiPlayerColour;
    friend std::ostream& operator<<(std::ostream& o, const MachGuiPlayerListItem& t);

    MachGuiPlayerListItem(const MachGuiPlayerListItem&);
    MachGuiPlayerListItem& operator=(const MachGuiPlayerListItem&);

    bool highlighted_;
    bool colourHighlighted_;
    MachGuiDropDownListBox* pDropDownList_;
    DECL_DEPRECATED MachGuiStartupScreens* pStartupScreens_;
    MachGuiPlayerColour* pPlayerColour_;
    MachGuiSharedPlayerInfo playerInfo_;

    // A GuiRoot such as MachGuiStartupScreens
    GuiRoot* pRootParent_;
};
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class MachGuiColourList
    : public MachGuiAutoDeleteDisplayable
    , public GuiDisplayable
{
public:
    MachGuiColourList(
        MachGuiStartupScreens* pStartupScreens,
        MachGuiPlayerColour* pParent,
        const Gui::Box& box,
        MachGuiSharedPlayerInfo*);
    ~MachGuiColourList() override;

    bool containsMousePointer() override;

protected:
    void doDisplay() override;

private:
    MachGuiColourList(const MachGuiColourList&);
    MachGuiColourList& operator=(const MachGuiColourList&);
};
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class MachGuiColourSelector : public GuiDisplayable
{
public:
    MachGuiColourSelector(
        MachGuiStartupScreens* pStartupScreens,
        MachGuiColourList* pParent,
        const Gui::Box& box,
        MachPhys::Race race,
        MachGuiSharedPlayerInfo*);
    ~MachGuiColourSelector() override;

protected:
    void doDisplay() override;

    void doHandleMouseEnterEvent(const GuiMouseEvent& rel) override;
    void doHandleMouseExitEvent(const GuiMouseEvent& rel) override;
    void doHandleMouseClickEvent(const GuiMouseEvent& rel) override;

private:
    MachGuiColourSelector(const MachGuiColourSelector&);
    MachGuiColourSelector& operator=(const MachGuiColourSelector&);

    // Data members
    bool highlighted_;
    MachGuiStartupScreens* pStartupScreens_;
    MachPhys::Race race_;
    MachGuiColourList* pColourList_;
    MachGuiSharedPlayerInfo* pPlayerInfo_;
};

#endif

/* End PLAYERIT.HPP *************************************************/
