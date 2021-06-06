/*
 * start rewrite from:
 * https://github.com/adafruit/Adafruit-GFX-Library.git
 * 
 * Arc function come from:
 * https://github.com/lovyan03/LovyanGFX.git
 */
#include "Arduino_DataBus.h"
#include "Arduino_GFX.h"
#include "font/glcdfont.h"
#include "float.h"
#ifdef __AVR__
#include <avr/pgmspace.h>
#elif defined(ESP8266) || defined(ESP32)
#include <pgmspace.h>
#endif

/**************************************************************************/
/*!
    @brief  Instatiate a GFX context for graphics! Can only be done by a superclass
    @param  w   Display width, in pixels
    @param  h   Display height, in pixels
*/
/**************************************************************************/
#if defined(LITTLE_FOOT_PRINT)
Arduino_GFX::Arduino_GFX(int16_t w, int16_t h) : WIDTH(w), HEIGHT(h)
#else
Arduino_GFX::Arduino_GFX(int16_t w, int16_t h) : Arduino_G(w, h)
#endif // !defined(LITTLE_FOOT_PRINT)
{
    _width = WIDTH;
    _height = HEIGHT;
    _max_x = _width - 1;  ///< x zero base bound
    _max_y = _height - 1; ///< y zero base bound
    _rotation = 0;
    cursor_y = cursor_x = 0;
    textsize_x = textsize_y = 1;
    text_pixel_margin = 0;
    textcolor = textbgcolor = 0xFFFF;
    wrap = true;
    _cp437 = false;
#if !defined(ATTINY_CORE)
    gfxFont = NULL;
#endif // !defined(ATTINY_CORE)
}

/**************************************************************************/
/*!
    @brief  Write a line. Check straight or slash line and call corresponding function
    @param  x0      Start point x coordinate
    @param  y0      Start point y coordinate
    @param  x1      End point x coordinate
    @param  y1      End point y coordinate
    @param  color   16-bit 5-6-5 Color to draw with
*/
/**************************************************************************/
void Arduino_GFX::writeLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1,
                            uint16_t color)
{
    if (x0 == x1)
    {
        if (y0 > y1)
        {
            _swap_int16_t(y0, y1);
        }
        writeFastVLine(x0, y0, y1 - y0 + 1, color);
    }
    else if (y0 == y1)
    {
        if (x0 > x1)
        {
            _swap_int16_t(x0, x1);
        }
        writeFastHLine(x0, y0, x1 - x0 + 1, color);
    }
    else
    {
        writeSlashLine(x0, y0, x1, y1, color);
    }
}

/**************************************************************************/
/*!
    @brief  Write a line.  Bresenham's algorithm - thx wikpedia
    @param  x0      Start point x coordinate
    @param  y0      Start point y coordinate
    @param  x1      End point x coordinate
    @param  y1      End point y coordinate
    @param  color   16-bit 5-6-5 Color to draw with
*/
/**************************************************************************/
void Arduino_GFX::writeSlashLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1,
                                 uint16_t color)
{
    bool steep = _diff(y1, y0) > _diff(x1, x0);
    if (steep)
    {
        _swap_int16_t(x0, y0);
        _swap_int16_t(x1, y1);
    }

    if (x0 > x1)
    {
        _swap_int16_t(x0, x1);
        _swap_int16_t(y0, y1);
    }

    int16_t dx = x1 - x0;
    int16_t dy = _diff(y1, y0);
    int16_t err = dx >> 1;
    int16_t step = (y0 < y1) ? 1 : -1;

    for (; x0 <= x1; x0++)
    {
        if (steep)
        {
            writePixel(y0, x0, color);
        }
        else
        {
            writePixel(x0, y0, color);
        }
        err -= dy;
        if (err < 0)
        {
            err += dx;
            y0 += step;
        }
    }
}

/**************************************************************************/
/*!
    @brief  Start a display-writing routine, overwrite in subclasses.
*/
/**************************************************************************/
INLINE void Arduino_GFX::startWrite()
{
}

void Arduino_GFX::writePixel(int16_t x, int16_t y, uint16_t color)
{
    if (_ordered_in_range(x, 0, _max_x) && _ordered_in_range(y, 0, _max_y))
    {
        writePixelPreclipped(x, y, color);
    }
}

/**************************************************************************/
/*!
    @brief  Write a pixel, overwrite in subclasses if startWrite is defined!
    @param  x       x coordinate
    @param  y       y coordinate
    @param  color   16-bit 5-6-5 Color to fill with
*/
/**************************************************************************/
void Arduino_GFX::drawPixel(int16_t x, int16_t y, uint16_t color)
{
    startWrite();
    writePixel(x, y, color);
    endWrite();
}

/**************************************************************************/
/*!
    @brief  Write a perfectly vertical line, overwrite in subclasses if startWrite is defined!
    @param  x       Top-most x coordinate
    @param  y       Top-most y coordinate
    @param  h       Height in pixels
    @param  color   16-bit 5-6-5 Color to fill with
*/
/**************************************************************************/
void Arduino_GFX::writeFastVLine(int16_t x, int16_t y,
                                 int16_t h, uint16_t color)
{
    for (int16_t i = y; i < y + h; i++)
    {
        writePixel(x, i, color);
    }
}

/**************************************************************************/
/*!
    @brief  Write a perfectly horizontal line, overwrite in subclasses if startWrite is defined!
    @param  x       Left-most x coordinate
    @param  y       Left-most y coordinate
    @param  w       Width in pixels
    @param  color   16-bit 5-6-5 Color to fill with
*/
/**************************************************************************/
void Arduino_GFX::writeFastHLine(int16_t x, int16_t y,
                                 int16_t w, uint16_t color)
{
    for (int16_t i = x; i < x + w; i++)
    {
        writePixel(i, y, color);
    }
}

/**************************************************************************/
/*!
    @brief  Draw a filled rectangle to the display. Not self-contained;
            should follow startWrite(). Typically used by higher-level
            graphics primitives; user code shouldn't need to call this and
            is likely to use the self-contained fillRect() instead.
            writeFillRect() performs its own edge clipping and rejection;
            see writeFillRectPreclipped() for a more 'raw' implementation.
    @param  x       Horizontal position of first corner.
    @param  y       Vertical position of first corner.
    @param  w       Rectangle width in pixels (positive = right of first
                    corner, negative = left of first corner).
    @param  h       Rectangle height in pixels (positive = below first
                    corner, negative = above first corner).
    @param  color   16-bit fill color in '565' RGB format.
    @note   Written in this deep-nested way because C by definition will
            optimize for the 'if' case, not the 'else' -- avoids branches
            and rejects clipped rectangles at the least-work possibility.
*/
/**************************************************************************/
void Arduino_GFX::writeFillRect(int16_t x, int16_t y, int16_t w, int16_t h,
                                uint16_t color)
{
    if (w && h)
    { // Nonzero width and height?
        if (w < 0)
        {               // If negative width...
            x += w + 1; //   Move X to left edge
            w = -w;     //   Use positive width
        }
        if (x <= _max_x)
        { // Not off right
            if (h < 0)
            {               // If negative height...
                y += h + 1; //   Move Y to top edge
                h = -h;     //   Use positive height
            }
            if (y <= _max_y)
            { // Not off bottom
                int16_t x2 = x + w - 1;
                if (x2 >= 0)
                { // Not off left
                    int16_t y2 = y + h - 1;
                    if (y2 >= 0)
                    { // Not off top
                        // Rectangle partly or fully overlaps screen
                        if (x < 0)
                        {
                            x = 0;
                            w = x2 + 1;
                        } // Clip left
                        if (y < 0)
                        {
                            y = 0;
                            h = y2 + 1;
                        } // Clip top
                        if (x2 > _max_x)
                        {
                            w = _max_x - x + 1;
                        } // Clip right
                        if (y2 > _max_y)
                        {
                            h = _max_y - y + 1;
                        } // Clip bottom
                        writeFillRectPreclipped(x, y, w, h, color);
                    }
                }
            }
        }
    }
}

/**************************************************************************/
/*!
    @brief  Write a rectangle completely with one color, overwrite in subclasses if startWrite is defined!
    @param  x       Top left corner x coordinate
    @param  y       Top left corner y coordinate
    @param  w       Width in pixels
    @param  h       Height in pixels
    @param  color   16-bit 5-6-5 Color to fill with
*/
/**************************************************************************/
void Arduino_GFX::writeFillRectPreclipped(int16_t x, int16_t y, int16_t w, int16_t h,
                                          uint16_t color)
{
    // Overwrite in subclasses if desired!
    for (int16_t i = x; i < x + w; i++)
    {
        writeFastVLine(i, y, h, color);
    }
}

/**************************************************************************/
/*!
    @brief  End a display-writing routine, overwrite in subclasses if startWrite is defined!
*/
/**************************************************************************/
INLINE void Arduino_GFX::endWrite()
{
}

/**************************************************************************/
/*!
    @brief  Draw a perfectly vertical line (this is often optimized in a subclass!)
    @param  x       Top-most x coordinate
    @param  y       Top-most y coordinate
    @param  h       Height in pixels
    @param  color   16-bit 5-6-5 Color to fill with
*/
/**************************************************************************/
void Arduino_GFX::drawFastVLine(int16_t x, int16_t y,
                                int16_t h, uint16_t color)
{
    startWrite();
    writeFastVLine(x, y, h, color);
    endWrite();
}

/**************************************************************************/
/*!
    @brief  Draw a perfectly horizontal line (this is often optimized in a subclass!)
    @param  x       Left-most x coordinate
    @param  y       Left-most y coordinate
    @param  w       Width in pixels
    @param  color   16-bit 5-6-5 Color to fill with
*/
/**************************************************************************/
void Arduino_GFX::drawFastHLine(int16_t x, int16_t y,
                                int16_t w, uint16_t color)
{
    startWrite();
    writeFastHLine(x, y, w, color);
    endWrite();
}

/**************************************************************************/
/*!
    @brief  Fill a rectangle completely with one color. Update in subclasses if desired!
    @param  x       Top left corner x coordinate
    @param  y       Top left corner y coordinate
    @param  w       Width in pixels
    @param  h       Height in pixels
    @param  color   16-bit 5-6-5 Color to fill with
*/
/**************************************************************************/
void Arduino_GFX::fillRect(int16_t x, int16_t y, int16_t w, int16_t h,
                           uint16_t color)
{
    startWrite();
    writeFillRect(x, y, w, h, color);
    endWrite();
}

/**************************************************************************/
/*!
    @brief  Fill the screen completely with one color. Update in subclasses if desired!
    @param  color 16-bit 5-6-5 Color to fill with
*/
/**************************************************************************/
void Arduino_GFX::fillScreen(uint16_t color)
{
    fillRect(0, 0, _width, _height, color);
}

/**************************************************************************/
/*!
    @brief  Draw a line
    @param  x0      Start point x coordinate
    @param  y0      Start point y coordinate
    @param  x1      End point x coordinate
    @param  y1      End point y coordinate
    @param  color   16-bit 5-6-5 Color to draw with
*/
/**************************************************************************/
void Arduino_GFX::drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1,
                           uint16_t color)
{
    // Update in subclasses if desired!
    startWrite();
    writeLine(x0, y0, x1, y1, color);
    endWrite();
}

/**************************************************************************/
/*!
    @brief  Draw a circle outline
    @param  x       Center-point x coordinate
    @param  y       Center-point y coordinate
    @param  r       Radius of circle
    @param  color   16-bit 5-6-5 Color to draw with
*/
/**************************************************************************/
void Arduino_GFX::drawCircle(int16_t x, int16_t y,
                             int16_t r, uint16_t color)
{
    startWrite();
    drawEllipseHelper(x, y, r, r, 0xf, color);
    endWrite();
}

/**************************************************************************/
/*!
    @brief  Quarter-ellipse drawer, used to do circles and roundrects
    @param  x           Center-point x coordinate
    @param  y           Center-point y coordinate
    @param  rx          radius of x coordinate
    @param  ry          radius of y coordinate
    @param  cornername  Mask bit #1 or bit #2 to indicate which quarters of the circle we're doing
    @param  color       16-bit 5-6-5 Color to draw with
*/
/**************************************************************************/
void Arduino_GFX::drawEllipseHelper(int32_t x, int32_t y,
                                    int32_t rx, int32_t ry,
                                    uint8_t cornername, uint16_t color)
{
    if (rx < 0 || ry < 0 || ((rx == 0) && (ry == 0)))
    {
        return;
    }
    if (ry == 0)
    {
        drawFastHLine(x - rx, y, (ry << 2) + 1, color);
        return;
    }
    if (rx == 0)
    {
        drawFastVLine(x, y - ry, (rx << 2) + 1, color);
        return;
    }

    int32_t xt, yt, s, i;
    int32_t rx2 = rx * rx;
    int32_t ry2 = ry * ry;

    i = -1;
    xt = 0;
    yt = ry;
    s = (ry2 << 1) + rx2 * (1 - (ry << 1));
    do
    {
        while (s < 0)
            s += ry2 * ((++xt << 2) + 2);
        if (cornername & 0x1)
        {
            writeFastHLine(x - xt, y - yt, xt - i, color);
        }
        if (cornername & 0x2)
        {
            writeFastHLine(x + i + 1, y - yt, xt - i, color);
        }
        if (cornername & 0x4)
        {
            writeFastHLine(x + i + 1, y + yt, xt - i, color);
        }
        if (cornername & 0x8)
        {
            writeFastHLine(x - xt, y + yt, xt - i, color);
        }
        i = xt;
        s -= (--yt) * rx2 << 2;
    } while (ry2 * xt <= rx2 * yt);

    i = -1;
    yt = 0;
    xt = rx;
    s = (rx2 << 1) + ry2 * (1 - (rx << 1));
    do
    {
        while (s < 0)
            s += rx2 * ((++yt << 2) + 2);
        if (cornername & 0x1)
        {
            writeFastVLine(x - xt, y - yt, yt - i, color);
        }
        if (cornername & 0x2)
        {
            writeFastVLine(x + xt, y - yt, yt - i, color);
        }
        if (cornername & 0x4)
        {
            writeFastVLine(x + xt, y + i + 1, yt - i, color);
        }
        if (cornername & 0x8)
        {
            writeFastVLine(x - xt, y + i + 1, yt - i, color);
        }
        i = yt;
        s -= (--xt) * ry2 << 2;
    } while (rx2 * yt <= ry2 * xt);
}

/**************************************************************************/
/*!
    @brief  Draw a circle with filled color
    @param  x       Center-point x coordinate
    @param  y       Center-point y coordinate
    @param  r       Radius of circle
    @param  color   16-bit 5-6-5 Color to fill with
*/
/**************************************************************************/
void Arduino_GFX::fillCircle(int16_t x, int16_t y,
                             int16_t r, uint16_t color)
{
    startWrite();
    fillEllipseHelper(x, y, r, r, 3, 0, color);
    endWrite();
}

/**************************************************************************/
/*!
    @brief  Quarter-circle drawer with fill, used for circles and roundrects
    @param  x       Center-point x coordinate
    @param  y       Center-point y coordinate
    @param  rx      Radius of x coordinate
    @param  ry      Radius of y coordinate
    @param  corners Mask bits indicating which quarters we're doing
    @param  delta   Offset from center-point, used for round-rects
    @param  color   16-bit 5-6-5 Color to fill with
*/
/**************************************************************************/
void Arduino_GFX::fillEllipseHelper(int32_t x, int32_t y,
                                    int32_t rx, int32_t ry,
                                    uint8_t corners, int16_t delta, uint16_t color)
{
    if (rx < 0 || ry < 0 || ((rx == 0) && (ry == 0)))
    {
        return;
    }
    if (ry == 0)
    {
        drawFastHLine(x - rx, y, (ry << 2) + 1, color);
        return;
    }
    if (rx == 0)
    {
        drawFastVLine(x, y - ry, (rx << 2) + 1, color);
        return;
    }

    int32_t xt, yt, i;
    int32_t rx2 = (int32_t)rx * rx;
    int32_t ry2 = (int32_t)ry * ry;
    int32_t s;

    writeFastHLine(x - rx, y, (rx << 1) + 1, color);
    i = 0;
    yt = 0;
    xt = rx;
    s = (rx2 << 1) + ry2 * (1 - (rx << 1));
    do
    {
        while (s < 0)
        {
            s += rx2 * ((++yt << 2) + 2);
        }
        if (corners & 1)
        {
            writeFillRect(x - xt, y - yt, (xt << 1) + 1 + delta, yt - i, color);
        }
        if (corners & 2)
        {
            writeFillRect(x - xt, y + i + 1, (xt << 1) + 1 + delta, yt - i, color);
        }
        i = yt;
        s -= (--xt) * ry2 << 2;
    } while (rx2 * yt <= ry2 * xt);

    xt = 0;
    yt = ry;
    s = (ry2 << 1) + rx2 * (1 - (ry << 1));
    do
    {
        while (s < 0)
        {
            s += ry2 * ((++xt << 2) + 2);
        }
        if (corners & 1)
        {
            writeFastHLine(x - xt, y - yt, (xt << 1) + 1 + delta, color);
        }
        if (corners & 2)
        {
            writeFastHLine(x - xt, y + yt, (xt << 1) + 1 + delta, color);
        }
        s -= (--yt) * rx2 << 2;
    } while (ry2 * xt <= rx2 * yt);
}

/**************************************************************************/
/*!
    @brief  Draw an ellipse outline
    @param  x       Center-point x coordinate
    @param  y       Center-point y coordinate
    @param  rx      radius of x coordinate
    @param  ry      radius of y coordinate
    @param  start   degree of ellipse start
    @param  end     degree of ellipse end
    @param  color   16-bit 5-6-5 Color to draw with
*/
/**************************************************************************/
void Arduino_GFX::drawEllipse(int16_t x, int16_t y, int16_t rx, int16_t ry, uint16_t color)
{
    startWrite();
    drawEllipseHelper(x, y, rx, ry, 0xf, color);
    endWrite();
}

/**************************************************************************/
/*!
    @brief  Draw an ellipse with filled color
    @param  x       Center-point x coordinate
    @param  y       Center-point y coordinate
    @param  rx      radius of x coordinate
    @param  ry      radius of y coordinate
    @param  start   degree of ellipse start
    @param  end     degree of ellipse end
    @param  color   16-bit 5-6-5 Color to fill with
*/
/**************************************************************************/
void Arduino_GFX::fillEllipse(int16_t x, int16_t y, int16_t rx, int16_t ry, uint16_t color)
{
    startWrite();
    fillEllipseHelper(x, y, rx, ry, 3, 0, color);
    endWrite();
}

/**************************************************************************/
/*!
    @brief  Draw an arc outline
    @param  x       Center-point x coordinate
    @param  y       Center-point y coordinate
    @param  r1      Outer radius of arc
    @param  r2      Inner radius of arc
    @param  start   degree of arc start
    @param  end     degree of arc end
    @param  color   16-bit 5-6-5 Color to draw with
*/
/**************************************************************************/
void Arduino_GFX::drawArc(int16_t x, int16_t y, int16_t r1, int16_t r2, float start, float end, uint16_t color)
{
    if (r1 < r2)
    {
        _swap_int16_t(r1, r2);
    }
    if (r1 < 1)
    {
        r1 = 1;
    }
    if (r2 < 1)
    {
        r2 = 1;
    }
    bool equal = fabsf(start - end) < FLT_EPSILON;
    start = fmodf(start, 360);
    end = fmodf(end, 360);
    if (start < 0)
        start += 360.0;
    if (end < 0)
        end += 360.0;

    startWrite();
    fillArcHelper(x, y, r1, r2, start, start, color);
    fillArcHelper(x, y, r1, r2, end, end, color);
    if (!equal && (fabsf(start - end) <= 0.0001))
    {
        start = .0;
        end = 360.0;
    }
    fillArcHelper(x, y, r1, r1, start, end, color);
    fillArcHelper(x, y, r2, r2, start, end, color);
    endWrite();
}

/**************************************************************************/
/*!
    @brief  Draw an arc with filled color
    @param  x       Center-point x coordinate
    @param  y       Center-point y coordinate
    @param  r1      Outer radius of arc
    @param  r2      Inner radius of arc
    @param  start   degree of arc start
    @param  end     degree of arc end
    @param  color   16-bit 5-6-5 Color to fill with
*/
/**************************************************************************/
void Arduino_GFX::fillArc(int16_t x, int16_t y, int16_t r1, int16_t r2, float start, float end, uint16_t color)
{
    if (r1 < r2)
    {
        _swap_int16_t(r1, r2);
    }
    if (r1 < 1)
    {
        r1 = 1;
    }
    if (r2 < 1)
    {
        r2 = 1;
    }
    bool equal = fabsf(start - end) < FLT_EPSILON;
    start = fmodf(start, 360);
    end = fmodf(end, 360);
    if (start < 0)
        start += 360.0;
    if (end < 0)
        end += 360.0;
    if (!equal && (fabsf(start - end) <= 0.0001))
    {
        start = .0;
        end = 360.0;
    }

    startWrite();
    fillArcHelper(x, y, r1, r2, start, end, color);
    endWrite();
}

/**************************************************************************/
/*!
    @brief  Arc drawer with fill
    @param  cx      Center-point x coordinate
    @param  cy      Center-point y coordinate
    @param  oradius Outer radius of arc
    @param  iradius Inner radius of arc
    @param  start   degree of arc start
    @param  end     degree of arc end
    @param  color   16-bit 5-6-5 Color to fill with
*/
/**************************************************************************/
void Arduino_GFX::fillArcHelper(int16_t cx, int16_t cy, int16_t oradius, int16_t iradius, float start, float end, uint16_t color)
{
    float s_cos = (cos(start * DEGTORAD));
    float e_cos = (cos(end * DEGTORAD));
    float sslope = s_cos / (sin(start * DEGTORAD));
    float eslope = -1000000;
    if (end != 360.0)
    {
        eslope = e_cos / (sin(end * DEGTORAD));
    }
    float swidth = 0.5 / s_cos;
    float ewidth = -0.5 / e_cos;
    --iradius;
    int32_t ir2 = iradius * iradius + iradius;
    int32_t or2 = oradius * oradius + oradius;

    bool start180 = !(start < 180);
    bool end180 = end < 180;
    bool reversed = start + 180 < end || (end < start && start < end + 180);

    int32_t xs = -oradius;
    int32_t y = -oradius;
    int32_t ye = oradius;
    int32_t xe = oradius + 1;
    if (!reversed)
    {
        if ((end >= 270 || end < 90) && (start >= 270 || start < 90))
        {
            xs = 0;
        }
        else if (end < 270 && end >= 90 && start < 270 && start >= 90)
        {
            xe = 1;
        }
        if (end >= 180 && start >= 180)
        {
            ye = 0;
        }
        else if (end < 180 && start < 180)
        {
            y = 0;
        }
    }
    do
    {
        int32_t y2 = y * y;
        int32_t x = xs;
        if (x < 0)
        {
            while (x * x + y2 >= or2)
            {
                ++x;
            }
            if (xe != 1)
            {
                xe = 1 - x;
            }
        }
        float ysslope = (y + swidth) * sslope;
        float yeslope = (y + ewidth) * eslope;
        int32_t len = 0;
        do
        {
            bool flg1 = start180 != (x <= ysslope);
            bool flg2 = end180 != (x <= yeslope);
            int32_t distance = x * x + y2;
            if (distance >= ir2 && ((flg1 && flg2) || (reversed && (flg1 || flg2))) && x != xe && distance < or2)
            {
                ++len;
            }
            else
            {
                if (len)
                {
                    writeFastHLine(cx + x - len, cy + y, len, color);
                    len = 0;
                }
                if (distance >= or2)
                    break;
                if (x < 0 && distance < ir2)
                {
                    x = -x;
                }
            }
        } while (++x <= xe);
    } while (++y <= ye);
}

/**************************************************************************/
/*!
    @brief  Draw a rectangle with no fill color
    @param  x       Top left corner x coordinate
    @param  y       Top left corner y coordinate
    @param  w       Width in pixels
    @param  h       Height in pixels
    @param  color   16-bit 5-6-5 Color to draw with
*/
/**************************************************************************/
void Arduino_GFX::drawRect(int16_t x, int16_t y, int16_t w, int16_t h,
                           uint16_t color)
{
    startWrite();
    writeFastHLine(x, y, w, color);
    writeFastHLine(x, y + h - 1, w, color);
    writeFastVLine(x, y, h, color);
    writeFastVLine(x + w - 1, y, h, color);
    endWrite();
}

/**************************************************************************/
/*!
    @brief  Draw a rounded rectangle with no fill color
    @param  x       Top left corner x coordinate
    @param  y       Top left corner y coordinate
    @param  w       Width in pixels
    @param  h       Height in pixels
    @param  r       Radius of corner rounding
    @param  color   16-bit 5-6-5 Color to draw with
*/
/**************************************************************************/
void Arduino_GFX::drawRoundRect(int16_t x, int16_t y, int16_t w,
                                int16_t h, int16_t r, uint16_t color)
{
    int16_t max_radius = ((w < h) ? w : h) / 2; // 1/2 minor axis
    if (r > max_radius)
        r = max_radius;
    // smarter version
    startWrite();
    writeFastHLine(x + r, y, w - 2 * r, color);         // Top
    writeFastHLine(x + r, y + h - 1, w - 2 * r, color); // Bottom
    writeFastVLine(x, y + r, h - 2 * r, color);         // Left
    writeFastVLine(x + w - 1, y + r, h - 2 * r, color); // Right
    // draw four corners
    drawEllipseHelper(x + r, y + r, r, r, 1, color);
    drawEllipseHelper(x + w - r - 1, y + r, r, r, 2, color);
    drawEllipseHelper(x + w - r - 1, y + h - r - 1, r, r, 4, color);
    drawEllipseHelper(x + r, y + h - r - 1, r, r, 8, color);
    endWrite();
}

/**************************************************************************/
/*!
    @brief  Draw a rounded rectangle with fill color
    @param  x       Top left corner x coordinate
    @param  y       Top left corner y coordinate
    @param  w       Width in pixels
    @param  h       Height in pixels
    @param  r       Radius of corner rounding
    @param  color   16-bit 5-6-5 Color to draw/fill with
*/
/**************************************************************************/
void Arduino_GFX::fillRoundRect(int16_t x, int16_t y, int16_t w,
                                int16_t h, int16_t r, uint16_t color)
{
    int16_t max_radius = ((w < h) ? w : h) / 2; // 1/2 minor axis
    if (r > max_radius)
        r = max_radius;
    // smarter version
    startWrite();
    writeFillRect(x, y + r, w, h - (r << 1), color);
    // draw four corners
    fillEllipseHelper(x + r, y + r, r, r, 1, w - 2 * r - 1, color);
    fillEllipseHelper(x + r, y + h - r - 1, r, r, 2, w - 2 * r - 1, color);
    endWrite();
}

/**************************************************************************/
/*!
    @brief  Draw a triangle with no fill color
    @param  x0      Vertex #0 x coordinate
    @param  y0      Vertex #0 y coordinate
    @param  x1      Vertex #1 x coordinate
    @param  y1      Vertex #1 y coordinate
    @param  x2      Vertex #2 x coordinate
    @param  y2      Vertex #2 y coordinate
    @param  color   16-bit 5-6-5 Color to draw with
*/
/**************************************************************************/
void Arduino_GFX::drawTriangle(int16_t x0, int16_t y0,
                               int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color)
{
    startWrite();
    writeLine(x0, y0, x1, y1, color);
    writeLine(x1, y1, x2, y2, color);
    writeLine(x2, y2, x0, y0, color);
    endWrite();
}

/**************************************************************************/
/*!
    @brief  Draw a triangle with color-fill
    @param  x0      Vertex #0 x coordinate
    @param  y0      Vertex #0 y coordinate
    @param  x1      Vertex #1 x coordinate
    @param  y1      Vertex #1 y coordinate
    @param  x2      Vertex #2 x coordinate
    @param  y2      Vertex #2 y coordinate
    @param  color   16-bit 5-6-5 Color to fill/draw with
*/
/**************************************************************************/
void Arduino_GFX::fillTriangle(int16_t x0, int16_t y0,
                               int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color)
{
    int16_t a, b, y, last;

    // Sort coordinates by Y order (y2 >= y1 >= y0)
    if (y0 > y1)
    {
        _swap_int16_t(y0, y1);
        _swap_int16_t(x0, x1);
    }
    if (y1 > y2)
    {
        _swap_int16_t(y2, y1);
        _swap_int16_t(x2, x1);
    }
    if (y0 > y1)
    {
        _swap_int16_t(y0, y1);
        _swap_int16_t(x0, x1);
    }

    startWrite();
    if (y0 == y2)
    { // Handle awkward all-on-same-line case as its own thing
        a = b = x0;
        if (x1 < a)
            a = x1;
        else if (x1 > b)
            b = x1;
        if (x2 < a)
            a = x2;
        else if (x2 > b)
            b = x2;
        writeFastHLine(a, y0, b - a + 1, color);
        endWrite();
        return;
    }

    int16_t
        dx01 = x1 - x0,
        dy01 = y1 - y0,
        dx02 = x2 - x0,
        dy02 = y2 - y0,
        dx12 = x2 - x1,
        dy12 = y2 - y1;
    int32_t
        sa = 0,
        sb = 0;

    // For upper part of triangle, find scanline crossings for segments
    // 0-1 and 0-2.  If y1=y2 (flat-bottomed triangle), the scanline y1
    // is included here (and second loop will be skipped, avoiding a /0
    // error there), otherwise scanline y1 is skipped here and handled
    // in the second loop...which also avoids a /0 error here if y0=y1
    // (flat-topped triangle).
    if (y1 == y2)
    {
        last = y1; // Include y1 scanline
    }
    else
    {
        last = y1 - 1; // Skip it
    }

    for (y = y0; y <= last; y++)
    {
        a = x0 + sa / dy01;
        b = x0 + sb / dy02;
        sa += dx01;
        sb += dx02;
        /* longhand:
        a = x0 + (x1 - x0) * (y - y0) / (y1 - y0);
        b = x0 + (x2 - x0) * (y - y0) / (y2 - y0);
        */
        if (a > b)
        {
            _swap_int16_t(a, b);
        }
        writeFastHLine(a, y, b - a + 1, color);
    }

    // For lower part of triangle, find scanline crossings for segments
    // 0-2 and 1-2.  This loop is skipped if y1=y2.
    sa = (int32_t)dx12 * (y - y1);
    sb = (int32_t)dx02 * (y - y0);
    for (; y <= y2; y++)
    {
        a = x1 + sa / dy12;
        b = x0 + sb / dy02;
        sa += dx12;
        sb += dx02;
        /* longhand:
        a = x1 + (x2 - x1) * (y - y1) / (y2 - y1);
        b = x0 + (x2 - x0) * (y - y0) / (y2 - y0);
        */
        if (a > b)
        {
            _swap_int16_t(a, b);
        }
        writeFastHLine(a, y, b - a + 1, color);
    }
    endWrite();
}

// BITMAP / XBITMAP / GRAYSCALE / RGB BITMAP FUNCTIONS ---------------------

/**************************************************************************/
/*!
    @brief  Draw a PROGMEM-resident 1-bit image at the specified (x,y) position, using the specified foreground color (unset bits are transparent).
    @param  x       Top left corner x coordinate
    @param  y       Top left corner y coordinate
    @param  bitmap  byte array with monochrome bitmap
    @param  w       Width of bitmap in pixels
    @param  h       Height of bitmap in pixels
    @param  color   16-bit 5-6-5 Color to draw with
*/
/**************************************************************************/
void Arduino_GFX::drawBitmap(int16_t x, int16_t y,
                             const uint8_t bitmap[], int16_t w, int16_t h, uint16_t color)
{
    int16_t byteWidth = (w + 7) / 8; // Bitmap scanline pad = whole byte
    uint8_t byte = 0;

    startWrite();
    for (int16_t j = 0; j < h; j++, y++)
    {
        for (int16_t i = 0; i < w; i++)
        {
            if (i & 7)
            {
                byte <<= 1;
            }
            else
            {
                byte = pgm_read_byte(&bitmap[j * byteWidth + i / 8]);
            }
            if (byte & 0x80)
            {
                writePixel(x + i, y, color);
            }
        }
    }
    endWrite();
}

/**************************************************************************/
/*!
    @brief  Draw a PROGMEM-resident 1-bit image at the specified (x,y) position, using the specified foreground (for set bits) and background (unset bits) colors.
    @param  x       Top left corner x coordinate
    @param  y       Top left corner y coordinate
    @param  bitmap  byte array with monochrome bitmap
    @param  w       Width of bitmap in pixels
    @param  h       Height of bitmap in pixels
    @param  color   16-bit 5-6-5 Color to draw pixels with
    @param  bg      16-bit 5-6-5 Color to draw background with
*/
/**************************************************************************/
void Arduino_GFX::drawBitmap(int16_t x, int16_t y,
                             const uint8_t bitmap[], int16_t w, int16_t h,
                             uint16_t color, uint16_t bg)
{
    int16_t byteWidth = (w + 7) / 8; // Bitmap scanline pad = whole byte
    uint8_t byte = 0;

    startWrite();
    for (int16_t j = 0; j < h; j++, y++)
    {
        for (int16_t i = 0; i < w; i++)
        {
            if (i & 7)
            {
                byte <<= 1;
            }
            else
            {
                byte = pgm_read_byte(&bitmap[j * byteWidth + i / 8]);
            }
            writePixel(x + i, y, (byte & 0x80) ? color : bg);
        }
    }
    endWrite();
}

/**************************************************************************/
/*!
    @brief  Draw a RAM-resident 1-bit image at the specified (x,y) position, using the specified foreground color (unset bits are transparent).
    @param  x       Top left corner x coordinate
    @param  y       Top left corner y coordinate
    @param  bitmap  byte array with monochrome bitmap
    @param  w       Width of bitmap in pixels
    @param  h       Height of bitmap in pixels
    @param  color   16-bit 5-6-5 Color to draw with
*/
/**************************************************************************/
void Arduino_GFX::drawBitmap(int16_t x, int16_t y,
                             uint8_t *bitmap, int16_t w, int16_t h, uint16_t color)
{
    int16_t byteWidth = (w + 7) / 8; // Bitmap scanline pad = whole byte
    uint8_t byte = 0;

    startWrite();
    for (int16_t j = 0; j < h; j++, y++)
    {
        for (int16_t i = 0; i < w; i++)
        {
            if (i & 7)
            {
                byte <<= 1;
            }
            else
            {
                byte = bitmap[j * byteWidth + i / 8];
            }
            if (byte & 0x80)
            {
                writePixel(x + i, y, color);
            }
        }
    }
    endWrite();
}

/**************************************************************************/
/*!
    @brief  Draw a RAM-resident 1-bit image at the specified (x,y) position, using the specified foreground (for set bits) and background (unset bits) colors.
    @param  x       Top left corner x coordinate
    @param  y       Top left corner y coordinate
    @param  bitmap  byte array with monochrome bitmap
    @param  w       Width of bitmap in pixels
    @param  h       Height of bitmap in pixels
    @param  color   16-bit 5-6-5 Color to draw pixels with
    @param  bg      16-bit 5-6-5 Color to draw background with
*/
/**************************************************************************/
void Arduino_GFX::drawBitmap(int16_t x, int16_t y,
                             uint8_t *bitmap, int16_t w, int16_t h, uint16_t color, uint16_t bg)
{
    int16_t byteWidth = (w + 7) / 8; // Bitmap scanline pad = whole byte
    uint8_t byte = 0;

    startWrite();
    for (int16_t j = 0; j < h; j++, y++)
    {
        for (int16_t i = 0; i < w; i++)
        {
            if (i & 7)
            {
                byte <<= 1;
            }
            else
            {
                byte = bitmap[j * byteWidth + i / 8];
            }
            writePixel(x + i, y, (byte & 0x80) ? color : bg);
        }
    }
    endWrite();
}

/**************************************************************************/
/*!
   @brief   Draw PROGMEM-resident XBitMap Files (*.xbm), exported from GIMP.
            Usage: Export from GIMP to *.xbm, rename *.xbm to *.c and open in editor.
            C Array can be directly used with this function.
            There is no RAM-resident version of this function; if generating bitmaps
            in RAM, use the format defined by drawBitmap() and call that instead.
    @param  x       Top left corner x coordinate
    @param  y       Top left corner y coordinate
    @param  bitmap  byte array with monochrome bitmap
    @param  w       Width of bitmap in pixels
    @param  h       Height of bitmap in pixels
    @param  color   16-bit 5-6-5 Color to draw pixels with
*/
/**************************************************************************/
void Arduino_GFX::drawXBitmap(int16_t x, int16_t y,
                              const uint8_t bitmap[], int16_t w, int16_t h, uint16_t color)
{
    int16_t byteWidth = (w + 7) / 8; // Bitmap scanline pad = whole byte
    uint8_t byte = 0;

    startWrite();
    for (int16_t j = 0; j < h; j++, y++)
    {
        for (int16_t i = 0; i < w; i++)
        {
            if (i & 7)
            {
                byte >>= 1;
            }
            else
            {
                byte = pgm_read_byte(&bitmap[j * byteWidth + i / 8]);
            }
            // Nearly identical to drawBitmap(), only the bit order
            // is reversed here (left-to-right = LSB to MSB):
            if (byte & 0x01)
            {
                writePixel(x + i, y, color);
            }
        }
    }
    endWrite();
}

/**************************************************************************/
/*!
    @brief  Draw a PROGMEM-resident 8-bit image (grayscale) at the specified (x,y) pos.
    @param  x       Top left corner x coordinate
    @param  y       Top left corner y coordinate
    @param  bitmap  byte array with grayscale bitmap
    @param  w       Width of bitmap in pixels
    @param  h       Height of bitmap in pixels
*/
/**************************************************************************/
void Arduino_GFX::drawGrayscaleBitmap(int16_t x, int16_t y,
                                      const uint8_t bitmap[], int16_t w, int16_t h)
{
    uint8_t v;
    startWrite();
    for (int16_t j = 0; j < h; j++, y++)
    {
        for (int16_t i = 0; i < w; i++)
        {
            v = (uint8_t)pgm_read_byte(&bitmap[j * w + i]);
            writePixel(x + i, y, color565(v, v, v));
        }
    }
    endWrite();
}

/**************************************************************************/
/*!
    @brief  Draw a RAM-resident 8-bit image (grayscale) at the specified (x,y) pos.
    @param  x       Top left corner x coordinate
    @param  y       Top left corner y coordinate
    @param  bitmap  byte array with grayscale bitmap
    @param  w       Width of bitmap in pixels
    @param  h       Height of bitmap in pixels
*/
/**************************************************************************/
void Arduino_GFX::drawGrayscaleBitmap(int16_t x, int16_t y,
                                      uint8_t *bitmap, int16_t w, int16_t h)
{
    uint8_t v;
    startWrite();
    for (int16_t j = 0; j < h; j++, y++)
    {
        for (int16_t i = 0; i < w; i++)
        {
            v = bitmap[j * w + i];
            writePixel(x + i, y, color565(v, v, v));
        }
    }
    endWrite();
}

/**************************************************************************/
/*!
    @brief  Draw a PROGMEM-resident 8-bit image (grayscale) with a 1-bit mask
            (set bits = opaque, unset bits = clear) at the specified (x,y) position.
            BOTH buffers (grayscale and mask) must be PROGMEM-resident.
    @param  x       Top left corner x coordinate
    @param  y       Top left corner y coordinate
    @param  bitmap  byte array with grayscale bitmap
    @param  mask    byte array with mask bitmap
    @param  w       Width of bitmap in pixels
    @param  h       Height of bitmap in pixels
*/
/**************************************************************************/
void Arduino_GFX::drawGrayscaleBitmap(int16_t x, int16_t y,
                                      const uint8_t bitmap[], const uint8_t mask[],
                                      int16_t w, int16_t h)
{
    int16_t bw = (w + 7) / 8; // Bitmask scanline pad = whole byte
    uint8_t byte = 0;
    uint8_t v;
    startWrite();
    for (int16_t j = 0; j < h; j++, y++)
    {
        for (int16_t i = 0; i < w; i++)
        {
            if (i & 7)
            {
                byte <<= 1;
            }
            else
            {
                byte = pgm_read_byte(&mask[j * bw + i / 8]);
            }
            if (byte & 0x80)
            {
                v = (uint8_t)pgm_read_byte(&bitmap[j * w + i]);
                writePixel(x + i, y, color565(v, v, v));
            }
        }
    }
    endWrite();
}

/**************************************************************************/
/*!
    @brief  Draw a RAM-resident 8-bit image (grayscale) with a 1-bit mask
            (set bits = opaque, unset bits = clear) at the specified (x,y) position.
            BOTH buffers (grayscale and mask) must be RAM-residentt, no mix-and-match
    @param  x       Top left corner x coordinate
    @param  y       Top left corner y coordinate
    @param  bitmap  byte array with grayscale bitmap
    @param  mask    byte array with mask bitmap
    @param  w       Width of bitmap in pixels
    @param  h       Height of bitmap in pixels
*/
/**************************************************************************/
void Arduino_GFX::drawGrayscaleBitmap(int16_t x, int16_t y,
                                      uint8_t *bitmap, uint8_t *mask, int16_t w, int16_t h)
{
    int16_t bw = (w + 7) / 8; // Bitmask scanline pad = whole byte
    uint8_t byte = 0;
    uint8_t v;
    startWrite();
    for (int16_t j = 0; j < h; j++, y++)
    {
        for (int16_t i = 0; i < w; i++)
        {
            if (i & 7)
            {
                byte <<= 1;
            }
            else
            {
                byte = mask[j * bw + i / 8];
            }
            if (byte & 0x80)
            {
                v = bitmap[j * w + i];
                writePixel(x + i, y, color565(v, v, v));
            }
        }
    }
    endWrite();
}

/**************************************************************************/
/*!
    @brief  Draw a Indexed 16-bit image (RGB 5/6/5) at the specified (x,y) position.
    @param  x           Top left corner x coordinate
    @param  y           Top left corner y coordinate
    @param  bitmap      byte array of Indexed color bitmap
    @param  color_index byte array of 16-bit color index
    @param  w           Width of bitmap in pixels
    @param  h           Height of bitmap in pixels
*/
/**************************************************************************/
void Arduino_GFX::drawIndexedBitmap(int16_t x, int16_t y,
                                    uint8_t *bitmap, uint16_t *color_index, int16_t w, int16_t h)
{
    int32_t offset = 0;
    startWrite();
    for (int16_t j = 0; j < h; j++, y++)
    {
        for (int16_t i = 0; i < w; i++)
        {
            writePixel(x + i, y, color_index[bitmap[offset++]]);
        }
    }
    endWrite();
}

/**************************************************************************/
/*!
    @brief  Draw a RAM-resident 3-bit image (RGB 1/1/1) at the specified (x,y) position.
    @param  x       Top left corner x coordinate
    @param  y       Top left corner y coordinate
    @param  bitmap  byte array with 3-bit color bitmap
    @param  w       Width of bitmap in pixels
    @param  h       Height of bitmap in pixels
*/
/**************************************************************************/
void Arduino_GFX::draw3bitRGBBitmap(int16_t x, int16_t y,
                                    uint8_t *bitmap, int16_t w, int16_t h)
{
    int32_t offset = 0, idx = 0;
    uint8_t c = 0;
    uint16_t d;
    startWrite();
    for (int16_t j = 0; j < h; j++, y++)
    {
        for (int16_t i = 0; i < w; i++)
        {
            if (offset & 1)
            {
                d = (((c & 0b100) ? RED : 0) |
                     ((c & 0b010) ? GREEN : 0) |
                     ((c & 0b001) ? BLUE : 0));
            }
            else
            {
                c = bitmap[idx++];
                d = (((c & 0b100000) ? RED : 0) |
                     ((c & 0b010000) ? GREEN : 0) |
                     ((c & 0b001000) ? BLUE : 0));
            }
            writePixel(x + i, y, d);
            offset++;
        }
    }
    endWrite();
}

/**************************************************************************/
/*!
    @brief  Draw a PROGMEM-resident 16-bit image (RGB 5/6/5) at the specified (x,y) position.
    @param  x       Top left corner x coordinate
    @param  y       Top left corner y coordinate
    @param  bitmap  byte array with 16-bit color bitmap
    @param  w       Width of bitmap in pixels
    @param  h       Height of bitmap in pixels
*/
/**************************************************************************/
void Arduino_GFX::draw16bitRGBBitmap(int16_t x, int16_t y,
                                     const uint16_t bitmap[], int16_t w, int16_t h)
{
    int32_t offset = 0;
    startWrite();
    for (int16_t j = 0; j < h; j++, y++)
    {
        for (int16_t i = 0; i < w; i++)
        {
            writePixel(x + i, y, pgm_read_word(&bitmap[offset++]));
        }
    }
    endWrite();
}

/**************************************************************************/
/*!
    @brief  Draw a RAM-resident 16-bit image (RGB 5/6/5) at the specified (x,y) position.
    @param  x       Top left corner x coordinate
    @param  y       Top left corner y coordinate
    @param  bitmap  byte array with 16-bit color bitmap
    @param  w       Width of bitmap in pixels
    @param  h       Height of bitmap in pixels
*/
/**************************************************************************/
void Arduino_GFX::draw16bitRGBBitmap(int16_t x, int16_t y,
                                     uint16_t *bitmap, int16_t w, int16_t h)
{
    int32_t offset = 0;
    startWrite();
    for (int16_t j = 0; j < h; j++, y++)
    {
        for (int16_t i = 0; i < w; i++)
        {
            writePixel(x + i, y, bitmap[offset++]);
        }
    }
    endWrite();
}

/**************************************************************************/
/*!
    @brief  Draw a RAM-resident 16-bit Big Endian image (RGB 5/6/5) at the specified (x,y) position.
    @param  x       Top left corner x coordinate
    @param  y       Top left corner y coordinate
    @param  bitmap  byte array with 16-bit color bitmap
    @param  w       Width of bitmap in pixels
    @param  h       Height of bitmap in pixels
*/
/**************************************************************************/
void Arduino_GFX::draw16bitBeRGBBitmap(int16_t x, int16_t y,
                                       uint16_t *bitmap, int16_t w, int16_t h)
{
    int32_t offset = 0;
    uint16_t p;
    startWrite();
    for (int16_t j = 0; j < h; j++, y++)
    {
        for (int16_t i = 0; i < w; i++)
        {
            p = bitmap[offset++];
            MSB_16_SET(p, p);
            writePixel(x + i, y, p);
        }
    }
    endWrite();
}

/**************************************************************************/
/*!
    @brief  Draw a PROGMEM-resident 16-bit image (RGB 5/6/5) with a 1-bit mask
            (set bits = opaque, unset bits = clear) at the specified (x,y) position.
            BOTH buffers (color and mask) must be PROGMEM-resident.
    @param  x       Top left corner x coordinate
    @param  y       Top left corner y coordinate
    @param  bitmap  byte array with 16-bit color bitmap
    @param  mask    byte array with monochrome mask bitmap
    @param  w       Width of bitmap in pixels
    @param  h       Height of bitmap in pixels
*/
/**************************************************************************/
void Arduino_GFX::draw16bitRGBBitmap(int16_t x, int16_t y,
                                     const uint16_t bitmap[], const uint8_t mask[],
                                     int16_t w, int16_t h)
{
    int32_t offset = 0;
    int16_t bw = (w + 7) / 8; // Bitmask scanline pad = whole byte
    uint8_t byte = 0;
    startWrite();
    for (int16_t j = 0; j < h; j++, y++)
    {
        for (int16_t i = 0; i < w; i++)
        {
            if (i & 7)
            {
                byte <<= 1;
            }
            else
            {
                byte = pgm_read_byte(&mask[j * bw + i / 8]);
            }
            if (byte & 0x80)
            {
                writePixel(x + i, y, pgm_read_word(&bitmap[offset]));
            }
            offset++;
        }
    }
    endWrite();
}

/**************************************************************************/
/*!
    @brief  Draw a RAM-resident 16-bit image (RGB 5/6/5) with a 1-bit mask
            (set bits = opaque, unset bits = clear) at the specified (x,y) position.
            BOTH buffers (color and mask) must be RAM-resident.
    @param  x       Top left corner x coordinate
    @param  y       Top left corner y coordinate
    @param  bitmap  byte array with 16-bit color bitmap
    @param  mask    byte array with monochrome mask bitmap
    @param  w       Width of bitmap in pixels
    @param  h       Height of bitmap in pixels
*/
/**************************************************************************/
void Arduino_GFX::draw16bitRGBBitmap(int16_t x, int16_t y,
                                     uint16_t *bitmap, uint8_t *mask, int16_t w, int16_t h)
{
    int32_t offset = 0, maskIdx = 0;
    uint8_t byte = 0;
    startWrite();
    for (int16_t j = 0; j < h; j++, y++)
    {
        for (int16_t i = 0; i < w; i++)
        {
            if (i & 7)
            {
                byte <<= 1;
            }
            else
            {
                byte = mask[maskIdx++];
            }
            if (byte & 0x80)
            {
                writePixel(x + i, y, bitmap[offset]);
            }
            offset++;
        }
    }
    endWrite();
}

/**************************************************************************/
/*!
    @brief  Draw a PROGMEM-resident 24-bit image (RGB 5/6/5) at the specified (x,y) position.
    @param  x       Top left corner x coordinate
    @param  y       Top left corner y coordinate
    @param  bitmap  byte array with 16-bit color bitmap
    @param  w       Width of bitmap in pixels
    @param  h       Height of bitmap in pixels
*/
/**************************************************************************/
void Arduino_GFX::draw24bitRGBBitmap(int16_t x, int16_t y,
                                     const uint8_t bitmap[], int16_t w, int16_t h)
{
    int32_t offset = 0;
    startWrite();
    for (int16_t j = 0; j < h; j++, y++)
    {
        for (int16_t i = 0; i < w; i++)
        {
            writePixel(x + i, y, color565(pgm_read_byte(&bitmap[offset]), pgm_read_byte(&bitmap[offset + 1]), pgm_read_byte(&bitmap[offset + 2])));
            offset += 3;
        }
    }
    endWrite();
}

/**************************************************************************/
/*!
    @brief  Draw a RAM-resident 24-bit image (RGB 5/6/5) at the specified (x,y) position.
    @param  x       Top left corner x coordinate
    @param  y       Top left corner y coordinate
    @param  bitmap  byte array with 16-bit color bitmap
    @param  w       Width of bitmap in pixels
    @param  h       Height of bitmap in pixels
*/
/**************************************************************************/
void Arduino_GFX::draw24bitRGBBitmap(int16_t x, int16_t y,
                                     uint8_t *bitmap, int16_t w, int16_t h)
{
    int32_t offset = 0;
    startWrite();
    for (int16_t j = 0; j < h; j++, y++)
    {
        for (int16_t i = 0; i < w; i++)
        {
            writePixel(x + i, y, color565(bitmap[offset], bitmap[offset + 1], bitmap[offset + 2]));
            offset += 3;
        }
    }
    endWrite();
}

/**************************************************************************/
/*!
    @brief  Draw a PROGMEM-resident 24-bit image (RGB 5/6/5) with a 1-bit mask
            (set bits = opaque, unset bits = clear) at the specified (x,y) position.
            BOTH buffers (color and mask) must be PROGMEM-resident.
    @param  x       Top left corner x coordinate
    @param  y       Top left corner y coordinate
    @param  bitmap  byte array with 16-bit color bitmap
    @param  mask    byte array with monochrome mask bitmap
    @param  w       Width of bitmap in pixels
    @param  h       Height of bitmap in pixels
*/
/**************************************************************************/
void Arduino_GFX::draw24bitRGBBitmap(int16_t x, int16_t y,
                                     const uint8_t bitmap[], const uint8_t mask[],
                                     int16_t w, int16_t h)
{
    int32_t offset = 0;
    int16_t bw = (w + 7) / 8; // Bitmask scanline pad = whole byte
    uint8_t byte = 0;
    startWrite();
    for (int16_t j = 0; j < h; j++, y++)
    {
        for (int16_t i = 0; i < w; i++)
        {
            if (i & 7)
            {
                byte <<= 1;
            }
            else
            {
                byte = pgm_read_byte(&mask[j * bw + i / 8]);
            }
            if (byte & 0x80)
            {
                writePixel(x + i, y, color565(pgm_read_byte(&bitmap[offset]), pgm_read_byte(&bitmap[offset + 1]), pgm_read_byte(&bitmap[offset + 2])));
            }
            offset += 3;
        }
    }
    endWrite();
}

/**************************************************************************/
/*!
    @brief  Draw a RAM-resident 24-bit image (RGB 5/6/5) with a 1-bit mask
            (set bits = opaque, unset bits = clear) at the specified (x,y) position.
            BOTH buffers (color and mask) must be RAM-resident.
    @param  x       Top left corner x coordinate
    @param  y       Top left corner y coordinate
    @param  bitmap  byte array with 16-bit color bitmap
    @param  mask    byte array with monochrome mask bitmap
    @param  w       Width of bitmap in pixels
    @param  h       Height of bitmap in pixels
*/
/**************************************************************************/
void Arduino_GFX::draw24bitRGBBitmap(int16_t x, int16_t y,
                                     uint8_t *bitmap, uint8_t *mask, int16_t w, int16_t h)
{
    int32_t offset = 0;
    int16_t bw = (w + 7) / 8; // Bitmask scanline pad = whole byte
    uint8_t byte = 0;
    startWrite();
    for (int16_t j = 0; j < h; j++, y++)
    {
        for (int16_t i = 0; i < w; i++)
        {
            if (i & 7)
            {
                byte <<= 1;
            }
            else
            {
                byte = mask[j * bw + i / 8];
            }
            if (byte & 0x80)
            {
                writePixel(x + i, y, color565(bitmap[offset], bitmap[offset + 1], bitmap[offset + 2]));
            }
            offset += 3;
        }
    }
    endWrite();
}

// TEXT- AND CHARACTER-HANDLING FUNCTIONS ----------------------------------

// Draw a character
/**************************************************************************/
/*!
    @brief  Draw a single character
    @param  x       Bottom left corner x coordinate
    @param  y       Bottom left corner y coordinate
    @param  c       The 8-bit font-indexed character (likely ascii)
    @param  color   16-bit 5-6-5 Color to draw chraracter with
    @param  bg      16-bit 5-6-5 Color to fill background with (if same as color, no background)
*/
/**************************************************************************/
void Arduino_GFX::drawChar(int16_t x, int16_t y, unsigned char c,
                           uint16_t color, uint16_t bg)
{
    int16_t block_w;
    int16_t block_h;

#if !defined(ATTINY_CORE)
    if (gfxFont) // custom font
    {
        // Character is assumed previously filtered by write() to eliminate
        // newlines, returns, non-printable characters, etc.  Calling
        // drawChar() directly with 'bad' characters of font may cause mayhem!

        c -= (uint8_t)pgm_read_byte(&gfxFont->first);
        GFXglyph *glyph = pgm_read_glyph_ptr(gfxFont, c);
        uint8_t *bitmap = pgm_read_bitmap_ptr(gfxFont);

        uint16_t bo = pgm_read_word(&glyph->bitmapOffset);
        uint8_t w = pgm_read_byte(&glyph->width),
                h = pgm_read_byte(&glyph->height),
                xAdvance = pgm_read_byte(&glyph->xAdvance),
                yAdvance = pgm_read_byte(&gfxFont->yAdvance),
                baseline = yAdvance * 2 / 3; // TODO: baseline is an arbitrary currently, may be define in font file
        int8_t xo = pgm_read_byte(&glyph->xOffset),
               yo = pgm_read_byte(&glyph->yOffset);
        uint8_t xx, yy, bits = 0, bit = 0;
        int16_t xo16 = xo, yo16 = yo;

        if (xAdvance < w)
        {
            xAdvance = w; // Don't know why it exists
        }

        block_w = xAdvance * textsize_x;
        block_h = yAdvance * textsize_y;
        if (
            (x > _max_x) ||            // Clip right
            (y > _max_y) ||            // Clip bottom
            ((x + block_w - 1) < 0) || // Clip left
            ((y + block_h - 1) < 0)    // Clip top
        )
        {
            return;
        }

        // NOTE: Different from Adafruit_GFX design, Adruino_GFX also cater background.
        // Since it may introduce many ugly output, it should limited using on mono font only.
        startWrite();
        if (bg != color) // have background color
        {
            writeFillRect(x, y - (baseline * textsize_y), block_w, block_h, bg);
        }
        for (yy = 0; yy < h; yy++)
        {
            for (xx = 0; xx < w; xx++)
            {
                if (!(bit++ & 7))
                {
                    bits = pgm_read_byte(&bitmap[bo++]);
                }
                if (bits & 0x80)
                {
                    if (textsize_x == 1 && textsize_y == 1)
                    {
                        writePixel(x + xo + xx, y + yo + yy, color);
                    }
                    else
                    {
                        writeFillRect(x + (xo16 + xx) * textsize_x, y + (yo16 + yy) * textsize_y,
                                      textsize_x - text_pixel_margin, textsize_y - text_pixel_margin, color);
                    }
                }
                bits <<= 1;
            }
        }
        endWrite();
    }
    else // 'Classic' built-in font
#endif   // !defined(ATTINY_CORE)
    {
        block_w = 6 * textsize_x;
        block_h = 8 * textsize_y;
        if (
            (x > _max_x) ||            // Clip right
            (y > _max_y) ||            // Clip bottom
            ((x + block_w - 1) < 0) || // Clip left
            ((y + block_h - 1) < 0)    // Clip top
        )
        {
            return;
        }

        if (!_cp437 && (c >= 176))
        {
            c++; // Handle 'classic' charset behavior
        }

        startWrite();
        for (int8_t i = 0; i < 5; i++)
        { // Char bitmap = 5 columns
            uint8_t line = pgm_read_byte(&font[c * 5 + i]);
            for (int8_t j = 0; j < 8; j++, line >>= 1)
            {
                if (line & 1)
                {
                    if (textsize_x == 1 && textsize_y == 1)
                    {
                        writePixel(x + i, y + j, color);
                    }
                    else
                    {
                        if (text_pixel_margin > 0)
                        {
                            writeFillRect(x + (i * textsize_x), y + j * textsize_y, textsize_x - text_pixel_margin, textsize_y - text_pixel_margin, color);
                            writeFillRect(x + ((i + 1) * textsize_x) - text_pixel_margin, y + j * textsize_y, text_pixel_margin, textsize_y, bg);
                            writeFillRect(x + (i * textsize_x), y + ((j + 1) * textsize_y) - text_pixel_margin, textsize_x - text_pixel_margin, text_pixel_margin, bg);
                        }
                        else
                        {
                            writeFillRect(x + i * textsize_x, y + j * textsize_y, textsize_x, textsize_y, color);
                        }
                    }
                }
                else if (bg != color)
                {
                    if (textsize_x == 1 && textsize_y == 1)
                    {
                        writePixel(x + i, y + j, bg);
                    }
                    else
                    {
                        writeFillRect(x + i * textsize_x, y + j * textsize_y, textsize_x, textsize_y, bg);
                    }
                }
            }
        }
        if (bg != color)
        { // If opaque, draw vertical line for last column
            if (textsize_x == 1 && textsize_y == 1)
            {
                writeFastVLine(x + 5, y, 8, bg);
            }
            else
            {
                writeFillRect(x + 5 * textsize_x, y, textsize_x, 8 * textsize_y, bg);
            }
        }
        endWrite();
    }
}

/**************************************************************************/
/*!
    @brief  Print one byte/character of data, used to support print()
    @param  c   The 8-bit ascii character to write
*/
/**************************************************************************/
size_t Arduino_GFX::write(uint8_t c)
{
#if !defined(ATTINY_CORE)
    if (gfxFont) // custom font
    {
        if (c == '\n')
        {
            cursor_x = 0;
            cursor_y += (int16_t)textsize_y *
                        (uint8_t)pgm_read_byte(&gfxFont->yAdvance);
        }
        else if (c != '\r')
        {
            uint8_t first = pgm_read_byte(&gfxFont->first);
            if ((c >= first) && (c <= (uint8_t)pgm_read_byte(&gfxFont->last)))
            {
                GFXglyph *glyph = pgm_read_glyph_ptr(gfxFont, c - first);
                uint8_t w = pgm_read_byte(&glyph->xAdvance);
                // int16_t xo = (int8_t)pgm_read_byte(&glyph->xOffset); // sic
                if (wrap && ((cursor_x + (textsize_x * w) - 1) > _max_x))
                {
                    cursor_x = 0;
                    cursor_y += (int16_t)textsize_y *
                                (uint8_t)pgm_read_byte(&gfxFont->yAdvance);
                }
                drawChar(cursor_x, cursor_y, c, textcolor, textbgcolor);
                cursor_x += textsize_x * w;
            }
        }
    }
    else // 'Classic' built-in font
#endif   // !defined(ATTINY_CORE)
    {
        if (c == '\n')
        {                               // Newline?
            cursor_x = 0;               // Reset x to zero,
            cursor_y += textsize_y * 8; // advance y one line
        }
        else if (c != '\r')
        { // Ignore carriage returns
            if (wrap && ((cursor_x + (textsize_x * 6) - 1) > _max_x))
            {                               // Off right?
                cursor_x = 0;               // Reset x to zero,
                cursor_y += textsize_y * 8; // advance y one line
            }
            drawChar(cursor_x, cursor_y, c, textcolor, textbgcolor);
            cursor_x += textsize_x * 6; // Advance x one char
        }
    }
    return 1;
}

/**************************************************************************/
/*!
    @brief  Set text 'magnification' size. Each increase in s makes 1 pixel that much bigger.
    @param  s   Desired text size. 1 is default 6x8, 2 is 12x16, 3 is 18x24, etc
*/
/**************************************************************************/
void Arduino_GFX::setTextSize(uint8_t s)
{
    setTextSize(s, s, 0);
}

/**************************************************************************/
/*!
    @brief  Set text 'magnification' size. Each increase in s makes 1 pixel that much bigger.
    @param  s_x Desired text width magnification level in X-axis. 1 is default
    @param  s_y Desired text width magnification level in Y-axis. 1 is default
*/
/**************************************************************************/
void Arduino_GFX::setTextSize(uint8_t s_x, uint8_t s_y)
{
    setTextSize(s_x, s_y, 0);
}

/**************************************************************************/
/*!
    @brief  Set text 'magnification' size. Each increase in s makes 1 pixel that much bigger.
    @param  s_x             Desired text width magnification level in X-axis. 1 is default
    @param  s_y             Desired text width magnification level in Y-axis. 1 is default
    @param  pixel_margin    Margin for each text pixel. 0 is default
*/
/**************************************************************************/
void Arduino_GFX::setTextSize(uint8_t s_x, uint8_t s_y, uint8_t pixel_margin)
{
    text_pixel_margin = ((pixel_margin < s_x) && (pixel_margin < s_y)) ? pixel_margin : 0;
    textsize_x = (s_x > 0) ? s_x : 1;
    textsize_y = (s_y > 0) ? s_y : 1;
}

/**************************************************************************/
/*!
    @brief  Set rotation setting for display
    @param  r   0 thru 3 corresponding to 4 cardinal rotations
*/
/**************************************************************************/
void Arduino_GFX::setRotation(uint8_t r)
{
    _rotation = (r & 3);
    switch (_rotation)
    {
    case 0:
    case 2:
        _width = WIDTH;
        _height = HEIGHT;
        _max_x = _width - 1;  ///< x zero base bound
        _max_y = _height - 1; ///< y zero base bound
        break;
    case 1:
    case 3:
        _width = HEIGHT;
        _height = WIDTH;
        _max_x = _width - 1;  ///< x zero base bound
        _max_y = _height - 1; ///< y zero base bound
        break;
    }
}

#if !defined(ATTINY_CORE)
/**************************************************************************/
/*!
    @brief  Set the font to display when print()ing, either custom or default
    @param  f   The GFXfont object, if NULL use built in 6x8 font
*/
/**************************************************************************/
void Arduino_GFX::setFont(const GFXfont *f)
{
    if (f)
    { // Font struct pointer passed in?
        if (!gfxFont)
        { // And no current font struct?
            // Switching from classic to new font behavior.
            // Move cursor pos down 6 pixels so it's on baseline.
            cursor_y += 6;
        }
    }
    else if (gfxFont)
    { // NULL passed.  Current font struct defined?
        // Switching from new to classic font behavior.
        // Move cursor pos up 6 pixels so it's at top-left of char.
        cursor_y -= 6;
    }
    gfxFont = (GFXfont *)f;
}

/**************************************************************************/
/*!
    @brief  flush framebuffer to output (for Canvas or NeoPixel sub-class)
*/
/**************************************************************************/
void Arduino_GFX::flush()
{
}
#endif // !defined(ATTINY_CORE)

/**************************************************************************/
/*!
    @brief  Helper to determine size of a character with current font/size.
            Broke this out as it's used by both the PROGMEM- and RAM-resident getTextBounds() functions.
    @param  c       The ascii character in question
    @param  x       Pointer to x location of character
    @param  y       Pointer to y location of character
    @param  minx    Minimum clipping value for X
    @param  miny    Minimum clipping value for Y
    @param  maxx    Maximum clipping value for X
    @param  maxy    Maximum clipping value for Y
*/
/**************************************************************************/
void Arduino_GFX::charBounds(char c, int16_t *x, int16_t *y,
                             int16_t *minx, int16_t *miny, int16_t *maxx, int16_t *maxy)
{
#if !defined(ATTINY_CORE)
    if (gfxFont) // custom font
    {
        if (c == '\n')
        {           // Newline?
            *x = 0; // Reset x to zero, advance y by one line
            *y += textsize_y * (uint8_t)pgm_read_byte(&gfxFont->yAdvance);
        }
        else if (c != '\r')
        { // Not a carriage return; is normal char
            uint8_t first = pgm_read_byte(&gfxFont->first),
                    last = pgm_read_byte(&gfxFont->last);
            if ((c >= first) && (c <= last))
            { // Char present in this font?
                GFXglyph *glyph = pgm_read_glyph_ptr(gfxFont, c - first);
                uint8_t gw = pgm_read_byte(&glyph->width),
                        gh = pgm_read_byte(&glyph->height),
                        xa = pgm_read_byte(&glyph->xAdvance);
                int8_t xo = pgm_read_byte(&glyph->xOffset),
                       yo = pgm_read_byte(&glyph->yOffset);
                if (wrap && ((*x + (((int16_t)xo + gw) * textsize_x) - 1) > _max_x))
                {
                    *x = 0; // Reset x to zero, advance y by one line
                    *y += textsize_y * (uint8_t)pgm_read_byte(&gfxFont->yAdvance);
                }
                int16_t tsx = (int16_t)textsize_x,
                        tsy = (int16_t)textsize_y,
                        x1 = *x + xo * tsx,
                        y1 = *y + yo * tsy,
                        x2 = x1 + gw * tsx - 1,
                        y2 = y1 + gh * tsy - 1;
                if (x1 < *minx)
                {
                    *minx = x1;
                }
                if (y1 < *miny)
                {
                    *miny = y1;
                }
                if (x2 > *maxx)
                {
                    *maxx = x2;
                }
                if (y2 > *maxy)
                {
                    *maxy = y2;
                }
                *x += xa * tsx;
            }
        }
    }
    else // 'Classic' built-in font
#endif   // !defined(ATTINY_CORE)
    {
        if (c == '\n')
        {                         // Newline?
            *x = 0;               // Reset x to zero,
            *y += textsize_y * 8; // advance y one line
            // min/max x/y unchaged -- that waits for next 'normal' character
        }
        else if (c != '\r')
        { // Normal char; ignore carriage returns
            if (wrap && ((*x + (textsize_x * 6) - 1) > _max_x))
            {                         // Off right?
                *x = 0;               // Reset x to zero,
                *y += textsize_y * 8; // advance y one line
            }
            int16_t x2 = *x + textsize_x * 6 - 1; // Lower-right pixel of char
            int16_t y2 = *y + textsize_y * 8 - 1;
            if (x2 > *maxx)
            {
                *maxx = x2; // Track max x, y
            }
            if (y2 > *maxy)
            {
                *maxy = y2;
            }
            if (*x < *minx)
            {
                *minx = *x; // Track min x, y
            }
            if (*y < *miny)
            {
                *miny = *y;
            }
            *x += textsize_x * 6; // Advance x one char
        }
    }
}

/**************************************************************************/
/*!
    @brief  Helper to determine size of a string with current font/size. Pass string and a cursor position, returns UL corner and W,H.
    @param  str The ascii string to measure
    @param  x   The current cursor X
    @param  y   The current cursor Y
    @param  x1  The boundary X coordinate, set by function
    @param  y1  The boundary Y coordinate, set by function
    @param  w   The boundary width, set by function
    @param  h   The boundary height, set by function
*/
/**************************************************************************/
void Arduino_GFX::getTextBounds(const char *str, int16_t x, int16_t y,
                                int16_t *x1, int16_t *y1, uint16_t *w, uint16_t *h)
{
    uint8_t c; // Current character

    *x1 = x;
    *y1 = y;
    *w = *h = 0;

    int16_t minx = _width, miny = _height, maxx = -1, maxy = -1;

    while ((c = *str++))
    {
        charBounds(c, &x, &y, &minx, &miny, &maxx, &maxy);
    }

    if (maxx >= minx)
    {
        *x1 = minx;
        *w = maxx - minx + 1;
    }
    if (maxy >= miny)
    {
        *y1 = miny;
        *h = maxy - miny + 1;
    }
}

/**************************************************************************/
/*!
    @brief  Helper to determine size of a string with current font/size. Pass string and a cursor position, returns UL corner and W,H.
    @param  str The ascii string to measure (as an arduino String() class)
    @param  x   The current cursor X
    @param  y   The current cursor Y
    @param  x1  The boundary X coordinate, set by function
    @param  y1  The boundary Y coordinate, set by function
    @param  w   The boundary width, set by function
    @param  h   The boundary height, set by function
*/
/**************************************************************************/
void Arduino_GFX::getTextBounds(const String &str, int16_t x, int16_t y,
                                int16_t *x1, int16_t *y1, uint16_t *w, uint16_t *h)
{
    if (str.length() != 0)
    {
        getTextBounds(const_cast<char *>(str.c_str()), x, y, x1, y1, w, h);
    }
}

/**************************************************************************/
/*!
    @brief  Helper to determine size of a PROGMEM string with current font/size. Pass string and a cursor position, returns UL corner and W,H.
    @param  str The flash-memory ascii string to measure
    @param  x   The current cursor X
    @param  y   The current cursor Y
    @param  x1  The boundary X coordinate, set by function
    @param  y1  The boundary Y coordinate, set by function
    @param  w   The boundary width, set by function
    @param  h   The boundary height, set by function
*/
/**************************************************************************/
void Arduino_GFX::getTextBounds(const __FlashStringHelper *str,
                                int16_t x, int16_t y, int16_t *x1, int16_t *y1, uint16_t *w, uint16_t *h)
{
    uint8_t *s = (uint8_t *)str, c;

    *x1 = x;
    *y1 = y;
    *w = *h = 0;

    int16_t minx = _width, miny = _height, maxx = -1, maxy = -1;

    while ((c = pgm_read_byte(s++)))
        charBounds(c, &x, &y, &minx, &miny, &maxx, &maxy);

    if (maxx >= minx)
    {
        *x1 = minx;
        *w = maxx - minx + 1;
    }
    if (maxy >= miny)
    {
        *y1 = miny;
        *h = maxy - miny + 1;
    }
}

/**************************************************************************/
/*!
    @brief  Invert the display (ideally using built-in hardware command)
    @param  i   True if you want to invert, false to make 'normal'
*/
/**************************************************************************/
void Arduino_GFX::invertDisplay(bool i)
{
    // Do nothing, must be subclassed if supported by hardware
    UNUSED(i);
}
