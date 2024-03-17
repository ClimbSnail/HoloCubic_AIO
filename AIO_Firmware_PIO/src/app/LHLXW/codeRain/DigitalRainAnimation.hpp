/*
  DigitalRainAnimation.hpp - Library for Digital Rain Animation(MATRIX EFFECT).
  Created by Eric Nam, November 08, 2021.
  Released into the public domain.
*/

#ifndef _DIGITAL_RAIN_ANIMATION_H
#define _DIGITAL_RAIN_ANIMATION_H
#define FONT_SIZE 2               //set font size 2
#define LINE_WIDTH 12             //width for font size 2
#define LETTER_HEIGHT 14          //height for font size 2
#define KEY_RESET_TIME 60 * 1000  //60 seconds reset time
#include <vector>
#include <string>
#include "TFT_eSPI.h"
#include "common.h"
enum AnimMode { SHOWCASE,
                MATRIX,
                TEXT };

template<class T>
class DigitalRainAnimation {
private:
  TFT_eSPI* _gfx = NULL;
  AnimMode _animMode;
  int line_len_min;              //minimum length of characters
  int line_len_max;              //maximum length of characters
  int line_speed_min;            //minimum vertical move speed
  int line_speed_max;            //maximum vertical move speed
  int width, height;             //width, height of display
  int numOfline;                 //number of calculated row
  int timeFrame;                 //time frame for drawing
  int matrixTimeFrame;           //time frame for drawing
  uint8_t fontSize;              //default font size 2
  uint8_t lineWidth;             //default line width
  uint8_t letterHeight;          //default letter height
  bool isPlaying;                //boolean for play or pause
  bool isAlphabetOnly;           //boolean for showing Alphabet only
  uint32_t lastDrawTime;         //checking last drawing time
  uint32_t lastUpdatedKeyTime;   //checking last generating key time
  uint16_t headCharColor;        //having a text color
  uint16_t textColor;            //having a text color
  uint16_t bgColor;              //having a bg color
  std::vector<int> line_length;  //dynamic array for each line of vertical length
  std::vector<int> line_pos;     //dynamic array for eacg line Y position
  std::vector<int> line_speed;   //dynamic array for eacg line speed
  std::string keyString;         //storing generated key

  uint8_t textStartX;            //cursor X for text
  uint8_t textStartY;            //cursor Y for text
  int textTimeFrame;             //text time frame for drawing
  std::string textMessage;       //storing text message
  std::string workingTextMessage; //working text message

  void prepareAnim() {
    if (_gfx == NULL) return;

    setHeadCharColor(255, 255, 255);
    setTextColor(0, 255, 0);
    setBGColor(0, 0, 0);

    lastDrawTime = millis() - timeFrame;
    width = _gfx->width();
    height = _gfx->height();
    _gfx->fillRect(0, 0, width, height, bgColor);
    _gfx->setTextColor(textColor, bgColor);
    numOfline = width / lineWidth + 1;

    for (int i = 0; i < numOfline; i++) {
      line_length.push_back(getRandomNum(line_len_min, line_len_max));
      line_pos.push_back(setYPos(line_length[i]) - letterHeight);
      line_speed.push_back(getRandomNum(line_speed_min, line_speed_max));
    }

    isPlaying = true;
    lastUpdatedKeyTime = millis() - timeFrame;
  }

  //updating each line with a new length, Y position, and speed.
  void lineUpdate(int lineNum) {
    line_length[lineNum] = getRandomNum(line_len_min, line_len_max);
    line_pos[lineNum] = setYPos(line_length[lineNum]);
    line_speed[lineNum] = getRandomNum(line_speed_min, line_speed_max);
  }

  //while moving vertically, the color value changes and the character changes as well.
  //if a random key is generated, switch to red.
  void lineAnimation(int lineNum) {
    int startX = lineNum * lineWidth;
    int currentY = -letterHeight;
    tft->fillRect(startX, 0, lineWidth, height, bgColor);

    bool isKeyMode = keyString.length() > 0;

    for (int i = 0; i < line_length[lineNum]; i++) {
      int colorVal = map(i, 0, line_length[lineNum], 10, 255);
      uint16_t lumColor = luminance(textColor, colorVal);
      tft->setTextColor(isKeyMode ? _gfx->color565(colorVal, 0, 0) : lumColor, bgColor);
      // _gfx->setCursor(startX, line_pos[lineNum] + currentY);
      // _gfx->setTextSize(fontSize);
      // _gfx->print(isAlphabetOnly ? getAbcASCIIChar() : getASCIIChar());

      char letter = (char)random(64,126);
      tft->drawChar(letter,startX,line_pos[lineNum] + currentY,fontSize);
      currentY = (i * letterHeight);
    }

    tft->setTextColor(headCharColor, bgColor);

    if (keyString.length() > lineNum) {
      // char _char = keyString.at(lineNum);
      // const char* keyChar = &_char;
      // _gfx->setCursor(startX, line_pos[lineNum] + currentY);
      // _gfx->setTextSize(fontSize);
      // _gfx->print(keyChar);
      tft->drawChar('a',startX,line_pos[lineNum] + currentY,fontSize);
    } else {
      // _gfx->setCursor(startX, line_pos[lineNum] + currentY);
      // _gfx->setTextSize(fontSize);
      //_gfx->print(isAlphabetOnly ? getAbcASCIIChar() : getASCIIChar());
      // char letter = getASCIIChar().at(0);
      char letter = (char)random(64,126);
      tft->drawChar(letter,startX,line_pos[lineNum] + currentY,fontSize);
    }

    line_pos[lineNum] += line_speed[lineNum];

    if (line_pos[lineNum] >= height) {
      lineUpdate(lineNum);
    }
  }



  //a function that gets randomly from ASCII codes 33 to 65 and 91 to 126. (For MatrixCodeNFI)
  String getASCIIChar() {
    return String((char)(random(0, 2) == 0 ? random(33, 65) : random(91, 126)));
  }

  //a function that gets only alphabets from ASCII code.
  String getAbcASCIIChar() {
    return String((char)(random(0, 2) == 0 ? random(65, 91) : random(97, 123)));
  }

  //move the position to start from out of the screen.
  int setYPos(int lineLen) {
    return lineLen * -20;
  }

  //the function is to get the random number (including max)
  int getRandomNum(int min, int max) {
    return random(min, max + 1);
  }

  //the function is to generate a random key with length with a length
  std::string getKey(int key_length) {
    resetKey();
    int maxKeyLength = (key_length > 0 ? (key_length > numOfline ? numOfline : key_length) : numOfline);

    for (int i = 0; i < maxKeyLength; i++) {
      keyString.append((getAbcASCIIChar()).c_str());
    }

    return keyString;
  }

  //the function is to remove the generated key
  void resetKey() {
    keyString = "";
    lastUpdatedKeyTime = millis();
  }

  //set Text Bigger
  void setBigText(bool isOn) {
    fontSize = isOn ? FONT_SIZE * 2 : FONT_SIZE;
    lineWidth = isOn ? LINE_WIDTH * 2 : LINE_WIDTH;
    letterHeight = isOn ? LETTER_HEIGHT * 1.6 : LETTER_HEIGHT;
  }

  //From TFT_eFEX
  //https://github.com/Bodmer/TFT_eFEX
  uint16_t luminance(uint16_t color, uint8_t luminance) {
    // Extract rgb colours and stretch range to 0 - 255
    uint16_t r = (color & 0xF800) >> 8;
    r |= (r >> 5);
    uint16_t g = (color & 0x07E0) >> 3;
    g |= (g >> 6);
    uint16_t b = (color & 0x001F) << 3;
    b |= (b >> 5);

    b = ((b * (uint16_t)luminance + 255) >> 8) & 0x00F8;
    g = ((g * (uint16_t)luminance + 255) >> 8) & 0x00FC;
    r = ((r * (uint16_t)luminance + 255) >> 8) & 0x00F8;

    return (r << 8) | (g << 3) | (b >> 3);
  }

  //the function is to run text animation
  void textAnimation() {
    int textMsgLength = workingTextMessage.length();

    if (textMsgLength <= 0) {
      _gfx->fillRect(0, 0, _gfx->width(), _gfx->height(), bgColor);
      if (_animMode == TEXT) {
        _gfx->setTextSize(fontSize);
        _gfx->setCursor(textStartX, textStartY);
        workingTextMessage = textMessage;
      } else {
        _animMode = MATRIX;
        timeFrame = matrixTimeFrame;
      }
      return;
    }

    char letter = workingTextMessage.at(0);
    if (letter == '\n') {
      _gfx->fillRect(0, 0, _gfx->width(), _gfx->height(), bgColor);
      _gfx->setTextSize(fontSize);
      _gfx->setCursor(textStartX, textStartY);
    } else {
      _gfx->print(letter);
    }

    workingTextMessage.erase(0, 1);
  }


public:
  DigitalRainAnimation() {}

  //initialization
  void init(T* gfx ,bool biggerText) {
    _gfx = gfx;
    _animMode = SHOWCASE;
    line_len_min = 3;
    line_len_max = 20;
    line_speed_min = 3;
    line_speed_max = 15;
    timeFrame = 100;


    setBigText(biggerText);
    prepareAnim();
  }

  //setup for Matrix
  void setup(int new_line_len_min, int new_line_len_max, int new_line_speed_min, int new_line_speed_max, int matrix_timeFrame) {
    line_len_min = new_line_len_min;
    line_len_max = new_line_len_max;
    line_speed_min = new_line_speed_min;
    line_speed_max = new_line_speed_max;
    matrixTimeFrame = matrix_timeFrame;
    prepareAnim();
  }

  //setup for Text Animation
  void setTextAnimMode(AnimMode animMode, std::string msg, int startX = 60, int startY = 60, int text_timeFrame = 100) {
    _animMode = animMode;
    setAnimText(msg);
    textStartX = startX;
    textStartY = startY;
    textTimeFrame = text_timeFrame;
    timeFrame = textTimeFrame;
  }

  //set Animation Text
  void setAnimText(std::string msg) {
    textMessage = msg;
    workingTextMessage = textMessage;
    timeFrame = 400;
  }

  //set Head Char Color
  void setHeadCharColor(uint8_t red, uint8_t green, uint8_t blue) {
    headCharColor = _gfx->color565(red, green, blue);
  }

  //set Text Color
  void setTextColor(uint8_t red, uint8_t green, uint8_t blue) {
    textColor = _gfx->color565(red, green, blue);
  }

  //set BG Color
  void setBGColor(uint8_t red, uint8_t green, uint8_t blue) {
    bgColor = _gfx->color565(red, green, blue);
  }

  //updating screen
  void loop() {
    if (_gfx == NULL) return;

    uint32_t currentTime = millis();
    if (((currentTime - lastUpdatedKeyTime) > KEY_RESET_TIME)) {
      resetKey();
    }

    if (((currentTime - lastDrawTime) < timeFrame)) {
      return;
    }

    if (isPlaying) {
      for (int i = 0; i < numOfline; i++) lineAnimation(i);
    }

    lastDrawTime = currentTime;
  }

  //a function to stop animation.
  void pause() {
    isPlaying = false;
  }

  //a function to resume animation.
  void resume() {
    isPlaying = true;
  }
};

#endif