#include <stdint.h>
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

void load_image(struct frame *current_frame) {
  int width, height, channels;
  unsigned char *data = stbi_load("eva.jpg", &width, &height, &channels, 3);
  if (data == NULL) {
    printf("Error in loading image!\n");
    exit(1);
  }
  printf("Loaded image with width %dpx, height %dpx\n", width, height);

  // Resize image to terminal dimensions
  struct winsize w = get_window_size();
  int desired_width = w.ws_col;
  // int desired_height = desired_width / ((float)width / height); //(Correct aspect ratio calculation)
  int desired_height = w.ws_row;
  current_frame->data = malloc(desired_width * desired_height * 3);
  printf("Resized width: %d, height: %d\n", desired_width, desired_height);

  stbir_resize_uint8_srgb(data, width, height, width * 3, current_frame->data, desired_width, desired_height,
                          desired_width * 3, STBIR_RGB);
  current_frame->width = desired_width;
  current_frame->height = desired_height;
  stbi_image_free(data);
}

int get_intensity(unsigned char *pixel_ptr) {
  unsigned char r = *pixel_ptr;
  unsigned char g = *(pixel_ptr + 1);
  unsigned char b = *(pixel_ptr + 2);

  float intensity =
      (r * 0.299 + g * 0.587 + 0.114 * b); // human eyes perceieve different colours with different brightness
  return intensity;
}

int main() {
  struct frame current_frame;
  load_image(&current_frame);

  float startTime = (float)clock() / CLOCKS_PER_SEC; // This returns  processor seconds, it
                                                     // doesn't translate to the real world seconds
  float endTime = (float)clock() / CLOCKS_PER_SEC;
  if (BENCHMARK)
    printf("Loading the image took %f seconds\n", endTime - startTime);

  size_t image_size = current_frame.height * current_frame.width * 3;
  size_t string_size = current_frame.height * current_frame.width +
                       current_frame.height; // +current_frame.height for accounting newlines

  char *ascii_string = (char *)malloc(string_size + 1);
  ascii_string[string_size] = '\0';

  unsigned char *input_ptr = current_frame.data;
  char *ascii_ptr = ascii_string;

  startTime = (float)clock() / CLOCKS_PER_SEC;
  int pixel_count = 1;
  while (input_ptr != current_frame.data + image_size) {
    int intensity = get_intensity(input_ptr);
    *ascii_ptr = ASCII_CHARS[(int)(intensity / 255.0 * (strlen(ASCII_CHARS) - 1))];

    if (pixel_count % current_frame.width == 0) {
      *(ascii_ptr + 1) = '\n';
      ascii_ptr++;
    }

    pixel_count++;
    ascii_ptr++;
    input_ptr += 3; // the pixel data is in the format - [R1, G1, B1, R2, G2, B3] where each pixel is (R1, G1, B1)
  }
  endTime = (float)clock() / CLOCKS_PER_SEC;
  if (BENCHMARK)
    printf("Converting the image took %f seconds\n", endTime - startTime);

  printf("%s", ascii_string);

  free(ascii_string);
  stbi_image_free(current_frame.data);
}
