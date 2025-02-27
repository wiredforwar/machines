/*
 * D B H A N D L R . H P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

/*
    MachGuiDatabaseHandler

    Provides an interface for the functionality needed between the player database and machlog.
*/

#ifndef _MACHGUI_DBHANDLR_HPP
#define _MACHGUI_DBHANDLR_HPP

#include "base/base.hpp"
#include "machlog/dbhandlr.hpp"

class MachGuiDatabaseHandler : public MachLogDatabaseHandler
// Canonical form revoked
{
public:
    MachGuiDatabaseHandler();

    ~MachGuiDatabaseHandler();

    /////////////////////////////////////////////
    // Inherited from Machlog

    // Return a production unit list for the machines surviving for the specified arc at the
    // end of the named scenario (for current player)
    const Units& survivingUnits(MachPhys::Race race, const std::string& scenarioName) const override;

    // true if the named flag was set by the current player during the named scenario
    bool isFlagSet(const std::string& flag, const std::string& scenarioName) const override;

    // The number of sub-tasks if any defined in the current scenario
    uint nTasksInCurrentScenario() const override;

    // True iff the index'th task of the current task is available from the start
    bool taskStartsAvailable(uint index) const override;

    /////////////////////////////////////////////

    void CLASS_INVARIANT;

private:
    friend std::ostream& operator<<(std::ostream& o, const MachGuiDatabaseHandler& t);

    MachGuiDatabaseHandler(const MachGuiDatabaseHandler&);
    MachGuiDatabaseHandler& operator=(const MachGuiDatabaseHandler&);
};

#endif

/* End DBHANDLR.HPP *************************************************/
