#pragma once

#include <atomic>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>

#include <Bounce2.h>
#include "Joystick2Axis.h"
#include "Board.h"


// ---- InputController -------------------------------------------------------------------
// The InputController class processes events of the buttons and the analog joystick.
// A dedicated task on CPU core 0 is used to process the events independently of the 
// main loop() function.


class InputController
{
  public:
    struct State 
    {
      // true if the button was physically pressed (change from released to pressed)      
      bool a, b, aux, sel, start, joy;

      // true if the button is currently physically pressed
      bool aIsPressed, bIsPressed, auxIsPressed, selIsPressed, startIsPressed, joyIsPressed;             

      bool up, down, left, right;  // true == joystick moved in that direction
      int x, y;                    // joystick position
    };

  public:
    InputController();
    ~InputController();

    // Initialize and start event processing
    void begin();

    // Returns state of each button and the joystick
    State getState();

    // Direct access to the joystick object, e.g. for calibration
    Joystick2Axis& joy() { return myJoy; }

  private:
    // The buttons (Aux only on boards that have it)
    Bounce2::Button myBtnA, myBtnB, myBtnSel, myBtnStart; 
#if BOARD_HAS_AUX_BUTTON()
    Bounce2::Button myBtnAux;
#endif

    // The joystick with a single button
    Bounce2::Button myBtnJoy;
    Joystick2Axis myJoy;

    // State of each button as detected in the "processEvents" task 
    State myState;

    static void processEvents(void* pvParameters);
    std::atomic<bool> myIsTaskRunning;
    TaskHandle_t      myTaskHandle;
    SemaphoreHandle_t myMutex;
};