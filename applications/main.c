/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-11-06     SummerGift   first version
 * 2019-04-09     WillianChan  add stm32f469-st-disco bsp
 */

#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>
#include "lvgl.h"

/* defined the LED1 pin: PG6 */
#define LED1_PIN    GET_PIN(G, 6)
#define MAIN_BG_COLOR lv_color_make(0x44, 0x44, 0x44)
#define GRAD_BG_COLOR lv_color_make(0x44, 0x44, 0x44)

static const char * btnm_default[] = {"MENU", "SET V", "SET I", "OVP/OCP", ""};

static lv_chart_cursor_t* cursor[2];
static lv_obj_t* chart;
lv_chart_series_t * serie_current;
lv_chart_series_t * serie_voltage;

#ifndef MIN
#define MIN(a,b) ((a) < (b) ? (a) : (b))
#endif

#ifndef MAX
#define MAX(a,b) ((a) > (b) ? (a) : (b))
#endif

static void update_stat()
{
    // TODO: calculate stats on selection and update GUI
}

static void chart_event_cb(lv_event_t * e)
{
    static rt_tick_t last_spress = 0;
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t* selectArea = lv_event_get_user_data(e);
    if(code == LV_EVENT_PRESSED)
    {
        lv_indev_t * indev = lv_indev_get_act();
        if(indev == NULL)  return;

        lv_point_t point;
        lv_indev_get_point(indev, &point);

        lv_chart_set_cursor_pos(chart, cursor[0], &point);
    }
    else if(code == LV_EVENT_PRESSING)
    {
        lv_indev_t * indev = lv_indev_get_act();
        if(indev == NULL)  return;

        lv_point_t point;
        lv_indev_get_point(indev, &point);
        point.y=0;

        lv_chart_set_cursor_pos(chart, cursor[1], &point);
    }
    else if(code == LV_EVENT_RELEASED)
    {
        lv_indev_t * indev = lv_indev_get_act();
        if(indev == NULL)  return;

        lv_point_t point;
        lv_indev_get_point(indev, &point);
        point.y=10;

        lv_chart_set_cursor_pos(chart, cursor[1], &point);

        lv_obj_set_size(selectArea, abs(cursor[1]->pos.x - cursor[0]->pos.x), lv_obj_get_content_height(chart));
        lv_obj_align_to(selectArea, chart, LV_ALIGN_TOP_LEFT, MIN(cursor[0]->pos.x, cursor[1]->pos.x), 0);
        lv_obj_clear_flag(selectArea, LV_OBJ_FLAG_HIDDEN);

        update_stat();
    }
    else if(code == LV_EVENT_SHORT_CLICKED)
    {
        rt_tick_t now = rt_tick_get();
        if((now - last_spress) < 50)
        {
            lv_point_t p;
            p.x = -10;
            p.y = -10;
            lv_chart_set_cursor_pos(chart, cursor[0], &p);
            lv_chart_set_cursor_pos(chart, cursor[1], &p);
            lv_obj_add_flag(selectArea, LV_OBJ_FLAG_HIDDEN);
        }
        last_spress = now;
    }
}

void lv_user_gui_init(void)
{
    lv_obj_t* mainScreen = lv_scr_act();
    lv_obj_set_style_bg_color(mainScreen, MAIN_BG_COLOR, LV_PART_MAIN);
    lv_obj_set_style_bg_grad_color(mainScreen, GRAD_BG_COLOR, LV_PART_MAIN);

    lv_obj_t* statusBar = lv_label_create(mainScreen);
    lv_obj_set_style_bg_opa(statusBar, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_border_opa(statusBar, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_border_width(statusBar, 1, LV_PART_MAIN);
    lv_obj_set_style_border_side(statusBar, LV_BORDER_SIDE_BOTTOM, LV_PART_MAIN);
    lv_obj_set_style_pad_all(statusBar, 2, LV_PART_MAIN);
    lv_obj_set_style_border_color(statusBar, lv_color_make(0x80, 0x80, 0x80), LV_PART_MAIN);
    lv_obj_set_size(statusBar, LV_HOR_RES, 32);
    lv_obj_set_style_bg_color(statusBar, lv_color_make(0x30, 0x30, 0x30), LV_PART_MAIN);
    lv_obj_set_style_bg_grad_color(statusBar, lv_color_make(0x16, 0x16, 0x16), LV_PART_MAIN);
    lv_obj_align(statusBar, LV_ALIGN_TOP_MID, 0, 0);
    lv_label_set_text(statusBar, "");

    lv_obj_t* statusBarTitle = lv_label_create(statusBar);
    lv_obj_set_style_text_font(statusBarTitle, &lv_font_montserrat_24, LV_PART_MAIN);
    lv_obj_set_style_text_color(statusBarTitle, lv_color_white(), LV_PART_MAIN);
    lv_obj_set_style_text_align(statusBarTitle, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN);
    lv_label_set_text(statusBarTitle, "Power Supply Profiler");

    lv_obj_t* statusBarIcons = lv_label_create(statusBar);
    lv_label_set_recolor(statusBarIcons, true);
    lv_obj_set_style_text_font(statusBarIcons, &lv_font_montserrat_24, LV_PART_MAIN);
    lv_obj_set_style_text_color(statusBarIcons, lv_color_white(), LV_PART_MAIN);
    lv_obj_set_style_text_align(statusBarIcons, LV_TEXT_ALIGN_RIGHT, LV_PART_MAIN);
    lv_label_set_text(statusBarIcons, "#FF3010 "LV_SYMBOL_WARNING"# ""#3050FF "LV_SYMBOL_BLUETOOTH"# "LV_SYMBOL_WIFI" "LV_SYMBOL_BATTERY_2 "#FFFF00  "LV_SYMBOL_CHARGE"#");
    lv_obj_align(statusBarIcons, LV_ALIGN_RIGHT_MID, 0, 0);


    lv_obj_t* buttonBar = lv_obj_create(mainScreen);
    lv_obj_set_size(buttonBar, LV_HOR_RES - 1, 48);
    lv_obj_align(buttonBar, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_set_style_radius(buttonBar, 0, LV_PART_MAIN);
    lv_obj_set_style_border_width(buttonBar, 1, LV_PART_MAIN);
    lv_obj_set_style_border_color(buttonBar, lv_color_make(0x80, 0x80, 0x80), LV_PART_MAIN);
    lv_obj_set_style_border_side(buttonBar, LV_BORDER_SIDE_TOP, LV_PART_MAIN);
    lv_obj_set_style_bg_color(buttonBar, lv_color_make(0x30, 0x30, 0x30), LV_PART_MAIN);
    lv_obj_set_style_bg_grad_color(buttonBar, lv_color_make(0x16, 0x16, 0x16), LV_PART_MAIN);
    lv_obj_set_style_pad_all(buttonBar, 0, LV_PART_MAIN);
    lv_obj_set_scrollbar_mode(buttonBar, LV_SCROLLBAR_MODE_OFF);


    lv_obj_t* buttons[4];

    for(int i = 0; i < 4; i++)
    {
        buttons[i] = lv_btn_create(buttonBar);
        lv_obj_set_size(buttons[i], LV_HOR_RES/4 - 1, 48);
        lv_obj_align(buttons[i], LV_ALIGN_BOTTOM_LEFT, i * (LV_HOR_RES / 4 - 1), 0);
        lv_obj_set_style_radius(buttons[i], 0, LV_PART_MAIN);
        lv_obj_set_style_bg_color(buttons[i], lv_color_make(0x30, 0x30, 0x30), LV_PART_MAIN);
        lv_obj_set_style_bg_grad_color(buttons[i], lv_color_make(0x16, 0x16, 0x16), LV_PART_MAIN);
        lv_obj_set_style_bg_grad_dir(buttons[i], LV_GRAD_DIR_VER, LV_PART_MAIN);
        lv_obj_set_style_border_color(buttons[i], lv_color_make(0x80, 0x80, 0x80), LV_PART_MAIN);
        lv_obj_set_style_border_width(buttons[i], 1, LV_PART_MAIN);
        lv_obj_set_style_border_side(buttons[i], i != 0 ? LV_BORDER_SIDE_LEFT : LV_BORDER_SIDE_NONE, LV_PART_MAIN);
        lv_obj_set_style_border_opa(buttons[i], LV_OPA_COVER, LV_PART_MAIN);


        static lv_style_t style_pr;
        lv_style_init(&style_pr);
        lv_style_set_bg_color(&style_pr, lv_color_make(0x60, 0x60, 0x60));
        lv_style_set_bg_grad_color(&style_pr, lv_color_make(0x37, 0x62, 0x90));
        lv_style_set_bg_grad_dir(&style_pr, LV_GRAD_DIR_VER);
        lv_obj_remove_style(buttons[i], NULL, LV_STATE_PRESSED);
        lv_obj_add_style(buttons[i], &style_pr, LV_STATE_PRESSED);

        lv_obj_t* label = lv_label_create(buttons[i]);
        lv_obj_set_style_text_font(label, &lv_font_montserrat_36, LV_PART_MAIN);
        lv_obj_set_style_text_color(label, lv_color_white(), LV_PART_MAIN);
        lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
        lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);
        lv_label_set_text(label, btnm_default[i]);
    }

    /*Create a chart*/
    chart = lv_chart_create(mainScreen);
    lv_obj_set_size(chart, LV_HOR_RES, LV_VER_RES - 48 - 32 - 64);
    lv_obj_align_to(chart, statusBar, LV_ALIGN_OUT_BOTTOM_RIGHT, 0, 0);
    lv_chart_set_type(chart, LV_CHART_TYPE_LINE);
    lv_obj_set_style_radius(chart, 0, LV_PART_MAIN);
    lv_obj_set_style_border_side(chart, LV_BORDER_SIDE_NONE, LV_PART_MAIN);
    lv_obj_set_style_bg_color(chart, lv_color_black(), LV_PART_MAIN);
    lv_obj_set_style_bg_grad_color(chart, lv_color_black(), LV_PART_MAIN);
    lv_obj_set_style_line_color(chart, lv_color_make(0x44,0x44,0x44), LV_PART_MAIN);
    lv_obj_set_style_pad_all(chart, 0, LV_PART_MAIN);
    lv_obj_set_style_width(chart, 0, LV_PART_INDICATOR);
    lv_chart_set_div_line_count(chart, 8, 9);
    lv_chart_set_point_count(chart, LV_HOR_RES);
    lv_chart_set_range(chart, LV_CHART_AXIS_PRIMARY_Y, 0, lv_obj_get_height(chart));
    lv_chart_set_range(chart, LV_CHART_AXIS_SECONDARY_Y, 0, lv_obj_get_height(chart));


    lv_obj_t * chartAxisLabel = lv_label_create(chart);
    lv_obj_set_style_text_font(chartAxisLabel, &lv_font_montserrat_16, LV_PART_MAIN);
    lv_obj_set_style_text_opa(chartAxisLabel, LV_OPA_80, LV_PART_MAIN);
    lv_obj_set_style_text_color(chartAxisLabel, lv_color_make(0x00, 0xFF, 0xFF), LV_PART_MAIN);

    lv_label_set_text(chartAxisLabel, "1A");
    lv_obj_align(chartAxisLabel, LV_ALIGN_TOP_LEFT, 3, 0);

    chartAxisLabel = lv_label_create(chartAxisLabel);
    lv_label_set_text(chartAxisLabel, "100mA");
    lv_obj_align_to(chartAxisLabel, chart, LV_ALIGN_TOP_LEFT, 3, lv_obj_get_height(chart)/7);

    chartAxisLabel = lv_label_create(chartAxisLabel);
    lv_label_set_text(chartAxisLabel, "10mA");
    lv_obj_align_to(chartAxisLabel, chart, LV_ALIGN_TOP_LEFT, 3, 2*lv_obj_get_height(chart)/7);

    chartAxisLabel = lv_label_create(chartAxisLabel);
    lv_label_set_text(chartAxisLabel, "1mA");
    lv_obj_align_to(chartAxisLabel, chart, LV_ALIGN_TOP_LEFT, 3, 3*lv_obj_get_height(chart)/7);

    chartAxisLabel = lv_label_create(chartAxisLabel);
    lv_label_set_text(chartAxisLabel, "100uA");
    lv_obj_align_to(chartAxisLabel, chart, LV_ALIGN_TOP_LEFT, 3, 4*lv_obj_get_height(chart)/7);

    chartAxisLabel = lv_label_create(chartAxisLabel);
    lv_label_set_text(chartAxisLabel, "10uA");
    lv_obj_align_to(chartAxisLabel, chart, LV_ALIGN_TOP_LEFT, 3, 5*lv_obj_get_height(chart)/7);

    chartAxisLabel = lv_label_create(chartAxisLabel);
    lv_label_set_text(chartAxisLabel, "1uA");
    lv_obj_align_to(chartAxisLabel, chart, LV_ALIGN_TOP_LEFT, 3, 6*lv_obj_get_height(chart)/7);



    chartAxisLabel = lv_label_create(chart);
    lv_obj_set_style_text_font(chartAxisLabel, &lv_font_montserrat_16, LV_PART_MAIN);
    lv_obj_set_style_text_opa(chartAxisLabel, LV_OPA_80, LV_PART_MAIN);
    lv_obj_set_style_text_color(chartAxisLabel, lv_color_make(0xFF, 0xA5, 0x00), LV_PART_MAIN);

    lv_label_set_text(chartAxisLabel, "14V");
    lv_obj_align(chartAxisLabel, LV_ALIGN_TOP_RIGHT, -3, 0);

    chartAxisLabel = lv_label_create(chartAxisLabel);
    lv_label_set_text(chartAxisLabel, "12V");
    lv_obj_align_to(chartAxisLabel, chart, LV_ALIGN_TOP_RIGHT, -3, lv_obj_get_height(chart)/7);

    chartAxisLabel = lv_label_create(chartAxisLabel);
    lv_label_set_text(chartAxisLabel, "10V");
    lv_obj_align_to(chartAxisLabel, chart, LV_ALIGN_TOP_RIGHT, -3, 2*lv_obj_get_height(chart)/7);

    chartAxisLabel = lv_label_create(chartAxisLabel);
    lv_label_set_text(chartAxisLabel, "8V");
    lv_obj_align_to(chartAxisLabel, chart, LV_ALIGN_TOP_RIGHT, -3, 3*lv_obj_get_height(chart)/7);

    chartAxisLabel = lv_label_create(chartAxisLabel);
    lv_label_set_text(chartAxisLabel, "6V");
    lv_obj_align_to(chartAxisLabel, chart, LV_ALIGN_TOP_RIGHT, -3, 4*lv_obj_get_height(chart)/7);

    chartAxisLabel = lv_label_create(chartAxisLabel);
    lv_label_set_text(chartAxisLabel, "4V");
    lv_obj_align_to(chartAxisLabel, chart, LV_ALIGN_TOP_RIGHT, -3, 5*lv_obj_get_height(chart)/7);

    chartAxisLabel = lv_label_create(chartAxisLabel);
    lv_label_set_text(chartAxisLabel, "2V");
    lv_obj_align_to(chartAxisLabel, chart, LV_ALIGN_TOP_RIGHT, -3, 6*lv_obj_get_height(chart)/7);


    serie_current = lv_chart_add_series(chart, lv_color_make(0x00, 0xFF, 0xFF), LV_CHART_AXIS_PRIMARY_Y);
    serie_voltage = lv_chart_add_series(chart, lv_color_make(0xFF, 0xA5, 0x00), LV_CHART_AXIS_SECONDARY_Y);

    for(int i = 0; i < LV_HOR_RES; i++)
    {
        //        lv_chart_set_next_value(chart, serie_current, sin((double)(2*3.14*10*i/LV_HOR_RES)) * 20 + lv_obj_get_height(chart)/2);
        lv_chart_set_next_value(chart, serie_current, 120   + (rand() % 10)
                + sin(2*3.14*i*0.01)*12
                + sin(2*3.14*i*0.052)*15
                //                + sin(2*3.14*i*0.34)*24
                + (((int)(i+50)%100)<10?45:0)
                + (((int)(i+100)%250)<24?80:0));
        lv_chart_set_next_value(chart, serie_voltage, rand()%10 + 5*lv_obj_get_height(chart)/8 - (((int)(i+50)%100)<10?4:0) - (((int)(i+100)%250)<10?10:0));
    }

    lv_obj_t* statArea = lv_obj_create(mainScreen);
    lv_obj_set_size(statArea, LV_HOR_RES/2, 64);
    lv_obj_align_to(statArea, chart, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 0);
    lv_obj_set_style_radius(statArea, 0, LV_PART_MAIN);
    lv_obj_set_style_border_side(statArea, LV_BORDER_SIDE_NONE, LV_PART_MAIN);
    lv_obj_set_style_bg_color(statArea, lv_color_make(0x30, 0x30, 0x30), LV_PART_MAIN);
    lv_obj_set_style_bg_grad_color(statArea, lv_color_make(0x16, 0x16, 0x16), LV_PART_MAIN);
    lv_obj_set_style_pad_all(statArea, 0, LV_PART_MAIN);


    lv_obj_t* statLabelTime = lv_label_create(statArea);
    lv_obj_set_style_text_font(statLabelTime, &lv_font_montserrat_20, LV_PART_MAIN);
    lv_obj_set_style_text_color(statLabelTime, lv_color_make(0x80, 0x80, 0x80), LV_PART_MAIN);
    lv_label_set_recolor(statLabelTime, 1);
    lv_obj_align_to(statLabelTime, statArea, LV_ALIGN_TOP_LEFT, 0, 0);
    lv_label_set_text(statLabelTime, "Session time : #ffffff 12:43#");

    lv_obj_t* statLabelMillis = lv_label_create(statLabelTime);
    lv_label_set_recolor(statLabelMillis, 1);
    lv_obj_set_style_text_font(statLabelMillis, &lv_font_montserrat_14, LV_PART_MAIN);
    lv_obj_align_to(statLabelMillis, statLabelTime, LV_ALIGN_OUT_RIGHT_BOTTOM, 0, 0);
    lv_label_set_text(statLabelMillis, "#ffffff 879#");

    lv_obj_t * calcLabelV = lv_label_create(statLabelTime);
    lv_label_set_recolor(calcLabelV, 1);
    lv_obj_t * calcLabelI = lv_label_create(statLabelTime);
    lv_label_set_recolor(calcLabelI, 1);
    lv_obj_t * calcLabelP = lv_label_create(statLabelTime);
    lv_label_set_recolor(calcLabelP, 1);
    lv_obj_t * calcLabelE = lv_label_create(statLabelTime);
    lv_label_set_recolor(calcLabelE, 1);
    lv_label_set_text(calcLabelE, "#ffffff 898.8#uWh");
    lv_obj_align_to(calcLabelE, statArea, LV_ALIGN_BOTTOM_LEFT, 140, 0);
    lv_label_set_text(calcLabelP, "#ffffff 4.230#mW");
    lv_obj_align_to(calcLabelP, statArea, LV_ALIGN_TOP_LEFT, 140, 20);
    lv_label_set_text(calcLabelI, "#ffffff 472.5#uA");
    lv_obj_align_to(calcLabelI, statArea, LV_ALIGN_BOTTOM_LEFT, 0, 0);
    lv_label_set_text(calcLabelV, "#ffffff 8.952#V");
    lv_obj_align_to(calcLabelV, statArea, LV_ALIGN_TOP_LEFT, 0, 20);

    cursor[0] = lv_chart_add_cursor(chart, lv_color_white(), LV_DIR_BOTTOM | LV_DIR_TOP);
    cursor[1] = lv_chart_add_cursor(chart, lv_color_white(), LV_DIR_BOTTOM | LV_DIR_TOP);
    lv_point_t p;
    p.x = -10;
    p.y = -10;
    lv_chart_set_cursor_pos(chart, cursor[0], &p);
    lv_chart_set_cursor_pos(chart, cursor[1], &p);

    lv_obj_t* selectArea = lv_obj_create(chart);
    lv_obj_set_style_radius(selectArea, 0, LV_PART_MAIN);
    lv_obj_set_style_border_side(selectArea, LV_BORDER_SIDE_NONE, LV_PART_MAIN);
    lv_obj_set_style_bg_color(selectArea, lv_color_white(), LV_PART_MAIN);
    lv_obj_set_style_bg_grad_color(selectArea, lv_color_white(), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(selectArea, LV_OPA_30, LV_PART_MAIN);
    lv_obj_set_style_pad_all(selectArea, 0, LV_PART_MAIN);
    lv_obj_add_flag(selectArea, LV_OBJ_FLAG_HIDDEN);

    lv_obj_add_event_cb(chart, chart_event_cb, LV_EVENT_ALL, (void*)selectArea);
    // pass the event to the chart when we click on the selected area
    lv_obj_add_event_cb(selectArea, chart_event_cb, LV_EVENT_ALL, (void*)selectArea);


    lv_obj_t* outSetArea = lv_obj_create(mainScreen);
    lv_obj_set_size(outSetArea, LV_HOR_RES/2, 64);
    lv_obj_align_to(outSetArea, chart, LV_ALIGN_OUT_BOTTOM_RIGHT, 0, 0);
    lv_obj_set_style_radius(outSetArea, 0, LV_PART_MAIN);
    lv_obj_set_style_border_side(outSetArea, LV_BORDER_SIDE_NONE, LV_PART_MAIN);
    lv_obj_set_style_bg_color(outSetArea, lv_color_make(0x30, 0x30, 0x30), LV_PART_MAIN);
    lv_obj_set_style_bg_grad_color(outSetArea, lv_color_make(0x16, 0x16, 0x16), LV_PART_MAIN);
    lv_obj_set_style_pad_all(outSetArea, 0, LV_PART_MAIN);

    lv_obj_t* outSetLabel = lv_label_create(outSetArea);
    lv_label_set_recolor(outSetLabel, 1);
    lv_obj_set_style_text_color(outSetLabel, lv_color_make(0x80, 0x80, 0x80), LV_PART_MAIN);
    lv_obj_set_style_text_font(outSetLabel, &lv_font_montserrat_48, LV_PART_MAIN);
    lv_obj_align(outSetLabel, LV_ALIGN_LEFT_MID, 0, 0);
    lv_obj_set_style_text_align(outSetLabel, LV_TEXT_ALIGN_RIGHT, LV_PART_MAIN);
    lv_label_set_text(outSetLabel, "#ffffff 9.000#V  #ffffff 0.100#A");
}

int main(void)
{
    /* set LED1 pin mode to output */
    rt_pin_mode(LED1_PIN, PIN_MODE_OUTPUT);

    while (1)
    {
        rt_pin_write(LED1_PIN, PIN_HIGH);
        rt_thread_mdelay(500);
        rt_pin_write(LED1_PIN, PIN_LOW);
        rt_thread_mdelay(500);
    }
}
