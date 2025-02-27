/*
 * D I S P L A Y A B L E . C P P
 * (c) Charybdis Limited, 1997. All Rights Reserved.
 */

#define CB_GUIDISPLAYABLE_DEPIMPL()                                                                                    \
    CB_DEPIMPL(GuiDisplayable*, pParent_);                                                                             \
    CB_DEPIMPL(Gui::Box, relativeBox_);                                                                                \
    CB_DEPIMPL(Gui::Box, absoluteBox_);                                                                                \
    CB_DEPIMPL(bool, isVisible_);                                                                                      \
    CB_DEPIMPL(int, changed_);                                                                                         \
    CB_DEPIMPL(bool, redrawEveryFrame_);                                                                               \
    CB_DEPIMPL_ARRAY(Children, children_);                                                                             \
    CB_DEPIMPL(Children, allChildren_);                                                                                \
    CB_DEPIMPL(bool, useFastSecondDisplay_);

#include "gui/displaya.hpp"
#include "gui/internal/displayi.hpp"
#include "ctl/vector.hpp"
#include "ctl/algorith.hpp"
#include "mathex/vec2.hpp"
#include "world4d/manager.hpp"
#include "world4d/scenemgr.hpp"
#include "render/device.hpp"
#include "gui/manager.hpp"
#include "gui/painter.hpp"
#include "device/butevent.hpp"

//////////////////////////////////////////////////////////////////////

GuiDisplayable::GuiDisplayable(GuiDisplayable* pParent, Layer layer)
    : pImpl_(new GuiDisplayableImpl)
{
    pImpl_->pParent_ = pParent;
    pImpl_->enabled_ = true;
    pImpl_->isVisible_ = true;
    pImpl_->redrawEveryFrame_ = false;
    pImpl_->useFastSecondDisplay_ = true;

    for (int layer = LAYER1; layer < NUMLAYERS; ++layer)
    {
        pImpl_->children_[layer].reserve(2);
    }
    pImpl_->allChildren_.reserve(4);

    changed(true);

    if (pParent)
    {
        pImpl_->pParent_->addChild(this, layer);
    }
}

GuiDisplayable::GuiDisplayable(GuiDisplayable* pParent, const Gui::Boundary& relativeBoundary, Layer myLayer)
    : GuiDisplayable(pParent, myLayer)
{
    CB_GUIDISPLAYABLE_DEPIMPL();

    PRE(pParent != nullptr);

    setRelativeBoundary(relativeBoundary);
    POST_INFO(pParent->absoluteBoundary());
    POST_INFO(absoluteBoundary());
    POST(pParent->absoluteBoundary().contains(absoluteBoundary()));
    POST(useFastSecondDisplay());
}

GuiDisplayable::GuiDisplayable(const Gui::Boundary& absBoundary)
    : GuiDisplayable(nullptr)
{
    CB_GUIDISPLAYABLE_DEPIMPL();

    relativeBox_ = absBoundary;
    absoluteBox_ = absBoundary;

    POST(useFastSecondDisplay());
}

GuiDisplayable::~GuiDisplayable()
{
    CB_GUIDISPLAYABLE_DEPIMPL();

    deleteAllChildren();

    if (pParent_ != nullptr)
        pParent_->removeChild(this);

    GuiManager::instance().isBeingDeleted(this);

    delete pImpl_;
}

//////////////////////////////////////////////////////////////////////

Gui::Coord GuiDisplayable::relativeCoord() const
{
    CB_GUIDISPLAYABLE_DEPIMPL();

    return relativeBox_.minCorner();
}

Gui::Coord GuiDisplayable::absoluteCoord() const
{
    CB_GUIDISPLAYABLE_DEPIMPL();

    return absoluteBox_.minCorner();
}

void GuiDisplayable::relativeCoord(const Gui::Coord& relCoord)
{
    CB_GUIDISPLAYABLE_DEPIMPL();

    // Construct the displacement vector
    MexVec2 diff(relativeBox_.minCorner(), relCoord);
    PRE_INFO(relativeBox_.minCorner());
    PRE_INFO(relCoord);

    POST_DATA(Gui::Box oldBoundary = absoluteBoundary());

    PRE_DATA(Gui::Coord c(diff.x(), diff.y()));
    PRE_DATA(Gui::Box parentBoundary = isRoot() ? absoluteBoundary() : parent()->absoluteBoundary());
    PRE_DATA(Gui::Box newBoundary = translateBox(absoluteBoundary(), c));
    PRE_INFO(c);
    PRE_INFO(absoluteBoundary());
    PRE_INFO(parentBoundary);
    PRE_INFO(newBoundary);

    Gui::Coord newRelMin = relativeBox_.minCorner();
    newRelMin += diff;
    Gui::Coord newRelMax = relativeBox_.maxCorner();
    newRelMax += diff;
    relativeBox_.corners(newRelMin, newRelMax);

    Gui::Coord absCoord = relativeCoord();
    if (! isRoot())
        absCoord += parent()->absoluteCoord();

    absoluteBox_ = Gui::Box(absCoord, relativeBox_.size());

    for (Children::iterator i = allChildren_.begin(); i != allChildren_.end(); ++i)
    {
        Gui::Coord childCoord = (*i)->absoluteCoord();
        childCoord += diff;
        positionChildAbsolute(*i, childCoord);
    }
}

void GuiDisplayable::absoluteCoord(const Gui::Coord& absCoord)
{
    CB_GUIDISPLAYABLE_DEPIMPL();

    PRE_DATA(Gui::Box newBoundary(absCoord, width(), height()));
    PRE_DATA(Gui::Box parentBoundary = isRoot() ? absoluteBoundary() : parent()->absoluteBoundary());
    PRE_INFO(absCoord);
    PRE_INFO(newBoundary);
    PRE_INFO(parentBoundary);
    PRE(implies(! isRoot() && isVisible(), parentBoundary.contains(newBoundary)));

    // Construct the displacement vector
    MexVec2 diff(absoluteBox_.minCorner(), absCoord);

    Gui::Coord newAbsMin = absoluteBox_.minCorner();
    newAbsMin += diff;

    Gui::Coord newAbsMax = absoluteBox_.maxCorner();
    newAbsMax += diff;
    absoluteBox_.corners(newAbsMin, newAbsMax);

    Gui::Coord relCoord = absCoord;
    if (! isRoot())
        relCoord -= parent()->absoluteCoord();
    relativeBox_ = Gui::Box(relCoord, absoluteBox_.size());

    for (Children::iterator i = allChildren_.begin(); i != allChildren_.end(); ++i)
    {
        Gui::Coord childCoord = (*i)->absoluteCoord();
        childCoord += diff;
        positionChildAbsolute(*i, childCoord);
    }

    POST(absoluteCoord() == absCoord);
    POST(absoluteBoundary() == Gui::Box(absCoord, width(), height()));
}

//////////////////////////////////////////////////////////////////////

void GuiDisplayable::positionChildAbsolute(GuiDisplayable* pChild, const Gui::Coord& absCoord)
{
    CB_GUIDISPLAYABLE_DEPIMPL();

    PRE(pChild != nullptr);
    PRE(hasChild(pChild));

    pChild->absoluteCoord(absCoord);

    POST(pChild->absoluteCoord() == absCoord);
}

void GuiDisplayable::positionChildRelative(GuiDisplayable* pChild, const Gui::Coord& relCoord)
{
    CB_GUIDISPLAYABLE_DEPIMPL();

    PRE(pChild != nullptr);
    PRE(hasChild(pChild));

    pChild->relativeCoord(relCoord);

    POST(pChild->relativeCoord() == relCoord);
}

//////////////////////////////////////////////////////////////////////

void GuiDisplayable::isVisible(bool visible)
{
    CB_GUIDISPLAYABLE_DEPIMPL();

    bool makeVisible = visible && isEligableForVisibility() && ! isVisible_;

    if (makeVisible)
        changed(true);

    isVisible_ = visible;
}

//////////////////////////////////////////////////////////////////////

bool GuiDisplayable::isVisible() const
{
    CB_GUIDISPLAYABLE_DEPIMPL();

    bool parentVisible = true;

    // Check parents visibility
    if (pParent_)
        parentVisible = pParent_->isVisible();

    // Return true if "this" is visible and my parent(s) is visible etc.
    return isVisible_ && parentVisible && isEligableForVisibility();
}

// virtual
bool GuiDisplayable::isEligableForVisibility() const
{
    return true;
}

void GuiDisplayable::changed(bool change)
{
    CB_DEPIMPL(int, changed_);

    if (change)
    {
        // This indicates that the gui displayable needs to be rendered twice ( back buffer and front buffer )
        if (useFourTimesRender())
        {
            changed_ = 4;
        }
        else
        {
            changed_ = 2;
        }
    }
    else if (changed_ != 0)
    {
        --changed_;
    }
}

bool GuiDisplayable::hasChanged() const
{
    CB_DEPIMPL(int, changed_);

    return changed_ != 0;
}

bool GuiDisplayable::isRoot() const
{
    CB_GUIDISPLAYABLE_DEPIMPL();

    return pParent_ == nullptr;
}

//////////////////////////////////////////////////////////////////////

void GuiDisplayable::addChild(GuiDisplayable* pNewChild, Layer childsLayer)
{
    CB_GUIDISPLAYABLE_DEPIMPL();

    PRE(! hasChild(pNewChild));
    PRE_INFO(absoluteBoundary());
    PRE_INFO(pNewChild->absoluteBoundary());

    allChildren_.push_back(pNewChild);
    children_[childsLayer].push_back(pNewChild);

    POST(hasChild(pNewChild));
}

// virtual
void GuiDisplayable::doRemoveChild(GuiDisplayable* /*pChild*/)
{
}

void GuiDisplayable::removeChild(GuiDisplayable* pChild)
{
    CB_GUIDISPLAYABLE_DEPIMPL();

    PRE(hasChild(pChild));

    Children::iterator i = find(allChildren_.begin(), allChildren_.end(), pChild);

    if (i != allChildren_.end())
    {
        doRemoveChild(pChild);
        allChildren_.erase(i);
    }

    for (Layer layer = LAYER1; layer < NUMLAYERS; ++((int&)layer))
    {
        Children::iterator i = find(children_[layer].begin(), children_[layer].end(), pChild);

        if (i != children_[layer].end())
        {
            children_[layer].erase(i);
            break;
        }
    }

    POST(! hasChild(pChild));
}

void GuiDisplayable::deleteChild(GuiDisplayable* pChild)
{
    CB_GUIDISPLAYABLE_DEPIMPL();

    Children::iterator i = find(allChildren_.begin(), allChildren_.end(), pChild);

    if (i != allChildren_.end())
    {
        delete *i; // Child automatically removes itself from allChildren_ collection on deletion
    }

    POST(! hasChild(pChild));
}

void GuiDisplayable::deleteAllChildren()
{
    CB_GUIDISPLAYABLE_DEPIMPL();

    // This works because on deletion the child will remove itself from the parents child collection.
    while (allChildren_.size() != 0)
        delete *allChildren_.begin();
}

//////////////////////////////////////////////////////////////////////

GuiDisplayable::Children& GuiDisplayable::children()
{
    CB_GUIDISPLAYABLE_DEPIMPL();

    return allChildren_;
}

const GuiDisplayable::Children& GuiDisplayable::children() const
{
    CB_GUIDISPLAYABLE_DEPIMPL();

    return allChildren_;
}

GuiDisplayable::Children& GuiDisplayable::children(Layer layer)
{
    CB_GUIDISPLAYABLE_DEPIMPL();

    return children_[layer];
}

const GuiDisplayable::Children& GuiDisplayable::children(Layer layer) const
{
    CB_GUIDISPLAYABLE_DEPIMPL();

    return children_[layer];
}

//////////////////////////////////////////////////////////////////////

bool GuiDisplayable::hasChild(const GuiDisplayable* pChild) const
{
    CB_GUIDISPLAYABLE_DEPIMPL();

    bool result = false;

    Children::iterator i = find(allChildren_.begin(), allChildren_.end(), pChild);

    if (i != allChildren_.end())
    {
        result = true;
    }

    return result;
}

bool GuiDisplayable::recursivelyHasChild(const GuiDisplayable* pChild) const
{
    CB_GUIDISPLAYABLE_DEPIMPL();

    bool found = false;

    if (this == pChild)
    {
        found = true;
    }

    for (Children::iterator i = allChildren_.begin(); ! found && i != allChildren_.end(); ++i)
    {
        found = recursivelyHasChild(*i);
    }

    return found;
}

//////////////////////////////////////////////////////////////////////

void GuiDisplayable::display()
{
    CB_GUIDISPLAYABLE_DEPIMPL();

    if (isVisible()) // No point continuing if this displayable is invisible
    {
        // If this has changed and needs rendering then draw it followed by all it's children
        if (hasChanged() || redrawEveryFrame())
        {
            if (secondDisplay() && useFastSecondDisplay() && ! redrawEveryFrame())
            {
                fastDisplay();
            }
            else
            {
                normalDisplay();
            }

            changed(false);
        }
        else
        {
            // Check children to see if they need displaying
            for (Layer layer = LAYER1; layer < NUMLAYERS; ++((int&)layer))
            {
                for (Children::iterator i = children_[layer].begin(); i != children_[layer].end(); ++i)
                {
                    // Call childs display method.
                    (*i)->display();
                }
            }
        }
    }
}

void GuiDisplayable::normalDisplay()
{
    CB_GUIDISPLAYABLE_DEPIMPL();

    doDisplay();

    // Display all children
    for (Layer layer = LAYER1; layer < NUMLAYERS; ++((int&)layer))
    {
        for (Children::iterator i = children_[layer].begin(); i != children_[layer].end(); ++i)
        {
            if ((*i)->isVisible())
            {
                // If the parent ( this ) has just changed ( i.e. this is the first display out of two )
                // then tell all children to display for the next 2 frames.
                if (firstDisplay() || redrawEveryFrame())
                {
                    (*i)->changed(true);
                }
                // Call childs display method.
                (*i)->display();
            }
        }
    }
}

void GuiDisplayable::fastDisplay()
{
    CB_GUIDISPLAYABLE_DEPIMPL();

    // Blit from front to back buffer.
    GuiBitmap frontBuffer = W4dManager::instance().sceneManager()->pDevice()->frontSurface();
    GuiPainter::instance().blit(frontBuffer, absoluteBoundary(), absoluteBoundary().minCorner());

    // Display children if necessary
    fastDisplayChildren();
}

void GuiDisplayable::fastDisplayChildren()
{
    CB_GUIDISPLAYABLE_DEPIMPL();

    for (Layer layer = LAYER1; layer < NUMLAYERS; ++((int&)layer))
    {
        for (Children::iterator i = children_[layer].begin(); i != children_[layer].end(); ++i)
        {
            GuiDisplayable* pChild = (*i);
            if (pChild->isVisible())
            {
                if (pChild->firstDisplay() || pChild->redrawEveryFrame()
                    || (pChild->secondDisplay() && ! pChild->useFastSecondDisplay()))
                {
                    // Call childs display method.
                    pChild->display();
                }
                else
                {
                    // Don't render child ( taken care of by fast display of parent! )
                    pChild->changed(false);
                    pChild->fastDisplayChildren();
                }
            }
        }
    }
}

bool GuiDisplayable::firstDisplay() const
{
    CB_DEPIMPL(int, changed_);

    return changed_ > 1;
}

bool GuiDisplayable::secondDisplay() const
{
    CB_DEPIMPL(int, changed_);

    return changed_ == 1;
}

//////////////////////////////////////////////////////////////////////

bool GuiDisplayable::empty() const
{
    CB_GUIDISPLAYABLE_DEPIMPL();

    return allChildren_.empty();
}

//////////////////////////////////////////////////////////////////////

void GuiDisplayable::filledRectangle(const Gui::Box& rel, const Gui::Colour& c)
{
    GuiPainter::instance().filledRectangle(translate(rel), c);
}

void GuiDisplayable::hollowRectangle(const Gui::Box& rel, const Gui::Colour& c, unsigned thickness)
{
    GuiPainter::instance().hollowRectangle(translate(rel), c, thickness);
}

void GuiDisplayable::fill(const Gui::Colour& c)
{
    GuiPainter::instance().filledRectangle(absoluteBoundary(), c);
}

//////////////////////////////////////////////////////////////////////

Gui::Coord GuiDisplayable::translate(Gui::XCoord x, Gui::YCoord y) const
{
    return Gui::Coord(absoluteCoord().x() + x, absoluteCoord().y() + y);
}

Gui::Coord GuiDisplayable::translate(const Gui::Coord& r) const
{
    return Gui::Coord(absoluteCoord().x() + r.x(), absoluteCoord().y() + r.y());
}

Gui::Box GuiDisplayable::translate(const Gui::Box& b) const
{
    return GuiDisplayable::translateBox(b, absoluteCoord());
}

//////////////////////////////////////////////////////////////////////

void GuiDisplayable::line(const Gui::Coord& rel1, const Gui::Coord& rel2, const Gui::Colour& c, unsigned thickness)
{
    //  PRE( absoluteBoundary().contains( translate( rel1 ) ) );
    //  PRE( absoluteBoundary().contains( translate( rel2 ) ) );

    GuiPainter::instance().line(translate(rel1), translate(rel2), c, thickness);
}

void GuiDisplayable::horizontalLine(const Gui::Coord& rel1, unsigned length, const Gui::Colour& c, unsigned thickness)
{
    //  PRE( absoluteBoundary().contains( translate( rel1 ) ) );
    //  PRE( absoluteBoundary().contains( translate( rel1.x() + length, rel1.y() ) ) );

    GuiPainter::instance().horizontalLine(translate(rel1), length, c, thickness);
}

void GuiDisplayable::verticalLine(const Gui::Coord& rel1, unsigned height, const Gui::Colour& c, unsigned thickness)
{
    //  PRE( absoluteBoundary().contains( translate( rel1 ) ) );
    //  PRE( absoluteBoundary().contains( translate( rel1.x(), rel1.y() + height ) ) );

    GuiPainter::instance().verticalLine(translate(rel1), height, c, thickness);
}

void GuiDisplayable::bevel(const Gui::Box& rel, unsigned thickness, const Gui::Colour& hiCol, const Gui::Colour& loCol)
{
    GuiPainter::instance().bevel(translate(rel), thickness, hiCol, loCol);
}

//////////////////////////////////////////////////////////////////////

GuiDisplayable* GuiDisplayable::parent()
{
    return pImpl_->pParent_;
}

const GuiDisplayable* GuiDisplayable::parent() const
{
    return pImpl_->pParent_;
}

bool GuiDisplayable::isEnabled() const
{
    return pImpl_->enabled_;
}

void GuiDisplayable::setEnabled(bool enabled)
{
    pImpl_->enabled_ = enabled;
}

//////////////////////////////////////////////////////////////////////

const Gui::Boundary& GuiDisplayable::absoluteBoundary() const
{
    CB_GUIDISPLAYABLE_DEPIMPL();

    return absoluteBox_;
}

void GuiDisplayable::setRelativeBoundary(const Gui::Boundary& boundary)
{
    pImpl_->relativeBox_ = boundary;
    if (pImpl_->pParent_)
    {
        pImpl_->absoluteBox_ = translateBox(boundary, pImpl_->pParent_->absoluteCoord());
    }
    else
    {
        pImpl_->absoluteBox_ = boundary;
    }
}

const Gui::Boundary& GuiDisplayable::relativeBoundary() const
{
    CB_GUIDISPLAYABLE_DEPIMPL();

    return relativeBox_;
}

Gui::Boundary GuiDisplayable::relativeBoundary(const GuiDisplayable& ancestor) const
{
    CB_GUIDISPLAYABLE_DEPIMPL();

    Gui::Coord coord = relativeCoord(ancestor);

    Gui::Box box(coord, width(), height());

    return box;
}

Gui::Coord GuiDisplayable::relativeCoord(const GuiDisplayable& ancestor) const
{
    PRE(! isRoot());

    Gui::Coord coord = relativeCoord();

    if (parent() != &ancestor)
    {
        Gui::Coord parentCoord = parent()->relativeCoord(ancestor);
        coord.x(coord.x() + parentCoord.x());
        coord.y(coord.y() + parentCoord.y());
    }

    return coord;
}

unsigned GuiDisplayable::width() const
{
    CB_GUIDISPLAYABLE_DEPIMPL();

    return relativeBox_.width();
}

unsigned GuiDisplayable::height() const
{
    CB_GUIDISPLAYABLE_DEPIMPL();

    return relativeBox_.height();
}

Gui::Size GuiDisplayable::size() const
{
    CB_GUIDISPLAYABLE_DEPIMPL();

    return relativeBox_.size();
}

//////////////////////////////////////////////////////////////////////

bool GuiDisplayable::contains(Gui::XCoord x, Gui::YCoord y) const
{
    return contains(Gui::Coord(x, y));
}

// virtual
bool GuiDisplayable::contains(const Gui::Coord& c) const
{
    if (c.x() >= absoluteBoundary().minCorner().x() + width() || c.y() >= absoluteBoundary().minCorner().y() + height())
        return false;

    return absoluteBoundary().contains(c);
}

//////////////////////////////////////////////////////////////////////

bool GuiDisplayable::doHandleKeyEvent(const GuiKeyEvent& gke)
{
    PRE(gke.buttonEvent().isKeyEvent());

    // Default implementation does not "use" the GuiKeyEvent therefore false is returned.
    return false;
}

bool GuiDisplayable::doHandleCharEvent(const GuiCharEvent& gce)
{
    PRE(gce.isCharEvent());

    // Default implementation does not "use" the GuiCharEvent therefore false is returned.
    return false;
}

void GuiDisplayable::doHandleMouseClickEvent(const GuiMouseEvent&)
{
    // Intentionally Empty
}

void GuiDisplayable::doHandleMouseEnterEvent(const GuiMouseEvent&)
{
    // Intentionally Empty
}

void GuiDisplayable::doHandleMouseExitEvent(const GuiMouseEvent&)
{
    // Intentionally Empty
}

void GuiDisplayable::doHandleMouseScrollEvent(GuiMouseEvent* event)
{
    // Intentionally Empty
}

void GuiDisplayable::doHandleContainsMouseEvent(const GuiMouseEvent&)
{
    // Intentionally Empty
}

//////////////////////////////////////////////////////////////////////

// static
Gui::Box GuiDisplayable::translateBox(const Gui::Box& b, const Gui::Coord& c)
{
    MexPoint2d minC = b.minCorner();
    minC += c;
    MexPoint2d maxC = b.maxCorner();
    maxC += c;
    return Gui::Box(minC, maxC);
}

GuiDisplayable* GuiDisplayable::innermostContaining(const Gui::Coord& c)
{
    CB_GUIDISPLAYABLE_DEPIMPL();

    // If we are visible and the point is contained in the boundary then we have found
    // a gui displayable that contains the mouse.
    if (!isVisible() || !absoluteBoundary().contains(c))
        return nullptr;

    for (int layer = NUMLAYERS; layer != LAYER1;)
    {
        --layer;

        // Check to see if any of the children contain the mouse pointer
        for (GuiDisplayable* pChild : children_[layer])
        {
            GuiDisplayable* result = pChild->innermostContaining(c);
            if (result)
            {
                return result;
            }
        }
    }

    // No children contain coord therefore we are most derived displayable containing coord.
    return this;
}

void GuiDisplayable::changed()
{
    changed(true);
}

void GuiDisplayable::setLayer(Layer layer)
{
    PRE(! isRoot());

    parent()->removeChild(this);
    parent()->addChild(this, layer);
}

bool GuiDisplayable::redrawEveryFrame() const
{
    CB_DEPIMPL(bool, redrawEveryFrame_);

    return redrawEveryFrame_;
}

void GuiDisplayable::redrawEveryFrame(bool redraw)
{
    CB_DEPIMPL(bool, redrawEveryFrame_);

    redrawEveryFrame_ = redraw;
}

bool GuiDisplayable::useFastSecondDisplay() const
{
    CB_DEPIMPL(bool, useFastSecondDisplay_);

    return useFastSecondDisplay_;
}

void GuiDisplayable::useFastSecondDisplay(bool fast)
{
    CB_DEPIMPL(bool, useFastSecondDisplay_);

    useFastSecondDisplay_ = fast;
}

#ifndef _PRODUCTION_RELEASE
// virtual
const char* GuiDisplayable::description() const
{
    return "GuiDisplayable";
}
#endif

GuiDisplayable* GuiDisplayable::innermostContainingCheckProcessesMouseEvents(const Gui::Coord& c)
{
    CB_GUIDISPLAYABLE_DEPIMPL();

    // If we are visible and the point is contained in the boundary then we have found
    // a gui displayable that contains the mouse.
    if (!isVisible() || !absoluteBoundary().contains(c) || !processesMouseEvents())
        return nullptr;

    for (int layer = NUMLAYERS; layer != LAYER1;)
    {
        --layer;

        // Check to see if any of the children contain the mouse pointer
        for (GuiDisplayable* pChild : children_[layer])
        {
            GuiDisplayable* result = pChild->innermostContainingCheckProcessesMouseEvents(c);
            if (result)
            {
                return result;
            }
        }
    }

    // No children contain coord therefore we are most derived displayable containing coord.
    return this;
}

// virtual
bool GuiDisplayable::processesMouseEvents() const
{
    return true;
}

// static
bool& GuiDisplayable::useFourTimesRender()
{
    static bool useFourTimesRenderBool = false;
    return useFourTimesRenderBool;
}

GuiDisplayable* GuiDisplayable::findRoot(GuiDisplayable* current)
{
    if (current->isRoot())
    {
        return current;
    }

    return findRoot(current->parent());
}

//////////////////////////////////////////////////////////////////////

/* End **************************************************************/
