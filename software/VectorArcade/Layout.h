// Layout.h - screen geometry of the console selected in BoardSelect.h
#pragma once

#if __has_include("BoardSelect.h")
  #include "BoardSelect.h"
#else
  #error "BoardSelect.h is missing: copy BoardSelect.h.template to BoardSelect.h and select the console"
#endif

#if BOARD_IS_LITTLEGAMECONSOLE()
  #include "boards/LittleGameConsole/Layout.h"
#elif BOARD_IS_TINTINROCKETSHOOTER()
  #include "boards/TintinRocketShooter/Layout.h"
#endif
