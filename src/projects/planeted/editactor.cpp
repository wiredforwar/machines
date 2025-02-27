/*
 * E D I T A C T O R . C P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

//  Definitions of non-inline non-template methods and global functions
#include "planeted/editactor.hpp"
#include "base/base.hpp"
#include "utility/linetok.hpp"
#include "device/keyboard.hpp"
#include "device/eventq.hpp"
#include "device/mouse.hpp"
#include "world4d/domain.hpp"
#include "world4d/entity.hpp"
#include "world4d/scenemgr.hpp"
#include "world4d/camera.hpp"
#include "world4d/composit.hpp"
#include "machlog/planet.hpp"
#include "machphys/plansurf.hpp"
#include "machphys/marker.hpp"
#include "planeted/wdebug.hpp"
#include "planeted/vetxmrkr.hpp"

PedActorEditor::PedActorEditor(const string& actorType)
    : actorType_(actorType)
{
    TEST_INVARIANT;
}

// virtual
PedActorEditor::~PedActorEditor()
{
    TEST_INVARIANT;
}

void PedActorEditor::CLASS_INVARIANT
{
    INVARIANT(this != nullptr);
}

// virtual
void PedActorEditor::initialise(W4dSceneManager* pSceneManager, MachLogPlanet* pPlanet)
{
    // Only really for debug...

    PRE(pSceneManager != nullptr);
    PRE(pPlanet != nullptr);

    PedEditorMode::initialise(pSceneManager, pPlanet);
    errors = new wdebug(pSceneManager);

    POST(errors != nullptr);
}

// virtual
void PedActorEditor::processInput(const DevButtonEvent& devButtonEvent)
{
    // Process common keyboard/mouse events

    if (devButtonEvent.action() == DevButtonEvent::PRESS and active_)
    {
        if (devButtonEvent.scanCode() == Device::KeyCode::KEY_D and pVertexMarker_)
        {
            processDrop();
        }
        else if (devButtonEvent.scanCode() == Device::KeyCode::KEY_I)
        {
            processCycle(PREV);
        }
        else if (devButtonEvent.scanCode() == Device::KeyCode::KEY_O)
        {
            processCycle(NEXT);
        }
        else if (devButtonEvent.scanCode() == Device::KeyCode::KEY_C)
        {
            processRace();
        }
        else if (devButtonEvent.scanCode() == Device::KeyCode::KEY_Y)
        {
            processMove(UP);
        }
        else if (devButtonEvent.scanCode() == Device::KeyCode::KEY_B)
        {
            processMove(DOWN);
        }
        else if (devButtonEvent.scanCode() == Device::KeyCode::KEY_H)
        {
            processMove(RIGHT);
        }
        else if (devButtonEvent.scanCode() == Device::KeyCode::KEY_G)
        {
            processMove(LEFT);
        }
        else if (devButtonEvent.scanCode() == Device::KeyCode::MOUSE_LEFT and pVertexMarker_)
        {
            processSelection();
            /*   if( not alreadySelected_ )
            {
                // Select on first mouse click
                processSelection();
            }
            else
            {
                // Signify perform drag with subsequent clicks
                mouseDrag_ = true;
            } */
        }
        else if (devButtonEvent.scanCode() == Device::KeyCode::KEY_X and devButtonEvent.wasShiftPressed())
        {
            processDelete();
        }
    }
    if (devButtonEvent.action() == DevButtonEvent::RELEASE and devButtonEvent.scanCode() == Device::KeyCode::MOUSE_LEFT)
    {
        processReleaseSelection();
    }
}

// virtual
void PedActorEditor::displayKeyboardCtrls()
{
    PRE(pSceneManager_ != nullptr);

    pSceneManager_->out() << "D : drop new " << actorType_ << " (at cursor pos)" << std::endl;
    pSceneManager_->out() << "Shift + X : delete " << actorType_ << std::endl;
    pSceneManager_->out() << "I : previous" << actorType_ << std::endl;
    pSceneManager_->out() << "O : next " << actorType_ << std::endl;
    pSceneManager_->out() << "Y : " << actorType_ << " up" << std::endl;
    pSceneManager_->out() << "B : " << actorType_ << " down" << std::endl;
    pSceneManager_->out() << "H : " << actorType_ << " right" << std::endl;
    pSceneManager_->out() << "G : " << actorType_ << " left" << std::endl;

    TEST_INVARIANT;
}

// virtual
void PedActorEditor::preRenderUpdate()
{
    highlightVertex();
    errors->display();
    if (mouseDrag_)
        processDrag();
}

// virtual
void PedActorEditor::displayModeInfo()
{
    PRE(pSceneManager_ != nullptr);

    TEST_INVARIANT;

    pSceneManager_->out() << "MODE : " << actorType_ << " Edit" << std::endl;

    displayVertexCoords();

    // any additional display stuff here...
}

// virtual
void PedActorEditor::changingMode()
{
    PedEditorMode::changingMode();
    delete pVertexMarker_;
    pVertexMarker_ = nullptr;
    delete pMarker_;
    pMarker_ = nullptr;

    changeAllSolidities(W4dEntity::NOT_SOLID);
}

// virtual
void PedActorEditor::activateMode()
{
    PedEditorMode::activateMode();
    changeAllSolidities(W4dEntity::SOLID);
}

void PedActorEditor::processDrop()
{
    W4dEntity* pEntity;
    changeAllSolidities(W4dEntity::NOT_SOLID);
    if (domainAtMousePoint(&pEntity))
    {
        createEntity(*pEntity, localTransformAtMouse());
    }
    changeAllSolidities(W4dEntity::SOLID);
}

// virtual
void PedActorEditor::processMove(PedActorEditor::MoveDir dir)
{
    W4dEntity* pSelectedActor = currentActor();
    if (pSelectedActor)
    {
        const W4dTransform3d& globalTransform = pSelectedActor->globalTransform();
        MexPoint3d position = globalTransform.position();

        MATHEX_SCALAR x = position.x();
        MATHEX_SCALAR y = position.y();
        MATHEX_SCALAR z = position.z();

        switch (dir)
        {
            case UP:
                y -= 0.1;
                break;
            case DOWN:
                y += 0.1;
                break;
            case RIGHT:
                x += 0.1;
                break;
            case LEFT:
                x -= 0.1;
                break;
        }
        MexPoint3d translationPoint(x, y, z);
        MexTransform3d newGlobalTransform(translationPoint);
        pSelectedActor->globalTransform(newGlobalTransform);

        W4dEntity* parent = domain(translationPoint);
        pSelectedActor->attachTo(parent);
        rotateAfterMove();
    }
}

void PedActorEditor::processReleaseSelection()
{
    W4dEntity* pSelectedActor = currentActor();

    if (pSelectedActor)
    {
        pSelectedActor->solid(W4dEntity::SOLID);
    }
    if (alreadySelected_ and mouseDrag_)
    {
        alreadySelected_ = mouseDrag_ = false;
    }
}

void PedActorEditor::processDrag()
{
    W4dEntity* pSelectedActor = currentActor();
    if (mouseDrag_ && pSelectedActor)
    {
        W4dEntity* parent;

        MexPoint3d translationPoint;

        if (mouseVertexPoint(&translationPoint))
        {
            domainAtMousePoint(&parent);
            MexTransform3d newGlobalTransform(translationPoint);
            pSelectedActor->globalTransform(newGlobalTransform);

            W4dEntity* parent = domain(translationPoint);
            pSelectedActor->attachTo(parent);
            rotateAfterMove();
        }
    }
}

void PedActorEditor::highlightSelected()
{
    // Create marker to highlight selected construction
    PRE(currentActor() != nullptr);

    W4dEntity* pEntity = currentActor();

    MexTransform3d identity;

    delete pMarker_;
    const MexAlignedBox3d& boundary
        = (pEntity->isComposite() ? pEntity->asComposite().compositeBoundingVolume() : pEntity->boundingVolume());

    pMarker_ = new MachPhysMarker(pEntity, identity, boundary);
    alreadySelected_ = true;
}

bool PedActorEditor::domainAtMousePoint(W4dEntity** ppEntity)
{
    MexPoint3d intersectPoint;

    if (mouseVertexPoint(&intersectPoint))
    {
        *ppEntity = domain(intersectPoint);
        return true;
    }
    return false;
}

W4dEntity* PedActorEditor::domain(const MexPoint3d& point)
{
    MachPhysPlanetSurface* pSurface = pPlanet_->surface();
    return pSurface->domainAt(point);
}

const MexTransform3d PedActorEditor::localTransformAtMouse()
{
    // change to call localTransformAtPoint
    MexPoint3d mouseCoords;
    MexTransform3d resultTransform;

    if (mouseVertexPoint(&mouseCoords))
    {
        MachPhysPlanetSurface* pSurface = pPlanet_->surface();
        MATHEX_SCALAR height = pSurface->terrainHeight(mouseCoords.x(), mouseCoords.y());
        MexPoint3d newMouseCoords(mouseCoords.x(), mouseCoords.y(), height);
        // replace all of below with
        // resultTransform = localTransformAtPoint( newMouseCoords );
        const MexTransform3d globalTransform(newMouseCoords);
        W4dEntity* pEntity = nullptr;
        domainAtMousePoint(&pEntity);
        const MexTransform3d domainTransform(pEntity->globalTransform());
        domainTransform.transformInverse(globalTransform, &resultTransform);
    }
    return resultTransform;
}

const MexTransform3d PedActorEditor::localTransformAtPoint(W4dEntity** ppDomain, const MexPoint3d& point)
{
    MexTransform3d resultTransform;

    *ppDomain = domain(point);
    const MexTransform3d domainTransform((*ppDomain)->globalTransform());
    const MexTransform3d globalTransform(point);
    domainTransform.transformInverse(globalTransform, &resultTransform);

    return resultTransform;
}

bool PedActorEditor::actorSelected(W4dEntity** ppActor)
{
    PRE(pSceneManager_ != nullptr);

    MexLine3d cursorLine = cameraThroughCursorLine(
        MexPoint2d(DevMouse::instance().position().first, DevMouse::instance().position().second));
    W4dDomain* pCameraDomain = pSceneManager_->currentCamera()->containingDomain();

    // Find any entity intersecting the line
    W4dEntity* pEntity;
    MATHEX_SCALAR distance;
    bool result = pCameraDomain->findNearerEntity(
        cursorLine,
        cursorLine.length(),
        W4dEntity::nextCheckId(),
        W4dEntity::MEDIUM,
        &pEntity,
        &distance);
    if (result)
    {
        *ppActor = pEntity;
    }
    return result;
}

bool PedActorEditor::mouseVertexPoint(MexPoint3d* intersectPoint)
{
    PRE(pSceneManager_ != nullptr);

    // Highlight vertex mouse cursor is over
    MexLine3d cursorLine = cameraThroughCursorLine(
        MexPoint2d(DevMouse::instance().position().first, DevMouse::instance().position().second));
    W4dDomain* pCameraDomain = pSceneManager_->currentCamera()->containingDomain();

    // Find any entity intersecting the line
    W4dEntity* pEntity;
    MATHEX_SCALAR distance;
    bool result = pCameraDomain->findNearerEntity(
        cursorLine,
        cursorLine.length(),
        W4dEntity::nextCheckId(),
        W4dEntity::MEDIUM,
        &pEntity,
        &distance);

    if (result)
        *intersectPoint = cursorLine.pointAtDistance(distance);

    return result;
}

void PedActorEditor::highlightVertex()
{
    PRE(pSceneManager_ != nullptr);

    // Clear last highlighted vertex
    delete pVertexMarker_;
    pVertexMarker_ = nullptr;

    MexPoint3d intersectPoint;
    if (mouseVertexPoint(&intersectPoint))
    {
        size_t vx = intersectPoint.x();
        size_t vy = intersectPoint.y();

        // vx -= vx % vertexSpacingX();
        // vy -= vy % vertexSpacingY();

        pVertexMarker_ = new PedVertexMarker(vx, vy, true /*, globalPos.z()*/);
        // pVertexMarker_ = new PedVertexMarker( intersectPoint, true );
    }
}

void PedActorEditor::displayVertexCoords()
{
    PRE(pSceneManager_ != nullptr);

    TEST_INVARIANT;

    if (pVertexMarker_)
        pSceneManager_->out() << "Cursor Pos (" << pVertexMarker_->position().x() << ","
                              << pVertexMarker_->position().y() << ")" << std::endl;

    TEST_INVARIANT;
}

size_t PedActorEditor::vertexSpacingX() const
{
    MachPhysPlanetSurface* pSurface = pPlanet_->surface();
    return /*pSurface->xTileSpacing() /*/ pSurface->nTileXVertices();
}

size_t PedActorEditor::vertexSpacingY() const
{
    MachPhysPlanetSurface* pSurface = pPlanet_->surface();
    return /*pSurface->xTileSpacing() /*/ pSurface->nTileYVertices();
}

void PedActorEditor::parseRace(const string& race)
{
    PRE(pParser_ != nullptr);

    if (race == "RED")
        race_ = MachPhys::RED;
    else if (race == "BLUE")
        race_ = MachPhys::BLUE;
    else if (race == "GREEN")
        race_ = MachPhys::GREEN;
    else if (race == "YELLOW")
        race_ = MachPhys::YELLOW;
    else
        race_ = MachPhys::N_RACES;
}

void PedActorEditor::processRace()
{
    changeRace(&race_);
}

void PedActorEditor::changeRace(MachPhys::Race* race)
{
    if (*race == MachPhys::RED)
        *race = MachPhys::BLUE;
    else if (*race == MachPhys::BLUE)
        *race = MachPhys::GREEN;
    else if (*race == MachPhys::GREEN)
        *race = MachPhys::YELLOW;
    else if (*race == MachPhys::YELLOW)
        *race = MachPhys::RED;
}

void PedActorEditor::processHide(bool hidden)
{
    hidden_ = not hidden;
}

std::ostream& operator<<(std::ostream& o, const PedActorEditor& t)
{

    o << "PedActorEditor " << static_cast<const void*>(&t) << " start" << std::endl;
    o << "PedActorEditor " << static_cast<const void*>(&t) << " end" << std::endl;

    return o;
}

/* End EDITACTOR.CPP ************************************************/
