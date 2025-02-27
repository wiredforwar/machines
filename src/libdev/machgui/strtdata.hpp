/*
 * S T R T D A T A . H P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

/*
    MachGuiStartupData

    A brief description of the class should go in here
*/

#ifndef _MACHGUI_STRTDATA_HPP
#define _MACHGUI_STRTDATA_HPP

#include "base/base.hpp"
#include "stdlib/string.hpp"
#include "ctl/vector.hpp"
#include "machphys/machphys.hpp"
#include "machlog/machlog.hpp"

class StartupInfo;
class MachGuiDbScenario;
class MachGuiCtxImReady;
class MachGuiStartupScreens;

#define MAX_PLAYERNAME_LEN 20
#define MAX_GAMENAME_LEN 30
#define MAX_CHATMESSAGE_LEN 511
#define MAX_SCENARIONAME_LEN 20
#define MAX_PLANETNAME_LEN 20
#define MAX_SAVEGAMENAME_LEN 25
#define MAX_INGAMECHATMESSAGE_LEN 200

class MachGuiStartupData
// Canonical form revoked
{
public:
    enum VictoryCondition : uint8
    {
        VC_DEFAULT,
        VC_TOTALANNIHILATION,
        VC_DESTROYPOD,
        VC_TIMED5,
        VC_TIMED10,
        VC_TIMED15,
        VC_TIMED30,
        VC_TIMED45,
        VC_TIMED60,
        VC_TIMED90,
        VC_TIMED120,
        VC_TIMED180
    };
    enum InitialiseConnection : uint8
    {
        INITIALISE_CONNECTION,
        DO_NOT_INITIALISE_CONNECTION
    };

    MachGuiStartupData(MachGuiStartupScreens*);
    ~MachGuiStartupData();

    void CLASS_INVARIANT;

#pragma pack(push, 1)
    struct PlayerInfo
    {
        PlayerInfo();

        enum Status : uint8
        {
            COMPUTER,
            OPEN,
            CLOSED,
            HUMAN,
            UNAVAILABLE,
            CONNECTION_LOST
        };

        std::string getDisplayName() const;
        void setPlayerName(const std::string&);

        void reset();

        char playerName_[MAX_PLAYERNAME_LEN + 1];
        MachPhys::Race race_;
        Status status_;
        bool ready_;
        bool host_;
        bool hasMachinesCD_;
    };

    struct GameSettings
    {
        GameSettings();
        ~GameSettings();

        uint mapSizeId_;
        uint terrainTypeId_;
        uint scenarioId_;
        MachLog::ResourcesAvailable resources_;
        bool fogOfWar_;
        MachLog::StartingResources startingResources_;
        MachLog::RandomStarts startingPosition_;
        VictoryCondition victoryCondition_;
        int numPlayers_;
        MachPhys::Race playerRace_;
        MachLog::TechnologyLevel techLevel_;
        uint32_t randomStartsSeed_;
        bool broadcastAlliances_;
        bool disableFirstPerson_;
    };
#pragma pack(pop)

    // Name of player using this computer in multiplayer game
    const std::string& playerName() const;
    void playerName(const std::string&);

    // Set/Get the scenario that will be played.
    MachGuiDbScenario* scenario();
    void scenario(MachGuiDbScenario*);

    // Set and get network protocol
    const std::string& connectionType() const;
    void connectionType(const std::string&, InitialiseConnection = DO_NOT_INITIALISE_CONNECTION);

    void resetData();

    // Name of new game if one is created
    const std::string& newGameName() const;
    void newGameName(const std::string& gameName);

    // Network message handling
    void receivedJoinMessage(const std::string& playerName, int uniqueMachineNumber);
    void receivedImReadyMessage(const std::string& playerName, bool ready);
    void receivedUpdatePlayersMessage();
    void receivedRaceChangeRequest(const std::string& playerName, size_t playerIndex, MachPhys::Race);
    void receivedHostCancelMessage();
    void receivedClientCancelMessage(const std::string& playerName);
    void receivedStartMessage();
    void receivedUpdateGameSettingsMessage();
    void receivedInGameChatMessage(const std::string& message, MachPhys::Race intendedRace);
    void receivedHasMachinesCDMessage(const std::string& playerName, bool hasMachinesCD);
    void receivedNameChangeMessage(const std::string& newPlayerName, int uniqueMachineNumber);

    // Set nework message telling all nodes to update their player information.
    void sendUpdatePlayersMessage();
    // PRE( isHost() );

    // Set network message to get all players to update there game setting info.
    void sendUpdateGameSettingsMessage();
    // PRE( isHost() );

    // Get/Set host, host name etc.
    void hostGame(bool);
    bool isHost() const;
    std::string getHostName() const;

    // When certain network messages are received the Ready context needs to be
    // refreshed to ensure that the correct information is displayed.
    void setCtxImReady(MachGuiCtxImReady*);
    void removeCtxImReady();

    // Returns list of available players ( NOT including host playerName )
    ctl_vector<std::string>& availablePlayers();

    // Get player information. 4 slots.
    PlayerInfo* players();

    // Reset all 4 player info slots
    void resetPlayers();

    // Check to see if a particular race ( e.g. MachPhys::RED ) is still available
    // in a multiplayer game
    bool isRaceAvailable(MachPhys::Race) const;

    // Has a network player pressed the "I'm Ready" button?
    bool isReady(const std::string& playerName);

    // Can the host start the game ( i.e. has everyone pressed the "I'm Ready" button )
    bool canStartMultiPlayerGame() const;

    bool doesAtLeastOnePlayerHaveMachinesCD() const;
    // PRE( isHost() );

    // As more than 4 players can attempt to join a game this query is used to
    // see if the player on the local computer is being considered for the game.
    bool includedInGame() const;
    bool updateIncludedInGame();

    // True if host has died/canceled or game has started without player.
    bool terminalMultiPlayerGameProblem() const;

    // Setup MachLogNetwork just before a network game begins.
    void initMachLogNetwork();

    // Get multiplayer game settings
    GameSettings* gameSettings();

    // Get/set number of resources ( high, normal, low ).
    void resources(MachLog::ResourcesAvailable);
    void resources(const std::string&);
    MachLog::ResourcesAvailable resources() const;
    std::string resourcesStr() const;

    // Get/set number of starting resources.
    void startingResources(MachLog::StartingResources);
    void startingResources(const std::string&);
    MachLog::StartingResources startingResources() const;
    std::string startingResourcesStr() const;

    // Get/set broadcast alliances on/off for multi player games.
    void broadcastAlliances(bool);
    void broadcastAlliances(const std::string&);
    bool broadcastAlliances() const;
    std::string broadcastAlliancesStr() const;

    // Get/set 1st person on/off for multi player games.
    void disableFirstPerson(bool);
    void disableFirstPerson(const std::string&);
    bool disableFirstPerson() const;
    std::string disableFirstPersonStr() const;

    // Get/set fogOfWar on/off for skirmish and multi player games.
    void fogOfWar(bool);
    void fogOfWar(const std::string&);
    bool fogOfWar() const;
    std::string fogOfWarStr() const;

    // Get/set starting position ( fixed/random ).
    void startingPosition(MachLog::RandomStarts);
    void startingPosition(const std::string&);
    MachLog::RandomStarts startingPosition() const;
    std::string startingPositionStr() const;

    // Get/set victory condition for a multiplayer/skirmish game.
    void victoryCondition(VictoryCondition);
    void victoryCondition(const std::string&);
    VictoryCondition victoryCondition() const;
    std::string victoryConditionStr() const;

    // Get/set number of players for a multiplayer/skirmish game.
    void numPlayers(int);
    // PRE( numPlayers >= 2 and numPlayers <= 4 );
    void numPlayers(const std::string&);
    int numPlayers() const;
    std::string numPlayersStr() const;

    // Get/set tech level for a multiplayer/skirmish game.
    void techLevel(MachLog::TechnologyLevel);
    void techLevel(const std::string&);
    MachLog::TechnologyLevel techLevel() const;
    std::string techLevelStr() const;

    // Get/set player race for a skirmish game.
    void playerRace(MachPhys::Race);
    void playerRace(const std::string&);
    MachPhys::Race playerRace() const;
    std::string playerRaceStr() const;

    // get the random starts seed such that all nodes on the network are looking at the same thing.
    int randomStartSeed() const;

    // Get/set flag to specify if transtion flics wil be played
    bool transitionFlicsOn() const;
    void transitionFlicsOn(bool flicOn);

    int uniqueMachineNumber() const;

protected:
    // 0-3. -1 indicates full
    int getNextAvailablePlayerSlot() const;

    MachPhys::Race getNextAvailableRace() const;

private:
    friend std::ostream& operator<<(std::ostream& o, const MachGuiStartupData& t);
    friend std::ostream& operator<<(std::ostream& o, const MachGuiStartupData::PlayerInfo& t);
    friend std::ostream& operator<<(std::ostream& o, const MachGuiStartupData::PlayerInfo::Status& t);
    friend std::ostream& operator<<(std::ostream& o, const MachGuiStartupData::GameSettings& t);

    MachGuiStartupData(const MachGuiStartupData&);
    MachGuiStartupData& operator=(const MachGuiStartupData&);

    std::string playerName_;
    MachGuiDbScenario* pScenario_;
    bool transitionFlicsOn_;
    bool host_;
    MachGuiCtxImReady* pCtxImReady_;
    ctl_vector<std::string> availablePlayers_;
    PlayerInfo players_[4]; // Four available slots for players
    MachGuiStartupScreens* pStartupScreens_;
    bool includedInGame_;
    bool terminalMultiPlayerGameProblem_;
    std::string newGameName_;
    GameSettings gameSettings_;
    MachPhys::Race playerRace_;
    std::string lastProtocol_;
    int uniqueMachineNumber_; // Used to identify this machine on the network
};

#endif

/* End STRTDATA.HPP *************************************************/
