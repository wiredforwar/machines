/*
 * P L A N E T E D . C P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

//  Definitions of non-inline non-template methods and global functions

#include "planeted/planeted.hpp"
#include "stdlib/string.hpp"
#include "device/keyboard.hpp"
#include "device/eventq.hpp"
#include "mathex/eulerang.hpp"
#include "mathex/quatern.hpp"
#include "world4d/domain.hpp"
#include "world4d/entity.hpp"
#include "world4d/scenemgr.hpp"
#include "world4d/camera.hpp"
#include "render/device.hpp"
#include "planeted/edittile.hpp"
#include "planeted/editdoma.hpp"
#include "planeted/editobst.hpp"
#include "planeted/editport.hpp"
#include "planeted/editconstr.hpp"
#include "planeted/editmach.hpp"
#include "planeted/editart.hpp"
#include "planeted/editcamr.hpp"
#include "planeted/mapbmp.hpp"
#include "machlog/planet.hpp"
#include "machphys/plansurf.hpp"

#include "profiler/profiler.hpp"

#include <fstream>

// static
PedPlanetEditor& PedPlanetEditor::instance()
{
    static PedPlanetEditor instance_;
    return instance_;
}

PedPlanetEditor::PedPlanetEditor()
    : dispKeyboardControls_(true)
    , dispWarnings_(true)
{
    PedDomainEditor* pDomainMode = new PedDomainEditor();
    PedPortalEditor* pPortalMode = new PedPortalEditor();
    pPortalMode->domainEditor(pDomainMode);

    pObstacleMode_ = new PedObstacleEditor();
    pDomainMode_ = pDomainMode;
    pPortalMode_ = pPortalMode;

    pConstructionMode_ = new PedConstructionEditor();
    pMachineMode_ = new PedMachineEditor();
    PedArtefactEditor* tmpArtefactMode_ = new PedArtefactEditor();
    pArtefactMode_ = tmpArtefactMode_;
    pTileMode_ = new PedTileEditor(*tmpArtefactMode_);

    pCameraMode_ = new PedCameraEditor();

    pCurrentMode_ = pTileMode_;
    pCurrentMode_->activateMode();

    TEST_INVARIANT;
}

PedPlanetEditor::~PedPlanetEditor()
{

    TEST_INVARIANT;
    delete pObstacleMode_;
    delete pDomainMode_;
    delete pPortalMode_;
    delete pConstructionMode_;
    delete pMachineMode_;
    delete pArtefactMode_;
    delete pTileMode_;
    delete pCameraMode_;
}

void PedPlanetEditor::CLASS_INVARIANT
{
    INVARIANT(this != nullptr);
}

std::ostream& operator<<(std::ostream& o, const PedPlanetEditor& t)
{

    o << "PedPlanetEditor " << static_cast<const void*>(&t) << " start" << std::endl;
    o << "PedPlanetEditor " << static_cast<const void*>(&t) << " end" << std::endl;

    return o;
}

void PedPlanetEditor::initialise(W4dSceneManager* pSceneManager, MachLogPlanet* pPlanet)
{
    PRE(pPlanet != nullptr);
    PRE(pSceneManager != nullptr);

    initDeviceEvents();

    pPlanet_ = pPlanet;
    pSceneManager_ = pSceneManager;

    pTileMode_->initialise(pSceneManager, pPlanet);
    pObstacleMode_->initialise(pSceneManager, pPlanet);
    pDomainMode_->initialise(pSceneManager, pPlanet);
    pPortalMode_->initialise(pSceneManager, pPlanet);
    pConstructionMode_->initialise(pSceneManager, pPlanet);
    pMachineMode_->initialise(pSceneManager, pPlanet);
    pArtefactMode_->initialise(pSceneManager, pPlanet);
    pCameraMode_->initialise(pSceneManager, pPlanet);
}

void PedPlanetEditor::preRenderUpdate()
{
    // Must wait until scene has been rendered once.
    static bool firstTime = true;
    if (not firstTime)
    {
        // Daniel: Quick fix, may remove condition on validation.
        // Validate all modes if the warnings are enabled.
        if (dispWarnings_)
        {
            pTileMode_->validate();
            pDomainMode_->validate();
            pObstacleMode_->validate();
            pPortalMode_->validate();
            pConstructionMode_->validate();
            pCameraMode_->validate();
            pMachineMode_->validate();
            pArtefactMode_->validate();
        }

        // This makes it clear when the profiling is on.
        if (ProProfiler::instance().isProfilingEnabled())
            pSceneManager_->out() << "NOTE : Profiling is enabled" << std::endl;

        pCurrentMode_->displayModeInfo();
        displaySavedStatus();

        if (dispWarnings_)
        {
            pTileMode_->displayWarningMsgs();
            pDomainMode_->displayWarningMsgs();
            pObstacleMode_->displayWarningMsgs();
            pPortalMode_->displayWarningMsgs();
            pCameraMode_->displayWarningMsgs();
            pSceneManager_->out() << "F9 : warnings off" << std::endl;
        }
        else
            pSceneManager_->out() << "F9 : warnings on" << std::endl;

        if (dispKeyboardControls_)
        {
            pCurrentMode_->displayKeyboardCtrls();
            displayKeyboardCtrls();
            pSceneManager_->out() << "F10 : controls display off" << std::endl;
        }
        else
            pSceneManager_->out() << "F10 : controls display on" << std::endl;

        pCurrentMode_->preRenderUpdate();
    }
    firstTime = false;
}

void PedPlanetEditor::displayKeyboardCtrls()
{
    PRE(pSceneManager_ != nullptr);

    TEST_INVARIANT;

    const char* tab = "\t";

    pSceneManager_->out() << "F1 : first person view" << std::endl;
    pSceneManager_->out() << "F2 : zenith view" << std::endl;
    pSceneManager_->out() << "F4 : free camera view" << std::endl;
    pSceneManager_->out() << "F5 : tile edit mode" << std::endl;
    pSceneManager_->out() << "F6 : domain edit mode" << tab << "( + Ctrl : hide domains )" << std::endl;
    pSceneManager_->out() << "F7 : obstacle edit mode" << tab << "( + Ctrl : hide obstacles )" << std::endl;
    pSceneManager_->out() << "F8 : portal edit mode" << tab << "( + Ctrl : hide portals )" << std::endl;
    pSceneManager_->out() << "F9 : warnings on/off" << std::endl;
    pSceneManager_->out() << "1 : construction edit" << tab << "( + Ctrl : hide )" << std::endl;
    pSceneManager_->out() << "2 : machine edit" << tab << "( + Ctrl : hide )" << std::endl;
    pSceneManager_->out() << "3 : artefact edit" << tab << "( + Ctrl : hide )" << std::endl;
    pSceneManager_->out() << "4 : camera edit" << tab << "( + ctrl : hide )" << std::endl;
    pSceneManager_->out() << "F11 : create planet map" << std::endl;
    pSceneManager_->out() << "Ctrl + S : save" << std::endl;
    pSceneManager_->out() << "HOME : Enable profiling" << std::endl;
    pSceneManager_->out() << "END : Disable profiling" << std::endl;

    TEST_INVARIANT;
}

void PedPlanetEditor::processSave()
{
    PRE(pPlanet_ != nullptr);

    saved_ = true;
    saveSuccessful_ = pPlanet_->surface()->save();

    if (saveSuccessful_)
    {
        std::ofstream cspFileStream(cspFileName_.pathname().c_str());
        std::ofstream scnFileStream(scnFileName_.pathname().c_str());
        bool cspSaveSuccessful = cspFileStream.is_open();
        bool scnSaveSuccessful = scnFileStream.is_open();

        if (saveSuccessful_)
        {
            pTileMode_->writeCspFile(cspFileStream);
            pDomainMode_->writeCspFile(cspFileStream);
            pPortalMode_->writeCspFile(cspFileStream);
            pObstacleMode_->writeCspFile(cspFileStream);

            pArtefactMode_->writeScnFile(scenarioFile_);
            pCameraMode_->writeScnFile(scenarioFile_);
            pConstructionMode_->writeScnFile(scenarioFile_);
            pMachineMode_->writeScnFile(scenarioFile_);
            scenarioFile_.write(scnFileStream);
            saveSuccessful_ = not(scnFileStream.fail() or scnFileStream.fail());
        }
    }
}

void PedPlanetEditor::displaySavedStatus()
{
    if (saved_)
    {
        if (saveSuccessful_)
            pSceneManager_->out() << "Last save was successful" << std::endl;
        else
            pSceneManager_->out() << "*** Last save failed ***" << std::endl;
    }
}

void PedPlanetEditor::readCspFile(const SysPathName& cspFileName)
{
    cspFileName_ = cspFileName;

    if (cspFileName_.existsAsFile())
    {
        pTileMode_->readCspFile(cspFileName_);
        pObstacleMode_->readCspFile(cspFileName_);
        pDomainMode_->readCspFile(cspFileName_);
        pPortalMode_->readCspFile(cspFileName_);
    }
}

void PedPlanetEditor::readScnFile(const SysPathName& scnFileName)
{
    scnFileName_ = scnFileName;

    if (scnFileName_.existsAsFile())
    {
        scenarioFile_.read(scnFileName);
        pConstructionMode_->readScnFile(scenarioFile_);
        pMachineMode_->readScnFile(scenarioFile_);
        pArtefactMode_->readScnFile(scenarioFile_);
        pCameraMode_->readScnFile(scenarioFile_);
    }
}

void PedPlanetEditor::readArfFile(const SysPathName& arfFileName)
{
    arfFileName_ = arfFileName;

    if (arfFileName_.existsAsFile())
    {
        scenarioFile_.arfFilename(arfFileName);
    }
}

void PedPlanetEditor::initDeviceEvents()
{
    // Setup event queue to respond to certain key pressed
    DevEventQueue::instance().queueEvents(Device::KeyCode::MOUSE_LEFT, DevButtonEvent::PRESS);
    DevEventQueue::instance().queueEvents(Device::KeyCode::MOUSE_LEFT, DevButtonEvent::RELEASE);
    DevEventQueue::instance().queueEvents(Device::KeyCode::ESCAPE, DevButtonEvent::PRESS);
    DevEventQueue::instance().queueEvents(Device::KeyCode::F1, DevButtonEvent::PRESS);
    DevEventQueue::instance().queueEvents(Device::KeyCode::F2, DevButtonEvent::PRESS);
    DevEventQueue::instance().queueEvents(Device::KeyCode::F4, DevButtonEvent::PRESS);
    DevEventQueue::instance().queueEvents(Device::KeyCode::F5, DevButtonEvent::PRESS);
    DevEventQueue::instance().queueEvents(Device::KeyCode::F6, DevButtonEvent::PRESS);
    DevEventQueue::instance().queueEvents(Device::KeyCode::F7, DevButtonEvent::PRESS);
    DevEventQueue::instance().queueEvents(Device::KeyCode::F8, DevButtonEvent::PRESS);
    DevEventQueue::instance().queueEvents(Device::KeyCode::F9, DevButtonEvent::PRESS);
    DevEventQueue::instance().queueEvents(Device::KeyCode::F10, DevButtonEvent::PRESS);
    DevEventQueue::instance().queueEvents(Device::KeyCode::F11, DevButtonEvent::PRESS);
    DevEventQueue::instance().queueEvents(Device::KeyCode::F12, DevButtonEvent::PRESS);
    DevEventQueue::instance().queueEvents(Device::KeyCode::KEY_B, DevButtonEvent::PRESS);
    DevEventQueue::instance().queueEvents(Device::KeyCode::KEY_C, DevButtonEvent::PRESS);
    DevEventQueue::instance().queueEvents(Device::KeyCode::KEY_D, DevButtonEvent::PRESS);
    DevEventQueue::instance().queueEvents(Device::KeyCode::KEY_E, DevButtonEvent::PRESS);
    DevEventQueue::instance().queueEvents(Device::KeyCode::KEY_F, DevButtonEvent::PRESS);
    DevEventQueue::instance().queueEvents(Device::KeyCode::KEY_G, DevButtonEvent::PRESS);
    DevEventQueue::instance().queueEvents(Device::KeyCode::KEY_H, DevButtonEvent::PRESS);
    DevEventQueue::instance().queueEvents(Device::KeyCode::KEY_I, DevButtonEvent::PRESS);
    DevEventQueue::instance().queueEvents(Device::KeyCode::KEY_J, DevButtonEvent::PRESS);
    DevEventQueue::instance().queueEvents(Device::KeyCode::KEY_K, DevButtonEvent::PRESS);
    DevEventQueue::instance().queueEvents(Device::KeyCode::KEY_L, DevButtonEvent::PRESS);
    DevEventQueue::instance().queueEvents(Device::KeyCode::KEY_N, DevButtonEvent::PRESS);
    DevEventQueue::instance().queueEvents(Device::KeyCode::KEY_O, DevButtonEvent::PRESS);
    DevEventQueue::instance().queueEvents(Device::KeyCode::KEY_P, DevButtonEvent::PRESS);
    DevEventQueue::instance().queueEvents(Device::KeyCode::KEY_Q, DevButtonEvent::PRESS);
    DevEventQueue::instance().queueEvents(Device::KeyCode::KEY_R, DevButtonEvent::PRESS);
    DevEventQueue::instance().queueEvents(Device::KeyCode::KEY_S, DevButtonEvent::PRESS);
    DevEventQueue::instance().queueEvents(Device::KeyCode::KEY_U, DevButtonEvent::PRESS);
    DevEventQueue::instance().queueEvents(Device::KeyCode::KEY_V, DevButtonEvent::PRESS);
    DevEventQueue::instance().queueEvents(Device::KeyCode::KEY_V, DevButtonEvent::RELEASE);
    DevEventQueue::instance().queueEvents(Device::KeyCode::KEY_W, DevButtonEvent::PRESS);
    DevEventQueue::instance().queueEvents(Device::KeyCode::KEY_X, DevButtonEvent::PRESS);
    DevEventQueue::instance().queueEvents(Device::KeyCode::KEY_Y, DevButtonEvent::PRESS);
    DevEventQueue::instance().queueEvents(Device::KeyCode::KEY_1, DevButtonEvent::PRESS);
    DevEventQueue::instance().queueEvents(Device::KeyCode::KEY_2, DevButtonEvent::PRESS);
    DevEventQueue::instance().queueEvents(Device::KeyCode::KEY_3, DevButtonEvent::PRESS);
    DevEventQueue::instance().queueEvents(Device::KeyCode::KEY_4, DevButtonEvent::PRESS);
    DevEventQueue::instance().queueEvents(Device::KeyCode::HOME, DevButtonEvent::PRESS);
    DevEventQueue::instance().queueEvents(Device::KeyCode::END, DevButtonEvent::PRESS);
}

void PedPlanetEditor::processInput(const DevButtonEvent& devButtonEvent)
{
    PRE(pSceneManager_ != nullptr);
    PRE(pPlanet_ != nullptr);

    TEST_INVARIANT;

    if (devButtonEvent.action() == DevButtonEvent::PRESS)
    {
        // Check for mode change first...
        if (devButtonEvent.scanCode() == Device::KeyCode::F5 and not devButtonEvent.wasCtrlPressed()
            and not devButtonEvent.wasShiftPressed() and pCurrentMode_ != pTileMode_)
        {
            pCurrentMode_->changingMode();
            pCurrentMode_ = pTileMode_;
            pCurrentMode_->activateMode();
        }
        else if (
            devButtonEvent.scanCode() == Device::KeyCode::F6 and not devButtonEvent.wasCtrlPressed()
            and not devButtonEvent.wasShiftPressed() and pCurrentMode_ != pDomainMode_)
        {
            pCurrentMode_->changingMode();
            pCurrentMode_ = pDomainMode_;
            pCurrentMode_->activateMode();
        }
        else if (
            devButtonEvent.scanCode() == Device::KeyCode::F7 and not devButtonEvent.wasCtrlPressed()
            and not devButtonEvent.wasShiftPressed() and pCurrentMode_ != pObstacleMode_)
        {
            pCurrentMode_->changingMode();
            pCurrentMode_ = pObstacleMode_;
            pCurrentMode_->activateMode();
        }
        else if (
            devButtonEvent.scanCode() == Device::KeyCode::F8 and not devButtonEvent.wasCtrlPressed()
            and not devButtonEvent.wasShiftPressed() and pCurrentMode_ != pPortalMode_)
        {
            pCurrentMode_->changingMode();
            pCurrentMode_ = pPortalMode_;
            pCurrentMode_->activateMode();
        }
        else if (
            devButtonEvent.scanCode() == Device::KeyCode::KEY_1 and not devButtonEvent.wasCtrlPressed()
            and not devButtonEvent.wasShiftPressed() and pCurrentMode_ != pConstructionMode_)
        {
            pCurrentMode_->changingMode();
            pCurrentMode_ = pConstructionMode_;
            pCurrentMode_->activateMode();
        }
        else if (
            devButtonEvent.scanCode() == Device::KeyCode::KEY_2 and not devButtonEvent.wasCtrlPressed()
            and not devButtonEvent.wasShiftPressed() and pCurrentMode_ != pMachineMode_)
        {
            pCurrentMode_->changingMode();
            pCurrentMode_ = pMachineMode_;
            pCurrentMode_->activateMode();
        }
        else if (
            devButtonEvent.scanCode() == Device::KeyCode::KEY_3 and not devButtonEvent.wasCtrlPressed()
            and not devButtonEvent.wasShiftPressed() and pCurrentMode_ != pArtefactMode_)
        {
            pCurrentMode_->changingMode();
            pCurrentMode_ = pArtefactMode_;
            pCurrentMode_->activateMode();
        }
        else if (
            devButtonEvent.scanCode() == Device::KeyCode::KEY_4 and not devButtonEvent.wasCtrlPressed()
            and not devButtonEvent.wasShiftPressed() and pCurrentMode_ != pCameraMode_)
        {
            pCurrentMode_->changingMode();
            pCurrentMode_ = pCameraMode_;
            pCurrentMode_->activateMode();
        }

        else if (devButtonEvent.scanCode() == Device::KeyCode::F10)
        {
            dispKeyboardControls_ = not dispKeyboardControls_;
        }
        else if (devButtonEvent.scanCode() == Device::KeyCode::KEY_S)
        {
            if (devButtonEvent.wasCtrlPressed())
                processSave();
        }
        else if (devButtonEvent.scanCode() == Device::KeyCode::F9)
        {
            //          pTileMode_->validate();
            //          pDomainMode_->validate();
            //          pObstacleMode_->validate();
            //          pPortalMode_->validate();
            dispWarnings_ = not dispWarnings_;
        }
        else if (devButtonEvent.scanCode() == Device::KeyCode::F11)
        {
            // Create map bitmap
            PedMapCreator mapCreator(
                *_REINTERPRET_CAST(PedPolygonEditor*, pDomainMode_),
                *_REINTERPRET_CAST(PedPolygonEditor*, pPortalMode_),
                *_REINTERPRET_CAST(PedPolygonEditor*, pObstacleMode_));
            mapCreator.createBmp();
        }
        else if (devButtonEvent.scanCode() == Device::KeyCode::HOME)
        {
            if (not ProProfiler::instance().isProfilingEnabled())
                ProProfiler::instance().enableProfiling();
        }
        else if (devButtonEvent.scanCode() == Device::KeyCode::END)
        {
            if (ProProfiler::instance().isProfilingEnabled())
                ProProfiler::instance().disableProfiling();
        }
    }

    pTileMode_->processInput(devButtonEvent);
    pDomainMode_->processInput(devButtonEvent);
    pObstacleMode_->processInput(devButtonEvent);
    pPortalMode_->processInput(devButtonEvent);
    pConstructionMode_->processInput(devButtonEvent);
    pMachineMode_->processInput(devButtonEvent);
    pArtefactMode_->processInput(devButtonEvent);
    pCameraMode_->processInput(devButtonEvent);

    TEST_INVARIANT;
}

/* End PLANETED.CPP *************************************************/
