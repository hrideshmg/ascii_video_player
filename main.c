#include <stdint.h>
#include <time.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image/stb_image_write.h"
#define DEBUG 0

#define ASCII_CHARS "@%#*+=-:. "

int main() {
  int height, width, channels;
  float startTime = (float)clock() / CLOCKS_PER_SEC; // This returns  processor seconds, it
                                                     // doesn't translate to the real world time
  unsigned char *input = stbi_load("aperture.jpg", &width, &height, &channels, 3);
  float endTime = (float)clock() / CLOCKS_PER_SEC;
  if (DEBUG)
    printf("Loading the image took %f seconds\n", endTime - startTime);

  if (input == NULL) {
    printf("Error in loading image!\n");
    return 1;
  }

  printf("Loaded image with height %dpx, width %dpx and %d channels\n", height, width, channels);
  size_t image_size = height * width * channels;
  size_t grayscale_image_size = height * width + height; // Only one channel, +height for newlines
  unsigned char *ascii_string = (unsigned char *)malloc(grayscale_image_size + 1);
  ascii_string[grayscale_image_size] = '\0';
  ascii_string[0] = 'h';
  printf("%s", ascii_string);

  startTime = (float)clock() / CLOCKS_PER_SEC;
  int count = 1;
  for (unsigned char *input_p = input, *ascii_p = ascii_string; input_p != input + image_size;
       input_p += channels, ascii_p++) {
    unsigned char r = *input_p;
    unsigned char g = *(input_p + 1);
    unsigned char b = *(input_p + 2);

    float intensity =
        (r * 0.299 + g * 0.587 + 0.114 * b); // human eyes perceieve different colours with different brightness
    printf("%c", ASCII_CHARS[(int)(intensity / 255 * 10)]);
    // *ascii_p = ASCII_CHARS[(int)((float)intensity / 255 * 10)];
    if (count % width == 0) {
      printf("\n");
      *(ascii_p + 1) = '\n';
      ascii_p++;
    }
    count++;
  }
  endTime = (float)clock() / CLOCKS_PER_SEC;
  if (DEBUG)
    printf("Converting the image took %f seconds\n", endTime - startTime);
  printf("%s", ascii_string);

  // stbi_write_jpg("evagray.jpg", width, height, 1, gray_image, 100);
  free(ascii_string);
  stbi_image_free(input);
}
