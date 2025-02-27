/*
 * A B O X 2 D . H P P
 * (c) Charybdis Limited, 1995, 1996. All Rights Reserved
 */

//////////////////////////////////////////////////////////////////////

#ifndef _MEX_BOX2_HPP
#define _MEX_BOX2_HPP

#include "base/persist.hpp"

#include "mathex/mathex.hpp"
#include "mathex/point2d.hpp"
#include "mathex/size2d.hpp"

class MexLine2d;
class MexTriangle2d;
class MexConvexPolygon2d;
class MexCompressedAlignedBox2d;

class MexAlignedBox2d
{
public:
    MexAlignedBox2d() = default;

    // Constructs the aligend box bounding both p1 and p2
    MexAlignedBox2d(const MexPoint2d& p1, const MexPoint2d& p2);
    MexAlignedBox2d(const MexPoint2d& position, const MexSize2d& size);
    MexAlignedBox2d(const MexSize2d& size);

    MexAlignedBox2d(const MexPoint2d& minCorner, double width, double height);
    // PRE(width  >= 0);
    // PRE(height >= 0);

    MexAlignedBox2d(const MATHEX_SCALAR x1, const MATHEX_SCALAR y1, const MATHEX_SCALAR x2, const MATHEX_SCALAR y2);
    // PRE( x1 <= x2 and y1 <= y2 );

    MexAlignedBox2d(const MexAlignedBox2d& rhs);
    // POST( *this == rhs );

    // constructs aligned box bounding p1, p2 plus at least clearance in all directions
    MexAlignedBox2d(const MexPoint2d& p1, const MexPoint2d& p2, MATHEX_SCALAR clearance);
    // PRE( clearance >= 0 )

    // constructs square aligned box with midpoint at centre and dimension 2*halfSideLength
    MexAlignedBox2d(const MexPoint2d& centre, MATHEX_SCALAR halfSideLength);
    // PRE( halfSideLength > 0 )

    MexAlignedBox2d(const MexCompressedAlignedBox2d&);

    ~MexAlignedBox2d();

    bool contains(const MexAlignedBox2d&) const;
    bool contains(const MexPoint2d&) const;

    bool intersects(const MexLine2d& line) const;
    bool intersects(const MexTriangle2d& tri) const;
    bool intersects(const MexAlignedBox2d& box, Mathex::TouchingRule rule = Mathex::TOUCH_IS_INTERSECT) const;
    // POST( iff( result, at least one point in rhs is in this ) )

    void intersectWith(const MexAlignedBox2d& rhs);
    // POST( iff( *this == old( *this ), isEmpty() or rhs.contains( *this ) ) );

    void unionWith(const MexAlignedBox2d& rhs);
    void unionWith(const MexConvexPolygon2d& rhs);
    void unionWith(const MexPoint2d& rhs);
    // POST( iff( *this == old( *this ), rhs.isEmpty() or rhs == *this ) );

    //  bool isEmpty() const;
    // POST( iff( result, vmin_.isZeroVector() and vmax_.isZeroVector() ); // private

    const MexAlignedBox2d& operator=(const MexAlignedBox2d&);
    // POST( vmin_ == result.vmin_ and vmax_ == result.vmax_ );

    bool operator==(const MexAlignedBox2d&) const;
    bool operator!=(const MexAlignedBox2d&) const;
    // POST( iff( result, rhs.contains( *this ) and contains( rhs ) ) )

    MexAlignedBox2d& operator*=(MATHEX_SCALAR multiplier);

    const MexPoint2d& minCorner() const;
    // POST( &result == &vmin_ ); // private

    const MexPoint2d& maxCorner() const;
    // POST( &result == &vmax_ ); // private

    MATHEX_SCALAR width() const;
    MATHEX_SCALAR height() const;
    MexSize2d size() const;

    MATHEX_SCALAR area() const;

    void corners(const MexPoint2d& newMinCorner, const MexPoint2d& newMaxCorner);
    // PRE( newMinCorner.x() <= newMaxCorner.x() )
    // PRE( newMinCorner.y() <= newMaxCorner.y() )

    void minCorner(const MexPoint2d& newVal);
    // PRE( newVal is lexicographically less than or equal to maxCorner() );

    void maxCorner(const MexPoint2d& newVal);
    // PRE( newVal is lexicographically greater than or equal to maxCorner() );

    void xMin(MATHEX_SCALAR xNew);
    // PRE( xNew <= maxCorner.x() )

    void xMax(MATHEX_SCALAR xNew);
    // PRE( xNew >= minCorner.x() )

    void yMin(MATHEX_SCALAR yNew);
    // PRE( yNew <= maxCorner.y() )

    void yMax(MATHEX_SCALAR yNew);
    // PRE( yNew >= minCorner.y() )

    MATHEX_SCALAR top() const { return vmin_.y(); }
    MATHEX_SCALAR bottom() const { return vmax_.y(); }
    MATHEX_SCALAR left() const { return vmin_.x(); }
    MATHEX_SCALAR right() const { return vmax_.x(); }

    MexPoint2d topLeft() const { return vmin_; }
    MexPoint2d topRight() const { return { right(), top() }; }
    MexPoint2d bottomLeft() const { return { left(), bottom() }; }
    MexPoint2d bottomRight() const { return { right(), bottom() }; }

    // returns the point on the box boundary that is the nearest to the external reference point
    MexPoint2d nearestPointToExternalPoint(const MexPoint2d& externalPoint) const;
    // PRE( not (    externalPoint.x() > minCorner().x()
    //           and externalPoint.x() < maxCorner().x()
    //           and externalPoint.y() > minCorner().y()
    //           and externalPoint.y() < maxCorner().y() ) );

    // Persistence
    PER_MEMBER_PERSISTENT_DEFAULT(MexAlignedBox2d)
    PER_FRIEND_READ_WRITE(MexAlignedBox2d)

protected:
    void CLASS_INVARIANT
    {
        INVARIANT(minCorner().x() <= maxCorner().x());
        INVARIANT(minCorner().y() <= maxCorner().y());
    };

private:
    friend bool operator<(const MexAlignedBox2d&, const MexAlignedBox2d&);
    friend bool operator>(const MexAlignedBox2d&, const MexAlignedBox2d&);
    friend bool operator<=(const MexAlignedBox2d&, const MexAlignedBox2d&);
    friend bool operator>=(const MexAlignedBox2d&, const MexAlignedBox2d&);
    friend class MexCompressedAlignedBox2d;

    MexPoint2d vmin_;
    MexPoint2d vmax_;
    bool isEmpty_ = false;
};

inline MexAlignedBox2d& MexAlignedBox2d::operator*=(MATHEX_SCALAR multiplier)
{
    vmin_ *= multiplier;
    vmax_ *= multiplier;
    return *this;
}

inline MexAlignedBox2d operator*(MexAlignedBox2d rhs, MATHEX_SCALAR c)
{
    return rhs *= c;
}

PER_DECLARE_PERSISTENT(MexAlignedBox2d);

// class ostream;
std::ostream& operator<<(std::ostream&, const MexAlignedBox2d&);

//////////////////////////////////////////////////////////////////////

#ifdef _INLINE
#include "mathex/abox2d.ipp"
#endif

//////////////////////////////////////////////////////////////////////////

#endif

/* End ABOX2D.HPP *****************************************************/
