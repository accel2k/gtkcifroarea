/*
 * GtkCifroArea - 2D layers image management library.
 *
 * Copyright 2013-2017 Andrei Fadeev (andrei@webcontrol.ru)
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

#ifndef __GTK_CIFRO_AREA_H__
#define __GTK_CIFRO_AREA_H__

#include <gtk/gtk.h>
#include <gtk-cifro-area-exports.h>

G_BEGIN_DECLS

/**
 * GtkCifroAreaStickType:
 * @GTK_CIFRO_AREA_STICK_CENTER: Центрировать изображение.
 * @GTK_CIFRO_AREA_STICK_LEFT: Сдвигать изображение влево.
 * @GTK_CIFRO_AREA_STICK_TOP: Сдвигать изображение вверх.
 * @GTK_CIFRO_AREA_STICK_BOTTOM: Сдвигать изображение вниз.
 * @GTK_CIFRO_AREA_STICK_RIGHT: Сдвигать изображение вправо.
 *
 * Определяет направление сдвига координат в случае формирования изображения больше
 * чем требуется для видимой области.
 *
 */
typedef enum
{
  GTK_CIFRO_AREA_STICK_CENTER,
  GTK_CIFRO_AREA_STICK_LEFT,
  GTK_CIFRO_AREA_STICK_TOP,
  GTK_CIFRO_AREA_STICK_RIGHT,
  GTK_CIFRO_AREA_STICK_BOTTOM
} GtkCifroAreaStickType;

/**
 * GtkCifroAreaZoomType:
 * @GTK_CIFRO_AREA_ZOOM_NONE: Не изменять масштаб.
 * @GTK_CIFRO_AREA_ZOOM_IN: Приближение.
 * @GTK_CIFRO_AREA_ZOOM_OUT: Отдаление.
 *
 * Определяет направление изменения масштаба.
 *
 */
typedef enum
{
  GTK_CIFRO_AREA_ZOOM_NONE,
  GTK_CIFRO_AREA_ZOOM_IN,
  GTK_CIFRO_AREA_ZOOM_OUT
} GtkCifroAreaZoomType;

#define GTK_TYPE_CIFRO_AREA             (gtk_cifro_area_get_type ())
#define GTK_CIFRO_AREA(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), GTK_TYPE_CIFRO_AREA, GtkCifroArea))
#define GTK_IS_CIFRO_AREA(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GTK_TYPE_CIFRO_AREA))
#define GTK_CIFRO_AREA_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), GTK_TYPE_CIFRO_AREA, GtkCifroAreaClass))
#define GTK_IS_CIFRO_AREA_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), GTK_TYPE_CIFRO_AREA))
#define GTK_CIFRO_AREA_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), GTK_TYPE_CIFRO_AREA, GtkCifroAreaClass))

typedef struct _GtkCifroArea GtkCifroArea;
typedef struct _GtkCifroAreaPrivate GtkCifroAreaPrivate;
typedef struct _GtkCifroAreaClass GtkCifroAreaClass;

struct _GtkCifroArea
{
  GtkDrawingArea parent_instance;

  GtkCifroAreaPrivate *priv;
};

/**
 * GtkCifroAreaClass:
 * @parent_class: Структура базового класса.
 * @get_rotate: Возвращает значение параметра разрешения поворота изображения.
 * @get_swap: Возвращает параметры зеркального отражения изображения по осям.
 * @get_stick: Возвращает направление смещения координат при отображении вне диапазона.
 * @get_border: Возвращает текущие размеры области окантовки виджета.
 * @get_limits: Возвращает текущие значения пределов перемещения изображения.
 * @check_scale: Проверяет значения масштаба и корректирует их при необходимости.
 * @zoom: Изменяет текущий масштаб изображения.
 *
 */
struct _GtkCifroAreaClass
{
  GtkDrawingAreaClass parent_class;

  gboolean             (*get_rotate)                           (GtkCifroArea          *carea);

  void                 (*get_swap)                             (GtkCifroArea          *carea,
                                                                gboolean              *swap_x,
                                                                gboolean              *swap_y);

  void                 (*get_stick)                            (GtkCifroArea          *carea,
                                                                GtkCifroAreaStickType *stick_x,
                                                                GtkCifroAreaStickType *stick_y);

  void                 (*get_border)                           (GtkCifroArea          *carea,
                                                                guint                 *border_top,
                                                                guint                 *border_bottom,
                                                                guint                 *border_left,
                                                                guint                 *border_right);

  void                 (*get_limits)                           (GtkCifroArea          *carea,
                                                                gdouble               *min_x,
                                                                gdouble               *max_x,
                                                                gdouble               *min_y,
                                                                gdouble               *max_y);

  void                 (*check_scale)                          (GtkCifroArea          *carea,
                                                                gdouble               *scale_x,
                                                                gdouble               *scale_y);

  void                 (*zoom)                                 (GtkCifroArea          *carea,
                                                                GtkCifroAreaZoomType   direction_x,
                                                                GtkCifroAreaZoomType   direction_y,
                                                                gdouble                center_x,
                                                                gdouble                center_y);
};

GTK_CIFROAREA_EXPORT
GType                  gtk_cifro_area_get_type                 (void);

GTK_CIFROAREA_EXPORT
GtkWidget             *gtk_cifro_area_new                      (void);

GTK_CIFROAREA_EXPORT
gboolean               gtk_cifro_area_get_rotate               (GtkCifroArea          *carea);

GTK_CIFROAREA_EXPORT
void                   gtk_cifro_area_get_swap                 (GtkCifroArea          *carea,
                                                                gboolean              *swap_x,
                                                                gboolean              *swap_y);

GTK_CIFROAREA_EXPORT
void                   gtk_cifro_area_get_stick                (GtkCifroArea          *carea,
                                                                GtkCifroAreaStickType *stick_x,
                                                                GtkCifroAreaStickType *stick_y);

GTK_CIFROAREA_EXPORT
void                   gtk_cifro_area_get_border               (GtkCifroArea          *carea,
                                                                guint                 *border_top,
                                                                guint                 *border_bottom,
                                                                guint                 *border_left,
                                                                guint                 *border_right);

GTK_CIFROAREA_EXPORT
void                   gtk_cifro_area_get_limits               (GtkCifroArea          *carea,
                                                                gdouble               *min_x,
                                                                gdouble               *max_x,
                                                                gdouble               *min_y,
                                                                gdouble               *max_y);

GTK_CIFROAREA_EXPORT
void                   gtk_cifro_area_check_scale              (GtkCifroArea          *carea,
                                                                gdouble               *scale_x,
                                                                gdouble               *scale_y);

GTK_CIFROAREA_EXPORT
void                   gtk_cifro_area_set_scale_on_resize      (GtkCifroArea          *carea,
                                                                gboolean               scale_on_resize);

GTK_CIFROAREA_EXPORT
gboolean               gtk_cifro_area_get_scale_on_resize      (GtkCifroArea          *carea);

GTK_CIFROAREA_EXPORT
void                   gtk_cifro_area_set_scale                (GtkCifroArea          *carea,
                                                                gdouble                scale_x,
                                                                gdouble                scale_y,
                                                                gdouble                center_x,
                                                                gdouble                center_y);

GTK_CIFROAREA_EXPORT
void                   gtk_cifro_area_get_scale                (GtkCifroArea          *carea,
                                                                gdouble               *scale_x,
                                                                gdouble               *scale_y);

GTK_CIFROAREA_EXPORT
void                   gtk_cifro_area_set_view_center          (GtkCifroArea          *carea,
                                                                gdouble                center_x,
                                                                gdouble                center_y);

GTK_CIFROAREA_EXPORT
void                   gtk_cifro_area_set_view                 (GtkCifroArea          *carea,
                                                                gdouble                from_x,
                                                                gdouble                to_x,
                                                                gdouble                from_y,
                                                                gdouble                to_y);

GTK_CIFROAREA_EXPORT
void                   gtk_cifro_area_get_view                 (GtkCifroArea          *carea,
                                                                gdouble               *from_x,
                                                                gdouble               *to_x,
                                                                gdouble               *from_y,
                                                                gdouble               *to_y);

GTK_CIFROAREA_EXPORT
void                   gtk_cifro_area_set_angle                (GtkCifroArea          *carea,
                                                                gdouble                angle);

GTK_CIFROAREA_EXPORT
gdouble                gtk_cifro_area_get_angle                (GtkCifroArea          *carea);

GTK_CIFROAREA_EXPORT
void                   gtk_cifro_area_move                     (GtkCifroArea          *carea,
                                                                gint                   step_x,
                                                                gint                   step_y);

GTK_CIFROAREA_EXPORT
void                   gtk_cifro_area_rotate                   (GtkCifroArea          *carea,
                                                                gdouble                angle);

GTK_CIFROAREA_EXPORT
void                   gtk_cifro_area_zoom                     (GtkCifroArea          *carea,
                                                                GtkCifroAreaZoomType   direction_x,
                                                                GtkCifroAreaZoomType   direction_y,
                                                                gdouble                center_x,
                                                                gdouble                center_y);

GTK_CIFROAREA_EXPORT
void                   gtk_cifro_area_get_size                 (GtkCifroArea          *carea,
                                                                guint                 *width,
                                                                guint                 *height);

GTK_CIFROAREA_EXPORT
void                   gtk_cifro_area_get_visible_size         (GtkCifroArea          *carea,
                                                                guint                 *width,
                                                                guint                 *height);

GTK_CIFROAREA_EXPORT
void                   gtk_cifro_area_point_to_value           (GtkCifroArea          *carea,
                                                                gdouble                x,
                                                                gdouble                y,
                                                                gdouble               *x_val,
                                                                gdouble               *y_val);

GTK_CIFROAREA_EXPORT
void                   gtk_cifro_area_value_to_point           (GtkCifroArea          *carea,
                                                                gdouble               *x,
                                                                gdouble               *y,
                                                                gdouble                x_val,
                                                                gdouble                y_val);

GTK_CIFROAREA_EXPORT
void                   gtk_cifro_area_visible_point_to_value   (GtkCifroArea          *carea,
                                                                gdouble                x,
                                                                gdouble                y,
                                                                gdouble               *x_val,
                                                                gdouble               *y_val);

GTK_CIFROAREA_EXPORT
void                   gtk_cifro_area_visible_value_to_point   (GtkCifroArea          *carea,
                                                                gdouble               *x,
                                                                gdouble               *y,
                                                                gdouble                x_val,
                                                                gdouble                y_val);

GTK_CIFROAREA_EXPORT
gboolean               gtk_cifro_area_get_axis_step            (gdouble                scale,
                                                                gdouble                step_width,
                                                                gdouble               *from,
                                                                gdouble               *step,
                                                                guint                 *range,
                                                                gint                  *power);

GTK_CIFROAREA_EXPORT
gdouble                gtk_cifro_area_point_to_cairo           (gdouble                point);

G_END_DECLS

#endif /* __GTK_CIFRO_AREA_H__ */
