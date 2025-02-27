/*
 * C S 2 D O M A I . C P P
 * (c) Charybdis Limited, 1997. All Rights Reserved
 */

//  Definitions of non-inline non-template methods and global functions

#include "mathex/abox2d.hpp"
#include "mathex/poly2d.hpp"

#include "phys/internal/cs2domai.hpp"

#ifndef _INLINE
#include "phys/internal/cs2domai.ipp"
#endif

//////////////////////////////////////////////////////////////////////////////////////////

PhysCS2dDomain::PhysCS2dDomain(const DomainId& id, const MexAlignedBox2d& boundary)
    : id_(id)
    , boundary_(boundary)
    , pPolygon_(nullptr)
{
    // Set up allocation size for portals vector
    portals_.reserve(4);

    TEST_INVARIANT;
}
//////////////////////////////////////////////////////////////////////////////////////////

PhysCS2dDomain::PhysCS2dDomain(
    const DomainId& id,
    const MexAlignedBox2d& boundary,
    std::unique_ptr<MexPolygon2d>& polygonUPtr)
    : id_(id)
    , boundary_(boundary)
    , pPolygon_(polygonUPtr.release())
{

    TEST_INVARIANT;
}
//////////////////////////////////////////////////////////////////////////////////////////

PhysCS2dDomain::~PhysCS2dDomain()
{
    TEST_INVARIANT;

    if (pPolygon_ != nullptr)
        delete pPolygon_;
}
//////////////////////////////////////////////////////////////////////////////////////////

void PhysCS2dDomain::CLASS_INVARIANT
{
    INVARIANT(this != nullptr);
}
//////////////////////////////////////////////////////////////////////////////////////////

std::ostream& operator<<(std::ostream& o, const PhysCS2dDomain& t)
{
    o << "Boundary " << t.boundary_ << std::endl;
    if (t.pPolygon_ != nullptr)
        o << "Polygon " << (void*)t.pPolygon_ << std::endl;

    o << "Portal ids ";
    for (size_t i = 0; i != t.portals_.size(); ++i)
        o << " " << t.portals_[i].asScalar();
    o << std::endl;
    return o;
}
//////////////////////////////////////////////////////////////////////////////////////////

bool PhysCS2dDomain::contains(const MexPoint2d& point) const
{
    // Check inside bounding box
    bool containsIt = boundary_.contains(point);

    // If a polygon supplied, check inside it
    if (pPolygon_ != nullptr)
        containsIt = pPolygon_->contains(point);

    return containsIt;
}
//////////////////////////////////////////////////////////////////////////////////////////
/* End CS2DOMAI.CPP *************************************************/
