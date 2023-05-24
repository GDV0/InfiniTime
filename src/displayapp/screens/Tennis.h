#pragma once

#include <lvgl/lvgl.h>
#include "displayapp/screens/Screen.h"


namespace Pinetime {
  namespace Components {
    class LittleVgl;
  }

  namespace Controllers {
    class Settings;
  }

  namespace Applications {
    namespace Screens {

      class Tennis : public Screen {
      public:
        Tennis(Pinetime::Components::LittleVgl& lvgl, Controllers::Settings& settingsController);
        ~Tennis() override;

        void Refresh() override;

        bool OnTouchEvent(TouchEvents event) override;
        bool OnTouchEvent(uint16_t x, uint16_t y) override;

      private:
        Pinetime::Components::LittleVgl& lvgl;
        Controllers::Settings& settingsController;

      };
    }
  }
}

