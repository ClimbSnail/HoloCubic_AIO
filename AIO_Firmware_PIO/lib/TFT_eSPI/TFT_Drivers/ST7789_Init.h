
// This is the command sequence that initialises the ST7789 driver
//
// This setup information uses simple 8 bit SPI writecommand() and writedata() functions
//
// See ST7735_Setup.h file for an alternative format

{
  writecommand(ST7789_SLPOUT);   // Sleep out
  delay(120);

  writecommand(ST7789_NORON);    // Normal display mode on

  //------------------------------display and color format setting--------------------------------//
  writecommand(ST7789_MADCTL);
  writedata(0x00);
  writedata(TFT_MAD_COLOR_ORDER);

  // JLX240 display datasheet
  writecommand(0xB6);
  writedata(0x0A);
  writedata(0x82);

  writecommand(ST7789_COLMOD);
  writedata(0x55);  //色彩模式
  delay(10);

  writecommand(ST7789_RAMCTRL); // 0xB0
  writedata(0x00);
  writedata(0xC0); // 0xC0，而不是默认的0xF0

  //--------------------------------ST7789V Frame rate setting----------------------------------//
  writecommand(ST7789_PORCTRL);
  writedata(0x0c);
  writedata(0x0c);
  writedata(0x00);
  writedata(0x33);
  writedata(0x33);

  writecommand(ST7789_GCTRL);      // Voltages: VGH / VGL
  writedata(0x04);

  //---------------------------------ST7789V Power setting--------------------------------------//
  writecommand(ST7789_VCOMS);  //VCOMS+VDV=0.1V~1.675V //馈通电压补偿 (1.3寸你给的资料最大值应该是30)
  writedata(0x30);		//查询显示屏资料设置(资料不详细，取最大值) //效果类似清晰度

  writecommand(ST7789_LCMCTRL);
  writedata(0x0C);

  writecommand(ST7789_VDVVRHEN);  //开启下面两个控制 VRHS VDV
  writedata(0x01);
//  writedata(0xFF);

  writecommand(ST7789_VRHS);       //GVDD // voltage VRHS //效果灰度，类似对比度(无资料，肉眼校色)
  writedata(0x06);

  writecommand(ST7789_VDVSET);   //voltage VDV //效果灰度，类似对比度(无资料，肉眼校色)(20设置为0，可单调整VRHS)
  writedata(0x29);

  writecommand(ST7789_FRCTR2);   //帧数，默认60
  writedata(0x0F);

  writecommand(ST7789_PWCTRL1);
  writedata(0xa4);
  writedata(0xa1);

  
//  writecommand(ST7789_GAMSET);  //启用预设伽马值设置
//  writedata(0x01);  //预设伽马值（2.2和电脑一样）

//--------------------------------ST7789V gamma setting---------------------------------------//
  writecommand(ST7789_PVGAMCTRL);
  writedata(0xD0);
  writedata(0x08);
  writedata(0x11);
  writedata(0x08);
  writedata(0x0C);
  writedata(0x15);
  writedata(0x39);
  writedata(0x33);
  writedata(0x50);
  writedata(0x36);
  writedata(0x13);
  writedata(0x14);
  writedata(0x29);
  writedata(0x2D);

  writecommand(ST7789_NVGAMCTRL);
  writedata(0xD0);
  writedata(0x08);
  writedata(0x10);
  writedata(0x08);
  writedata(0x06);
  writedata(0x06);
  writedata(0x39);
  writedata(0x44);
  writedata(0x51);
  writedata(0x0B);
  writedata(0x16);
  writedata(0x14);
  writedata(0x2F);
  writedata(0x31);
//--------------------------------ST7789V gamma setting---------------------------------------//
//(不会玩自定义gamma曲线，直接预设，反正也不知道他校准的什么屏幕)

  writecommand(ST7789_INVON);  //颜色反转关闭
//  writecommand(ST7789_INVOFF);  //颜色反转打开

  writecommand(ST7789_CASET);    // Column address set 
  writedata(0x00);    //列起始地址
  writedata(0x00);
  writedata(0x00);    //列结束地址
  writedata(0xE5);    // 240像素

  writecommand(ST7789_RASET);    // Row address set
  writedata(0x00);    //行起始地址
  writedata(0x00);
  writedata(0x00);    //行结束地址
  writedata(0xE5);    // 240像素

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//   end_tft_write();
//   delay(120);
//   begin_tft_write();

//   writecommand(ST7789_DISPON);    //Display on
//   delay(12);

#ifdef TFT_BL
  // Turn on the back-light LED
  digitalWrite(TFT_BL, HIGH);
  pinMode(TFT_BL, OUTPUT);
#endif
}
