/*
 * C T X J O I N . C P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

//  Definitions of non-inline non-template methods and global functions

#include "base/diag.hpp"
#include "world4d/manager.hpp"
#include "device/time.hpp"
#include "machgui/ctxjoin.hpp"
#include "machgui/sslistit.hpp"
#include "machgui/ui/MenuButton.hpp"
#include "machgui/ui/MenuStyle.hpp"
#include "machgui/ui/MenuText.hpp"
#include "machgui/startup.hpp"
#include "machgui/strtdata.hpp"
#include "machgui/sslistbx.hpp"
#include "machgui/messbrok.hpp"
#include "machgui/gui.hpp"
#include "machgui/editbox.hpp"
#include "machgui/ui/VerticalScrollBar.hpp"
#include "machgui/editlist.hpp"
#include "machgui/database.hpp"
#include "machgui/dbsystem.hpp"
#include "machgui/dbplanet.hpp"
#include "machgui/dbscenar.hpp"
#include "machlog/network.hpp"
#include "machgui/mdnetwork.hpp"
#include "machgui/mdserial.hpp"
#include "machgui/mdmodem.hpp"
#include "machgui/mdinternet.hpp"
#include "machgui/mdipx.hpp"
#include "gui/font.hpp"
#include "gui/scrolbut.hpp"
#include "gui/scrolbar.hpp"
#include "gui/painter.hpp"
#include "gui/restring.hpp"
#include "gui/manager.hpp"
#include "machgui/internal/strings.hpp"
#include "network/netnet.hpp"
#include "network/sessuid.hpp"
#include "network/session.hpp"

//////////////////////////////////////////////////////////////////////////////////////////////

class MachGuiCurGamesListBoxItem : public MachGuiSingleSelectionListBoxItem
// Canonical form revoked
{
public:
    MachGuiCurGamesListBoxItem(
        MachGuiStartupScreens* pStartupScreens,
        MachGuiSingleSelectionListBox* pListBox,
        size_t width,
        const NetSessionInfo& info,
        MachGuiCtxJoin& joinContext)
        : MachGuiSingleSelectionListBoxItem(pStartupScreens, pListBox, width, info.serverName)
        , joinContext_(joinContext)
        , sessionInfo_(info)
    {
    }

    ~MachGuiCurGamesListBoxItem() override
    {
        //      joinContext_.joinGameSelected( false ); // This produced some freed memory references
    }

    void CLASS_INVARIANT;

protected:
    void select() override
    {
        MachGuiSingleSelectionListBoxItem::select();
        joinContext_.onNetSessionSelected(sessionInfo_);
    }

    void unselect() override
    {
        MachGuiSingleSelectionListBoxItem::unselect();
        joinContext_.onNetSessionSelected({});
    }

private:
    MachGuiCurGamesListBoxItem(const MachGuiCurGamesListBoxItem&);
    MachGuiCurGamesListBoxItem& operator=(const MachGuiCurGamesListBoxItem&);

    // Data members...
    MachGuiCtxJoin& joinContext_;
    NetSessionInfo sessionInfo_;
};
//////////////////////////////////////////////////////////////////////////////////////////////

class MachGuiNewGameName : public MachGuiEditBoxListBoxItem
{
public:
    MachGuiNewGameName(
        MachGuiStartupScreens* pStartupScreens,
        MachGuiSingleSelectionListBox* pListBox,
        size_t width,
        const string& text,
        MachGuiCtxJoin& joinContext)
        : MachGuiEditBoxListBoxItem(pStartupScreens, pListBox, width, text)
        , joinContext_(joinContext)
    {
    }

    ~MachGuiNewGameName() override
    {
        updateNewGameName();

        //      joinContext_.editingGameName( false ); // Why it was there?
    }

    void updateNewGameName()
    {
        if (text() != "")
            startupScreens()->startupData()->newGameName(text());
    }

protected:
    void select() override
    {
        MachGuiEditBoxListBoxItem::select();

        singleLineEditBox()->maxChars(MAX_GAMENAME_LEN);
        singleLineEditBox()->setText(startupScreens()->startupData()->newGameName());
        joinContext_.onNewGameItemSelected();
    }

    void unselect() override
    {
        startupScreens()->startupData()->newGameName(singleLineEditBox()->text());

        MachGuiEditBoxListBoxItem::unselect();
        joinContext_.changeFocus();
        joinContext_.editingGameName(false);
    }

private:
    MachGuiCtxJoin& joinContext_;
};

//////////////////////////////////////////////////////////////////////////////////////////////

#define JOINGAME_LB_MINX 48
#define JOINGAME_LB_MAXX 262
#define JOINGAME_LB_MINY 56
#define JOINGAME_LB_MAXY 300
#define SCROLLBAR_WIDTH 17

MachGuiCtxJoin::MachGuiCtxJoin(MachGuiStartupScreens* pStartupScreens)
    : MachGui::GameMenuContext("sc", pStartupScreens)
{
    WAYNE_STREAM("JOINGAME_LB_MAXY: " << JOINGAME_LB_MAXY << std::endl);
    pJoinBtn_ = new MachGuiMenuButton(
        pStartupScreens,
        pStartupScreens,
        Gui::Box(414, 51, 605, 93) * MachGui::menuScaleFactor(),
        IDS_MENUBTN_JOIN,
        MachGui::ButtonEvent::JOIN);
    pCreateBtn_ = new MachGuiMenuButton(
        pStartupScreens,
        pStartupScreens,
        Gui::Box(414, 157, 605, 200) * MachGui::menuScaleFactor(),
        IDS_MENUBTN_CREATE,
        MachGui::ButtonEvent::CREATE);
    pShowGamesBtn_ = new MachGuiMenuButton(
        pStartupScreens,
        pStartupScreens,
        Gui::Box(414, 251, 605, 293) * MachGui::menuScaleFactor(),
        IDS_MENUBTN_SHOWGAMES,
        MachGui::ButtonEvent::SHOWGAMES);
    MachGuiMenuButton* pCancelBtn = new MachGuiMenuButton(
        pStartupScreens,
        pStartupScreens,
        Gui::Box(414, 348, 605, 390) * MachGui::menuScaleFactor(),
        IDS_MENUBTN_CANCEL,
        MachGui::ButtonEvent::EXIT);

    pCancelBtn->escapeControl(true);
    pShowGamesBtn_->defaultControl(true);

    // Display list box heading
    GuiResourceString currentGames(IDS_MENULB_GAMES);
    GuiBmpFont font(GuiBmpFont::getFont(MachGui::Menu::largeFontLight()));
    MachGuiMenuText* pCurrentGamesText = new MachGuiMenuText(
        pStartupScreens,
        Gui::Box(
            Gui::Coord(JOINGAME_LB_MINX, JOINGAME_LB_MINY) * MachGui::menuScaleFactor(),
            Gui::Size(font.textWidth(currentGames.asString()), font.charHeight() + 1 * MachGui::menuScaleFactor())),
        IDS_MENULB_GAMES,
        MachGui::Menu::largeFontLight());

    // Create games list box
    pGamesList_ = new MachGuiSingleSelectionListBox(
        pStartupScreens,
        pStartupScreens,
        Gui::Box(
            MachGui::menuScaleFactor() * JOINGAME_LB_MINX,
            pCurrentGamesText->absoluteBoundary().maxCorner().y() - getBackdropTopLeft().first,
            MachGui::menuScaleFactor() * (JOINGAME_LB_MAXX - SCROLLBAR_WIDTH),
            MachGui::menuScaleFactor() * JOINGAME_LB_MAXY),
        1000,
        MachGuiSingleSelectionListBoxItem::reqHeight(),
        1);

    MachGuiVerticalScrollBar::createWholeBar(
        pStartupScreens,
        Gui::Coord(JOINGAME_LB_MAXX - SCROLLBAR_WIDTH, JOINGAME_LB_MINY) * MachGui::menuScaleFactor(),
        (JOINGAME_LB_MAXY - JOINGAME_LB_MINY) * MachGui::menuScaleFactor(),
        pGamesList_);

    // Clear join game info.
    pStartupScreens->startupData()->resetData();

    // Setup edit box for new game name to be entered into...
    GuiResourceString enterName(IDS_MENU_ENTERGAMENAME);
    pNewGameName_ = new MachGuiNewGameName(
        pStartupScreens_,
        pGamesList_,
        (JOINGAME_LB_MAXX - JOINGAME_LB_MINX - SCROLLBAR_WIDTH) * MachGui::menuScaleFactor(),
        enterName.asString(),
        *this);
    pGamesList_->childrenUpdated();

    MachLogNetwork::instance().resetSession();
    // Query network to find which protocol has been selected
    NetNetwork::NetworkProtocol protocol = NetNetwork::instance().currentProtocol();

    switch (protocol)
    {
        case NetNetwork::NetworkProtocol::UDP:
            pNetworkProtocol_ = new MachGuiInternetNetworkMode(pStartupScreens, pStartupScreens);
            break;
        case NetNetwork::NetworkProtocol::IPX:
            pNetworkProtocol_ = new MachGuiIPXNetworkMode(pStartupScreens, pStartupScreens);
            break;
        case NetNetwork::NetworkProtocol::TCPIP:
            pNetworkProtocol_ = new MachGuiInternetNetworkMode(pStartupScreens, pStartupScreens);
            break;
        case NetNetwork::NetworkProtocol::MODEM:
            pNetworkProtocol_ = new MachGuiModemNetworkMode(pStartupScreens, pStartupScreens);
            break;
        case NetNetwork::NetworkProtocol::SERIAL:
            pNetworkProtocol_ = new MachGuiSerialNetworkMode(pStartupScreens, pStartupScreens);
            break;
        default:
            ASSERT(false, "Network protocol not currently supported by GUI");
    }

    TEST_INVARIANT;
}

MachGuiCtxJoin::~MachGuiCtxJoin()
{
    TEST_INVARIANT;

    GuiManager::instance().charFocus(pStartupScreens_);
}

void MachGuiCtxJoin::CLASS_INVARIANT
{
    INVARIANT(this != nullptr);
}

std::ostream& operator<<(std::ostream& o, const MachGuiCtxJoin& t)
{

    o << "MachGuiCtxJoin " << static_cast<const void*>(&t) << " start" << std::endl;
    o << "MachGuiCtxJoin " << static_cast<const void*>(&t) << " end" << std::endl;

    return o;
}

// virtual
void MachGuiCtxJoin::buttonEvent(MachGui::ButtonEvent be)
{
    if (be == MachGui::ButtonEvent::SHOWGAMES)
    {
        showGames();
    }
}

void MachGuiCtxJoin::showGames()
{
    PRE(pNetworkProtocol_->connectionSetNoRecord());

    bool isHost = false;

    if (validNetworkSettings(isHost))
    {
        // Display PleaseWait
        // if ( NetNetwork::instance().currentProtocol() != NetNetwork::IPX )
        //{
        // GuiBitmap loadingBmp = Gui::bitmap( "gui/menu/pleaseWait.bmp" );
        // loadingBmp.enableColourKeying();
        // GuiBitmap frontBuffer = W4dManager::instance().sceneManager()->pDevice()->frontSurface();
        // frontBuffer.simpleBlit( loadingBmp, Ren::Rect( 0,0,loadingBmp.width(),loadingBmp.height() ) );
        //}

        // Clear games list box
        pGamesList_->deleteAllItems();

        const int itemWidth = (JOINGAME_LB_MAXX - JOINGAME_LB_MINX - SCROLLBAR_WIDTH) * MachGui::menuScaleFactor();

        // Set up special enter-name edit box
        GuiResourceString enterName(IDS_MENU_ENTERGAMENAME);
        pNewGameName_ = new MachGuiNewGameName(
            pStartupScreens_,
            pGamesList_,
            itemWidth,
            enterName.asString(),
            *this);

        // Call children updated here to clean network status before polling for new sessions.
        pGamesList_->childrenUpdated();

        // Update network status
        NETWORK_STREAM("MachGuiCtxJoin::showGames() calling update to get most uptodate session list\n");
        NetNetwork::instance().refreshSessions();
    }
}

std::size_t MachGuiCtxJoin::numGamesInList() const
{
    // The list should always have at least the 'new game' item (which we exclude here)
    PRE(pGamesList_->numListItems() > 0);
    return pGamesList_->numListItems() - 1;
}

// virtual
bool MachGuiCtxJoin::okayToSwitchContext()
{
    // Initialise multiplayer settings ( assumes at least 1 planet and 1 scenario in Small map size ).
    MachGuiDbSystem& system = MachGuiDatabase::instance().multiPlayerSystem(MachGuiDatabase::SMALL);
    MachGuiDbPlanet& planet = system.planet(0);
    MachGuiDbScenario& scenario = planet.scenario(0);
    pStartupScreens_->startupData()->scenario(&scenario);
    // Initialise player slots to unavailable if scenario is limited in number of players.
    // Based on the maximum number of players allowed in this multiplayer game update the
    // player slots to "open" or "unavailable".
    for (int loop = 0; loop < 4; ++loop)
    {
        if (loop < scenario.maxPlayers())
        {
            pStartupScreens_->startupData()->players()[loop].setPlayerName(MachGui::openText());
        }
        else
        {
            pStartupScreens_->startupData()->players()[loop].setPlayerName(MachGui::unavailableText());
        }
    }

    // Extract typed in new game name and store in startup data
    pNewGameName_->updateNewGameName();

    // Was CREATE pressed...
    if (pStartupScreens_->lastButtonEvent() == MachGui::ButtonEvent::CREATE)
    {
        bool isHost = true;

        if (pStartupScreens_->startupData()->newGameName() == "" || ! editingGameName_ || pNewGameName_->text() == "")
        {
            // Display message box. Type in game name to create game.
            pStartupScreens_->displayMsgBox(IDS_MENUMSG_ENTERGAMENAME);
            return false;
        } // Can the network connection be established
        else if (! validNetworkSettings(isHost))
        {
            return false;
        }
        else
        {
            // Now pass through player name as input in gui to network library
            MachLogNetwork::instance().hostWithSessionId(
                pStartupScreens_->startupData()->newGameName(),
                pStartupScreens_->startupData()->playerName());

            if (NetNetwork::currentStatus() != NetNetwork::NETNET_OK)
            {
                pStartupScreens_->displayMsgBox(MachGui::convertNetworkError(NetNetwork::currentStatus()));
                NetNetwork::instance().resetStatus();
                return false;
            }

            pStartupScreens_->startupData()->resetData();
            pStartupScreens_->startupData()->hostGame(true);

            return true;
        }
    } // Was JOIN pressed...
    else if (pStartupScreens_->lastButtonEvent() == MachGui::ButtonEvent::JOIN)
    {
        std::string ipAddress = NetNetwork::instance().IPAddress();
        bool isHost = false;
        if (ipAddress.empty())
        {
            // Display message box. Must choose game to join.
            pStartupScreens_->displayMsgBox(IDS_MENUMSG_SELECTGAMETOJOIN);
            return false;
        } // Can the network connection be established
        // Complete rubbish! the connection settings have already been set. Hal 19/11/98
        //      else if ( not validNetworkSettings( isHost ) )
        //      {
        //          return false;
        //      }
        else
        {
            if (!MachLogNetwork::instance().joinSession(ipAddress, pStartupScreens_->startupData()->playerName()))
                if (!MachLogNetwork::instance().joinSession(ipAddress, pStartupScreens_->startupData()->playerName()))
                {
                    pStartupScreens_->displayMsgBox(IDS_MENUMSG_NETSESSIONERROR);
                    NetNetwork::instance().resetStatus();
                    return false;
                }

            if (NetNetwork::currentStatus() != NetNetwork::NETNET_OK)
            {
                pStartupScreens_->displayMsgBox(MachGui::convertNetworkError(NetNetwork::currentStatus()));
                NetNetwork::instance().resetStatus();
                return false;
            }
            // Reset join game incase player cancels "I'm Ready" context and wants to join a different game.
            pStartupScreens_->startupData()->resetData();
            pStartupScreens_->startupData()->hostGame(false);
            // Send join message so that host can update availablePlayers list etc.
            pStartupScreens_->messageBroker().sendJoinMessage(
                pStartupScreens_->startupData()->playerName(),
                pStartupScreens_->startupData()->uniqueMachineNumber());
            return true;
        }
    }
    return true;
}

// virtual
void MachGuiCtxJoin::update()
{
    // Get caret to flash in edit box.
    pNewGameName_->update();
    animations_.update();
    pNetworkProtocol_->updateGUI();
    NetNetwork::instance().update();

    const NetNetwork::Sessions& sessions = NetNetwork::instance().sessions();
    if (numGamesInList() != sessions.size())
    {
        const int itemWidth = (JOINGAME_LB_MAXX - JOINGAME_LB_MINX - SCROLLBAR_WIDTH) * MachGui::menuScaleFactor();
        for (std::size_t sessionIndex = numGamesInList(); sessionIndex < sessions.size(); ++sessionIndex)
        {
            new MachGuiCurGamesListBoxItem(
                pStartupScreens_,
                pGamesList_,
                itemWidth,
                sessions.at(sessionIndex),
                *this);
            NETWORK_STREAM("  item added.\n");
        }
        pGamesList_->childrenUpdated();
    }
}

void MachGuiCtxJoin::changeFocus()
{
    pNetworkProtocol_->charFocus();
}

bool MachGuiCtxJoin::validNetworkSettings(bool isHost)
{
    // If network details are not valid, the class handling the protocol will display
    // the appropriate dialog box
    if (! pNetworkProtocol_->validNetworkDetails(isHost))
    {
        return false;
    }
    else
    {
        // Set network protocol settings to be the ones from the GUI
        pNetworkProtocol_->setNetworkDetails();
    }
    // Test for successful network connection
    if (! pNetworkProtocol_->connectionSet())
    {
        return false;
    }
    return true;
}

void MachGuiCtxJoin::editingGameName(bool egn)
{
    editingGameName_ = egn;

    if (editingGameName_)
    {
        pShowGamesBtn_->defaultControl(false);
        pJoinBtn_->defaultControl(false);
        pCreateBtn_->defaultControl(true);

        if (pJoinBtn_->hasFocusSet())
        {
            pJoinBtn_->hasFocus(false);
            pCreateBtn_->hasFocus(true);
        }
    }
    else
    {
        pShowGamesBtn_->defaultControl(true);
        pJoinBtn_->defaultControl(false);
        pCreateBtn_->defaultControl(false);
    }
}

void MachGuiCtxJoin::joinGameSelected(bool jsg)
{
    joinGameSelected_ = jsg;

    if (joinGameSelected_)
    {
        pShowGamesBtn_->defaultControl(false);
        pJoinBtn_->defaultControl(true);
        pCreateBtn_->defaultControl(false);

        if (pCreateBtn_->hasFocusSet())
        {
            pCreateBtn_->hasFocus(false);
            pJoinBtn_->hasFocus(true);
        }
    }
    else
    {
        pShowGamesBtn_->defaultControl(true);
        pJoinBtn_->defaultControl(false);
        pCreateBtn_->defaultControl(false);
    }
}

void MachGuiCtxJoin::onNewGameItemSelected()
{
    editingGameName(true);
}

void MachGuiCtxJoin::onNetSessionSelected(const NetSessionInfo& info)
{
    NetNetwork::instance().setIPAddress(info.address);

    pNetworkProtocol_->readNetworkDetails();

    joinGameSelected(!info.address.empty());
}

/* End CTXJOIN.CPP **************************************************/
