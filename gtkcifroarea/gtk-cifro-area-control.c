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

/**
 * SECTION: gtk-cifro-area-control
 * @Short_description: GTK+ виджет управления видом отображения многослойных изображений
 * @Title: GtkCifroAreaControl
 * @See_also: #GtkCifroArea
 *
 * Виджет расширяет возможности #GtkCifroArea, добавляя функциональность управления
 * масштабированием, перемещением и поворотом с помощью клавиатуры и мышки.
 *
 */

#include "gtk-cifro-area-control.h"

#include <gdk/gdkkeysyms.h>
#include <string.h>
#include <math.h>

enum
{
  SCROLL_ACTION_NONE,
  SCROLL_ACTION_MOVE_X,
  SCROLL_ACTION_MOVE_Y,
  SCROLL_ACTION_ZOOM_X,
  SCROLL_ACTION_ZOOM_Y,
  SCROLL_ACTION_ZOOM_BOTH,
  SCROLL_ACTION_ROTATE
};

struct _GtkCifroAreaControlPrivate
{
  GtkCifroAreaScrollMode scroll_mode;          /* Режим обработки событий прокрутки колёсика мышки. */
  gdouble                move_step;            /* Величина шага перемещения. */
  gdouble                rotate_step;          /* Величина шага вращения. */

  gboolean               move_area;            /* Признак перемещения при нажатой клавише мыши. */
  gint                   move_start_x;         /* Начальная координата x перемещения. */
  gint                   move_start_y;         /* Начальная координата y перемещения. */
};

static gboolean  gtk_cifro_area_control_key_press             (GtkWidget                     *widget,
                                                               GdkEventKey                   *event);

static gboolean  gtk_cifro_area_control_button_press_release  (GtkWidget                     *widget,
                                                               GdkEventButton                *event);

static gboolean  gtk_cifro_area_control_motion                (GtkWidget                     *widget,
                                                               GdkEventMotion                *event);

static gboolean  gtk_cifro_area_control_scroll                (GtkWidget                     *widget,
                                                               GdkEventScroll                *event);

G_DEFINE_TYPE_WITH_PRIVATE (GtkCifroAreaControl, gtk_cifro_area_control, GTK_TYPE_CIFRO_AREA)

static void
gtk_cifro_area_control_init (GtkCifroAreaControl *control)
{
  GtkCifroAreaControlPrivate *priv = gtk_cifro_area_control_get_instance_private (control);
  gint event_mask = 0;

  control->priv = priv;

  priv->move_step = 1.0;
  priv->rotate_step = 1.0;

  event_mask |= GDK_KEY_PRESS_MASK;
  event_mask |= GDK_KEY_RELEASE_MASK;
  event_mask |= GDK_BUTTON_PRESS_MASK;
  event_mask |= GDK_BUTTON_RELEASE_MASK;
  event_mask |= GDK_POINTER_MOTION_MASK;
  event_mask |= GDK_POINTER_MOTION_HINT_MASK;
  event_mask |= GDK_SCROLL_MASK;
  gtk_widget_add_events (GTK_WIDGET (control), event_mask);
  gtk_widget_set_can_focus (GTK_WIDGET (control), TRUE);
}

static void
gtk_cifro_area_control_class_init (GtkCifroAreaControlClass *klass)
{
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  widget_class->key_press_event = gtk_cifro_area_control_key_press;
  widget_class->button_press_event = gtk_cifro_area_control_button_press_release;
  widget_class->button_release_event = gtk_cifro_area_control_button_press_release;
  widget_class->motion_notify_event = gtk_cifro_area_control_motion;
  widget_class->scroll_event = gtk_cifro_area_control_scroll;
}

/* Обработчик нажатия кнопок клавиатуры. */
static gboolean
gtk_cifro_area_control_key_press (GtkWidget   *widget,
                                  GdkEventKey *event)
{
  GtkCifroArea *carea = GTK_CIFRO_AREA (widget);
  GtkCifroAreaControl *control = GTK_CIFRO_AREA_CONTROL (widget);
  GtkCifroAreaControlPrivate *priv = control->priv;

  /* Перемещение области. */
  if (((event->keyval == GDK_KEY_Left) || (event->keyval == GDK_KEY_Right) ||
       (event->keyval == GDK_KEY_Up) || (event->keyval == GDK_KEY_Down) ||
       (event->keyval == GDK_KEY_Page_Up) || (event->keyval == GDK_KEY_Page_Down)) &&
      !(event->state & GDK_SHIFT_MASK))
    {
      gdouble move_step = 1.0;
      gdouble step_x = 0.0;
      gdouble step_y = 0.0;

      if ((event->state & GDK_CONTROL_MASK) ||
          (event->keyval == GDK_KEY_Page_Up) ||
          (event->keyval == GDK_KEY_Page_Down))
        {
          move_step = priv->move_step;
        }

      if (event->keyval == GDK_KEY_Left)
        step_x = -move_step;
      if (event->keyval == GDK_KEY_Right)
        step_x = move_step;
      if (event->keyval == GDK_KEY_Up)
        step_y = move_step;
      if (event->keyval == GDK_KEY_Down)
        step_y = -move_step;

      if (event->state & GDK_CONTROL_MASK)
        {
          if (event->keyval == GDK_KEY_Page_Up)
            step_x = -move_step;
          if (event->keyval == GDK_KEY_Page_Down)
            step_x = move_step;
        }
      else
        {
          if (event->keyval == GDK_KEY_Page_Up)
            step_y = move_step;
          if (event->keyval == GDK_KEY_Page_Down)
            step_y = -move_step;
        }

      gtk_cifro_area_move (carea, step_x, step_y);
    }

  /* Перемещение области в начало или конец. */
  if ((event->keyval == GDK_KEY_Home) || (event->keyval == GDK_KEY_End))
    {
      gdouble scale_x, scale_y;
      gdouble view_x, view_y;
      gdouble step_x, step_y;
      gdouble min_x, max_x;
      gdouble min_y, max_y;
      gdouble step;

      gtk_cifro_area_get_scale (carea, &scale_x, &scale_y);
      gtk_cifro_area_get_limits (carea, &min_x, &max_x, &min_y, &max_y);

      /* Расчитываем максимальное перемещение области. */
      view_x = (max_x - min_x) / scale_x;
      view_y = (max_y - min_y) / scale_y;
      step = sqrt (view_x * view_x + view_y * view_y);
      step_x = 0.0;
      step_y = 0.0;

      /* Смещаемся на эту величину. */
      if (event->state & GDK_CONTROL_MASK)
        {
          if (event->keyval == GDK_KEY_Home)
            step_x = -step;
          if (event->keyval == GDK_KEY_End)
            step_x = step;
        }
      else
        {
          if (event->keyval == GDK_KEY_Home)
            step_y = step;
          if (event->keyval == GDK_KEY_End)
            step_y = -step;
        }

      gtk_cifro_area_move (carea, step_x, step_y);
    }

  /* Поворот области. */
  if (((event->keyval == GDK_KEY_Left) || (event->keyval == GDK_KEY_Right)) &&
       (event->state & GDK_SHIFT_MASK))
    {
      gdouble rotate_step = (event->state & GDK_CONTROL_MASK) ? priv->rotate_step : 1.0;
      gdouble angle = 0.0;

      if (event->keyval == GDK_KEY_Left)
        angle = -G_PI / 180.0 * rotate_step;
      if (event->keyval == GDK_KEY_Right)
        angle = G_PI / 180.0 * rotate_step;

      gtk_cifro_area_rotate (carea, angle);
    }

  /* Масштабирование области. */
  if ((event->keyval == GDK_KEY_KP_Add) || (event->keyval == GDK_KEY_KP_Subtract) ||
      (event->keyval == GDK_KEY_plus) || (event->keyval == GDK_KEY_minus))
    {
      GtkCifroAreaZoomType direction_x = GTK_CIFRO_AREA_ZOOM_NONE;
      GtkCifroAreaZoomType direction_y = GTK_CIFRO_AREA_ZOOM_NONE;
      gdouble from_x, to_x;
      gdouble from_y, to_y;
      gdouble val_x, val_y;

      if ((event->keyval == GDK_KEY_KP_Add) || (event->keyval == GDK_KEY_plus))
        {
          direction_x = GTK_CIFRO_AREA_ZOOM_IN;
          direction_y = GTK_CIFRO_AREA_ZOOM_IN;
        }
      else if ((event->keyval == GDK_KEY_KP_Subtract) || (event->keyval == GDK_KEY_minus))
        {
          direction_x = GTK_CIFRO_AREA_ZOOM_OUT;
          direction_y = GTK_CIFRO_AREA_ZOOM_OUT;
        }

      if (event->state & GDK_CONTROL_MASK)
        direction_y = GTK_CIFRO_AREA_ZOOM_NONE;
      if (event->state & GDK_MOD1_MASK)
        direction_x = GTK_CIFRO_AREA_ZOOM_NONE;

      gtk_cifro_area_get_view (carea, &from_x, &to_x, &from_y, &to_y);

      val_x = from_x + (to_x - from_x) / 2.0;
      val_y = from_y + (to_y - from_y) / 2.0;

      gtk_cifro_area_zoom (carea, direction_x, direction_y, val_x, val_y);
    }

  return FALSE;
}

/* Обработчик нажатия кнопок мышки. */
static gboolean
gtk_cifro_area_control_button_press_release (GtkWidget      *widget,
                                             GdkEventButton *event)
{
  GtkCifroArea *carea = GTK_CIFRO_AREA (widget);
  GtkCifroAreaControl *control = GTK_CIFRO_AREA_CONTROL (widget);
  GtkCifroAreaControlPrivate *priv = control->priv;

  /* Нажата левая клавиша мышки в видимой области - переходим в режим перемещения. */
  if ((event->type == GDK_BUTTON_PRESS) && (event->button == 1))
    {
      guint widget_width, widget_height;
      guint border_top, border_bottom;
      guint border_left, border_right;
      gint clip_width, clip_height;

      gtk_cifro_area_get_size (carea, &widget_width, &widget_height);
      gtk_cifro_area_get_border (carea, &border_top, &border_bottom, &border_left, &border_right);

      clip_width = widget_width - border_left - border_right;
      clip_height = widget_height - border_top - border_bottom;
      if ((clip_width <= 0 ) || (clip_height <=0 ))
        return FALSE;

      if ((event->x > border_left) && (event->x < (border_left + clip_width)) &&
          (event->y > border_top) && (event->y < (border_top + clip_height)))
        {
          priv->move_area = TRUE;
          priv->move_start_x = event->x;
          priv->move_start_y = event->y;
        }
    }

  /* Выключаем режим перемещения. */
  if ((event->type == GDK_BUTTON_RELEASE) && (event->button == 1))
    {
      priv->move_area = FALSE;
    }

  return FALSE;
}

/* Обработчик перемещений курсора мыши. */
static gboolean
gtk_cifro_area_control_motion (GtkWidget      *widget,
                               GdkEventMotion *event)
{
  GtkCifroArea *carea = GTK_CIFRO_AREA (widget);
  GtkCifroAreaControl *control = GTK_CIFRO_AREA_CONTROL (widget);
  GtkCifroAreaControlPrivate *priv = control->priv;

  /* Режим перемещения - сдвигаем область. */
  if (priv->move_area)
    {
      gtk_cifro_area_move (carea, priv->move_start_x - event->x, event->y - priv->move_start_y);

      priv->move_start_x = event->x;
      priv->move_start_y = event->y;

      gdk_event_request_motions (event);
    }

  return FALSE;
}

/* Обработчик событий прокрутки колёсика мышки. */
static gboolean
gtk_cifro_area_control_scroll (GtkWidget      *widget,
                               GdkEventScroll *event)
{
  GtkCifroArea *carea = GTK_CIFRO_AREA (widget);
  GtkCifroAreaControl *control = GTK_CIFRO_AREA_CONTROL (widget);
  GtkCifroAreaControlPrivate *priv = control->priv;

  gint action = SCROLL_ACTION_NONE;
  GtkCifroAreaZoomType zoom_x = GTK_CIFRO_AREA_ZOOM_NONE;
  GtkCifroAreaZoomType zoom_y = GTK_CIFRO_AREA_ZOOM_NONE;
  gint step_x = 0;
  gint step_y = 0;

  /* Режимы обработки. */
  switch (priv->scroll_mode)
    {
    case GTK_CIFRO_AREA_SCROLL_MODE_ZOOM:
      if (event->state & GDK_CONTROL_MASK)
        action = SCROLL_ACTION_ZOOM_X;
      else if (event->state & GDK_MOD1_MASK)
        action = SCROLL_ACTION_ZOOM_Y;
      else if (event->state & GDK_SHIFT_MASK)
        action = SCROLL_ACTION_ZOOM_BOTH;
      break;

    case GTK_CIFRO_AREA_SCROLL_MODE_COMBINED:
      if (event->state & GDK_CONTROL_MASK)
        action = SCROLL_ACTION_MOVE_X;
      else if (event->state & GDK_MOD1_MASK)
        action = SCROLL_ACTION_ROTATE;
      else if (event->state & GDK_SHIFT_MASK)
        action = SCROLL_ACTION_ZOOM_BOTH;
      else
        action = SCROLL_ACTION_MOVE_Y;
      break;

    default:
      break;
    }

  /* Масштабирование. */
  if ((action == SCROLL_ACTION_ZOOM_X) || (action == SCROLL_ACTION_ZOOM_BOTH))
    zoom_x = (event->direction == GDK_SCROLL_UP) ? GTK_CIFRO_AREA_ZOOM_IN : GTK_CIFRO_AREA_ZOOM_OUT;
  if ((action == SCROLL_ACTION_ZOOM_Y) || (action == SCROLL_ACTION_ZOOM_BOTH))
    zoom_y = (event->direction == GDK_SCROLL_UP) ? GTK_CIFRO_AREA_ZOOM_IN : GTK_CIFRO_AREA_ZOOM_OUT;

  if ((zoom_x != GTK_CIFRO_AREA_ZOOM_NONE) || (zoom_y != GTK_CIFRO_AREA_ZOOM_NONE))
    {
      gdouble val_x, val_y;

      gtk_cifro_area_point_to_value (carea, event->x, event->y, &val_x, &val_y);
      gtk_cifro_area_zoom (carea, zoom_x, zoom_y, val_x, val_y);
    }

  /* Перемещение. */
  if (action == SCROLL_ACTION_MOVE_X)
    step_x = (event->direction == GDK_SCROLL_UP) ? -priv->move_step : priv->move_step;
  if (action == SCROLL_ACTION_MOVE_Y)
    step_y = (event->direction == GDK_SCROLL_UP) ? priv->move_step : -priv->move_step;

  if ((step_x != 0) || (step_y != 0))
    {
      gtk_cifro_area_move (carea, step_x, step_y);
    }

  /* Поворот. */
  if (action == SCROLL_ACTION_ROTATE)
    {
      gdouble angle;

      angle = (event->direction == GDK_SCROLL_UP) ? priv->rotate_step : -priv->rotate_step;
      gtk_cifro_area_rotate (carea, angle);
    }

  return FALSE;
}

/**
 * gtk_cifro_area_control_new:
 *
 * Функция создаёт виджет #GtkCifroAreaControl.
 *
 * Returns: #GtkCifroAreaControl.
 *
 */
GtkWidget *
gtk_cifro_area_control_new (void)
{
  return g_object_new (GTK_TYPE_CIFRO_AREA_CONTROL, NULL);
}

/**
 * gtk_cifro_area_control_set_scroll_mode:
 * @control: указатель на #GtkCifroAreaControl
 * @scroll_mode: режим обработки событий прокрутки
 *
 * Функция устанавливает режим обработки событий прокрутки колёсика мышки.
 *
 */
void
gtk_cifro_area_control_set_scroll_mode (GtkCifroAreaControl    *control,
                                        GtkCifroAreaScrollMode  scroll_mode)
{
  g_return_if_fail (GTK_IS_CIFRO_AREA_CONTROL (control));

  control->priv->scroll_mode = scroll_mode;
}

/**
 * gtk_cifro_area_control_get_scroll_mode:
 * @control: указатель на #GtkCifroArea
 *
 * Функция возвращает режим обработки событий прокрутки колёсика мышки.
 *
 * Returns: режим обработки событий прокрутки.
 *
 */
GtkCifroAreaScrollMode
gtk_cifro_area_control_get_scroll_mode (GtkCifroAreaControl *control)
{
  g_return_val_if_fail (GTK_IS_CIFRO_AREA_CONTROL (control), GTK_CIFRO_AREA_SCROLL_MODE_DISABLED);

  return control->priv->scroll_mode;
}

/**
 * gtk_cifro_area_control_set_move_step:
 * @control: указатель на #GtkCifroArea
 * @move_step: шаг перемещения, точки экрана
 *
 * Функция задаёт шаг перемещения изображения с помощью клавиш '↑', '↓', '←' и '→'
 * при нажатой клавише 'Ctrl', с помощью клавиш 'Page Up' и 'Page Down', а также при прокрутке
 * с помощью колёсика мышки. Шаг перемещения должен быть больше нуля.
 *
 */
void
gtk_cifro_area_control_set_move_step (GtkCifroAreaControl *control,
                                      gdouble              move_step)
{
  g_return_if_fail (GTK_IS_CIFRO_AREA_CONTROL (control));

  g_return_if_fail (move_step > 0.0);

  control->priv->move_step = move_step;
}

/**
 * gtk_cifro_area_control_get_move_step:
 * @control: указатель на #GtkCifroArea
 *
 * Функция возвращает текущий шаг перемещения изображения заданный функцией
 * gtk_cifro_area_control_set_move_step().
 *
 * Returns: шаг перемещения, точки экрана.
 *
 */
gdouble
gtk_cifro_area_control_get_move_step (GtkCifroAreaControl *control)
{
  g_return_val_if_fail (GTK_IS_CIFRO_AREA_CONTROL (control), 0.0);

  return control->priv->move_step;
}

/**
 * gtk_cifro_area_control_set_rotate_step:
 * @control: указатель на #GtkCifroArea
 * @rotate_step: шаг поворота, радианы
 *
 * Функция задаёт шаг поворота изображения с помощью клавиш '←' и '→' при нажатой
 * клавише 'Ctrl', а также при прокрутке с помощью колёсика мышки. Шаг поворота
 * должен быть больше нуля.
 *
 */
void
gtk_cifro_area_control_set_rotate_step (GtkCifroAreaControl *control,
                                        gdouble              rotate_step)
{
  g_return_if_fail (GTK_IS_CIFRO_AREA_CONTROL (control));

  g_return_if_fail (rotate_step > 0.0);

  control->priv->rotate_step = rotate_step;
}

/**
 * gtk_cifro_area_control_get_rotate_step:
 * @control: указатель на #GtkCifroArea
 *
 * Функция возвращает текущую величину поворота изображения заданную функцией
 * gtk_cifro_area_control_set_rotate_step().
 *
 * Returns: шаг поворота, радианы.
 *
 */
gdouble
gtk_cifro_area_control_get_rotate_step (GtkCifroAreaControl *control)
{
  g_return_val_if_fail (GTK_IS_CIFRO_AREA_CONTROL (control), 0.0);

  return control->priv->rotate_step;
}
