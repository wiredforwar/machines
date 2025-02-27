/*
 * T T F P O L Y . H P P
 * (c) Charybdis Limited, 1997. All Rights Reserved
 */

#ifndef _RENDER_TTFPOLY_HPP
#define _RENDER_TTFPOLY_HPP

#include "base/base.hpp"
#include "base/persist.hpp"
#include "utility/property.hpp"
#include "mathex/point3d.hpp"

#include "render/PrimitiveTopology.hpp"
#include "render/render.hpp"

#include <glm/gtc/matrix_transform.hpp>

class MexVec2;
class RenMaterial;
class RenUVTransform;
class RenTTFRectangle;
class RenITTFImpl;
class RenIVertex;

//////////////////////////////////////////////////////////////////////////
// A turn-to-face polygon, i.e. one which is always parallel to the 3D
// projection plane or screen.  This is an ABC, the 2D geometry in the
// projected plane is supplied by the derived classes.  The most typical
// example is a rectangle (below) which implements sprite-like polygons.
class RenTTFPolygon
{
public:
    virtual ~RenTTFPolygon();

    // The result is allocated with _NEW.  The caller must call _DELETE.
    virtual RenTTFPolygon* clone() const = 0;

    const RenMaterial& material() const;
    void material(const RenMaterial& m);

    UtlProperty<MexPoint3d> centre; // default = (0,0,0)

    // For z-buffering purposes, the polygon's z value is modified by this much.
    // This value is applied along the camera's view direction.  A +ve offset
    // is away from the camera.
    UtlProperty<MATHEX_SCALAR> depthOffset; // default = 0

    const MexVec2& uv(Ren::VertexIdx i) const; // PRE(i < nVertices());
    void uv(const MexVec2&, Ren::VertexIdx i); // PRE(i < nVertices());

    size_t nVertices() const;

    // TTFs effectively model spheres.  How wide is this TTF?
    virtual MATHEX_SCALAR radius() const = 0;

    // For backwards compatiblility: some libraries only have access to the
    // RenTTFPolygon interface and expect this to behave like RenTTFRectangle
    // (as it used to).  They have been reimplmented to use this hack.
    virtual bool isRectangle() const; // returns false
    virtual RenTTFRectangle& asRectangle() = 0; // PRE(isRectangle());
    virtual const RenTTFRectangle& asRectangle() const = 0; // PRE(isRectangle());

    void CLASS_INVARIANT;

    PER_MEMBER_PERSISTENT_ABSTRACT(RenTTFPolygon);
    PER_FRIEND_READ_WRITE(RenTTFPolygon);

protected:
    // The derived class must supply the vertex indices.  They are assumed to be
    // in a fixed pattern which does not vary after the contruction of the TTF.
    RenTTFPolygon(size_t nVertices, const Ren::VertexIdx*, size_t nIndices);
    RenTTFPolygon(const RenTTFPolygon&);

private:
    // Given a centre point and a scale, the derived class must populate the
    // given array with the appropriate 2D geometry (filling out x and y only).
    // The return value indicates the type of triangle geometry required, i.e.,
    // strips, fans or disjoint triangles.
    virtual Ren::PrimitiveTopology compose2DGeometry(RenIVertex*, const MexPoint3d& centre, MATHEX_SCALAR scale) const = 0;

    friend class RenMesh;
    void render(const glm::mat4&, MATHEX_SCALAR scale, const RenUVTransform*) const;
    void render(const glm::mat4&, const RenMaterial&, MATHEX_SCALAR scale, const RenUVTransform*) const;

    friend std::ostream& operator<<(std::ostream&, const RenTTFPolygon&);

    RenITTFImpl* pImpl_;

    virtual void write(std::ostream&) const;

    // Revoked.
    RenTTFPolygon();
    RenTTFPolygon& operator=(const RenTTFPolygon&);
};

PER_DECLARE_PERSISTENT(RenTTFPolygon);

//////////////////////////////////////////////////////////////////////////
// A turn-to-face whose 2D shape is a screen-alligned rectangle.
class RenTTFRectangle : public RenTTFPolygon
{
public:
    RenTTFRectangle();
    RenTTFRectangle(const RenTTFRectangle&);
    ~RenTTFRectangle() override;
    RenTTFPolygon* clone() const override;
    //  virtual RenTTFPolygon& operator=(const RenTTFPolygon&);

    MATHEX_SCALAR width() const; // default = 1
    MATHEX_SCALAR height() const; // default = 1
    void width(MATHEX_SCALAR w); // PRE(w > 0);
    void height(MATHEX_SCALAR h); // PRE(h > 0);

    MATHEX_SCALAR radius() const override;

    void CLASS_INVARIANT;

    bool isRectangle() const override; // returns true
    RenTTFRectangle& asRectangle() override; // PRE(isRectangle());
    const RenTTFRectangle& asRectangle() const override; // PRE(isRectangle());

    PER_MEMBER_PERSISTENT_VIRTUAL(RenTTFRectangle);
    PER_FRIEND_READ_WRITE(RenTTFRectangle);

private:
    Ren::PrimitiveTopology compose2DGeometry(RenIVertex*, const MexPoint3d& centre, MATHEX_SCALAR scale) const override;
    static const Ren::VertexIdx* indices();

    MATHEX_SCALAR width_, height_;

    friend std::ostream& operator<<(std::ostream&, const RenTTFRectangle&);
    void write(std::ostream&) const override;

    // Revoked.
    RenTTFRectangle& operator=(const RenTTFRectangle&);
};

PER_DECLARE_PERSISTENT(RenTTFRectangle);

#endif

/* End TTFPOLY.HPP **************************************************/
