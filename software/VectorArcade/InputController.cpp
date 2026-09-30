#include <Streaming.h>
#include "InputController.h"

// ---- InputController -------------------------------------------------------------------

// ----------------------------------------------------------------------------------------
InputController::InputController()
: myJoy(cPinJoyX, cPinJoyY, true), myIsTaskRunning(false)
{
  myMutex = xSemaphoreCreateMutex();
  xTaskCreatePinnedToCore(processEvents,     // Function to implement the task
                          "processEvents",   // Name of the task
                          2048,              // Stack size in words
                          this,              // Task input parameter
                          tskIDLE_PRIORITY,  // Priority of the task
                          &myTaskHandle,     // Task handle
                          0);                // Core where the task should run
  vTaskSuspend(myTaskHandle);
}

// ----------------------------------------------------------------------------------------
InputController::~InputController()
{
  myIsTaskRunning = false;
  vTaskDelete(myTaskHandle);
  vSemaphoreDelete(myMutex);  
}

// ----------------------------------------------------------------------------------------
void InputController::begin()
{
  // Buttons: attach to a pin and set that pin's mode
  myBtnA.attach(cPinBtnA, INPUT_PULLUP);
  myBtnA.setPressedState(LOW);
  myBtnA.interval(5);

  myBtnB.attach(cPinBtnB, INPUT_PULLUP);
  myBtnB.setPressedState(LOW);
  myBtnB.interval(5);

#if BOARD_HAS_AUX_BUTTON()
  myBtnAux.attach(cPinBtnAux, INPUT_PULLUP);
  myBtnAux.setPressedState(LOW);
  myBtnAux.interval(5);
#endif

  myBtnSel.attach(cPinBtnSel, INPUT_PULLUP);
  myBtnSel.setPressedState(LOW);
  myBtnSel.interval(5);

  myBtnStart.attach(cPinBtnStart, INPUT_PULLUP);
  myBtnStart.setPressedState(LOW);
  myBtnStart.interval(5);

  myBtnJoy.attach(cPinJoyBtn, INPUT_PULLUP);  
  myBtnJoy.setPressedState(LOW);
  myBtnJoy.interval(5);

  // Joystick: Initialize and start processing
  myJoy.begin();

  myIsTaskRunning = true;
  vTaskResume(myTaskHandle);
}

// ----------------------------------------------------------------------------------------
InputController::State InputController::getState()
{
  xSemaphoreTake(myMutex, portMAX_DELAY);

  State ret(myState);

  myJoy.update();
  ret.up    = myJoy.isUp();
  ret.down  = myJoy.isDown();
  ret.left  = myJoy.isLeft();
  ret.right = myJoy.isRight();
  ret.x     = myJoy.getX();
  ret.y     = myJoy.getY();

  // reset button change state
  myState.a = myState.b = myState.aux = myState.sel = myState.start = myState.joy = false;

  xSemaphoreGive(myMutex);

  return ret;
}

// ----------------------------------------------------------------------------------------
void InputController::processEvents(void* pvParameters)
{
  InputController* c = (InputController*) pvParameters;

  while (true) 
  {
    if (c->myIsTaskRunning == true)
    {
      xSemaphoreTake(c->myMutex, portMAX_DELAY);
      
      // ---- Process buttons

      c->myBtnA.update();
      if (c->myBtnA.pressed())
      {
        c->myState.a = true;
      }
      c->myState.aIsPressed = c->myBtnA.isPressed();

      c->myBtnB.update();
      if (c->myBtnB.pressed())
      {
        c->myState.b = true;
      }
      c->myState.bIsPressed = c->myBtnB.isPressed();

#if BOARD_HAS_AUX_BUTTON()
      c->myBtnAux.update();
      if (c->myBtnAux.pressed())
      {
        c->myState.aux = true;
      }
      c->myState.auxIsPressed = c->myBtnAux.isPressed();
#endif

      c->myBtnSel.update();
      if (c->myBtnSel.pressed())
      {
        c->myState.sel = true;
      }
      c->myState.selIsPressed = c->myBtnSel.isPressed();

      c->myBtnStart.update();
      if (c->myBtnStart.pressed())
      {
        c->myState.start = true;
      }
      c->myState.startIsPressed = c->myBtnStart.isPressed();

      c->myBtnJoy.update();
      if (c->myBtnJoy.pressed())
      {
        c->myState.joy = true;
      }
      c->myState.joyIsPressed = c->myBtnJoy.isPressed();

      xSemaphoreGive(c->myMutex);

      vTaskDelay(5 / portTICK_PERIOD_MS);
    }
  }
}

