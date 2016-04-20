/*
 * GtkCifroArea - 2D layers image management library.
 *
 * Copyright 2013-2016 Andrei Fadeev (andrei@webcontrol.ru)
 *
 * This file is part of GtkCifroArea.
 *
 * GtkCifroArea is dual-licensed: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * GtkCifroArea is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library. If not, see <http://www.gnu.org/licenses/>.
 *
 * Alternatively, you can license this code under a commercial license.
 * Contact the author in this case.
 *
 */

/*
 * \file cairo-sdline.c
 *
 * \brief Исходный файл функций рисования линий в image_surface тип cairo
 * \author Andrei Fadeev
 * \date 2013-2016
 * \license GNU General Public License version 3 или более поздняя<br>
 * Коммерческая лицензия - свяжитесь с автором
 *
 */

#include "cairo-sdline.h"

#include <stdlib.h>
#include <string.h>

/*
 * Clipping routines for line.
 *
 * Clipping based heavily on code from http://www.ncsa.uiuc.edu/Vis/Graphics/src/clipCohSuth.c
 *
 */
#define PIXEL_SIZE             4

#define CLIP_LEFT_EDGE         0x1
#define CLIP_RIGHT_EDGE        0x2
#define CLIP_BOTTOM_EDGE       0x4
#define CLIP_TOP_EDGE          0x8
#define CLIP_INSIDE(a)         (!a)
#define CLIP_REJECT(a,b)       (a&b)
#define CLIP_ACCEPT(a,b)       (!(a|b))

/*
 * Internal clip-encoding routine.
 * Calculates a segement-based clipping encoding for a point against a rectangle.
 *
 * Params:
 *  - X coordinate of point;
 *  - Y coordinate of point;
 *  - X coordinate of left edge of the rectangle;
 *  - Y coordinate of top edge of the rectangle;
 *  - X coordinate of right edge of the rectangle;
 *  - Y coordinate of bottom edge of the rectangle.
 *
 */
static gint
_clipEncode (gint x,
             gint y,
             gint left,
             gint top,
             gint right,
             gint bottom)
{
  gint code = 0;

  if (x < left)
    {
      code |= CLIP_LEFT_EDGE;
    }
  else if (x > right)
    {
      code |= CLIP_RIGHT_EDGE;
    }

  if (y < top)
    {
      code |= CLIP_TOP_EDGE;
    }
  else if (y > bottom)
    {
      code |= CLIP_BOTTOM_EDGE;
    }

  return code;
}

/*
 * Clip line to a the clipping rectangle of a surface.
 *
 * Params:
 *  - Target surface to draw on;
 *  - Pointer to X coordinate of first point of line;
 *  - Pointer to Y coordinate of first point of line;
 *  - Pointer to X coordinate of second point of line;
 *  - Pointer to Y coordinate of second point of line.
 *
 */
static gint
_clipLine (cairo_sdline_surface *surface,
           gint                 *x1,
           gint                 *y1,
           gint                 *x2,
           gint                 *y2)
{
  gint left, right, top, bottom;
  gint code1, code2;
  gint draw = 0;
  gint swaptmp;
  gfloat m;

  /*
   * Get clipping boundary
   */
  left = 0;
  right = surface->width - 1;
  top = 0;
  bottom = surface->height - 1;

  while (1)
    {
      code1 = _clipEncode (*x1, *y1, left, top, right, bottom);
      code2 = _clipEncode (*x2, *y2, left, top, right, bottom);
      if (CLIP_ACCEPT(code1, code2))
        {
          draw = 1;
          break;
        }
      else if (CLIP_REJECT(code1, code2))
        {
          break;
        }
      else
        {
          if (CLIP_INSIDE(code1))
            {
              swaptmp = *x2;
              *x2 = *x1;
              *x1 = swaptmp;
              swaptmp = *y2;
              *y2 = *y1;
              *y1 = swaptmp;
              swaptmp = code2;
              code2 = code1;
              code1 = swaptmp;
            }

          if (*x2 != *x1)
            {
              m = (float) (*y2 - *y1) / (float) (*x2 - *x1);
            }
          else
            {
              m = 1.0f;
            }

          if (code1 & CLIP_LEFT_EDGE)
            {
              *y1 += (int) ((left - *x1) * m);
              *x1 = left;
            }
          else if (code1 & CLIP_RIGHT_EDGE)
            {
              *y1 += (int) ((right - *x1) * m);
              *x1 = right;
            }
          else if (code1 & CLIP_BOTTOM_EDGE)
            {
              if (*x2 != *x1)
                *x1 += (int) ((bottom - *y1) / m);
              *y1 = bottom;
            }
          else if (code1 & CLIP_TOP_EDGE)
            {
              if (*x2 != *x1)
                *x1 += (int) ((top - *y1) / m);
              *y1 = top;
            }
        }
    }

  return draw;
}

/* Функция устанавливает значение цвета для контекста рисования cairo. */
void
cairo_sdline_set_cairo_color (cairo_t *cairo,
                              guint32  color)
{
  gdouble red, green, blue, alpha;

  alpha = (gdouble) ((color >> 24) & 0xFF) / 255.0;
  red = (gdouble) ((color >> 16) & 0xFF) / 255.0;
  green = (gdouble) ((color >> 8) & 0xFF) / 255.0;
  blue = (gdouble) (color & 0xFF) / 255.0;

  cairo_set_source_rgba (cairo, red, green, blue, alpha);
}

/* Функция создаёт поверхность для рисования указанного размера. */
cairo_sdline_surface *
cairo_sdline_surface_create (gint width,
                             gint height)
{
  cairo_surface_t *cairo_surface;
  cairo_sdline_surface *surface;

  cairo_surface = cairo_image_surface_create (CAIRO_FORMAT_ARGB32, width, height);
  surface = cairo_sdline_surface_create_for (cairo_surface);

  if (surface != NULL)
    surface->self_create = TRUE;

  return surface;
}

/* Функция создаёт поверхность для рисования для указанной поверхности cairo. */
cairo_sdline_surface *
cairo_sdline_surface_create_for (cairo_surface_t *cairo_surface)
{
  cairo_sdline_surface *surface;

  if (cairo_surface_get_type (cairo_surface) != CAIRO_SURFACE_TYPE_IMAGE)
    return NULL;

  if ((cairo_image_surface_get_format (cairo_surface) != CAIRO_FORMAT_ARGB32) &&
      (cairo_image_surface_get_format (cairo_surface) != CAIRO_FORMAT_RGB24))
    return NULL;

  surface = g_malloc (sizeof(cairo_sdline_surface));
  surface->cairo = cairo_create (cairo_surface);
  surface->cairo_surface = cairo_surface;
  surface->width = cairo_image_surface_get_width (cairo_surface);
  surface->height = cairo_image_surface_get_height (cairo_surface);
  surface->stride = cairo_image_surface_get_stride (cairo_surface);
  surface->data = cairo_image_surface_get_data (cairo_surface);
  surface->self_create = FALSE;

  return surface;
}

/* Функция удаляет поверхность для рисования. */
void
cairo_sdline_surface_destroy (cairo_sdline_surface *surface)
{
  if (surface == NULL)
    return;

  cairo_destroy (surface->cairo);

  if (surface->self_create)
    cairo_surface_destroy (surface->cairo_surface);

  g_free (surface);
}

/* Функция переводит значение цвета из отдельных компонентов в упакованное 32-х битное значение. */
guint32
cairo_sdline_color (double red,
                    double green,
                    double blue,
                    double alpha)
{
  guint32 ired, igreen, iblue, ialpha;
  guint32 color;

  if (red < 0.0)
    red = 0.0;
  if (red > 1.0)
    red = 1.0;
  if (green < 0.0)
    green = 0.0;
  if (green > 1.0)
    green = 1.0;
  if (blue < 0.0)
    blue = 0.0;
  if (blue > 1.0)
    blue = 1.0;
  if (alpha < 0.0)
    alpha = 0.0;
  if (alpha > 1.0)
    alpha = 1.0;

  ired = red * 0xFF;
  igreen = green * 0xFF;
  iblue = blue * 0xFF;
  ialpha = alpha * 0xFF;

  color = (ialpha << 24) | (ired << 16) | (igreen << 8) | (iblue);
  return color;
}

/* Функция полностью очищает поверхность до прозрачного состояния. */
void
cairo_sdline_clear (cairo_sdline_surface *surface)
{
  size_t size;

  if (surface == NULL)
    return;

  size = surface->height * surface->stride;
  size -= surface->width - surface->stride;

  memset (surface->data, 0, size);
}

/* Функция заливает всю поверхность указанным цветом. */
void
cairo_sdline_clear_color (cairo_sdline_surface *surface,
                          guint32               color)
{
  gint shift;
  gint i;

  if (surface == NULL)
    return;

  for (i = 0, shift = 0; i < surface->width; i++, shift += PIXEL_SIZE)
    *(guint32*) ((guchar*)surface->data + shift) = color;

  for (i = 1, shift = surface->stride; i < surface->height; i++, shift += surface->width * PIXEL_SIZE)
    memcpy ((guchar*)surface->data + shift, surface->data, surface->width * PIXEL_SIZE);
}

/* Функция рисует горизонтальную линию указанным цветом. */
void
cairo_sdline_h (cairo_sdline_surface *surface,
                gint                  x1,
                gint                  x2,
                gint                  y1,
                guint32               color)
{
  gint swaptmp;
  gint shift;
  gint i;

  if (surface == NULL)
    return;

  if ((y1 < 0) || (y1 >= surface->height))
    return;

  if (x1 < 0)
    x1 = 0;
  if (x1 >= surface->width)
    x1 = surface->width - 1;

  if (x2 < 0)
    x2 = 0;
  if (x2 >= surface->width)
    x2 = surface->width - 1;

  if (x1 > x2)
    {
      swaptmp = x1, x1 = x2;
      x2 = swaptmp;
    }

  shift = (y1 * surface->stride) + PIXEL_SIZE * x1;
  for (i = x1; i <= x2; i++, shift += PIXEL_SIZE)
    *(guint32*) ((guchar*)surface->data + shift) = color;
}

/* Функция рисует вертикальную линию указанным цветом. */
void
cairo_sdline_v (cairo_sdline_surface *surface,
                gint                  x1,
                gint                  y1,
                gint                  y2,
                guint32               color)
{
  gint swaptmp;
  gint shift;
  gint i;

  if (surface == NULL)
    return;

  if ((x1 < 0) || (x1 >= surface->width))
    return;

  if (y1 < 0)
    y1 = 0;
  if (y1 >= surface->height)
    y1 = surface->height - 1;

  if (y2 < 0)
    y2 = 0;
  if (y2 >= surface->height)
    y2 = surface->height - 1;

  if (y1 > y2)
    {
      swaptmp = y1, y1 = y2;
      y2 = swaptmp;
    }

  shift = (y1 * surface->stride) + PIXEL_SIZE * x1;
  for (i = y1; i <= y2; i++, shift += surface->stride)
    *(guint32*) ((guchar*)surface->data + shift) = color;
}

/* Функция рисует произвольную линию указанным цветом. */
void
cairo_sdline (cairo_sdline_surface *surface,
              gint                  x1,
              gint                  y1,
              gint                  x2,
              gint                  y2,
              guint32               color)
{
  gint pixx, pixy;
  gint dx, dy;
  gint sx, sy;
  gint x, y;

  gint swaptmp;
  guchar *pixel;

  if (surface == NULL)
    return;

  if (!(_clipLine (surface, &x1, &y1, &x2, &y2)))
    return;

  if (x1 == x2)
    {
      cairo_sdline_v (surface, x1, y1, y2, color);
      return;
    }

  if (y1 == y2)
    {
      cairo_sdline_h (surface, x1, x2, y1, color);
      return;
    }

  dx = x2 - x1;
  dy = y2 - y1;
  sx = (dx >= 0) ? 1 : -1;
  sy = (dy >= 0) ? 1 : -1;

  dx = sx * dx + 1;
  dy = sy * dy + 1;
  pixx = PIXEL_SIZE;
  pixy = surface->stride;
  pixel = (guchar*)surface->data + pixx * (int)x1 + pixy * (int)y1;
  pixx *= sx;
  pixy *= sy;
  if (dx < dy)
    {
      swaptmp = dx;
      dx = dy;
      dy = swaptmp;
      swaptmp = pixx;
      pixx = pixy;
      pixy = swaptmp;
    }

  for (x = 0, y = 0; x < dx; x++, pixel += pixx)
    {
      *(guint32*) pixel = color;
      y += dy;
      if (y >= dx)
        {
          y -= dx;
          pixel += pixy;
        }
    }
}

/* Функция рисует прямоугольник, закрашенный указанным цветом. */
void
cairo_sdline_bar (cairo_sdline_surface *surface,
                  gint                  x1,
                  gint                  y1,
                  gint                  x2,
                  gint                  y2,
                  guint32               color)
{
  gint swaptmp;
  gint shift;
  gint i, j;

  if (surface == NULL)
    return;

  if (x1 < 0)
    x1 = 0;
  if (x1 >= surface->width)
    x1 = surface->width - 1;

  if (x2 < 0)
    x2 = 0;
  if (x2 >= surface->width)
    x2 = surface->width - 1;

  if (y1 < 0)
    y1 = 0;
  if (y1 >= surface->height)
    y1 = surface->height - 1;

  if (y2 < 0)
    y2 = 0;
  if (y2 >= surface->height)
    y2 = surface->height - 1;

  if (x1 > x2)
    {
      swaptmp = x1, x1 = x2;
      x2 = swaptmp;
    }

  if (y1 > y2)
    {
      swaptmp = y1, y1 = y2;
      y2 = swaptmp;
    }

  for (j = y1; j <= y2; j++)
    {
      shift = (j * surface->stride) + PIXEL_SIZE * x1;
      for (i = x1; i <= x2; i++, shift += PIXEL_SIZE)
        *(guint32*)((guchar*)surface->data + shift) = color;
    }
}

/* Функция рисует точку указанным цветом. */
void
cairo_sdline_dot (cairo_sdline_surface *surface,
                  gint                  x,
                  gint                  y,
                  guint32               color)
{
  gint shift;

  if (surface == NULL)
    return;

  if (x < 0 || y < 0)
    return;

  if (x >= surface->width || y >= surface->height)
    return;

  shift = (y * surface->stride) + PIXEL_SIZE * x;
  *(guint32*) ((guchar*)surface->data + shift) = color;
}
