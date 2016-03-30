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

/**
 * \file cairo-sdline.h
 *
 * \brief Заголовочный файл функций рисования линий в image_surface тип cairo
 * \author Andrei Fadeev
 * \date 2013-2016
 * \license GNU General Public License version 3 или более поздняя<br>
 * Коммерческая лицензия - свяжитесь с автором
 *
 */

#ifndef __CAIRO_SDLINE_H__
#define __CAIRO_SDLINE_H__

#include <glib.h>
#include <cairo.h>

G_BEGIN_DECLS

/* Структура описания поверхности для рисования. */
typedef struct
{
  cairo_t             *cairo;                  /* Указатель на объект cairo. */
  cairo_surface_t     *cairo_surface;          /* Указатель на поверхность cairo. */

  gint                 width;                  /* Ширина поверхности. */
  gint                 height;                 /* Высота поверхности. */
  gint                 stride;                 /* Размер одной линии поверхности в байтах. */

  gpointer             data;                   /* Пиксели поверхности. */
  gboolean             self_create;            /* TRUE - поверхность создавали мы, удаляем сами, иначе - FALSE. */
} cairo_sdline_surface;

/* Функция устанавливает значение цвета для контекста рисования cairo. */
void                   cairo_sdline_set_cairo_color    (cairo_t               *cairo,
                                                        guint32                color);

/* Функция создаёт поверхность для рисования указанного размера. */
cairo_sdline_surface*  cairo_sdline_surface_create     (gint                   width,
                                                        gint                   height);

/* Функция создаёт поверхность для рисования для указанной поверхности cairo. */
cairo_sdline_surface*  cairo_sdline_surface_create_for (cairo_surface_t       *cairo_surface);

/* Функция удаляет поверхность для рисования. */
void                   cairo_sdline_surface_destroy    (cairo_sdline_surface  *surface);

/* Функция переводит значение цвета из отдельных компонентов в упакованное 32-х битное значение. */
guint32                cairo_sdline_color              (gdouble                red,
                                                        gdouble                green,
                                                        gdouble                blue,
                                                        gdouble                alpha);

/* Функция полностью очищает поверхность до прозрачного состояния. */
void                   cairo_sdline_clear              (cairo_sdline_surface  *surface);

/* Функция заливает всю поверхность указанным цветом. */
void                   cairo_sdline_clear_color        (cairo_sdline_surface  *surface,
                                                        guint32                color);

/* Функция рисует горизонтальную линию указанным цветом. */
void                   cairo_sdline_h                  (cairo_sdline_surface  *surface,
                                                        gint                   x1,
                                                        gint                   x2,
                                                        gint                   y1,
                                                        guint32                color);

/* Функция рисует вертикальную линию указанным цветом. */
void                   cairo_sdline_v                  (cairo_sdline_surface  *surface,
                                                        gint                   x1,
                                                        gint                   y1,
                                                        gint                   y2,
                                                        guint32                color);

/* Функция рисует произвольную линию указанным цветом. */
void                   cairo_sdline                    (cairo_sdline_surface  *surface,
                                                        gint                   x1,
                                                        gint                   y1,
                                                        gint                   x2,
                                                        gint                   y2,
                                                        guint32                color);

/* Функция рисует прямоугольник, закрашенный указанным цветом. */
void                   cairo_sdline_bar                (cairo_sdline_surface  *surface,
                                                        gint                   x1,
                                                        gint                   y1,
                                                        gint                   x2,
                                                        gint                   y2,
                                                        guint32                color);

/* Функция рисует точку указанным цветом. */
void                   cairo_sdline_dot                (cairo_sdline_surface  *surface,
                                                        gint                   x,
                                                        gint                   y,
                                                        guint32                color);

G_END_DECLS

#endif /* __CAIRO_SDLINE_H__ */
