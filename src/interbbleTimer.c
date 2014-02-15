#include <pebble.h>

//Labels for the various possible types of run/walk components
static char WARM_UP_LABEL[8] = "Warm Up";
static char COOL_DOWN_LABEL[10] = "Cool Down";

static char RUN_LABEL[4] = "Run";
static char RUN_FAST_LABEL[10] = "Brisk Run";
static char RECOVERY_JOB_LABEL[13] = "Recovery Jog";

static char WALK_LABEL[5] = "Walk";
static char FAST_WALK_LABEL[11] = "Brisk Walk";
static char RECOVERY_WALK_LABEL[14] = "Recovery Walk";

//interval string markers segmenting run/walk components
static char RUN_MARKER = 'R';
static char WALK_MARKER = 'W';
static char REPEAT_MARKER = 'X';

static Window *window;
static TextLayer *text_layer; 
static TextLayer *time_layer;
static TextLayer *title_layer;

enum {
  WARM_UP = 0x0,
  INTERVALS = 0x1,
  COOL_DOWN = 0x2
};

struct Segment {
  char* label;
  int minutes;
  //pointer to the 'next' segment in the list
  struct Segment *next;
};

static struct Segment *segments_start;
static AppTimer *timer;
static char time_str[6];

struct Segment * addSegment(char* label, int minutes, struct Segment *previous) {
  //if the minutes is 0 don't bother adding it
  if (minutes == 0) {
    return previous;
  }

  struct Segment *new_segment = (struct Segment *) malloc(sizeof(struct Segment));
  new_segment->label = label;
  new_segment->minutes = minutes;
  new_segment->next = 0;

  //if this is not the first item, update the previous items next pointer
  if (previous) {
    previous->next = new_segment;
  }

  APP_LOG(APP_LOG_LEVEL_DEBUG, "Added segment: %s=%d", label, minutes);

  return new_segment;
}

void freeMemory() {
   struct Segment *curr = segments_start;
   struct Segment *next = 0;

    while (curr != 0) { 
      next = curr->next;
      free(curr);
      curr = next;
    }
}

void startSegment(struct Segment *curr) {
  if (curr == 0) {
    //TODO: Program is over, display this on screen
    freeMemory();
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Program complete");
    text_layer_set_text(text_layer, "Done!");
    text_layer_set_text(time_layer, "00:00");
    return;
  }

  vibes_double_pulse();
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Found Segment: %s=%d", curr->label, curr->minutes);
  text_layer_set_text(text_layer, curr->label);

  struct tm *info;
  //generate a tm struct
  time_t rawtime;
  time( &rawtime);
  info = localtime( &rawtime );
  //set the minutes/seconds
  info->tm_min = curr->minutes;
  info->tm_sec = 0;  

  strftime(time_str,6,"%M:%S", info);
  time_str[5] = '\0';
   // printf("Formatted date & time : |%s|\n", buffer );

  text_layer_set_text(time_layer, time_str);
  APP_LOG(APP_LOG_LEVEL_DEBUG, "time for this segment: %s", time_str);
  //TODO: update to actually count down the time

  //register a listener to end this segment
  //FOR TESTING , lets just do this in seconds instead of minutes
  timer = app_timer_register((curr->minutes) * 1000, (AppTimerCallback) startSegment, curr->next);
  // timer = app_timer_register((curr->minutes) * 60 * 1000, (AppTimerCallback) startSegment, curr->next);
}

void traverseSegments(struct Segment *start) {
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Traversing Segments");

    struct Segment *curr = start;
    // timer = app_timer_register(curr->minutes * 60 * 1000, (AppTimerCallback) doSegment, curr->next);

    if (curr != 0) { 
      startSegment(curr);
      // curr = curr->next;
    } else {
      APP_LOG(APP_LOG_LEVEL_DEBUG, "No start point found.");
    }
}

struct Segment * splitIntervalString(char intervals_str[], int intervals_str_length, struct Segment *segments_curr) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "splitIntervalString: %s, %d", intervals_str, intervals_str_length);
  int num_start_index = 0;
  int repeat_start_index = 0;
  for (int i=0; i<intervals_str_length; i++) {
    char curr = intervals_str[i];

    if (curr == RUN_MARKER){
      char num_str[i - num_start_index + 1];
      strncpy(num_str, intervals_str + num_start_index, i - num_start_index);
      num_str[i - num_start_index] = '\0';//Null terminate for safety
      segments_curr = addSegment(RUN_LABEL, atoi(num_str), segments_curr);
      APP_LOG(APP_LOG_LEVEL_DEBUG, "Adding Run %s=%d", num_str, segments_curr->minutes);
      num_start_index = i + 1;
    } else if (curr == WALK_MARKER) {
      char num_str[i - num_start_index + 1];
      strncpy(num_str, intervals_str+num_start_index, i - num_start_index);
      num_str[i-num_start_index] = '\0';
      segments_curr = addSegment(WALK_LABEL, atoi(num_str), segments_curr);
      APP_LOG(APP_LOG_LEVEL_DEBUG, "Adding Walk %s=%d", num_str, segments_curr->minutes);
      num_start_index = i + 1;
    } else if (curr == REPEAT_MARKER) {
      char num_str[i - num_start_index + 1];
      strncpy(num_str, intervals_str+num_start_index, i - num_start_index);
      num_str[i-num_start_index] = '\0';
      int repeat = atoi(num_str);
      APP_LOG(APP_LOG_LEVEL_DEBUG, "Adding Repeat: %s=%d, repeat_start_index: %d", num_str, repeat, repeat_start_index);

      //add the run and walk again for each repeat (subtract one to exclude the already added ones)
      for (int j=0; j<repeat-1; j++) {
        int repeat_length = num_start_index - repeat_start_index;
        char sub_interval_string[repeat_length + 1];
        strncpy(sub_interval_string, intervals_str + repeat_start_index, repeat_length);
        sub_interval_string[repeat_length] = '\0';
        APP_LOG(APP_LOG_LEVEL_DEBUG, "generating repeat %d for substring %s -- repeat_length=%d", j, sub_interval_string, repeat_length);
        segments_curr = splitIntervalString(sub_interval_string, repeat_length, segments_curr);
      }

      num_start_index = i + 1;
      repeat_start_index = i + 1;
    }
  }

  return segments_curr;
}

void start_program() {
  //TODO: implement logic to actually do the program segment timings
  traverseSegments(segments_start);
}

/*
 *  Handle an appMessage received from the phone
 */
static void app_message_received_handler(DictionaryIterator *iter, void *context) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "App Message Received");
  Tuple *warm_up_tuple = dict_find(iter, WARM_UP);
  Tuple *intervals_tuple = dict_find(iter, INTERVALS);
  Tuple *cool_down_tuple = dict_find(iter, COOL_DOWN);

  //first check that all the tuples exist, otherwise display error msg to user
  if (!warm_up_tuple || !intervals_tuple || !cool_down_tuple) {
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Invalid Program");
    text_layer_set_text(title_layer, "Invalid Program");
    return;
  }

  //Create a linked list of Segments that we can later traverse through
  struct Segment *segments_curr;

  //Add the warm up segment
  segments_start = addSegment(WARM_UP_LABEL, atoi(warm_up_tuple->value->cstring), 0);
  segments_curr = segments_start;

  //copy the intervals string into a local variable (intervals_str)
  int intervals_str_length = strlen(intervals_tuple->value->cstring);
  char intervals_str[intervals_str_length+1];
  strncpy(intervals_str, intervals_tuple->value->cstring, intervals_str_length+1);
  APP_LOG(APP_LOG_LEVEL_DEBUG, intervals_str);

  //split the string on known characters to pull out the iterations
  segments_curr = splitIntervalString(intervals_str, intervals_str_length, segments_curr);

  //Add the cool down segment
  segments_curr = addSegment(COOL_DOWN_LABEL, atoi(cool_down_tuple->value->cstring), segments_curr);

 // traverseSegments(segments_start);
}

static void app_message_receive_failed_handler(AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "App Message Dropped!");
}

static void app_message_send_failed_handler(DictionaryIterator *failed, AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "App Message Failed to Send!");
}

static void app_message_sent_handler(DictionaryIterator *sent, void *context) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "App Message Sent");
}

static void select_click_handler(ClickRecognizerRef recognizer, void *context) {
  // text_layer_set_text(text_layer, "Select");
  start_program();
  //TODO: if click it again, should pause program
}

static void up_click_handler(ClickRecognizerRef recognizer, void *context) {
  // text_layer_set_text(text_layer, "Up");
}

static void down_click_handler(ClickRecognizerRef recognizer, void *context) {
  // text_layer_set_text(text_layer, "Down");
}

static void click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
  window_single_click_subscribe(BUTTON_ID_UP, up_click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, down_click_handler);
}

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  text_layer = text_layer_create((GRect) { .origin = { 0, 72 }, .size = { bounds.size.w, 20 } });
  text_layer_set_text(text_layer, "Press start");
  text_layer_set_text_alignment(text_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(text_layer));

  time_layer = text_layer_create((GRect) { .origin={0, 95}, .size = { bounds.size.w, 20} });
  text_layer_set_text(time_layer, "00:00"); //TODO: Update this to be the total time remaining? or add seperate total time countdown
  text_layer_set_text_alignment(time_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(time_layer));

  title_layer = text_layer_create((GRect) { .origin={0,30}, .size = { bounds.size.w, 20} });
  text_layer_set_text(title_layer, "Program title");
  text_layer_set_text_alignment(title_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(title_layer));

  //TODO: Send an event to the js side to retrieve the stored program

}

static void window_unload(Window *window) {
  text_layer_destroy(text_layer);
  text_layer_destroy(time_layer);
  text_layer_destroy(title_layer);
}

static void app_message_init(void){
  //Register message handlers
  app_message_register_inbox_received(app_message_received_handler);
  app_message_register_inbox_dropped(app_message_receive_failed_handler);
  app_message_register_outbox_sent(app_message_sent_handler);
  app_message_register_outbox_failed(app_message_send_failed_handler);
  app_message_open(64, 64);
}

static void init(void) {
  window = window_create();
  app_message_init();
  window_set_click_config_provider(window, click_config_provider);
  window_set_window_handlers(window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });
  const bool animated = true;
  window_stack_push(window, animated);
}

static void deinit(void) {
  window_destroy(window);
  app_message_deregister_callbacks();
}

int main(void) {
  init();

  APP_LOG(APP_LOG_LEVEL_DEBUG, "Done initializing, pushed window: %p", window);

  app_event_loop();
  deinit();
}
