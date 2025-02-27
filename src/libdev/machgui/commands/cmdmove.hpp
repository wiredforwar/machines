/*
 * C M D M O V E . H P P
 * (c) Charybdis Limited, 1997. All Rights Reserved
 */

/*
    MachGuiMoveCommand

    Implement interaction for the Move command
*/

#ifndef _MACHGUI_CMDMOVE_HPP
#define _MACHGUI_CMDMOVE_HPP

#include "machgui/commands/command.hpp"

#include "base/base.hpp"
#include "ctl/vector.hpp"
#include "mathex/point3d.hpp"

// Forward refs
class MachLogConstruction;
class MachLogAPC;
class MachLogMachine;

// orthodox canonical (revoked)
class MachGuiMoveCommand : public MachGuiCommand
{
public:
    // ctor.
    MachGuiMoveCommand(MachInGameScreen* pInGameScreen);

    // dtor
    ~MachGuiMoveCommand() override;

    /////////////////////////////////////////////////
    // Inherited from MachGuiCommand

    // Return new instance of this command
    std::unique_ptr<MachGuiCommand> clone() const override;

    // The resource file id of the prompt to be displayed when the cursor moves
    // over the command icon
    uint cursorPromptStringId() const override;

    // The resource file id of the prompt to be displayed when the command is clicked
    uint commandPromptStringid() const override;

    // reference to the up and down bitmap names for this command
    const std::pair<std::string, std::string>& iconNames() const override;

    // Respond to pick at location on the terrain.
    // ctrl/shift/altPressed indicate which modifier keys were pressed at the time.
    void pickOnTerrain(const MexPoint3d& location, bool ctrlPressed, bool shiftPressed, bool altPressed) override;

    // respond to pick on pActor in the world view window.
    // ctrl/shift/altPressed indicate which modifier keys were pressed at the time.
    void pickOnActor(MachActor* pActor, bool ctrlPressed, bool shiftPressed, bool altPressed) override;

    // Respond to cursor at location on the terrain.
    // ctrl/shift/altPressed indicate which modifier keys are pressed.
    // Returns 2d cursor to be displayed.
    MachGui::Cursor2dType
    cursorOnTerrain(const MexPoint3d& location, bool ctrlPressed, bool shiftPressed, bool altPressed) override;

    // Respond to cursor on pActor in the world view window.
    // ctrl/shift/altPressed indicate which modifier keys are pressed.
    // Returns 2d cursor to be displayed.
    MachGui::Cursor2dType
    cursorOnActor(MachActor* pActor, bool ctrlPressed, bool shiftPressed, bool altPressed) override;

    // Set the type data associated with the command.
    void typeData(MachLog::ObjectType objectType, int subType, uint level) override;

    // True if this is a kind of command that actor can ever execute.
    bool canActorEverExecute(const MachActor& actor) const override;

    // True if this command should be visible based on selected actor set.
    // Ignores actor race in determining result.
    // As a side-effect, updates lowest minimum construction access size index for selected group.
    bool actorsCanExecute() const override;

    // True iff there is an administer version of the command for squadrons
    // with an administrator. Default implementation returns false.
    bool canAdminApply() const override;

    // True if the interaction for the command is complete
    bool isInteractionComplete() const override;

    /////////////////////////////////////////////////
    // Hot key processing
    bool processButtonEvent(const DevButtonEvent&) override;

    static bool couldEnterAPC(const MachActor& actor, const MachLogAPC& apc);

    // at least one of corral actors is able to enter the building in question right now given the
    // access size details AND space restrictions inside (occupied stations etc.)
    static bool atLeastOneCorralActorCanEnterConstruction(
        const MachInGameScreen& inGameScreen,
        const MachLogConstruction& construction);

    void CLASS_INVARIANT;

    friend std::ostream& operator<<(std::ostream& o, const MachGuiMoveCommand& t);

private:
    // Operation deliberately revoked
    MachGuiMoveCommand(const MachGuiMoveCommand&);
    MachGuiMoveCommand& operator=(const MachGuiMoveCommand&);
    bool operator==(const MachGuiMoveCommand&);

    /////////////////////////////////////////////////
    // Inherited from MachGuiCommand

    // Execute the command for pActor
    bool doApply(MachActor* pActor, std::string* pReason) override;

    // Execute the command as a sqaudron whose most intelligent administrator
    // is pAdministrator. If successful return true. Otherwise false, with
    // a prompt std::string indicating reason for failure in pReason.
    bool doAdminApply(MachLogAdministrator* pAdministrator, std::string* pReason) override;
    // PRE( canAdminApply() );

    //  True iff this command can be applied to a group of actors at once
    bool canApplyToGroup() const override;

    using Actors = ctl_pvector<MachActor>;

    // Execute the command for all actors in actors, returning true if done so,
    // or false if not possible. If the latter, a prompt std::string explaining
    // the reason is returned in pReason.
    bool doGroupApply(const Actors& actors, std::string* pReason) override;
    // PRE( isInteractionComplete() )

    /////////////////////////////////////////////////

    // returns ref to static variable storing lowest minimum construction access size index of selected group
    static int& lowestMinimumAccessSize();

    enum Action
    {
        SIMPLE_MOVE,
        ENTER_BUILDING,
        ENTER_APC,
        FOLLOW_MACHINE
    };

    // Implement various flavours of command
    bool applySimpleMove(MachActor* pActor, std::string* pReason);
    bool applyEnterBuilding(MachActor* pActor, std::string* pReason);
    bool applyEnterAPC(MachActor* pActor, std::string* pReason);
    bool applyFollowMachine(MachActor* pActor, std::string* pReason);

    // Data members
    Action action_; // Denotes the action selected by the move command
    ctl_vector<MexPoint3d> points_; // The points to move to. All but the last are way-points
    MachLogConstruction* pConstruction_; // Building to attempt to enter
    MachLogMachine* pMachine_; // Target machine
    MachLogAPC* pAPC_; // Pointer to APC to enter
    bool hadFinalPick_; // True when the final destination pick has been received
};

#endif

/* End CMDMOVE.HPP **************************************************/
