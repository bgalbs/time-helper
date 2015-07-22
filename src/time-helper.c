#include "pebble.h"

Window *window;
TextLayer *text_date_layer;
TextLayer *text_time_layer;
TextLayer *text_focus_layer;
//Layer *line_layer;

int currentVibrateTest = -1;
int focusMode = 0;    // if 1, watch is in "focus mode"

uint32_t HOUR_SEGMENTS[]      = { 100, 250, 400 };
uint32_t FIVE_SEGMENTS[]      = { 100 };
uint32_t FIFTEEN_SEGMENTS[]   = { 100, 250, 400, 250, 400 };
uint32_t THIRTY_SEGMENTS[]    = { 100, 250, 400, 250, 400, 250, 400 };
uint32_t FORTYFIVE_SEGMENTS[] = { 100, 250, 400, 250, 400, 250, 400, 250, 400 };

void line_layer_update_callback(Layer *layer, GContext* ctx) {
  graphics_context_set_fill_color(ctx, GColorWhite);
  graphics_fill_rect(ctx, layer_get_bounds(layer), 0, GCornerNone);
}

void down_click_handler(ClickRecognizerRef recognizer, void *context) {
  currentVibrateTest++;

  if (currentVibrateTest > 4) currentVibrateTest = 0;

  if (currentVibrateTest == 0) {
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Five vibration");

    VibePattern pattern = {
      .durations = FIVE_SEGMENTS,
      .num_segments = ARRAY_LENGTH(FIVE_SEGMENTS)
    };
    vibes_enqueue_custom_pattern(pattern);

  } else if (currentVibrateTest == 1) {
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Fifteen vibration");

    VibePattern pattern = {
      .durations = FIFTEEN_SEGMENTS,
      .num_segments = ARRAY_LENGTH(FIFTEEN_SEGMENTS)
    };
    vibes_enqueue_custom_pattern(pattern);

  } else if (currentVibrateTest == 2) {
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Thirty vibration");

    VibePattern pattern = {
      .durations = THIRTY_SEGMENTS,
      .num_segments = ARRAY_LENGTH(THIRTY_SEGMENTS)
    };
    vibes_enqueue_custom_pattern(pattern);

  } else if (currentVibrateTest == 3) {
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Forth-Five vibration");

    VibePattern pattern = {
      .durations = FORTYFIVE_SEGMENTS,
      .num_segments = ARRAY_LENGTH(FORTYFIVE_SEGMENTS)
    };
    vibes_enqueue_custom_pattern(pattern);

  } else {
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Hour vibration");

    VibePattern pattern = {
      .durations = HOUR_SEGMENTS,
      .num_segments = ARRAY_LENGTH(HOUR_SEGMENTS)
    };
    vibes_enqueue_custom_pattern(pattern);

  }

}

void select_click_handler(ClickRecognizerRef recognizer, void *context) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Vibrate the time");

  time_t now = time(NULL);
  struct tm *tick_time = localtime(&now);

  int time_minutes = tick_time->tm_min;
  int time_tens = (time_minutes / 10) % 10;
  int time_ones = time_minutes - (time_tens * 10);

  int total_tens = time_tens;
  if (time_tens > 0) total_tens += (time_tens - 1);

  int total_ones = time_ones;
  if (time_ones > 0) total_ones += (time_ones - 1);

  int total_vibrations = total_tens + ( (total_tens > 0) ? 1 : 0 ) + total_ones; // extra gap in the middle for an extra long delay between minutes places

  APP_LOG(APP_LOG_LEVEL_DEBUG, "Total tens: %d; total ones: %d", total_tens, total_ones);

  uint32_t segments[total_vibrations];

  int i;
  for (i = 0; i < total_tens; i++) {
    segments[i] = 100;
    if ((i + 1) < total_tens) {
      segments[++i] = 200;
    }
  }

  if (total_tens > 0) segments[i++] = 350;

  int z;
  for (z = 0; z < total_ones; z++) {
    segments[i++] = 100;
    if ((z + 1) < total_ones) {
      segments[i++] = 200;
      z++;
    }
  }

  // debug
  // for (i = 0; i < total_vibrations; i++) {
  //   APP_LOG(APP_LOG_LEVEL_DEBUG, "Unit %d: %lu", i, (unsigned long) segments[i]);
  // }

  VibePattern pattern = {
    .durations = segments,
    .num_segments = total_vibrations
  };
  vibes_enqueue_custom_pattern(pattern);
}

void handle_minute_tick(struct tm *tick_time, TimeUnits units_changed) {
  // Need to be static because they're used by the system later.
  static char time_text[] = "00:00";
  static char date_text[] = "Xxxxxxxxx 00";

  char *time_format;

  if (!tick_time) {
    time_t now = time(NULL);
    tick_time = localtime(&now);
  }

  // TODO: Only update the date when it's changed.
  strftime(date_text, sizeof(date_text), "%B %e", tick_time);
  text_layer_set_text(text_date_layer, date_text);


  if (clock_is_24h_style()) {
    time_format = "%R";
  } else {
    time_format = "%I:%M";
  }

  strftime(time_text, sizeof(time_text), time_format, tick_time);

  // Kludge to handle lack of non-padded hour format string
  // for twelve hour clock.
  if (!clock_is_24h_style() && (time_text[0] == '0')) {
    memmove(time_text, &time_text[1], sizeof(time_text) - 1);
  }

  // check if a vibration is necessary and if so, which vibration

  if (focusMode == 1) {
    vibes_long_pulse();

    text_layer_set_text(text_focus_layer, "FOCUS");
  } else {
    int time_minutes = tick_time->tm_min;

    if (time_minutes == 0) {  // the hour
      APP_LOG(APP_LOG_LEVEL_DEBUG, "Hour Pulse");

      VibePattern pattern = {
        .durations = HOUR_SEGMENTS,
        .num_segments = ARRAY_LENGTH(HOUR_SEGMENTS)
      };
      vibes_enqueue_custom_pattern(pattern);

    } else if (time_minutes % 45 == 0) {
      APP_LOG(APP_LOG_LEVEL_DEBUG, "45 Minute Pulse");

      VibePattern pattern = {
        .durations = FORTYFIVE_SEGMENTS,
        .num_segments = ARRAY_LENGTH(FORTYFIVE_SEGMENTS)
      };
      vibes_enqueue_custom_pattern(pattern);

    } else if (time_minutes % 30 == 0) {
      APP_LOG(APP_LOG_LEVEL_DEBUG, "30 Minute Pulse");

      VibePattern pattern = {
        .durations = THIRTY_SEGMENTS,
        .num_segments = ARRAY_LENGTH(THIRTY_SEGMENTS)
      };
      vibes_enqueue_custom_pattern(pattern);

    } else if (time_minutes % 15 == 0) {  // 15 minute increment
      APP_LOG(APP_LOG_LEVEL_DEBUG, "15 Minute Pulse");

      VibePattern pattern = {
        .durations = FIFTEEN_SEGMENTS,
        .num_segments = ARRAY_LENGTH(FIFTEEN_SEGMENTS)
      };
      vibes_enqueue_custom_pattern(pattern);

    } else if (time_minutes % 5 == 0) {  // 5 minute increment vibration
      APP_LOG(APP_LOG_LEVEL_DEBUG, "5 Minute Pulse");

      VibePattern pattern = {
        .durations = FIVE_SEGMENTS,
        .num_segments = ARRAY_LENGTH(FIVE_SEGMENTS)
      };
      vibes_enqueue_custom_pattern(pattern);

    }

    text_layer_set_text(text_focus_layer, "");
  }

  text_layer_set_text(text_time_layer, time_text);
}

void up_click_handler(ClickRecognizerRef recognizer, void *context) {
  if (focusMode == 0) {
    focusMode = 1;
  } else {
    focusMode = 0;
  }

  handle_minute_tick(NULL, MINUTE_UNIT);
}

static void click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
  window_single_click_subscribe(BUTTON_ID_UP, up_click_handler);
//  window_single_click_subscribe(BUTTON_ID_DOWN, down_click_handler);
}

void handle_deinit(void) {
  tick_timer_service_unsubscribe();
}


void handle_init(void) {
  window = window_create();
  window_set_fullscreen(window, true);
  window_stack_push(window, true /* Animated */);
  window_set_background_color(window, GColorBlack);
  window_set_click_config_provider(window, click_config_provider);

  Layer *window_layer = window_get_root_layer(window);

  text_date_layer = text_layer_create(GRect(8, 68, 144-8, 168-68));
  text_layer_set_text_color(text_date_layer, GColorWhite);
  text_layer_set_background_color(text_date_layer, GColorClear);
  text_layer_set_font(text_date_layer, fonts_get_system_font(FONT_KEY_ROBOTO_CONDENSED_21));
  layer_add_child(window_layer, text_layer_get_layer(text_date_layer));

  text_time_layer = text_layer_create(GRect(7, 86, 144-7, 168-92));
  text_layer_set_text_color(text_time_layer, GColorWhite);
  text_layer_set_background_color(text_time_layer, GColorClear);
  text_layer_set_font(text_time_layer, fonts_get_system_font(FONT_KEY_ROBOTO_BOLD_SUBSET_49));
  layer_add_child(window_layer, text_layer_get_layer(text_time_layer));

  text_focus_layer = text_layer_create(GRect(17, 20, 144-7, 168-92));
  text_layer_set_text_color(text_focus_layer, GColorWhite);
  text_layer_set_background_color(text_focus_layer, GColorClear);
  text_layer_set_font(text_focus_layer, fonts_get_system_font(FONT_KEY_BITHAM_30_BLACK));
  layer_add_child(window_layer, text_layer_get_layer(text_focus_layer));

  //GRect line_frame = GRect(8, 97, 139, 2);
  //line_layer = layer_create(line_frame);
  //layer_set_update_proc(line_layer, line_layer_update_callback);
  //layer_add_child(window_layer, line_layer);

  tick_timer_service_subscribe(MINUTE_UNIT, handle_minute_tick);
  handle_minute_tick(NULL, MINUTE_UNIT);
}


int main(void) {
  handle_init();

  app_event_loop();
  
  handle_deinit();
}
