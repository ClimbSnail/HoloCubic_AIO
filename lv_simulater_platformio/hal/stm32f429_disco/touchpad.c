/**
 * @file indev.c
 * 
 */

/*********************
 *      INCLUDES
 *********************/
#include "tft.h"
#include "lvgl.h"

#include "stm32f4xx.h"
#include "stm32f429i_discovery.h"
#include "stmpe811.h"

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void touchpad_read(lv_indev_drv_t * drv, lv_indev_data_t *data);
static bool touchpad_get_xy(int16_t *x, int16_t *y);

/**********************
 *  STATIC VARIABLES
 **********************/

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

/**
 * Initialize your input devices here
 */
void touchpad_init(void)
{
  stmpe811_Init(TS_I2C_ADDRESS);
  stmpe811_TS_Start(TS_I2C_ADDRESS);

  lv_indev_drv_t indev_drv;
  lv_indev_drv_init(&indev_drv);
  indev_drv.read_cb = touchpad_read;
  indev_drv.type = LV_INDEV_TYPE_POINTER;
  lv_indev_drv_register(&indev_drv);
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

/**
 * Read an input device
 * @param indev_id id of the input device to read
 * @param x put the x coordinate here
 * @param y put the y coordinate here
 * @return true: the device is pressed, false: released
 */
static void touchpad_read(lv_indev_drv_t * drv, lv_indev_data_t *data)
{
	static int16_t last_x = 0;
	static int16_t last_y = 0;

	bool detected;
	int16_t x;
	int16_t y;
	detected = touchpad_get_xy(&x, &y);
	if(detected) {
		data->point.x = x;
		data->point.y = y;
		last_x = data->point.x;
		last_y = data->point.y;

		data->state = LV_INDEV_STATE_PR;
	} else {
		data->point.x = last_x;
		data->point.y = last_y;
		data->state = LV_INDEV_STATE_REL;
	}
}


static bool touchpad_get_xy(int16_t *x, int16_t *y)
{
	static int32_t _x = 0, _y = 0;
	int16_t xDiff, yDiff, xr, yr, x_raw, y_raw;;

	bool detected;
	detected = stmpe811_TS_DetectTouch(TS_I2C_ADDRESS);

	if(!detected) return false;


	stmpe811_TS_GetXY(TS_I2C_ADDRESS, &x_raw, &y_raw);

	/* Y value first correction */
	y_raw -= 360;

	/* Y value second correction */
	yr = y_raw / 11;

	/* Return y_raw position value */
	if(yr <= 0) yr = 0;
	else if (yr > TFT_VER_RES) yr = TFT_VER_RES - 1;

	y_raw = yr;

	/* X value first correction */
	if(x_raw <= 3000) x_raw = 3870 - x_raw;
	else  x_raw = 3800 - x_raw;

	/* X value second correction */
	xr = x_raw / 15;

	/* Return X position value */
	if(xr <= 0) xr = 0;
	else if (xr > TFT_HOR_RES) xr = TFT_HOR_RES - 1;

	x_raw = xr;
	xDiff = x_raw > _x? (x_raw - _x): (_x - x_raw);
	yDiff = y_raw > _y? (y_raw - _y): (_y - y_raw);

	if (xDiff + yDiff > 5) {
		_x = x_raw;
		_y = y_raw;
	}

	/* Update the X and Y position */
	*x = _x;
	*y = _y;

	return true;
}
