/*
 * S H R K E Y B D . C P P
 * (c) Charybdis Limited, 1996. All Rights Reserved
 */

#include "base/base.hpp"

#ifdef _DOSAPP
#include <i86.h>
#endif
#include "device/keyboard.hpp"

#include "recorder/recorder.hpp"
#include "recorder/private/recpriv.hpp"

#include <sstream>

DevKeyboard::DevKeyboard()
//: pressedCount_(0)
// Don't initialise pressedCount_ here, because it could cause a
// race condition: pressedCount_ and keyMap_ must be initialised
// together, both within the same critical section.
{
    // Rely on this to initilaise pressedCount_.
    allKeysReleased();
}

DevKeyboard::~DevKeyboard()
{
}

bool DevKeyboard::keyCode(ScanCode sCode) const
{
    PRE(Device::isValidCode(sCode));
    TEST_INVARIANT;

    bool result;
    if (RecRecorder::instance().state() == RecRecorder::PLAYING)
        result = RecRecorderPrivate::instance().playbackKeyCode();
    else
    {
        result = keyCodeNoRecord(sCode);

        if (RecRecorder::instance().state() == RecRecorder::RECORDING)
            RecRecorderPrivate::instance().recordKeyCode(result);
    }

    return result;
}

DevKeyboard::KeyState DevKeyboard::deltaKeyCode(ScanCode sCode) const
{
    PRE(static_cast<int>(sCode) < N_KEYS);
    TEST_INVARIANT;

    DevKeyboard::KeyState result;
    if (RecRecorder::instance().state() == RecRecorder::PLAYING)
        result = RecRecorderPrivate::instance().playbackDeltaKeyCode();
    else
    {
        result = deltaKeyCodeNoRecord(sCode);

        if (RecRecorder::instance().state() == RecRecorder::RECORDING)
            RecRecorderPrivate::instance().recordDeltaKeyCode(result);
    }

    return result;
}

bool DevKeyboard::key(unsigned char character) const
{
    TEST_INVARIANT;

    bool result;
    if (RecRecorder::instance().state() == RecRecorder::PLAYING)
        result = RecRecorderPrivate::instance().playbackKeyCode();
    else
    {
        result = keyNoRecord(character);

        if (RecRecorder::instance().state() == RecRecorder::RECORDING)
            RecRecorderPrivate::instance().recordKeyCode(result);
    }

    return result;
}

DevKeyboard::KeyState DevKeyboard::deltaKey(unsigned char character) const
{
    TEST_INVARIANT;
    return deltaKeyCode(getCharacterIndex(character));
}

DevKeyboard::ScanCode DevKeyboard::getCharacterIndex(unsigned char c) const
{
    // for a given character (it's ASCII value), this will return the
    // corresponding scan code
    static unsigned char characterMap[] = { /*   0 */ 0,  0,  0,  0,  0,  0,  0,  15,
                                            /*   8 */ 14, 0,  0,  0,  0,  28, 0,  0,
                                            /*  16 */ 0,  0,  0,  0,  0,  0,  0,  0,
                                            /*  24 */ 0,  0,  0,  1,  0,  0,  0,  0,
                                            /*  32 */ 57, 0,  40, 0,  0,  0,  0,  40,
                                            /*  40 */ 0,  0,  53, 13, 51, 12, 52, 53,
                                            /*  48 */ 11, 2,  3,  4,  5,  6,  7,  8,
                                            /*  56 */ 9,  10, 39, 39, 51, 13, 52, 53,
                                            /*  64 */ 0,  30, 48, 46, 32, 18, 33, 34,
                                            /*  72 */ 35, 23, 36, 37, 38, 50, 49, 24,
                                            /*  80 */ 25, 16, 19, 31, 20, 22, 47, 17,
                                            /*  88 */ 45, 21, 44, 26, 0,  27, 0,  12,
                                            /*  96 */ 0,  30, 48, 46, 32, 18, 33, 34,
                                            /* 104 */ 35, 23, 36, 37, 38, 50, 49, 24,
                                            /* 112 */ 25, 16, 19, 31, 20, 22, 47, 17,
                                            /* 120 */ 45, 21, 44, 26, 0,  27, 41, 0,
                                            /* 128 */ 0,  0,  0,  0,  0,  0,  0,  0,
                                            /* 136 */ 0,  0,  0,  0,  0,  0,  0,  0,
                                            /* 144 */ 0,  0,  0,  0,  0,  0,  0,  0,
                                            /* 152 */ 0,  0,  0,  0,  0,  0,  0,  0,
                                            /* 160 */ 0,  0,  0,  0,  0,  0,  0,  0,
                                            /* 168 */ 0,  0,  0,  0,  0,  0,  0,  0,
                                            /* 176 */ 0,  0,  0,  0,  0,  0,  0,  0,
                                            /* 184 */ 0,  0,  0,  0,  0,  0,  0,  0,
                                            /* 192 */ 0,  0,  0,  0,  0,  0,  0,  0,
                                            /* 200 */ 0,  0,  0,  0,  0,  0,  0,  0,
                                            /* 208 */ 0,  0,  0,  0,  0,  0,  0,  0,
                                            /* 216 */ 0,  0,  0,  0,  0,  0,  0,  0,
                                            /* 224 */ 0,  0,  0,  0,  0,  0,  0,  0,
                                            /* 232 */ 0,  0,  0,  0,  0,  0,  0,  0,
                                            /* 240 */ 0,  0,  0,  0,  0,  0,  0,  0,
                                            /* 248 */ 0,  0,  0,  0,  0,  0,  0,  0 };
    static_assert(std::size(characterMap) == 256);

    return static_cast<ScanCode>(characterMap[c]);
}

bool DevKeyboard::anyKey() const
{
    TEST_INVARIANT;

    bool result;
    if (RecRecorder::instance().state() == RecRecorder::PLAYING)
        result = RecRecorderPrivate::instance().playbackAnyKey();
    else
    {
        result = anyKeyNoRecord();

        if (RecRecorder::instance().state() == RecRecorder::RECORDING)
            RecRecorderPrivate::instance().recordAnyKey(result);
    }

    return result;
}

bool DevKeyboard::shiftPressed() const
{
    TEST_INVARIANT;

    bool result;
    if (RecRecorder::instance().state() == RecRecorder::PLAYING)
        result = RecRecorderPrivate::instance().playbackShiftPressed();
    else
    {
        result = shiftPressedNoRecord();

        if (RecRecorder::instance().state() == RecRecorder::RECORDING)
            RecRecorderPrivate::instance().recordShiftPressed(result);
    }

    return result;
}

bool DevKeyboard::ctrlPressed() const
{
    TEST_INVARIANT;

    bool result;
    if (RecRecorder::instance().state() == RecRecorder::PLAYING)
        result = RecRecorderPrivate::instance().playbackCtrlPressed();
    else
    {
        result = ctrlPressedNoRecord();

        if (RecRecorder::instance().state() == RecRecorder::RECORDING)
            RecRecorderPrivate::instance().recordCtrlPressed(result);
    }

    return result;
}

bool DevKeyboard::altPressed() const
{
    TEST_INVARIANT;

    bool result;
    if (RecRecorder::instance().state() == RecRecorder::PLAYING)
        result = RecRecorderPrivate::instance().playbackAltPressed();
    else
    {
        result = altPressedNoRecord();

        if (RecRecorder::instance().state() == RecRecorder::RECORDING)
            RecRecorderPrivate::instance().recordAltPressed(result);
    }

    return result;
}

void DevKeyboard::pressed(ScanCode code)
{
    // Although this is an OS independent piece of code, we must be
    // aware that it can be called from within a DOS interrupt
    // handler.  It appears that we are quite limited in the system
    // calls that can be made here.  Hence, no TEST_INVARIANT.
    PRE(static_cast<int>(code) < N_KEYS);

    if (!keyMap(code))
    {
        // An interrupt between the next two statements could cause this
        // object's state to become inconsistent.  We rely on the caller
        // having disabled interrupts under DOS!!
        keyMap(code) = true;
        ++pressedCount_;
    }
}

void DevKeyboard::released(ScanCode scanCode)
{
    const auto code = static_cast<std::size_t>(scanCode);
    // Don't test invariant, see DevKeyboard::pressed.
    PRE(code < N_KEYS);

    if (keyMap_[code])
    {
        // Interrupts must be disabled, see DevKeyboard::pressed.
        keyMap_[code] = false;
        --pressedCount_;
    }
}

void DevKeyboard::allKeysReleased()
{
    TEST_INVARIANT;

// Disable interrupts because pressedCount_ and keyMap_ can change
// asyncronously.
#ifdef _DOSAPP
    _disable();
#endif

    for (int i = 0; i < N_KEYS; ++i)
    {
        keyMap_[i] = false;
        lastKeyMap_[i] = false;
    }

    // An interrupt occuring here, between these statements, could set an
    // element of keyMap_ and increment pressedCount_, hence the disable.
    pressedCount_ = 0;

#ifdef _DOSAPP
    _enable();
#endif

    TEST_INVARIANT;
}

bool DevKeyboard::keyNoRecord(unsigned char character) const
{
    return keyMap(getCharacterIndex(character));
}

bool DevKeyboard::keyCodeNoRecord(ScanCode code) const
{
    return keyMap(code);
}

bool DevKeyboard::anyKeyNoRecord() const
{
    return pressedCount_ > 0;
}

bool DevKeyboard::shiftPressedNoRecord() const
{
    return keyMap(Device::KeyCode::RIGHT_SHIFT) || keyMap(Device::KeyCode::LEFT_SHIFT);
}

bool DevKeyboard::ctrlPressedNoRecord() const
{
    return keyMap(Device::KeyCode::RIGHT_CONTROL) || keyMap(Device::KeyCode::LEFT_CONTROL);
}

bool DevKeyboard::altPressedNoRecord() const
{
    return keyMap(Device::KeyCode::RIGHT_ALT) || keyMap(Device::KeyCode::LEFT_ALT);
}

DevKeyboard::KeyState DevKeyboard::deltaKeyCodeNoRecord(ScanCode code) const
{
    KeyState result;

    // An interrupt in the middle of the tests below could produce an incorrect
    // result.  keyMap_[sCode] was previously evaluated *within* each if
    // statement.  However, an interrupt could occur between the two if
    // tests, so that the second one could see a different value for the same
    // expression.  By sampling to a local variable, each if statement sees the
    // same value.
    const bool current = keyMap(code);
    const bool last = lastKeyMap(code);

    // keyMap_[sCode] may have changed since the last evaluation, so we must
    // use current.
    DevKeyboard* crufty = const_cast<DevKeyboard*>(this);
    crufty->lastKeyMap(code) = current;

    if (current == last)
        result = NO_CHANGE;
    else if (current && ! last)
        result = PRESSED;
    else
    {
        // This assertion could actually fail if in the presence of
        // race conditions (if keyMap_ were used rather than current).
        ASSERT(! current && last, logic_error());
        result = RELEASED;
    }

    return result;
}

#ifdef _TEST_INVARIANTS
#include <sstream>
#endif

// We appear to get a crash if this is called from within a DOS interrupt
// handler.  Probably due to complicated non-reentrant things (like malloc).
#ifdef _TEST_INVARIANTS
void DevKeyboard::keys_invariant(const char* file, const char* line) const
#else
void DevKeyboard::keys_invariant(const char*, const char*) const
#endif
{
// Disable interrupts because pressedCount_ and keyMap_ can change
// asyncronously.
#ifdef _DOSAPP
    _disable();
#endif

    // pressedCount_ should always reflect the number of keys depressed
    // in keyMap_.
    int count = 0;
    for (int i = 0; i != N_KEYS; ++i)
        if (keyMap_[i])
            ++count;

    // Re-enable interrupts before testing any invariant which may fail.
    const int pressed_count = pressedCount_;
#ifdef _DOSAPP
    _enable();
#endif

    if (pressed_count != count)
    {
        std::ostringstream ostr;
        ostr << "KeyMap is:\n";
        for (int j = 0; j != N_KEYS; ++j)
        {
            ostr << ((keyMap_[j]) ? "1 " : "0 ");

            if (!(j % 32))
                ostr << "\n";
        }

        ostr << std::ends;
        INVARIANT_INFO(ostr.str());
        // ostr.rdbuf()->freeze(0);
    }

    INVARIANT_INFO(pressed_count);
    INVARIANT_INFO(count);
    INVARIANT(pressed_count == count);
}

// static
void Device::printScanCode(std::ostream& o, KeyCode code)
{
    switch (code)
    {
        case Device::KeyCode::ENTER_PAD:
            o << "ENTER_PAD";
            break;
        case Device::KeyCode::ASTERISK_PAD:
            o << "ASTERISK_PAD";
            break;
        case Device::KeyCode::FORWARD_SLASH_PAD:
            o << "FORWARD_SLASH_PAD";
            break;
        case Device::KeyCode::PLUS_PAD:
            o << "PLUS_PAD";
            break;
        case Device::KeyCode::MINUS_PAD:
            o << "MINUS_PAD";
            break;
        case Device::KeyCode::PAD_0:
            o << "PAD_0";
            break;
        case Device::KeyCode::PAD_1:
            o << "PAD_1";
            break;
        case Device::KeyCode::PAD_2:
            o << "PAD_2";
            break;
        case Device::KeyCode::PAD_3:
            o << "PAD_3";
            break;
        case Device::KeyCode::PAD_4:
            o << "PAD_4";
            break;
        case Device::KeyCode::PAD_5:
            o << "PAD_5";
            break;
        case Device::KeyCode::PAD_6:
            o << "PAD_6";
            break;
        case Device::KeyCode::PAD_7:
            o << "PAD_7";
            break;
        case Device::KeyCode::PAD_8:
            o << "PAD_8";
            break;
        case Device::KeyCode::PAD_9:
            o << "PAD_9";
            break;
        case Device::KeyCode::SCROLL_LOCK:
            o << "SCROLL_LOCK";
            break;
        case Device::KeyCode::RIGHT_CONTROL:
            o << "RIGHT_CONTROL";
            break;
        case Device::KeyCode::LEFT_CONTROL:
            o << "LEFT_CONTROL";
            break;
        case Device::KeyCode::UP_ARROW:
            o << "UP_ARROW";
            break;
        case Device::KeyCode::DOWN_ARROW:
            o << "DOWN_ARROW";
            break;
        case Device::KeyCode::LEFT_ARROW:
            o << "LEFT_ARROW";
            break;
        case Device::KeyCode::RIGHT_ARROW:
            o << "RIGHT_ARROW";
            break;
        case Device::KeyCode::INSERT:
            o << "INSERT";
            break;
        case Device::KeyCode::KEY_DELETE:
            o << "KEY_DELETE";
            break;
        case Device::KeyCode::HOME:
            o << "HOME";
            break;
        case Device::KeyCode::END:
            o << "END";
            break;
        case Device::KeyCode::BREAK:
            o << "BREAK";
            break;
        case Device::KeyCode::PAGE_UP:
            o << "PAGE_UP";
            break;
        case Device::KeyCode::PAGE_DOWN:
            o << "PAGE_DOWN";
            break;
        case Device::KeyCode::ESCAPE:
            o << "ESCAPE";
            break;
        case Device::KeyCode::ENTER:
            o << "ENTER";
            break;
        case Device::KeyCode::SPACE:
            o << "SPACE";
            break;
        case Device::KeyCode::BACK_SPACE:
            o << "BACK_SPACE";
            break;
        case Device::KeyCode::TAB:
            o << "TAB";
            break;
        case Device::KeyCode::LEFT_SHIFT:
            o << "LEFT_SHIFT";
            break;
        case Device::KeyCode::RIGHT_SHIFT:
            o << "RIGHT_SHIFT";
            break;
        case Device::KeyCode::CAPS_LOCK:
            o << "CAPS_LOCK";
            break;
        case Device::KeyCode::F1:
            o << "F1";
            break;
        case Device::KeyCode::F2:
            o << "F2";
            break;
        case Device::KeyCode::F3:
            o << "F3";
            break;
        case Device::KeyCode::F4:
            o << "F4";
            break;
        case Device::KeyCode::F5:
            o << "F5";
            break;
        case Device::KeyCode::F6:
            o << "F6";
            break;
        case Device::KeyCode::F7:
            o << "F7";
            break;
        case Device::KeyCode::F8:
            o << "F8";
            break;
        case Device::KeyCode::F9:
            o << "F9";
            break;
        case Device::KeyCode::F10:
            o << "F10";
            break;
        case Device::KeyCode::F11:
            o << "F11";
            break;
        case Device::KeyCode::F12:
            o << "F12";
            break;
        case Device::KeyCode::KEY_A:
            o << "KEY_A";
            break;
        case Device::KeyCode::KEY_B:
            o << "KEY_B";
            break;
        case Device::KeyCode::KEY_C:
            o << "KEY_C";
            break;
        case Device::KeyCode::KEY_D:
            o << "KEY_D";
            break;
        case Device::KeyCode::KEY_E:
            o << "KEY_E";
            break;
        case Device::KeyCode::KEY_F:
            o << "KEY_F";
            break;
        case Device::KeyCode::KEY_G:
            o << "KEY_G";
            break;
        case Device::KeyCode::KEY_H:
            o << "KEY_H";
            break;
        case Device::KeyCode::KEY_I:
            o << "KEY_I";
            break;
        case Device::KeyCode::KEY_J:
            o << "KEY_J";
            break;
        case Device::KeyCode::KEY_K:
            o << "KEY_K";
            break;
        case Device::KeyCode::KEY_L:
            o << "KEY_L";
            break;
        case Device::KeyCode::KEY_M:
            o << "KEY_M";
            break;
        case Device::KeyCode::KEY_N:
            o << "KEY_N";
            break;
        case Device::KeyCode::KEY_O:
            o << "KEY_O";
            break;
        case Device::KeyCode::KEY_P:
            o << "KEY_P";
            break;
        case Device::KeyCode::KEY_Q:
            o << "KEY_Q";
            break;
        case Device::KeyCode::KEY_R:
            o << "KEY_R";
            break;
        case Device::KeyCode::KEY_S:
            o << "KEY_S";
            break;
        case Device::KeyCode::KEY_T:
            o << "KEY_T";
            break;
        case Device::KeyCode::KEY_U:
            o << "KEY_U";
            break;
        case Device::KeyCode::KEY_V:
            o << "KEY_V";
            break;
        case Device::KeyCode::KEY_W:
            o << "KEY_W";
            break;
        case Device::KeyCode::KEY_X:
            o << "KEY_X";
            break;
        case Device::KeyCode::KEY_Y:
            o << "KEY_Y";
            break;
        case Device::KeyCode::KEY_Z:
            o << "KEY_Z";
            break;
        case Device::KeyCode::KEY_0:
            o << "KEY_0";
            break;
        case Device::KeyCode::KEY_1:
            o << "KEY_1";
            break;
        case Device::KeyCode::KEY_2:
            o << "KEY_2";
            break;
        case Device::KeyCode::KEY_3:
            o << "KEY_3";
            break;
        case Device::KeyCode::KEY_4:
            o << "KEY_4";
            break;
        case Device::KeyCode::KEY_5:
            o << "KEY_5";
            break;
        case Device::KeyCode::KEY_6:
            o << "KEY_6";
            break;
        case Device::KeyCode::KEY_7:
            o << "KEY_7";
            break;
        case Device::KeyCode::KEY_8:
            o << "KEY_8";
            break;
        case Device::KeyCode::KEY_9:
            o << "KEY_9";
            break;
        case Device::KeyCode::MOUSE_LEFT:
            o << "MOUSE_LEFT";
            break;
        case Device::KeyCode::MOUSE_RIGHT:
            o << "MOUSE_RIGHT";
            break;
        case Device::KeyCode::MOUSE_MIDDLE:
            o << "MOUSE_MIDDLE";
            break;

        case Device::KeyCode::MOUSE_EXTRA1:
            o << "MOUSE_EXTRA1";
            break;
        case Device::KeyCode::MOUSE_EXTRA2:
            o << "MOUSE_EXTRA2";
            break;
        case Device::KeyCode::MOUSE_EXTRA3:
            o << "MOUSE_EXTRA3";
            break;
        case Device::KeyCode::MOUSE_EXTRA4:
            o << "MOUSE_EXTRA4";
            break;
        case Device::KeyCode::MOUSE_EXTRA5:
            o << "MOUSE_EXTRA5";
            break;
        case Device::KeyCode::MOUSE_EXTRA6:
            o << "MOUSE_EXTRA6";
            break;
        case Device::KeyCode::MOUSE_EXTRA7:
            o << "MOUSE_EXTRA7";
            break;
        case Device::KeyCode::MOUSE_EXTRA8:
            o << "MOUSE_EXTRA8";
            break;

        // NB: don't use ASSERT_BAD_CASE because it is non-extensible.  New scan
        // codes can be added and will work even if this fn doesn't print them.
        default:
            o << "Unknown scan code, numeric value=" << static_cast<int>(code);
            break;
    }
}
