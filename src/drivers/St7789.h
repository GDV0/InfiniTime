#pragma once
#include <cstddef>
#include <cstdint>

//#define DRIVER_DISPLAY_MIRROR

enum Orientation : uint8_t {
  Orientation_0   = 0x00, // MADCTL[MY=0,MX=0,MV=0]
  Orientation_90  = 0x60, // MADCTL[MY=0,MX=1,MV=1]
  Orientation_180 = 0xc0, // MADCTL[MY=1,MX=1,MV=0]
  Orientation_270 = 0xa0, // MADCTL[MY=1,MX=0,MV=1]
  };

namespace Pinetime {
  namespace Drivers {
    class Spi;

    class St7789 {
    public:
      explicit St7789(Spi& spi, uint8_t pinDataCommand, uint8_t pinReset);
      St7789(const St7789&) = delete;
      St7789& operator=(const St7789&) = delete;
      St7789(St7789&&) = delete;
      St7789& operator=(St7789&&) = delete;

      void Init();
      void Uninit();
      void DrawPixel(uint16_t x, uint16_t y, uint32_t color);

      void VerticalScrollDefinition(uint16_t topFixedLines, uint16_t scrollLines, uint16_t bottomFixedLines);
      void VerticalScrollStartAddress(uint16_t line);

      void DrawBuffer(uint16_t x, uint16_t y, uint16_t width, uint16_t height, const uint8_t* data, size_t size);

      
      void SetOrientation(Orientation orientation);
      Orientation GetOrientation();

      void Sleep();
      void Wakeup();

    private:
      Spi& spi;
      uint8_t pinDataCommand;
      uint8_t pinReset;
      uint8_t verticalScrollingStartAddress = 0;
      uint8_t madctlReg = Orientation_0;

      void HardwareReset();
      void SoftwareReset();
      void SleepOut();
      void SleepIn();
      void ColMod();
      void MemoryDataAccessControl();
      void DisplayInversionOn();
      void NormalModeOn();
      void WriteToRam();
      void DisplayOn();
      void DisplayOff();

      void SetAddrWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);
      void SetVdv();
      void WriteCommand(uint8_t cmd);
      void WriteSpi(const uint8_t* data, size_t size);

      enum class Commands : uint8_t {
        SoftwareReset = 0x01,
        SleepIn = 0x10,
        SleepOut = 0x11,
        NormalModeOn = 0x13,
        DisplayInversionOn = 0x21,
        DisplayOff = 0x28,
        DisplayOn = 0x29,
        ColumnAddressSet = 0x2a,
        RowAddressSet = 0x2b,
        WriteToRam = 0x2c,
        MemoryDataAccessControl = 0x36,
        VerticalScrollDefinition = 0x33,
        VerticalScrollStartAddress = 0x37,
        ColMod = 0x3a,
        VdvSet = 0xc4,
      };
      void WriteData(uint8_t data);
      void ColumnAddressSet();

      static constexpr uint16_t Width = 240;
      static constexpr uint16_t Height = 320;
//      static constexpr uint16_t Height = 240;
      void RowAddressSet();
    };
  }
}
