## Note: LVGL+LTDC+DMA2D+双缓冲例程

1. 在 lvgl 刷新函数中修改 ltdc 帧缓冲区地址，注意，修改帧缓冲地址要在消隐期间更新，否则会产生撕裂。

   ```c
   static void disp_flush(lv_disp_drv_t *disp_drv, const lv_area_t *area, lv_color_t *color_p) {
       if (disp_flush_enabled) {
           /*The most simple case (but also the slowest) to put all pixels to the screen one-by-one*/
           bsp_ltdc_set_framebuf((uint32_t)color_p);
           
       }
   }
   
   void bsp_ltdc_set_framebuf(uint32_t fbuf) {
       LTDC_Layer1->CFBAR = fbuf;  /* Set Layer1 frame buffer address */
       LTDC->IER |= LTDC_IT_RR;    /* Enable Reload interrupt */
       LTDC->SRCR = LTDC_RELOAD_VERTICAL_BLANKING;     /* Apply Reload type */
   }
   ```

2. 在 wait_fb 中等待 Reload完成，并通知 lvgl 刷新完成，rtos 中可以用信号量进行同步。

   ```c
   static void disp_wait_flush(lv_disp_drv_t *disp_drv) {
       bsp_ltdc_wait_reload();
       lv_disp_flush_ready(disp_drv);
   }
   
   void bsp_ltdc_wait_reload(void) {
       xSemaphoreTake(ltdc_sync, portMAX_DELAY);
   }
   
   void bsp_ltdc_reload_event_callback(void) {
       BaseType_t xHigherPriorityTaskWoken;
       xSemaphoreGiveFromISR(ltdc_sync, &xHigherPriorityTaskWoken);
       portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
   }
   ```

3. 如果使用单缓冲，可以使能行中断，在消隐期间用DMA2D刷新帧缓冲区，参考乐鑫ESP32S3 RGB例程。

   在 lvgl 需要进行拷贝时，先利用信号量等待帧 (vsync) 事件，当 vsync 事件产生并中断时，此时LCD控制器正好刷到垂直消隐区后沿(VBP)之前，这时再发送信号量令 lvgl 开始进行数据拷贝，从而达到消除撕裂的效果。

   ```c
   static bool example_on_vsync_event(esp_lcd_panel_handle_t panel, const esp_lcd_rgb_panel_event_data_t *event_data, void *user_data)
   {
       BaseType_t high_task_awoken = pdFALSE;
   #if CONFIG_EXAMPLE_AVOID_TEAR_EFFECT_WITH_SEM
       if (xSemaphoreTakeFromISR(sem_gui_ready, &high_task_awoken) == pdTRUE) {
           xSemaphoreGiveFromISR(sem_vsync_end, &high_task_awoken); // 发送帧同步信号量
       }
   #endif
       return high_task_awoken == pdTRUE;
   }
   
   static void example_lvgl_flush_cb(lv_disp_drv_t *drv, const lv_area_t *area, lv_color_t *color_map)
   {
       esp_lcd_panel_handle_t panel_handle = (esp_lcd_panel_handle_t) drv->user_data;
       int offsetx1 = area->x1;
       int offsetx2 = area->x2;
       int offsety1 = area->y1;
       int offsety2 = area->y2;
   #if CONFIG_EXAMPLE_AVOID_TEAR_EFFECT_WITH_SEM
       xSemaphoreGive(sem_gui_ready);
       xSemaphoreTake(sem_vsync_end, portMAX_DELAY);	// 等待帧同步信号量
   #endif
       // pass the draw buffer to the driver
       esp_lcd_panel_draw_bitmap(panel_handle, offsetx1, offsety1, offsetx2 + 1, offsety2 + 1, color_map);
       lv_disp_flush_ready(drv);
   }
   ```

4. 如果画面出现抖动，可能是因为 SDRAM 带宽不足，可以尝试将 LTDC 时钟频率降低。

​		Note:

​		LTDC 刷新率 计算：LTDC Clock Freq / （（Width + HSYNC_W + HBP + HFP）* (Height + VSYNC_W + VBP + VFP））

​		LTDC 带宽     计算：LTDC 刷新率 * （Width * Height * bytesPerPixel）