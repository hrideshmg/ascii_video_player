#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h> //handle formats
#include <libswscale/swscale.h>   //scaling and converting pixel formats
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <time.h>
#include <unistd.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image/stb_image_write.h"
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "stb_image/stb_image_resize2.h"
#define BENCHMARK 0

#define ASCII_CHARS " .`-_':;,^\\\"~|<>(){}[]1!il?+*xjrvctzunowmYXJLCOQ0Z@#&%$"

struct video_data {
  char **ascii_frames; // Array of ascii strings
  int frame_count;
  double frame_rate;
};

struct frame {
  unsigned char *data;
  int height;
  int width;
  int channels;
};

struct winsize get_window_size() {
  struct winsize w;
  ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
  return w;
}

// function declarations
char *convert_frame_to_ascii(struct frame *frame);
int get_intensity(uint8_t *pixel_ptr);
struct winsize get_window_size();
void load_frames(struct video_data *video_data, char *filename);
void adapt_frame(struct frame *frame);
void clear_screen(char *ascii_frame);

char *convert_frame_to_ascii(struct frame *frame) {
  size_t image_size = frame->height * frame->width * 3;
  size_t string_size = frame->height * frame->width + frame->height;
  char *ascii_string = (char *)malloc(string_size + 1);
  ascii_string[string_size] = '\0';

  unsigned char *input_ptr = frame->data;
  char *ascii_ptr = ascii_string;

  int pixel_count = 1;
  while (input_ptr != frame->data + image_size) {
    int intensity = get_intensity(input_ptr);
    *ascii_ptr = ASCII_CHARS[(int)(intensity / 255.0 * (strlen(ASCII_CHARS) - 1))];

    if (pixel_count % frame->width == 0) {
      *(ascii_ptr + 1) = '\n';
      ascii_ptr++;
    }

    pixel_count++;
    ascii_ptr++;
    input_ptr += 3; // the pixel data is in the format - [R1, G1, B1, R2, G2, B3] where each pixel is (R1, G1, B1)
  }
  return ascii_string;
}

void load_frames(struct video_data *video_data, char *filename) {
  AVFormatContext *format_ctx = NULL;
  avformat_open_input(&format_ctx, filename, NULL, NULL); // automatically detects the format, no special options
  avformat_find_stream_info(format_ctx, NULL);            // info about audio and video streams

  int video_stream_index = -1;            // to tell that no video stream is found when initialized
  AVCodecParameters *codec_params = NULL; // structure to store info about encoded data
  for (unsigned int i = 0; i < format_ctx->nb_streams; i++) {
    if (format_ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
      video_stream_index = i;
      codec_params = format_ctx->streams[i]->codecpar;
      break;
    }
  }

  const AVCodec *codec =
      avcodec_find_decoder(codec_params->codec_id);          // find the codec based on id to decode video frames
  AVCodecContext *codec_ctx = avcodec_alloc_context3(codec); // codec context which tells how to decode the video
  avcodec_parameters_to_context(codec_ctx, codec_params);    // copy from ctx to params
  avcodec_open2(codec_ctx, codec, NULL);                     // opens the codec to decode

  int total_frames = 0;
  AVRational frame_rate = format_ctx->streams[video_stream_index]->avg_frame_rate;
  int64_t duration = format_ctx->duration; // duration in AV_TIME_BASE (microseconds)

  if (frame_rate.num > 0 && frame_rate.den > 0) {
    double duration_in_seconds = (double)duration / AV_TIME_BASE; // converting to seconds
    total_frames =
        (int)(duration_in_seconds * (double)frame_rate.num /
              (double)frame_rate.den); // num = no. of frames in a time period, den = time period in which frames occur
  } else {
    total_frames = (int)(duration / AV_TIME_BASE * 30);
  }
  video_data->frame_rate = (double)frame_rate.num / frame_rate.den;

  struct frame current_frame;
  int frame_count = 0;
  video_data->ascii_frames = malloc(sizeof(char *) * total_frames);
  char **ascii_frame_ptr = video_data->ascii_frames; // pointer to first element of array
  AVFrame *frame = av_frame_alloc();
  AVPacket *packet = av_packet_alloc();

  while (av_read_frame(format_ctx, packet) >= 0) {
    if (packet->stream_index == video_stream_index) {
      avcodec_send_packet(codec_ctx, packet);
      while (avcodec_receive_frame(codec_ctx, frame) == 0) {
        struct SwsContext *sws_ctx = sws_getContext(frame->width, frame->height, codec_ctx->pix_fmt, frame->width,
                                                    frame->height, AV_PIX_FMT_RGB24, SWS_BILINEAR, NULL, NULL, NULL);

        uint8_t *rgb_data = malloc(frame->width * frame->height * 3); // allocate memory for rgb data of frame
        sws_scale(sws_ctx, (const uint8_t *const *)frame->data, frame->linesize, 0, frame->height, &rgb_data,
                  (int[]){frame->width * 3}); // convert the frame to rgb format using context from earlier

        current_frame.data = rgb_data;
        current_frame.width = frame->width;
        current_frame.height = frame->height;
        adapt_frame(&current_frame);

        *ascii_frame_ptr = convert_frame_to_ascii(&current_frame);
        printf("Finished processing %d frames\r", frame_count);

        frame_count++;
        ascii_frame_ptr++;
        free(current_frame.data);
        sws_freeContext(sws_ctx);
        break;
      }
    }
    av_packet_unref(packet);
  }
  printf("\n");
  video_data->frame_count = frame_count;

  av_packet_free(&packet);
  av_frame_free(&frame);
  avcodec_free_context(&codec_ctx);
  avformat_close_input(&format_ctx);
}

void adapt_frame(struct frame *frame) {
  // Resize image to terminal dimensions
  struct winsize w = get_window_size();
  int desired_width = w.ws_col;
  // int desired_height = desired_width / ((float)width / height); //(Correct aspect ratio calculation)
  int desired_height = w.ws_row;

  uint8_t *resized_data = malloc(desired_width * desired_height * 3);
  stbir_resize_uint8_srgb(frame->data, frame->width, frame->height, frame->width * 3, resized_data, desired_width,
                          desired_height, desired_width * 3, STBIR_RGB);
  free(frame->data);
  frame->data = resized_data;
  frame->width = desired_width;
  frame->height = desired_height;
}

void clear_screen(char *ascii_frame) {
  int newline_count = 0;
  while (*ascii_frame != '\0') {
    if (*ascii_frame == '\n') {
      newline_count++;
    }
    ascii_frame++;
  }
  for (int i = 0; i < newline_count; i++) {
    printf("\033[A\33[2K\r"); // clear previous line.
  }
}

int get_intensity(uint8_t *pixel_ptr) {
  unsigned char r = *pixel_ptr;
  unsigned char g = *(pixel_ptr + 1);
  unsigned char b = *(pixel_ptr + 2);

  float intensity =
      (r * 0.299 + g * 0.587 + 0.114 * b); // human eyes perceieve different colours with different brightness
  return intensity;
}

int main() {
  struct video_data video_data;
  char filename[20];

  printf("Enter filename of the video you wish to play: ");
  scanf("%s", filename);

  load_frames(&video_data, filename); // Populate video_data with the necessary data

  for (int i = 0; i < video_data.frame_count; i++) {
    char *current_ascii_frame = video_data.ascii_frames[i];
    printf("%s", current_ascii_frame);
    double time_for_each_frame = 1000 / video_data.frame_rate; // in milliseconds

    usleep(time_for_each_frame * 1000); // usleep expects microseconds
    clear_screen(video_data.ascii_frames[i]);

    free(current_ascii_frame);
  }
}
