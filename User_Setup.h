#define ST7735_DRIVER

#define TFT_WIDTH  80
#define TFT_HEIGHT 160

#define ST7735_REDTAB160x80

#define TFT_RGB_ORDER TFT_BGR

#define TFT_MISO -1
#define TFT_MOSI  3
#define TFT_SCLK  2
#define TFT_CS    7  
#define TFT_DC    6  
#define TFT_RST   10

#define LOAD_GLCD   
#define LOAD_FONT2  
#define LOAD_FONT4  
#define LOAD_FONT6  
#define LOAD_FONT7  
#define LOAD_FONT8
#define LOAD_GFXFF
#define SMOOTH_FONT

#define SPI_FREQUENCY  27000000