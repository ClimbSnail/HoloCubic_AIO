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
const void *weaImage_map[] = {&weather_0, &weather_9, &weather_14, &weather_5, &weather_25,
                              &weather_30, &weather_26, &weather_11, &weather_23};
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
    lv_style_set_pad_top(&bar_style, 1); //指示器到背景四周的距离
    lv_style_set_pad_bottom(&bar_style, 1);
    lv_style_set_pad_left(&bar_style, 1);
    lv_style_set_pad_right(&bar_style, 1);
}

void display_curve_init(lv_scr_load_anim_t anim_type)
{
    lv_obj_t *act_obj = lv_scr_act(); // 获取当前活动页
    if (act_obj == scr_2)
        return;

    weather_gui_release();
    lv_obj_clean(act_obj); // 清空此前页面

    scr_2 = lv_obj_create(NULL);
    lv_obj_add_style(scr_2, &default_style, LV_STATE_DEFAULT);

    titleLabel = lv_label_create(scr_2);
    lv_obj_add_style(titleLabel, &chFont_style, LV_STATE_DEFAULT);
    lv_label_set_recolor(titleLabel, true);
    lv_label_set_text(titleLabel, "查看更多天气");

    chart = lv_chart_create(scr_2);
    lv_obj_set_size(chart, 220, 180);
    lv_chart_set_range(chart, LV_CHART_AXIS_PRIMARY_Y, 0, 40); // todo
    lv_chart_set_point_count(chart, 7);
    lv_chart_set_type(chart, LV_CHART_TYPE_LINE); /*Show lines and points too*/
    // todo
    ser1 = lv_chart_add_series(chart, lv_palette_main(LV_PALETTE_RED), LV_CHART_AXIS_SECONDARY_Y);
    ser2 = lv_chart_add_series(chart, lv_palette_main(LV_PALETTE_BLUE), LV_CHART_AXIS_SECONDARY_Y);
    // lv_obj_set_style_pad_left(chart, LV_CHART_PART_BG, LV_STATE_DEFAULT, 40);
    // lv_chart_set_y_tick_texts(chart, "40\n30\n20\n10\n0", 0, LV_CHART_AXIS_DRAW_LAST_TICK);
    // lv_chart_set_zoom_y();
    // 绘制
    lv_obj_align(titleLabel, LV_ALIGN_TOP_MID, 0, 10);
    lv_obj_align(chart, LV_ALIGN_CENTER, 0, 10);

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
    display_curve_init(anim_type);
    for (int Ti = 0; Ti < 7; ++Ti)
    {
        // ser1->points[Ti] = maxT[Ti]; // todo
        ser1->y_points[Ti] = maxT[Ti];
    }
    for (int Ti = 0; Ti < 7; ++Ti)
    {
        // ser2->points[Ti] = minT[Ti]; // todo
        ser2->y_points[Ti] = minT[Ti];
    }
    lv_chart_refresh(chart);
}

void display_weather_init(lv_scr_load_anim_t anim_type)
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
    lv_obj_set_style_bg_color(btn, lv_palette_main(LV_PALETTE_ORANGE), LV_PART_MAIN);

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
    lv_bar_set_range(tempBar, -50, 50); // 设置进度条表示的温度为-20~50
    lv_obj_set_size(tempBar, 60, 12);
    lv_obj_set_style_bg_color(tempBar, lv_palette_main(LV_PALETTE_RED), LV_PART_MAIN);
    lv_bar_set_value(tempBar, 10, LV_ANIM_OFF);
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
    lv_obj_set_style_bg_color(humiBar, lv_palette_main(LV_PALETTE_BLUE), LV_PART_MAIN);
    lv_bar_set_value(humiBar, 49, LV_ANIM_OFF);
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

    // if (LV_SCR_LOAD_ANIM_NONE != anim_type)
    // {
    //     lv_scr_load_anim(scr_1, anim_type, 300, 300, false);
    // }
    // else
    // {
    // lv_scr_load(scr_1);
    // }
}

void display_weather(struct Weather weaInfo, lv_scr_load_anim_t anim_type)
{
    display_weather_init(anim_type);

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

    if (LV_SCR_LOAD_ANIM_NONE != anim_type)
    {
        lv_scr_load_anim(scr_1, anim_type, 300, 300, false);
    }
    else
    {
        lv_scr_load(scr_1);
    }
}

void display_time(struct TimeStr timeInfo, lv_scr_load_anim_t anim_type)
{
    display_weather_init(anim_type);
    lv_label_set_text_fmt(clockLabel_1, "%02d#ffa500 %02d#", timeInfo.hour, timeInfo.minute);
    lv_label_set_text_fmt(clockLabel_2, "%02d", timeInfo.second);
    lv_label_set_text_fmt(dateLabel, "%2d月%2d日   周%s", timeInfo.month, timeInfo.day,
                          weekDayCh[timeInfo.weekday]);

    // lv_obj_align(clockLabel_1, NULL, LV_ALIGN_LEFT_MID, 0, 10);
    // lv_obj_align(clockLabel_2, NULL, LV_ALIGN_LEFT_MID, 165, 9);
    // lv_obj_align(dateLabel, NULL, LV_ALIGN_LEFT_MID, 10, 32);

    // if (LV_SCR_LOAD_ANIM_NONE != anim_type)
    // {
    //     lv_scr_load_anim(scr_1, anim_type, 300, 300, false);
    // }
    // else
    // {
    //     lv_scr_load(scr_1);
    // }
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

int airQulityLevel(int q)
{
    if (q < 50)
    {
        return 0;
    }
    else if (q < 100)
    {
        return 1;
    }
    else if (q < 150)
    {
        return 2;
    }
    else if (q < 200)
    {
        return 3;
    }
    else if (q < 300)
    {
        return 4;
    }
    return 5;
}