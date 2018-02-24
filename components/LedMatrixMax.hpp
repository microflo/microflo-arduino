/* microflo_component yaml
name: LedMatrixMax
description: Set characters on MAX7219 display
icon: "lightbulb-o"
inports:
  in:
    type: all
    description: ""
  pincs:
    type: all
    description: ""
  pindin:
    type: all
    description: ""
  pinclk:
    type: all
    description: ""
outports:
  out:
    type: all
    description: ""
microflo_component */


static const unsigned char max7219_characters[38][8]={
    {0x3C,0x42,0x42,0x42,0x42,0x42,0x42,0x3C},//0
    {0x10,0x18,0x14,0x10,0x10,0x10,0x10,0x10},//1
    {0x7E,0x2,0x2,0x7E,0x40,0x40,0x40,0x7E},//2
    {0x3E,0x2,0x2,0x3E,0x2,0x2,0x3E,0x0},//3
    {0x8,0x18,0x28,0x48,0xFE,0x8,0x8,0x8},//4
    {0x3C,0x20,0x20,0x3C,0x4,0x4,0x3C,0x0},//5
    {0x3C,0x20,0x20,0x3C,0x24,0x24,0x3C,0x0},//6
    {0x3E,0x22,0x4,0x8,0x8,0x8,0x8,0x8},//7
    {0x0,0x3E,0x22,0x22,0x3E,0x22,0x22,0x3E},//8
    {0x3E,0x22,0x22,0x3E,0x2,0x2,0x2,0x3E},//9

    {0x8,0x14,0x22,0x3E,0x22,0x22,0x22,0x22},//A
    {0x3C,0x22,0x22,0x3E,0x22,0x22,0x3C,0x0},//B
    {0x3C,0x40,0x40,0x40,0x40,0x40,0x3C,0x0},//C
    {0x7C,0x42,0x42,0x42,0x42,0x42,0x7C,0x0},//D
    {0x7C,0x40,0x40,0x7C,0x40,0x40,0x40,0x7C},//E
    {0x7C,0x40,0x40,0x7C,0x40,0x40,0x40,0x40},//F
    {0x3C,0x40,0x40,0x40,0x40,0x44,0x44,0x3C},//G
    {0x44,0x44,0x44,0x7C,0x44,0x44,0x44,0x44},//H
    {0x7C,0x10,0x10,0x10,0x10,0x10,0x10,0x7C},//I
    {0x3C,0x8,0x8,0x8,0x8,0x8,0x48,0x30},//J
    {0x0,0x24,0x28,0x30,0x20,0x30,0x28,0x24},//K
    {0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x7C},//L
    {0x81,0xC3,0xA5,0x99,0x81,0x81,0x81,0x81},//M
    {0x0,0x42,0x62,0x52,0x4A,0x46,0x42,0x0},//N
    {0x3C,0x42,0x42,0x42,0x42,0x42,0x42,0x3C},//O
    {0x3C,0x22,0x22,0x22,0x3C,0x20,0x20,0x20},//P
    {0x1C,0x22,0x22,0x22,0x22,0x26,0x22,0x1D},//Q
    {0x3C,0x22,0x22,0x22,0x3C,0x24,0x22,0x21},//R
    {0x0,0x1E,0x20,0x20,0x3E,0x2,0x2,0x3C},//S
    {0x0,0x3E,0x8,0x8,0x8,0x8,0x8,0x8},//T
    {0x42,0x42,0x42,0x42,0x42,0x42,0x22,0x1C},//U
    {0x42,0x42,0x42,0x42,0x42,0x42,0x24,0x18},//V
    {0x0,0x49,0x49,0x49,0x49,0x2A,0x1C,0x0},//W
    {0x0,0x41,0x22,0x14,0x8,0x14,0x22,0x41},//X
    {0x41,0x22,0x14,0x8,0x8,0x8,0x8,0x8},//Y
    {0x0,0x7F,0x2,0x4,0x8,0x10,0x20,0x7F},//Z
};

// TODO: split out some of this into parallel->serial SPI like components?
class LedMatrixMax : public SingleOutputComponent {
public:
    LedMatrixMax()
        : pin_cs(-1)
        , pin_din(-1)
        , pin_clk(-1)
        , initialized(false)
    {}

    virtual void process(Packet in, MicroFlo::PortId port) {
        using namespace LedMatrixMaxPorts;
        if (port == InPorts::in) {
            if (in.isInteger() && in.asInteger() < 38) {
                charIndex = in.asInteger();
            } else if (in.isByte()) {
                const unsigned char c = in.asByte();
                if (c > 'A' && c <= 'Z') {
                    charIndex = 10 + c-'A';
                }
            }
            update();
        } else if (port == InPorts::pinclk) {
            pin_clk = in.asInteger();
            initialized = false;
            update();
        } else if (port == InPorts::pincs) {
            pin_cs = in.asInteger();
            initialized = false;
            update();
        } else if (port == InPorts::pindin) {
            pin_din = in.asInteger();
            initialized = false;
            update();
        }
    }
private:
    void update() {
        if (pin_cs < 0 || pin_din < 0 || pin_clk < 0) {
            return;
        }
        if (!initialized) {
            io->PinSetMode(pin_cs, IO::OutputPin);
            io->PinSetMode(pin_din, IO::OutputPin);
            io->PinSetMode(pin_clk, IO::OutputPin);
            max7219_init();
            initialized = true;
        }
        for (uint8_t i=1; i<9; i++) {
            max7219_write(i, max7219_characters[charIndex][i-1]);
        }
    }

    void max7219_write_byte(unsigned char DATA) {
       for (uint8_t i=8; i>=1; i--) {
            io->DigitalWrite(pin_clk, false);
            io->DigitalWrite(pin_din, DATA&0x80);
            io->DigitalWrite(pin_clk, true);
            DATA = DATA<<1;
       }
    }

    void max7219_write(unsigned char address, unsigned char dat) {
       io->DigitalWrite(pin_cs, false);
       max7219_write_byte(address);
       max7219_write_byte(dat);
       io->DigitalWrite(pin_cs, true);
    }

    void max7219_init() {
        max7219_write(0x09, 0x00);       // decoding: BCD
        max7219_write(0x0a, 0x03);       // brightness
        max7219_write(0x0b, 0x07);       // scanlimit: 8 leds
        max7219_write(0x0c, 0x01);       // 0=power-down,1=normal
        max7219_write(0x0f, 0x00);       // 1=test-display, 0=EOT
    }

private:
    int pin_cs;
    int pin_din;
    int pin_clk;
    bool initialized;
    uint8_t charIndex;
};
