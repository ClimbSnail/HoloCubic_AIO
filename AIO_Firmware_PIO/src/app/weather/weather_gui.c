#include "stdlib.h"

#include "weather_gui.h"
#include "weather_image.h"

#include "lvgl.h"

LV_FONT_DECLARE(lv_font_ibmplex_115);
LV_FONT_DECLARE(lv_font_ibmplex_64);
LV_FONT_DECLARE(ch_font20);
static lv_style_t default_style;
static lv_style_t chFont_style;
static lv_style_t numberSmall_style;
static lv_style_t numberBig_style;
static lv_style_t btn_style;
static lv_style_t bar_style;

static lv_obj_t *scr_1 = NULL;
static lv_obj_t *scr_2 = NULL;
static lv_obj_t *chart, *titleLabel;

static lv_obj_t *weatherImg = NULL;
static lv_obj_t *cityLabel = NULL;
static lv_obj_t *btn = NULL, *btnLabel = NULL;
static lv_obj_t *txtLabel = NULL;
static lv_obj_t *clockLabel_1 = NULL, *clockLabel_2 = NULL;
static lv_obj_t *dateLabel = NULL;
static lv_obj_t *tempImg = NULL, *tempBar = NULL, *tempLabel = NULL;
static lv_obj_t *humiImg = NULL, *humiBar = NULL, *humiLabel = NULL;
static lv_obj_t *spaceImg = NULL;

static lv_chart_series_t *ser1, *ser2;

// 天气图标路径的映射关系
const void *weaImage_map[] = {&qweather_100, &qweather_101, &qweather_102, &qweather_103, &qweather_104,
                                &qweather_150, &qweather_151, &qweather_152, &qweather_153, &qweather_300,
                                &qweather_301, &qweather_302, &qweather_303, &qweather_304, &qweather_305,
                                &qweather_306, &qweather_307, &qweather_308, &qweather_309, &qweather_310,
                                &qweather_311, &qweather_312, &qweather_313, &qweather_314, &qweather_315,
                                &qweather_316, &qweather_317, &qweather_318, &qweather_350, &qweather_351,
                                &qweather_399, &qweather_400, &qweather_401, &qweather_402, &qweather_403,
                                &qweather_404, &qweather_405, &qweather_406, &qweather_407, &qweather_408,
                                &qweather_409, &qweather_410, &qweather_456, &qweather_457, &qweather_499,
                                &qweather_500, &qweather_501, &qweather_502, &qweather_503, &qweather_504,
                                &qweather_507, &qweather_508, &qweather_509, &qweather_510, &qweather_511,
                                &qweather_512, &qweather_513, &qweather_514, &qweather_515, &qweather_900,
                                &qweather_901, &qweather_999};

// 太空人图标路径的映射关系
const void *manImage_map[] = {&man_0, &man_1, &man_2, &man_3, &man_4, &man_5, &man_6, &man_7, &man_8, &man_9};
static const char weekDayCh[7][4] = {"日", "一", "二", "三", "四", "五", "六"};
static const char airQualityCh[6][10] = {"优", "良", "轻度", "中度", "重度", "严重"};

void weather_gui_init(void)
{
    lv_style_init(&default_style);
    lv_style_set_bg_color(&default_style, lv_color_hex(0x000000));

    lv_style_init(&chFont_style);
    lv_style_set_text_opa(&chFont_style, LV_OPA_COVER);
    lv_style_set_text_color(&chFont_style, lv_color_hex(0xffffff));
    lv_style_set_text_font(&chFont_style, &ch_font20);

    lv_style_init(&numberSmall_style);
    lv_style_set_text_opa(&numberSmall_style, LV_OPA_COVER);
    lv_style_set_text_color(&numberSmall_style, lv_color_hex(0xffffff));
    lv_style_set_text_font(&numberSmall_style, &lv_font_ibmplex_64);

    lv_style_init(&numberBig_style);
    lv_style_set_text_opa(&numberBig_style, LV_OPA_COVER);
    lv_style_set_text_color(&numberBig_style, lv_color_hex(0xffffff));
    lv_style_set_text_font(&numberBig_style, &lv_font_ibmplex_115);

    lv_style_init(&btn_style);
    lv_style_set_border_width(&btn_style, 0);
    lv_style_init(&bar_style);
    lv_style_set_bg_color(&bar_style, lv_color_hex(0x000000));
    lv_style_set_border_width(&bar_style, 2);
    lv_style_set_border_color(&bar_style, lv_color_hex(0xFFFFFF));
    lv_style_set_pad_top(&bar_style, 1); // 指示器到背景四周的距离
    lv_style_set_pad_bottom(&bar_style, 1);
    lv_style_set_pad_left(&bar_style, 1);
    lv_style_set_pad_right(&bar_style, 1);
}

void display_curve_init(lv_scr_load_anim_t anim_type, short temp_max, short temp_min)
{
    static short temp_max_old = 0, temp_min_old = 0;
    lv_obj_t *act_obj = lv_scr_act(); // 获取当前活动页
    if(temp_max_old == temp_max && temp_min_old == temp_min && act_obj == scr_2) return;

    temp_max_old = temp_max;
    temp_min_old = temp_min;

    weather_gui_release();
    lv_obj_clean(act_obj); // 清空此前页面

    scr_2 = lv_obj_create(NULL);
    lv_obj_add_style(scr_2, &default_style, LV_STATE_DEFAULT);

    titleLabel = lv_label_create(scr_2);
    lv_obj_add_style(titleLabel, &chFont_style, LV_STATE_DEFAULT);
    lv_label_set_recolor(titleLabel, true);
    lv_label_set_text(titleLabel, "七日气温");

    chart = lv_chart_create(scr_2);
    lv_obj_set_size(chart, 200, 180);
    lv_obj_align(chart, LV_ALIGN_CENTER, 15, 10);
    lv_chart_set_range(chart, LV_CHART_AXIS_PRIMARY_Y, temp_min, temp_max);
    lv_chart_set_point_count(chart, 7);
    lv_chart_set_div_line_count(chart, 5, 7);
    lv_chart_set_type(chart, LV_CHART_TYPE_LINE); /*Show lines and points too*/

    lv_obj_set_style_text_color(chart, lv_palette_lighten(LV_PALETTE_YELLOW, 2), LV_PART_TICKS);
    lv_obj_set_style_line_color(chart, lv_palette_lighten(LV_PALETTE_YELLOW, 2), LV_PART_TICKS);

    // 设置Y轴上刻度线的数量
    lv_chart_set_axis_tick(chart, LV_CHART_AXIS_PRIMARY_Y,
                            10, 5, 10, 2, true, 100);

    ser1 = lv_chart_add_series(chart, lv_palette_main(LV_PALETTE_RED), LV_CHART_AXIS_SECONDARY_Y);
    ser2 = lv_chart_add_series(chart, lv_palette_main(LV_PALETTE_BLUE), LV_CHART_AXIS_SECONDARY_Y);
    // lv_obj_set_style_pad_left(chart, 40, LV_STATE_DEFAULT);

    // lv_chart_set_zoom_y();

    // 绘制
    lv_obj_align(titleLabel, LV_ALIGN_TOP_MID, 0, 10);

    lv_chart_refresh(chart);

    if (LV_SCR_LOAD_ANIM_NONE != anim_type)
    {
        lv_scr_load_anim(scr_2, anim_type, 300, 300, false);
    }
    else
    {
        lv_scr_load(scr_2);
    }
}

void display_curve(short maxT[], short minT[], lv_scr_load_anim_t anim_type)
{
    short temp_max = maxT[0], temp_min = minT[0];
    for (short Ti = 1; Ti < 7; ++Ti)
    {
        if ( maxT[Ti] > temp_max) temp_max = maxT[Ti];
        if ( minT[Ti] < temp_min) temp_min = minT[Ti];
    }
    short temp_diff = temp_max - temp_min;
    while (temp_diff%10 != 0)
    {
        if(temp_diff%2 == 0) temp_min--;
        else temp_max++;
        temp_diff++;
    }
    display_curve_init(anim_type, temp_max, temp_min);
    short temp_mid = temp_min + temp_diff/2;
    for (short Ti = 0; Ti < 7; ++Ti)
    {
        char tmp[5] = {0};

        double point = ((maxT[Ti] - temp_mid + (temp_diff/2.0))/temp_diff)*100;
        sprintf(tmp, "%.0f", point);
        ser1->y_points[Ti] = atoi(tmp);

        point = ((minT[Ti] - temp_mid + (temp_diff/2.0))/temp_diff)*100;
        sprintf(tmp, "%.0f", point);
        ser2->y_points[Ti] = atoi(tmp);
    }
    lv_chart_refresh(chart);
}

void display_weather_init(lv_scr_load_anim_t anim_type, int temperature, int humidity)
{
    lv_obj_t *act_obj = lv_scr_act(); // 获取当前活动页
    if (act_obj == scr_1)
        return;

    weather_gui_release();
    lv_obj_clean(act_obj); // 清空此前页面

    scr_1 = lv_obj_create(NULL);
    lv_obj_add_style(scr_1, &default_style, LV_STATE_DEFAULT);

    weatherImg = lv_img_create(scr_1);
    lv_img_set_src(weatherImg, weaImage_map[0]);

    cityLabel = lv_label_create(scr_1);
    lv_obj_add_style(cityLabel, &chFont_style, LV_STATE_DEFAULT);
    lv_label_set_recolor(cityLabel, true);
    lv_label_set_text(cityLabel, "上海");

    btn = lv_btn_create(scr_1);
    lv_obj_add_style(btn, &btn_style, LV_STATE_DEFAULT);
    lv_obj_set_pos(btn, 75, 15);
    lv_obj_set_size(btn, 50, 25);
    lv_obj_set_style_bg_color(btn, lv_palette_main(LV_PALETTE_ORANGE), LV_STATE_DEFAULT);

    btnLabel = lv_label_create(btn);
    lv_obj_add_style(btnLabel, &chFont_style, LV_STATE_DEFAULT);
    lv_obj_align(btnLabel, LV_ALIGN_CENTER, 0, 0);
    lv_label_set_text(btnLabel, airQualityCh[0]);

    txtLabel = lv_label_create(scr_1);
    lv_obj_add_style(txtLabel, &chFont_style, LV_STATE_DEFAULT);
    // lvgl8之前版本，模式一旦设置 LV_LABEL_LONG_SCROLL_CIRCULAR
    // 宽度恒定等于当前文本的长度，所以下面先设置以下长度
    lv_label_set_text(txtLabel, "最低气温12°C, ");
    lv_obj_set_size(txtLabel, 120, 30);
    lv_label_set_long_mode(txtLabel, LV_LABEL_LONG_SCROLL_CIRCULAR);
    lv_label_set_text_fmt(txtLabel, "最低气温%d°C, 最高气温%d°C, %s%d 级.   ", 15, 20, "西北风", 0);

    clockLabel_1 = lv_label_create(scr_1);
    lv_obj_add_style(clockLabel_1, &numberBig_style, LV_STATE_DEFAULT);
    lv_label_set_recolor(clockLabel_1, true);
    lv_label_set_text_fmt(clockLabel_1, "%02d#ffa500 %02d#", 10, 52);

    clockLabel_2 = lv_label_create(scr_1);
    lv_obj_add_style(clockLabel_2, &numberSmall_style, LV_STATE_DEFAULT);
    lv_label_set_recolor(clockLabel_2, true);
    lv_label_set_text_fmt(clockLabel_2, "%02d", 00);

    dateLabel = lv_label_create(scr_1);
    lv_obj_add_style(dateLabel, &chFont_style, LV_STATE_DEFAULT);
    lv_label_set_text_fmt(dateLabel, "%2d月%2d日   周%s", 11, 23, weekDayCh[1]);

    tempImg = lv_img_create(scr_1);
    lv_img_set_src(tempImg, &temp);
    lv_img_set_zoom(tempImg, 180);
    tempBar = lv_bar_create(scr_1);
    lv_obj_add_style(tempBar, &bar_style, LV_STATE_DEFAULT);
    lv_bar_set_range(tempBar, -50, 50); // 设置进度条表示的温度为-50~50
    lv_obj_set_size(tempBar, 60, 12);
    lv_obj_set_style_bg_color(tempBar, lv_palette_main(LV_PALETTE_RED), LV_PART_INDICATOR);
    lv_bar_set_value(tempBar, temperature, LV_ANIM_ON);
    tempLabel = lv_label_create(scr_1);
    lv_obj_add_style(tempLabel, &chFont_style, LV_STATE_DEFAULT);
    lv_label_set_text_fmt(tempLabel, "%2d°C", 18);

    humiImg = lv_img_create(scr_1);
    lv_img_set_src(humiImg, &humi);
    lv_img_set_zoom(humiImg, 180);
    humiBar = lv_bar_create(scr_1);
    lv_obj_add_style(humiBar, &bar_style, LV_STATE_DEFAULT);
    lv_bar_set_range(humiBar, 0, 100);
    lv_obj_set_size(humiBar, 60, 12);
    lv_obj_set_style_bg_color(humiBar, lv_palette_main(LV_PALETTE_BLUE), LV_PART_INDICATOR);
    lv_bar_set_value(humiBar, humidity, LV_ANIM_ON);
    humiLabel = lv_label_create(scr_1);
    lv_obj_add_style(humiLabel, &chFont_style, LV_STATE_DEFAULT);
    lv_label_set_text(humiLabel, "50%");

    // 太空人图标
    spaceImg = lv_img_create(scr_1);
    lv_img_set_src(spaceImg, manImage_map[0]);

    // 绘制图形
    lv_obj_align(weatherImg, LV_ALIGN_TOP_RIGHT, -10, 10);
    lv_obj_align(cityLabel, LV_ALIGN_TOP_LEFT, 20, 15);
    lv_obj_align(txtLabel, LV_ALIGN_TOP_LEFT, 10, 50);
    lv_obj_align(tempImg, LV_ALIGN_LEFT_MID, 10, 70);
    lv_obj_align(tempBar, LV_ALIGN_LEFT_MID, 35, 70);
    lv_obj_align(tempLabel, LV_ALIGN_LEFT_MID, 103, 70);
    lv_obj_align(humiImg, LV_ALIGN_LEFT_MID, 0, 100);
    lv_obj_align(humiBar, LV_ALIGN_LEFT_MID, 35, 100);
    lv_obj_align(humiLabel, LV_ALIGN_LEFT_MID, 103, 100);
    lv_obj_align(spaceImg, LV_ALIGN_BOTTOM_RIGHT, -10, -10);

    lv_obj_align(clockLabel_1, LV_ALIGN_LEFT_MID, 0, 10);
    lv_obj_align(clockLabel_2, LV_ALIGN_LEFT_MID, 165, 9);
    lv_obj_align(dateLabel, LV_ALIGN_LEFT_MID, 10, 32);

    if (LV_SCR_LOAD_ANIM_NONE != anim_type)
    {
        lv_scr_load_anim(scr_1, anim_type, 300, 300, false);
    }
    else
    {
        lv_scr_load(scr_1);
    }
}

void display_weather(struct Weather weaInfo, struct TimeStr timeInfo, lv_scr_load_anim_t anim_type)
{
    display_weather_init(anim_type, weaInfo.temperature, weaInfo.humidity);

    lv_label_set_text(cityLabel, weaInfo.cityname);
    if (strlen(weaInfo.cityname) > 6)
    {
        lv_obj_align(cityLabel, LV_ALIGN_TOP_LEFT, 5, 15);
    }
    else
    {
        lv_obj_align(cityLabel, LV_ALIGN_TOP_LEFT, 20, 15);
    }
    lv_label_set_text(btnLabel, airQualityCh[weaInfo.airQulity]);
    lv_img_set_src(weatherImg, weaImage_map[weaInfo.weather_code]);
    // 下面这行代码可能会出错
    lv_label_set_text_fmt(txtLabel, "最低气温%d°C, 最高气温%d°C, %s%d 级.   ",
                          weaInfo.minTemp, weaInfo.maxTemp, weaInfo.windDir, weaInfo.windLevel);

    lv_bar_set_value(tempBar, weaInfo.temperature, LV_ANIM_OFF);
    lv_label_set_text_fmt(tempLabel, "%2d°C", weaInfo.temperature);
    lv_bar_set_value(humiBar, weaInfo.humidity, LV_ANIM_OFF);
    lv_label_set_text_fmt(humiLabel, "%d%%", weaInfo.humidity);

    lv_label_set_text_fmt(clockLabel_1, "%02d#ffa500 %02d#", timeInfo.hour, timeInfo.minute);
    lv_label_set_text_fmt(clockLabel_2, "%02d", timeInfo.second);
    lv_label_set_text_fmt(dateLabel, "%2d月%2d日   周%s", timeInfo.month, timeInfo.day,
                          weekDayCh[timeInfo.weekday]);

    // // 绘制图形
    // lv_obj_align(weatherImg, NULL, LV_ALIGN_TOP_RIGHT, -10, 10);
    // lv_obj_align(cityLabel, NULL, LV_ALIGN_TOP_LEFT, 20, 15);
    // lv_obj_align(txtLabel, NULL, LV_ALIGN_TOP_LEFT, 0, 50);
    // lv_obj_align(tempImg, NULL, LV_ALIGN_LEFT_MID, 10, 70);
    // lv_obj_align(tempBar, NULL, LV_ALIGN_LEFT_MID, 35, 70);
    // lv_obj_align(tempLabel, NULL, LV_ALIGN_LEFT_MID, 100, 70);
    // lv_obj_align(humiImg, NULL, LV_ALIGN_LEFT_MID, 0, 100);
    // lv_obj_align(humiBar, NULL, LV_ALIGN_LEFT_MID, 35, 100);
    // lv_obj_align(humiLabel, NULL, LV_ALIGN_LEFT_MID, 100, 100);
    // lv_obj_align(spaceImg, NULL, LV_ALIGN_BOTTOM_RIGHT, -10, -10);
}

void weather_gui_release(void)
{
    if (scr_1 != NULL)
    {
        lv_obj_clean(scr_1);
        scr_1 = NULL;
        weatherImg = NULL;
        cityLabel = NULL;
        btn = NULL;
        btnLabel = NULL;
        txtLabel = NULL;
        clockLabel_1 = NULL;
        clockLabel_2 = NULL;
        dateLabel = NULL;
        tempImg = NULL;
        tempBar = NULL;
        tempLabel = NULL;
        humiImg = NULL;
        humiBar = NULL;
        humiLabel = NULL;
        spaceImg = NULL;
    }

    if (scr_2 != NULL)
    {
        lv_obj_clean(scr_2);
        scr_2 = NULL;
        chart = NULL;
        titleLabel = NULL;
        ser1 = NULL;
        ser2 = NULL;
    }
}

void weather_gui_del(void)
{
    weather_gui_release();

    // 手动清除样式，防止内存泄漏
    // lv_style_reset(&default_style);
    // lv_style_reset(&chFont_style);
    // lv_style_reset(&numberSmall_style);
    // lv_style_reset(&numberBig_style);
    // lv_style_reset(&btn_style);
    // lv_style_reset(&bar_style);
}

void display_space(void)
{
    static int _spaceIndex = 0;
    if (NULL != scr_1 && lv_scr_act() == scr_1)
    {
        lv_img_set_src(spaceImg, manImage_map[_spaceIndex]);
        _spaceIndex = (_spaceIndex + 1) % 10;
    }
}
