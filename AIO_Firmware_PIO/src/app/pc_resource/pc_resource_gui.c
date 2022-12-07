#include "pc_resource_gui.h"

LV_FONT_DECLARE(lv_font_ibmplex_24);
LV_FONT_DECLARE(lv_font_ibmplex_18);
LV_FONT_DECLARE(lv_font_ibmplex_16);

#define MODULE_CPU 0
#define MODULE_GPU 1
#define MODULE_RAM 2
#define MODULE_EXT 3

// 扩展组件属性更新函数
static void extension_1_update(struct PC_Resource info);
static void extension_2_update(struct PC_Resource info);
static void extension_3_update(struct PC_Resource info);
static void extension_4_update(struct PC_Resource info);
static void extension_5_update(struct PC_Resource info);

// 组件
static sensor_module smod[4] = {0};

// 各种样式
static lv_style_t default_style;        // 默认样式
static lv_style_t grid_cont_style;      // 网格布局容器样式
static lv_style_t grid_cont_cell_style; // 网格布局容器网格对象样式
static lv_style_t arc_style;            // 圆弧小工具样式
static lv_style_t arc_label_style;      // 圆弧内部百分比标签样式
static lv_style_t m_label_style;        // 组件标签样式
static lv_style_t buttom_label_style;   // 组件底部标签样式
static lv_style_t ext_label_style_1;    // 扩展标签样式1
static lv_style_t ext_label_style_2;    // 扩展标签样式2
static lv_style_t ext_label_style_3;    // 扩展标签样式3

// 界面布局容器
static lv_obj_t *cont = NULL;

// 网格布局行cell
static lv_coord_t col_dsc[] = {114, 114, LV_GRID_TEMPLATE_LAST};

// 网格布局列cell
static lv_coord_t row_dsc[] = {114, 114, LV_GRID_TEMPLATE_LAST};

// 当前活动页
static lv_obj_t *scr = NULL;

// 目前支持的界面组件
static const char *module_name[4] = {"CPU", "GPU", "RAM", "EXT"};

// 组件底部标签默认格式文本
static const char *mb_fmt[4] = {"%d.%d℃", "%d.%d℃", "%dMB", NULL};

/**
 * @brief UI样式显示初始化
 */
void display_pc_resource_gui_init(void)
{
    lv_style_init(&default_style);
    lv_style_set_bg_color(&default_style, lv_color_hex(0x000000)); // 背景黑色

    lv_style_init(&grid_cont_style);
    lv_style_set_bg_color(&grid_cont_style, lv_color_hex(0x000000)); // 背景黑色
    lv_style_set_pad_all(&grid_cont_style, 0);                       // 网格容器四周填充宽度为0
    lv_style_set_outline_width(&grid_cont_style, 0);                 // 网格容器四周外部线条宽度为0
    lv_style_set_border_width(&grid_cont_style, 0);                  // 网格容器四周边框宽度为0
    lv_style_set_pad_row(&grid_cont_style, 3);
    lv_style_set_pad_column(&grid_cont_style, 3);

    lv_style_init(&grid_cont_cell_style);
    lv_style_set_bg_color(&grid_cont_cell_style, lv_color_hex(0x000000));     // 背景黑色
    lv_style_set_border_color(&grid_cont_cell_style, lv_color_hex(0x00ffff)); // 边框颜色
    lv_style_set_border_opa(&grid_cont_cell_style, LV_OPA_70);                // 边框透明度
    lv_style_set_pad_all(&grid_cont_cell_style, 0);                           // 网格cell四周填充宽度为0
    lv_style_set_outline_width(&grid_cont_cell_style, 0);                     // 网格cell四周外部线条宽度为0

    lv_style_init(&arc_style);

    lv_style_init(&arc_label_style);
    lv_style_set_bg_color(&arc_label_style, lv_color_hex(0x000000));   // 背景黑色
    lv_style_set_text_font(&arc_label_style, &lv_font_ibmplex_24);     // 设置字体
    lv_style_set_text_opa(&arc_label_style, LV_OPA_90);                // 字体透明度
    lv_style_set_text_color(&arc_label_style, lv_color_hex(0x00ffff)); // 字体颜色

    lv_style_init(&m_label_style);
    lv_style_set_bg_color(&m_label_style, lv_color_hex(0x000000));   // 背景黑色
    lv_style_set_text_font(&m_label_style, &lv_font_ibmplex_18);     // 设置字体
    lv_style_set_text_opa(&m_label_style, LV_OPA_90);                // 字体透明度
    lv_style_set_text_color(&m_label_style, lv_color_hex(0x00ffff)); // 字体颜色

    lv_style_init(&buttom_label_style);
    lv_style_set_bg_color(&buttom_label_style, lv_color_hex(0x000000));   // 背景黑色
    lv_style_set_text_font(&buttom_label_style, &lv_font_ibmplex_16);     // 设置字体
    lv_style_set_text_opa(&buttom_label_style, LV_OPA_90);                // 字体透明度
    lv_style_set_text_color(&buttom_label_style, lv_color_hex(0x00ff00)); // 字体颜色

    lv_style_init(&ext_label_style_1);
    lv_style_set_bg_color(&ext_label_style_1, lv_color_hex(0x000000));   // 背景黑色
    lv_style_set_text_font(&ext_label_style_1, &lv_font_ibmplex_16);     // 设置字体
    lv_style_set_text_opa(&ext_label_style_1, LV_OPA_90);                // 字体透明度
    lv_style_set_text_color(&ext_label_style_1, lv_color_hex(0xff00ff)); // 字体颜色

    lv_style_init(&ext_label_style_2);
    lv_style_set_bg_color(&ext_label_style_2, lv_color_hex(0x000000));   // 背景黑色
    lv_style_set_text_font(&ext_label_style_2, &lv_font_ibmplex_16);     // 设置字体
    lv_style_set_text_opa(&ext_label_style_2, LV_OPA_90);                // 字体透明度
    lv_style_set_text_color(&ext_label_style_2, lv_color_hex(0xffffff)); // 字体颜色

    lv_style_init(&ext_label_style_3);
    lv_style_set_bg_color(&ext_label_style_3, lv_color_hex(0x000000));   // 背景黑色
    lv_style_set_text_font(&ext_label_style_3, &lv_font_ibmplex_16);     // 设置字体
    lv_style_set_text_opa(&ext_label_style_3, LV_OPA_90);                // 字体透明度
    lv_style_set_text_color(&ext_label_style_3, lv_color_hex(0xff0000)); // 字体颜色
}

/**
 * @brief UI界面显示初始化
 */
void display_pc_resource_init(void)
{
    lv_obj_t *act_obj = lv_scr_act(); // 获取当前活动页
    if (act_obj == scr)
        return;

    pc_resource_gui_release();
    lv_obj_clean(act_obj); // 清空此前页面

    scr = lv_obj_create(NULL);
    lv_obj_add_style(scr, &default_style, LV_STATE_DEFAULT);

    // 创建界面容器
    cont = lv_obj_create(scr);
    lv_obj_add_style(cont, &grid_cont_style, LV_STATE_DEFAULT);
    lv_obj_set_size(cont, 240, 240);
    lv_obj_center(cont);
    lv_obj_set_grid_dsc_array(cont, col_dsc, row_dsc);

    for (int i = 0; i < 4; i++)
    {
        uint8_t col = i % 2;
        uint8_t row = i / 2;

        lv_obj_t *cell = lv_obj_create(cont); // 创建一个cell
        lv_obj_add_style(cell, &grid_cont_cell_style, LV_STATE_DEFAULT);
        lv_obj_set_grid_cell(cell, LV_GRID_ALIGN_STRETCH, col, 1, LV_GRID_ALIGN_STRETCH, row, 1); // 行列cell缩放比例为1

        smod[i].name = module_name[i];
        if (strcmp(smod[i].name, "EXT")) // 非扩展组件
        {
            // 创建利用率圆弧
            lv_obj_t *arc = lv_arc_create(cell);
            lv_obj_set_size(arc, 80, 80);
            lv_obj_align(arc, LV_ALIGN_TOP_RIGHT, 0, 0);
            lv_obj_remove_style(arc, NULL, LV_PART_KNOB);  // 移除指示器部件样式
            lv_obj_clear_flag(arc, LV_OBJ_FLAG_CLICKABLE); // 不可点击
            lv_arc_set_rotation(arc, 270);                 // 0度位置旋转到12点位置
            lv_arc_set_range(arc, 0, 100);                 // 设置范围0~100
            lv_arc_set_bg_angles(arc, 0, 360);             // 弧形360°
            lv_arc_set_value(arc, 50);
            lv_obj_add_style(arc, &arc_style, LV_STATE_DEFAULT);

            // 创建圆弧内百分比标签
            lv_obj_t *arc_label = lv_label_create(arc);
            lv_obj_add_style(arc_label, &arc_label_style, LV_STATE_DEFAULT); // 添加样式
            lv_label_set_text_fmt(arc_label, "%d", 50);
            lv_obj_center(arc_label); // 居中对齐

            // 创建组件标签
            lv_obj_t *m_label = lv_label_create(cell);
            lv_obj_set_width(m_label, 23);
            lv_obj_add_style(m_label, &m_label_style, LV_STATE_DEFAULT); // 添加样式
            lv_label_set_text_fmt(m_label, "%s", module_name[i]);
            lv_obj_align(m_label, LV_ALIGN_TOP_LEFT, 5, -5); // 设置对齐

            // 创建组件底部标签
            lv_obj_t *b_label = lv_label_create(cell);
            lv_obj_add_style(b_label, &buttom_label_style, LV_STATE_DEFAULT); // 添加样式
            lv_label_set_text_fmt(b_label, mb_fmt[i], 35, 4);
            lv_obj_align(b_label, LV_ALIGN_BOTTOM_RIGHT, -5, -2); // 设置对齐

            smod[i].usage_rate_arc = arc;
            smod[i].usage_rate_label = arc_label;
            smod[i].module_label = m_label;
            smod[i].buttom_label = b_label;
        }
        else // 扩展组件，只使用组件的扩展属性
        {
            // 创建网络上行速率标签
            lv_obj_t *net_up_label = lv_label_create(cell);
            lv_obj_set_width(net_up_label, 114);                                  // 设定标签宽度以便于长文本模式滚动
            lv_label_set_long_mode(net_up_label, LV_LABEL_LONG_SCROLL);           // 长文本模式
            lv_obj_add_style(net_up_label, &ext_label_style_1, LV_STATE_DEFAULT); // 添加样式
            lv_label_set_text_fmt(net_up_label, "\uf093%d.%dKB/s", 12346, 5);
            lv_obj_align(net_up_label, LV_ALIGN_TOP_LEFT, 0, 0); // 设置对齐

            // 创建网络下行速率标签
            lv_obj_t *net_down_label = lv_label_create(cell);
            lv_obj_set_width(net_down_label, 114);                                  // 设定标签宽度以便于长文本模式滚动
            lv_label_set_long_mode(net_down_label, LV_LABEL_LONG_SCROLL);           // 长文本模式
            lv_obj_add_style(net_down_label, &ext_label_style_1, LV_STATE_DEFAULT); // 添加样式
            lv_label_set_text_fmt(net_down_label, "\uf019%d.%dKB/s", 22441, 5);
            lv_obj_align_to(net_down_label, net_up_label, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 0); // 设置对齐

            // 创建CPU主频标签
            lv_obj_t *cpu_clock_label = lv_label_create(cell);
            lv_obj_set_width(cpu_clock_label, 114);                                  // 设定标签宽度以便于长文本模式滚动
            lv_label_set_long_mode(cpu_clock_label, LV_LABEL_LONG_SCROLL);           // 长文本模式
            lv_obj_add_style(cpu_clock_label, &ext_label_style_2, LV_STATE_DEFAULT); // 添加样式
            lv_label_set_text_fmt(cpu_clock_label, "\uf2db%dMHz", 4211);
            lv_obj_align_to(cpu_clock_label, net_down_label, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 0); // 设置对齐

            // 创建CPU功耗标签
            lv_obj_t *cpu_power_label = lv_label_create(cell);
            lv_obj_set_width(cpu_power_label, 114);                                  // 设定标签宽度以便于长文本模式滚动
            lv_label_set_long_mode(cpu_power_label, LV_LABEL_LONG_SCROLL);           // 长文本模式
            lv_obj_add_style(cpu_power_label, &ext_label_style_2, LV_STATE_DEFAULT); // 添加样式
            lv_label_set_text_fmt(cpu_power_label, "\uf2db%d.%dW", 21, 4);
            lv_obj_align_to(cpu_power_label, cpu_clock_label, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 0); // 设置对齐

            // 创建GPU功耗标签
            lv_obj_t *gpu_power_label = lv_label_create(cell);
            lv_obj_set_width(gpu_power_label, 114);                                  // 设定标签宽度以便于长文本模式滚动
            lv_label_set_long_mode(gpu_power_label, LV_LABEL_LONG_SCROLL);           // 长文本模式
            lv_obj_add_style(gpu_power_label, &ext_label_style_3, LV_STATE_DEFAULT); // 添加样式
            lv_label_set_text_fmt(gpu_power_label, "\uf109%d.%dW", 124, 8);
            lv_obj_align_to(gpu_power_label, cpu_power_label, LV_ALIGN_OUT_BOTTOM_LEFT, 0, -2); // 设置对齐

            smod[i].extension_label_1 = net_up_label;    // 扩展属性1->net上行速率
            smod[i].extension_label_2 = net_down_label;  // 扩展属性2->net下行速率
            smod[i].extension_label_3 = cpu_clock_label; // 扩展属性3->CPU主频
            smod[i].extension_label_4 = cpu_power_label; // 扩展属性4->CPU功耗
            smod[i].extension_label_5 = gpu_power_label; // 扩展属性5->GPU功耗

            // 设置扩展更新函数
            smod[i].extension_update[0] = extension_1_update;
            smod[i].extension_update[1] = extension_2_update;
            smod[i].extension_update[2] = extension_3_update;
            smod[i].extension_update[3] = extension_4_update;
            smod[i].extension_update[4] = extension_5_update;
        }
    }
}

/**
 * @brief UI界面数据更新
 * @param sensorInfo 传感器数据
 */
void display_pc_resource(struct PC_Resource sensorInfo)
{
    display_pc_resource_init();

    // 更新CPU组件
    lv_arc_set_value(smod[MODULE_CPU].usage_rate_arc, sensorInfo.cpu_usage); // CPU利用率
    lv_label_set_text_fmt(smod[MODULE_CPU].usage_rate_label, "%d", sensorInfo.cpu_usage);
    lv_label_set_text_fmt(smod[MODULE_CPU].buttom_label, mb_fmt[MODULE_CPU],
                          sensorInfo.cpu_temp / 10, sensorInfo.cpu_temp % 10); // CPU温度

    // 更新GPU组件
    lv_arc_set_value(smod[MODULE_GPU].usage_rate_arc, sensorInfo.gpu_usage); // GPU利用率
    lv_label_set_text_fmt(smod[MODULE_GPU].usage_rate_label, "%d", sensorInfo.gpu_usage);
    lv_label_set_text_fmt(smod[MODULE_GPU].buttom_label, mb_fmt[MODULE_GPU],
                          sensorInfo.gpu_temp / 10, sensorInfo.gpu_temp % 10); // GPU温度

    // 更新RAM组件
    lv_arc_set_value(smod[MODULE_RAM].usage_rate_arc, sensorInfo.ram_usage); // RAM利用率
    lv_label_set_text_fmt(smod[MODULE_RAM].usage_rate_label, "%d", sensorInfo.ram_usage);
    lv_label_set_text_fmt(smod[MODULE_RAM].buttom_label, mb_fmt[MODULE_RAM],
                          sensorInfo.ram_use); // RAM已使用量

    // 更新扩展组件
    for (int i = 0; i < MAX_EXTENSION_NUM; i++)
    {
        if (smod[MODULE_EXT].extension_update[i] != NULL)
            smod[MODULE_EXT].extension_update[i](sensorInfo);
    }

    lv_scr_load(scr);
}

/**
 * @brief UI界面对象释放
 */
void pc_resource_gui_release(void)
{
    if (scr != NULL)
    {
        lv_obj_clean(scr);

        scr = NULL;
        cont = NULL;
        memset(smod, 0, sizeof(smod));
    }
}

/**
 * @brief 扩展对象1数据更新函数 -> net上行速率
 * @param sensorInfo 传感器数据
 */
static void extension_1_update(struct PC_Resource info)
{
    int num = info.net_upload_speed / 10;
    int mini_num = info.net_upload_speed % 10;

    lv_label_set_text_fmt(smod[MODULE_EXT].extension_label_1, "\uf093%d.%dKB/s", num, mini_num);
}

/**
 * @brief 扩展对象2数据更新函数 -> net下行速率
 * @param sensorInfo 传感器数据
 */
static void extension_2_update(struct PC_Resource info)
{
    int num = info.net_download_speed / 10;
    int mini_num = info.net_download_speed % 10;

    lv_label_set_text_fmt(smod[MODULE_EXT].extension_label_2, "\uf019%d.%dKB/s", num, mini_num);
}

/**
 * @brief 扩展对象3数据更新函数 -> CPU主频
 * @param sensorInfo 传感器数据
 */
static void extension_3_update(struct PC_Resource info)
{
    lv_label_set_text_fmt(smod[MODULE_EXT].extension_label_3, "\uf2db%dMHz", info.cpu_freq);
}

/**
 * @brief 扩展对象4数据更新函数 -> CPU功耗
 * @param sensorInfo 传感器数据
 */
static void extension_4_update(struct PC_Resource info)
{
    int num = info.cpu_power / 10;
    int mini_num = info.cpu_power % 10;

    lv_label_set_text_fmt(smod[MODULE_EXT].extension_label_4, "\uf2db%d.%dW", num, mini_num);
}

/**
 * @brief 扩展对象5数据更新函数 -> GPU功耗
 * @param sensorInfo 传感器数据
 */
static void extension_5_update(struct PC_Resource info)
{
    int num = info.gpu_power / 10;
    int mini_num = info.gpu_power % 10;

    lv_label_set_text_fmt(smod[MODULE_EXT].extension_label_5, "\uf109%d.%dW", num, mini_num);
}
