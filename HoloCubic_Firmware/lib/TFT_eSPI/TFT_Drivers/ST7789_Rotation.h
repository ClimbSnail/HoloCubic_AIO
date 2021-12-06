// This is the command sequence that rotates the ST7789 driver coordinate frame

writecommand(TFT_MADCTL);
uint8_t page_address_order = 0x00;
rotation = m & 0x0F;
switch (rotation)
{
case 0: // Portrait
#ifdef CGRAM_OFFSET
    if (_init_width == 135)
    {
        colstart = 52;
        rowstart = 40;
    }
    else
    {
        colstart = 0;
        rowstart = 0;
    }
#endif
    writedata(TFT_MAD_COLOR_ORDER);

    _width = _init_width;
    _height = _init_height;
    break;

case 1: // Landscape (Portrait + 90)
#ifdef CGRAM_OFFSET
    if (_init_width == 135)
    {
        colstart = 40;
        rowstart = 53;
    }
    else
    {
        colstart = 0;
        rowstart = 0;
    }
#endif
    writedata(TFT_MAD_MX | TFT_MAD_MV | TFT_MAD_COLOR_ORDER);

    _width = _init_height;
    _height = _init_width;
    break;

case 2: // Inverter portrait
#ifdef CGRAM_OFFSET
    if (_init_width == 135)
    {
        colstart = 53;
        rowstart = 40;
    }
    else
    {
        colstart = 0;
        rowstart = 80;
    }
#endif
    writedata(TFT_MAD_MX | TFT_MAD_MY | TFT_MAD_COLOR_ORDER);

    _width = _init_width;
    _height = _init_height;
    break;
case 3: // Inverted landscape
#ifdef CGRAM_OFFSET
    if (_init_width == 135)
    {
        colstart = 40;
        rowstart = 52;
    }
    else
    {
        colstart = 80;
        rowstart = 0;
    }
#endif
    writedata(TFT_MAD_MV | TFT_MAD_MY | TFT_MAD_COLOR_ORDER);

    _width = _init_height;
    _height = _init_width;
    break;
case 4: // Inverter portrait & Mirror Y
#ifdef CGRAM_OFFSET
    if (_init_width == 135)
    {
        colstart = 53;
        rowstart = 40;
    }
    else
    {
        colstart = 0;
        rowstart = 0;
    }
#endif
    writedata(TFT_MAD_MX | TFT_MAD_COLOR_ORDER);

    _width = _init_width;
    _height = _init_height;
    break;
case 5:
#ifdef CGRAM_OFFSET
    if (_init_width == 135)
    {
        colstart = 40;
        rowstart = 53;
    }
    else
    {
        colstart = 80;
        rowstart = 0;
    }
#endif
    // TFT_MAD_MY TFT_MAD_MX TFT_MAD_MV TFT_MAD_ML
    // writedata(TFT_MAD_MY | TFT_MAD_MX | TFT_MAD_MV | TFT_MAD_ML | TFT_MAD_COLOR_ORDER);
    // TFT_MAD_MX 列（水平）地址方向
    // TFT_MAD_MY 行（竖直）地址方向，然后会向下移半个屏幕
    // TFT_MAD_MV 列行（水平竖直）旋转90度且某一轴对调地址方向
    // TFT_MAD_ML 没反应，暂时不用
    writedata(TFT_MAD_MX | TFT_MAD_MY | TFT_MAD_MV | TFT_MAD_COLOR_ORDER);

    _width = _init_height;
    _height = _init_width;
    break;
}
