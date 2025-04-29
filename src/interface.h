#ifndef INTERFACE_H
#define INTERFACE_H

#include <string>

#ifndef INTERFACE_X_OFFSET
#define INTERFACE_X_OFFSET 0
#endif

#ifndef INTERFACE_Y_OFFSET
#define INTERFACE_Y_OFFSET 0
#endif

#ifndef COMMAND_DISPLAY_DURATION
#define COMMAND_DISPLAY_DURATION 2000
#endif

#ifndef FONT_SIZE
#define FONT_SIZE 2
#endif

#ifndef FONT_TYPE
#define FONT_TYPE 2
#endif

#ifndef TFT_WIDTH
#define TFT_WIDTH 240
#endif

#ifndef TFT_HEIGHT
#define TFT_HEIGHT 320
#endif

#ifndef TFT_MOSI
#define TFT_MOSI 5
#endif

#ifndef TFT_SCLK
#define TFT_SCLK 6
#endif

#ifndef TFT_CS
#define TFT_CS 7
#endif

#ifndef TFT_DC
#define TFT_DC 4
#endif

#ifndef TFT_RST
#define TFT_RST 8
#endif

#ifndef TFT_BL
#define TFT_BL 9
#endif

#ifndef SPI_FREQUENCY
#define SPI_FREQUENCY 27000000
#endif

#ifndef TFT_MISO
#define TFT_MISO -1
#endif

#ifndef TFT_BUSY
#define TFT_BUSY -1
#endif

class Interface {
public:
    void setup();
    void update_force_display();
    void show_command(const std::string& command);
    void loop();
    void update_x_offset(const int16_t x_offset);
    void update_y_offset(const int16_t y_offset);
    void update_line_height(const int16_t height);
    void update_unit(const std::string& new_unit);
    void update_force_zero();
    void update_force_slope(const float actual);
    void save_force_slope();
    void clear();

    int32_t force;

private:
    char current_force[14] = "-0000000000\0";
    char corrected_force_char[22] = "";
    char last_command[32] = "";
    unsigned long command_display_time = 0;

    int16_t x_offset = INTERFACE_X_OFFSET;
    int16_t y_offset = INTERFACE_Y_OFFSET;
    int16_t line_height = INTERFACE_LINE_HEIGHT;

    int32_t force_zero = 0;
    double force_slope = 1.0f;
    std::string unit = " mN";

    uint64_t masked_force_slope;
    
    bool command_used = false;
};

#endif // INTERFACE_H