#include <Wire.h>
#include "MLX90640_I2C_Driver.h"
#include "MLX90640_API.h"
#include <SPI.h>
#include <TFT_eSPI.h>

const byte MLX90640_address = 0x33; //Default 7-bit unshifted address of the MLX90640
 
#define TA_SHIFT 8 //Default shift for MLX90640 in open air

TFT_eSPI tft = TFT_eSPI( );

static float mlx90640To[768];
paramsMLX90640 mlx90640;

int posX, posY;
int R_colour, G_colour, B_colour;
int i, j;
float T_max, T_min;
float T_center;

void setup() {
    Serial.begin(115200);
    
    Wire.begin(4,5);
    Wire.setClock(400000); //Increase I2C clock speed to 400kHz

    //Wire.setClock(200000); //Increase I2C clock speed to 400kHz
    while (!Serial); //Wait for user to open terminal
    
    Serial.println("MLX90640 IR Array Example");
 
    if (isConnected() == false)
       {
        Serial.println("MLX90640 not detected at default I2C address. Please check wiring. Freezing.");
        while (1);
       }
       
    Serial.println("MLX90640 online!");
 
    //Get device parameters - We only have to do this once
    int status;
    uint16_t eeMLX90640[832];
    
    status = MLX90640_DumpEE(MLX90640_address, eeMLX90640);
  
    if (status != 0)
       Serial.println("Failed to load system parameters");
 
    status = MLX90640_ExtractParameters(eeMLX90640, &mlx90640);
  
    if (status != 0)
       {
        Serial.println("Parameter extraction failed");
        Serial.print(" status = ");
        Serial.println(status);
       }
 
    //Once params are extracted, we can release eeMLX90640 array
 
    MLX90640_I2CWrite(0x33, 0x800D, 6401);    // writes the value 1901 (HEX) = 6401 (DEC) in the register at position 0x800D to enable reading out the temperatures!!!
    // ===============================================================================================================================================================
 
    //MLX90640_SetRefreshRate(MLX90640_address, 0x00); //Set rate to 0.25Hz effective - Works
    //MLX90640_SetRefreshRate(MLX90640_address, 0x01); //Set rate to 0.5Hz effective - Works
    //MLX90640_SetRefreshRate(MLX90640_address, 0x02); //Set rate to 1Hz effective - Works
    //MLX90640_SetRefreshRate(MLX90640_address, 0x03); //Set rate to 2Hz effective - Works
    MLX90640_SetRefreshRate(MLX90640_address, 0x04); //Set rate to 4Hz effective - Works best
    //MLX90640_SetRefreshRate(MLX90640_address, 0x05); //Set rate to 8Hz effective - Works at 800kHz
    //MLX90640_SetRefreshRate(MLX90640_address, 0x06); //Set rate to 16Hz effective - Works at 800kHz
    //MLX90640_SetRefreshRate(MLX90640_address, 0x07); //Set rate to 32Hz effective - fails

    Wire.setClock(800000);


    tft.init();
    tft.setRotation(3);
    tft.fillScreen(TFT_BLACK);


    // drawing the colour-scale
    // ========================
    
    tft.drawLine(114, 4 + 0, 118, 4 + 0, tft.color565(255, 255, 255));
    tft.drawLine(114, 4 + 18, 118, 4 + 18, tft.color565(255, 255, 255));
    tft.drawLine(114, 4 + 36, 118, 4 + 36, tft.color565(255, 255, 255));
    tft.drawLine(114, 4 + 54, 118, 4 + 54, tft.color565(255, 255, 255));
    tft.drawLine(114, 4 + 72, 118, 4 + 72, tft.color565(255, 255, 255));

    for (i = 0; i < 73; i++)
      {
        getColour(2.5 * i);
        tft.drawLine(106, 76 - i, 114, 76 - i, tft.color565(R_colour, G_colour, B_colour));
      } 
   }

void loop() {

  // Calculating max/min temp
  // ====================================================
    for (byte x = 0 ; x < 2 ; x++) //Read both subpages
       {
        uint16_t mlx90640Frame[834];
        int status = MLX90640_GetFrameData(MLX90640_address, mlx90640Frame);
    
        if (status < 0)
           {
            Serial.print("GetFrame Error: ");
            Serial.println(status);
           }
 
        float vdd = MLX90640_GetVdd(mlx90640Frame, &mlx90640);
        float Ta = MLX90640_GetTa(mlx90640Frame, &mlx90640);
 
        float tr = Ta - TA_SHIFT; //Reflected temperature based on the sensor ambient temperature
        float emissivity = 0.95;
 
        MLX90640_CalculateTo(mlx90640Frame, &mlx90640, emissivity, tr, mlx90640To);
       }
 
       
    // determine T_min and T_max and eliminate error pixels
    // ====================================================
 
//    mlx90640To[1*32 + 21] = 0.5 * (mlx90640To[1*32 + 20] + mlx90640To[1*32 + 22]);    // eliminate the error-pixels
//    mlx90640To[4*32 + 30] = 0.5 * (mlx90640To[4*32 + 29] + mlx90640To[4*32 + 31]);    // eliminate the error-pixels
    
    T_min = mlx90640To[0];
    T_max = mlx90640To[0];
 
    for (i = 1; i < 768; i++)
       {
        if((mlx90640To[i] > -41) && (mlx90640To[i] < 301))
           {
            if(mlx90640To[i] < T_min)
               {
                T_min = mlx90640To[i];
               }
 
            if(mlx90640To[i] > T_max)
               {
                T_max = mlx90640To[i];
               }
           }
        else if(i > 0)   // temperature out of range
           {
            mlx90640To[i] = mlx90640To[i-1];
           }
        else
           {
            mlx90640To[i] = mlx90640To[i+1];
           }
       }


    // determine T_center
    // ==================
 
    T_center = mlx90640To[11* 32 + 15];    
 
    // drawing the picture
    // ===================
 
    for (i = 0 ; i < 24 ; i++)
       {
        for (j = 0; j < 32; j++)
           {
            mlx90640To[i*32 + j] = 180.0 * (mlx90640To[i*32 + j] - T_min) / (T_max - T_min);
                       
            getColour(mlx90640To[i*32 + j]);
            
            tft.fillRect(100 - j * 3, 4 + i * 3, 4, 4, tft.color565(R_colour, G_colour, B_colour));
           }
       }

    tft.setTextColor(TFT_WHITE, tft.color565(0, 0, 0));
    tft.setCursor(121, 1);
//    tft.print(T_max);
    tft.print(String(T_max, 1));
    tft.setCursor(121, 73);
//    tft.print(T_min);
    tft.print(String(T_min, 1));
    tft.setCursor(134, 37);
//    tft.print(T_center);
    tft.print(String(T_center, 1));
 
    tft.setCursor(145, 1);
    tft.print("C");
    tft.setCursor(145, 73);
    tft.print("C");
    tft.setCursor(121, 37);
    tft.print("T+"); 

    int w = tft.textWidth("+");
    int h = tft.fontHeight();
    posX = (96 - w) / 2 + 9;
    posY = (72 - h) / 2 + 6;
    
    if (T_center >= (T_max + T_min) / 2)
      {
         tft.setCursor(posX, posY);
         tft.setTextColor(TFT_BLACK);
         tft.print("+");
      } else {
         tft.setCursor(posX, posY);
         tft.setTextColor(TFT_WHITE);
         tft.print("+");
      }
   }

// ===============================
// ===== determine the colour ====
// ===============================
 
void getColour(int j)
   {
    if (j >= 0 && j < 30)
       {
        R_colour = 0;
        G_colour = 0;
        B_colour = 20 + (120.0/30.0) * j;
       }
    
    if (j >= 30 && j < 60)
       {
        R_colour = (120.0 / 30) * (j - 30.0);
        G_colour = 0;
        B_colour = 140 - (60.0/30.0) * (j - 30.0);
       }
 
    if (j >= 60 && j < 90)
       {
        R_colour = 120 + (135.0/30.0) * (j - 60.0);
        G_colour = 0;
        B_colour = 80 - (70.0/30.0) * (j - 60.0);
       }
 
    if (j >= 90 && j < 120)
       {
        R_colour = 255;
        G_colour = 0 + (60.0/30.0) * (j - 90.0);
        B_colour = 10 - (10.0/30.0) * (j - 90.0);
       }
 
    if (j >= 120 && j < 150)
       {
        R_colour = 255;
        G_colour = 60 + (175.0/30.0) * (j - 120.0);
        B_colour = 0;
       }
 
    if (j >= 150 && j <= 180)
       {
        R_colour = 255;
        G_colour = 235 + (20.0/30.0) * (j - 150.0);
        B_colour = 0 + 255.0/30.0 * (j - 150.0);
       }
   }

//Returns true if the MLX90640 is detected on the I2C bus
boolean isConnected()
   {
    Wire.beginTransmission((uint8_t)MLX90640_address);
  
    if (Wire.endTransmission() != 0)
       return (false); //Sensor did not ACK
    
    return (true);
   }
