/* -*- mode: c; c-basic-offset: 2; indent-tabs-mode: nil; -*-
 *
 * Using the C-API of this library.
 *
 */

// gcc_cmd:g++ -o pthread_test pthread_test.c -I /home/metoro/rpi-rgb-led-matrix/include -L /home/metoro/rpi-rgb-led-matrix/lib -lrgbmatrix -lwiringPi
// cmd:sudo '/home/metoro/led/pthread_test' --led-slowdown-gpio=2 --led-pwm-bits=4
#include <led-matrix-c.h>

#include <wiringPi.h>

#include <stdio.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char **argv)
{
  struct RGBLedMatrixOptions options;
  struct RGBLedMatrix *matrix;
  struct LedCanvas *offscreen_canvas;
  int width, height;
  int x, y, i;

  memset(&options, 0, sizeof(options));
  options.rows = 32; // パネルの行(デフォルト32)
  options.cols = 64; // パネルの列
  options.chain_length = 3;

  /* This supports all the led commandline options. Try --led-help */
  matrix = led_matrix_create_from_options(&options, &argc, &argv);
  if (matrix == NULL)
    return 1;

  /* Let's do an example with double-buffering. We create one extra
   * buffer onto which we draw, which is then swapped on each refresh.
   * This is typically a good aproach for animations and such.
   */
  offscreen_canvas = led_matrix_create_offscreen_canvas(matrix);

  led_canvas_get_size(offscreen_canvas, &width, &height);

  fprintf(stderr, "Size: %dx%d. Hardware gpio mapping: %s\n",
          width, height, options.hardware_mapping);

  for (i = 0; i < 10; ++i)
  {
    for (y = 0; y < height; ++y)
    {
      for (x = 0; x < width; ++x)
      {
        led_canvas_set_pixel(offscreen_canvas, x, y, 127, 127, 127);
      }
    }

    /* Now, we swap the canvas. We give swap_on_vsync the buffer we
     * just have drawn into, and wait until the next vsync happens.
     * we get back the unused buffer to which we'll draw in the next
     * iteration.
     */
    offscreen_canvas = led_matrix_swap_on_vsync(matrix, offscreen_canvas);
    delay(1000);
  }

  /*
   * Make sure to always call led_matrix_delete() in the end to reset the
   * display. Installing signal handlers for defined exit is a good idea.
   */
  led_matrix_delete(matrix);

  return 0;
}