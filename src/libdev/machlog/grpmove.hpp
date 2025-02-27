/*
 * G R P M O V E . H P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

/*
    MachLogGroupSimpleMove

    Implement a group of actors all being tasked with the same
    move operation at the same time.
*/

#ifndef _MACHLOG_GRPMOVE_HPP
#define _MACHLOG_GRPMOVE_HPP

#include "base/base.hpp"
#include "mathex/point3d.hpp"
#include "machlog/actor.hpp"

#include <string>

class MachLogGroupSimpleMoveImplementation;
template <class T> class ctl_pvector;
template <class T> class ctl_vector;

class MachLogGroupSimpleMove
// Canonical form revoked
{
public:
    using Actors = ctl_pvector<MachActor>;
    using Points = ctl_vector<MexPoint3d>;

    MachLogGroupSimpleMove(
        const Actors&,
        const Points&,
        size_t commandId,
        std::string* pReason,
        PhysPathFindingPriority pathFindingPriority);

    ~MachLogGroupSimpleMove();

    //  Return true iff the move command was successful
    bool moveOK() const;

    void CLASS_INVARIANT;

private:
    friend std::ostream& operator<<(std::ostream& o, const MachLogGroupSimpleMove& t);

    MachLogGroupSimpleMove(const MachLogGroupSimpleMove&);
    MachLogGroupSimpleMove& operator=(const MachLogGroupSimpleMove&);

    MachLogGroupSimpleMoveImplementation* pImpl_;
};

#endif

/* End GRPMOVE.HPP **************************************************/
