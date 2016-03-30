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
 * \file gtk-cifro-area.c
 *
 * \brief Исходный файл GTK+ виджета показа изображений сформированных интерфейсом IcaRenderer
 * \author Andrei Fadeev
 * \date 2013-2016
 * \license GNU General Public License version 3 или более поздняя<br>
 * Коммерческая лицензия - свяжитесь с автором
 *
 */

#include "gtk-cifro-area.h"
#include "gtk-cifro-area-marshallers.h"

#include <gdk/gdkkeysyms.h>
#include <string.h>
#include <math.h>

enum
{
  SIGNAL_CHECK_VISIBLE_REDRAW,
  SIGNAL_CHECK_AREA_REDRAW,
  SIGNAL_VISIBLE_DRAW,
  SIGNAL_AREA_DRAW,
  SIGNAL_LAST
};

struct _GtkCifroAreaPrivate
{
  GtkCifroAreaState   *state;                  /* Объект хранения состояния GtkCifroArea. */

  cairo_t             *visible_cairo;          /* Объект для рисования в видимой области. */

  gboolean             update_visible;         /* Перерисовать видимую область или использовать кэш. */

  gint                 border_left;            /* Размер области обрамления слева. */
  gint                 border_right;           /* Размер области обрамления справа. */
  gint                 border_top;             /* Размер области обрамления сверху. */
  gint                 border_bottom;          /* Размер области обрамления снизу. */

  gboolean             clip;                   /* Ограничивать или нет отрисовку областью внутри элемента обрамления. */
  gint                 clip_x;                 /* Начало области ограничения слева. */
  gint                 clip_y;                 /* Начало области ограничения сверху. */
  gint                 clip_width;             /* Ширина области ограничения. */
  gint                 clip_height;            /* Высота области ограничения. */

  gboolean             swap_x;                 /* TRUE - ось x направлена влево, FALSE - вправо. */
  gboolean             swap_y;                 /* TRUE - ось y направлена вниз, FALSE - вверх. */

  gboolean             scale_on_resize;        /* Изменять == TRUE или нет масштаб при изменении размера окна. */
  gdouble              scale_aspect;           /* Соотношение масштабов: scale_x / scale_y. В случае <= 0 - свободное соотношение. */

  gboolean             rotation;               /* Разрешён поворот или нет. */
  gdouble              angle;                  /* Угол поворота изображения в радианах. */
  gdouble              angle_cos;              /* Косинус угла поворота изображения. */
  gdouble              angle_sin;              /* Синус угла поворота изображения. */

  gdouble              from_x;                 /* Граница отображения по оси x слева. */
  gdouble              to_x;                   /* Граница отображения по оси x справа. */
  gdouble              from_y;                 /* Граница отображения по оси y снизу. */
  gdouble              to_y;                   /* Граница отображения по оси y сверху. */

  gdouble              min_x;                  /* Минимально возможное значение по оси x. */
  gdouble              max_x;                  /* Максимально возможное значение по оси x. */
  gdouble              min_y;                  /* Минимально возможное значение по оси y. */
  gdouble              max_y;                  /* Максимально возможное значение по оси y. */

  gdouble              cur_scale_x;            /* Текущий коэффициент масштаба по оси x. */
  gdouble              min_scale_x;            /* Минимально возможный коэффициент масштаба по оси x (приближение). */
  gdouble              max_scale_x;            /* Максимально возможный коэффициент масштаба по оси x (отдаление). */
  gdouble              cur_scale_y;            /* Текущий коэффициент масштаба по оси y. */
  gdouble              min_scale_y;            /* Минимально возможный коэффициент масштаба по оси y (приближение). */
  gdouble              max_scale_y;            /* Максимально возможный коэффициент масштаба по оси y (отдаление). */

  gint                 widget_width;           /* Ширина окна CifroArea. */
  gint                 widget_height;          /* Высота окна CifroArea. */
  gint                 visible_width;          /* Ширина видимой области лля отрисовки данных. */
  gint                 visible_height;         /* Высота видимой области для отрисовки данных. */

  gboolean             zoom_on_center;         /* Устанавливать центр области масштабирования по курсору или нет. */
  gdouble              zoom_scale;             /* Коэффициент изменения масштаба. */

  gdouble             *zoom_x_scales;          /* Набор коэффициентов масштабирования по оси x. */
  gdouble             *zoom_y_scales;          /* Набор коэффициентов масштабирования по оси y. */
  gint                 num_scales;             /* Число коэффициентов масштабирования. */
  gint                 cur_zoom_index;         /* Текущий индекс коэффициента масштабирования. */

  gboolean             draw_focus;             /* Рисовать или нет индикатор наличия фокуса ввода. */

  gdouble              move_multiplier;        /* Значение множителя скорости перемещения при нажатой клавише control. */
  gdouble              rotate_multiplier;      /* Значение множителя скорости вращения при нажатой клавише control. */

  gboolean             move_area;              /* Признак перемещения при нажатой клавише мыши. */
  gint                 move_start_x;           /* Начальная координата x перемещения. */
  gint                 move_start_y;           /* Начальная координата y перемещения. */

  GdkCursor           *current_cursor;         /* Текущий курсор. */
  GdkCursor           *point_cursor;           /* Курсор используемый при нахождении мышки в видимой области. */
  GdkCursor           *move_cursor;            /* Курсор используемый при перемещении видимой области. */
};

static void            gtk_cifro_area_object_constructed       (GObject                       *carea);
static void            gtk_cifro_area_object_finalize          (GObject                       *carea);

static gint            gtk_cifro_area_get_visible_width        (gdouble                        width,
                                                                gdouble                        height,
                                                                gdouble                        angle);
static gint            gtk_cifro_area_get_visible_height       (gdouble                        width,
                                                                gdouble                        height,
                                                                gdouble                        angle);
static gboolean        gtk_cifro_area_fix_scale_aspect         (gdouble                        scale_aspect,
                                                                gdouble                       *min_scale_x,
                                                                gdouble                       *max_scale_x,
                                                                gdouble                       *min_scale_y,
                                                                gdouble                       *max_scale_y );
static void            gtk_cifro_area_update_visible           (GtkCifroAreaPrivate           *priv);

static gboolean        gtk_cifro_area_key_press                (GtkWidget                     *widget,
                                                                GdkEventKey                   *event,
                                                                gpointer                       data);
static gboolean        gtk_cifro_area_button_press_release     (GtkWidget                     *widget,
                                                                GdkEventButton                *event,
                                                                gpointer                       data);
static gboolean        gtk_cifro_area_motion                   (GtkWidget                     *widget,
                                                                GdkEventMotion                *event,
                                                                gpointer                       data);
static gboolean        gtk_cifro_area_scroll                   (GtkWidget                     *widget,
                                                                GdkEventScroll                *event,
                                                                gpointer                       data);

static gboolean        gtk_cifro_area_configure                (GtkWidget                     *widget,
                                                                GdkEventConfigure             *event,
                                                                gpointer                       data);
static gboolean        gtk_cifro_area_draw                     (GtkWidget                     *widget,
                                                                cairo_t                       *cairo,
                                                                gpointer                       data);
#ifdef CIFRO_AREA_WITH_GTK2
static gboolean        gtk_cifro_area_expose                   (GtkWidget                     *widget,
                                                                GdkEventExpose                *event,
                                                                gpointer                       data);
#endif

static guint gtk_cifro_area_signals [SIGNAL_LAST] = {0};

G_DEFINE_TYPE_WITH_PRIVATE (GtkCifroArea, gtk_cifro_area, GTK_TYPE_DRAWING_AREA)

static void
gtk_cifro_area_init (GtkCifroArea *carea)
{
  carea->priv = gtk_cifro_area_get_instance_private (carea);
}

static void
gtk_cifro_area_class_init (GtkCifroAreaClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->constructed = gtk_cifro_area_object_constructed;
  object_class->finalize = gtk_cifro_area_object_finalize;

  gtk_cifro_area_signals[SIGNAL_CHECK_VISIBLE_REDRAW] =
    g_signal_new ("check-visible-redraw", GTK_TYPE_CIFRO_AREA,
                  G_SIGNAL_RUN_LAST, 0,
                  g_signal_accumulator_true_handled, NULL,
                  g_cclosure_user_marshal_BOOLEAN__VOID,
                  G_TYPE_BOOLEAN,
                  0);

  gtk_cifro_area_signals[SIGNAL_CHECK_AREA_REDRAW] =
    g_signal_new ("check-area-redraw", GTK_TYPE_CIFRO_AREA,
                  G_SIGNAL_RUN_LAST, 0,
                  g_signal_accumulator_true_handled, NULL,
                  g_cclosure_user_marshal_BOOLEAN__VOID,
                  G_TYPE_BOOLEAN,
                  0);

  gtk_cifro_area_signals[SIGNAL_VISIBLE_DRAW] =
    g_signal_new ("visible-draw", GTK_TYPE_CIFRO_AREA,
                  G_SIGNAL_RUN_LAST, 0,
                  NULL, NULL,
                  g_cclosure_marshal_VOID__POINTER,
                  G_TYPE_NONE,
                  1, G_TYPE_POINTER);

  gtk_cifro_area_signals[SIGNAL_AREA_DRAW] =
    g_signal_new ("area-draw", GTK_TYPE_CIFRO_AREA,
                  G_SIGNAL_RUN_LAST, 0,
                  NULL, NULL,
                  g_cclosure_marshal_VOID__POINTER,
                  G_TYPE_NONE,
                  1, G_TYPE_POINTER);
}

static void
gtk_cifro_area_object_constructed (GObject *object)
{
  GtkCifroArea *carea = GTK_CIFRO_AREA (object);
  GtkCifroAreaPrivate *priv = carea->priv;

  gint event_mask = 0;

  G_OBJECT_CLASS (gtk_cifro_area_parent_class)->constructed (object);

  priv->rotation = TRUE;
  priv->angle = 0.0;
  priv->angle_cos = cos (priv->angle);
  priv->angle_sin = sin (priv->angle);

  priv->cur_scale_x = 0.0;
  priv->min_scale_x = 0.001;
  priv->max_scale_x = 1000.0;
  priv->cur_scale_y = 0.0;
  priv->min_scale_y = 0.001;
  priv->max_scale_y = 1000.0;

  priv->from_x = -1.0;
  priv->to_x = 1.0;
  priv->from_y = -1.0;
  priv->to_y = 1.0;

  priv->min_x = -1.0;
  priv->max_x = 1.0;
  priv->min_y = -1.0;
  priv->max_y = 1.0;

  priv->zoom_on_center = FALSE;
  priv->zoom_scale = 10.0;

  priv->draw_focus = TRUE;

  priv->move_multiplier = 10.0;
  priv->rotate_multiplier = 2.0;

  priv->current_cursor = NULL;
  priv->point_cursor = gdk_cursor_new_for_display (gdk_display_get_default (), GDK_CROSSHAIR);
  priv->move_cursor = gdk_cursor_new_for_display (gdk_display_get_default (), GDK_FLEUR);

  event_mask |= GDK_KEY_PRESS_MASK;
  event_mask |= GDK_BUTTON_PRESS_MASK;
  event_mask |= GDK_BUTTON_RELEASE_MASK;
  event_mask |= GDK_POINTER_MOTION_MASK;
  event_mask |= GDK_POINTER_MOTION_HINT_MASK;
  event_mask |= GDK_SCROLL_MASK;
  gtk_widget_add_events (GTK_WIDGET (carea), event_mask);
  gtk_widget_set_can_focus (GTK_WIDGET (carea), TRUE);

  /* Объект хранения состояния GtkCifroArea. */
  priv->state = gtk_cifro_area_state_new ();

  g_signal_connect (carea, "configure-event", G_CALLBACK (gtk_cifro_area_configure), NULL);
  g_signal_connect_after (carea, "key-press-event", G_CALLBACK (gtk_cifro_area_key_press), NULL);
  g_signal_connect_after (carea, "button-press-event", G_CALLBACK (gtk_cifro_area_button_press_release), NULL);
  g_signal_connect_after (carea, "button-release-event", G_CALLBACK (gtk_cifro_area_button_press_release), NULL);
  g_signal_connect_after (carea, "motion-notify-event", G_CALLBACK (gtk_cifro_area_motion), NULL);
  g_signal_connect_after (carea, "scroll-event", G_CALLBACK (gtk_cifro_area_scroll), NULL);

#ifdef CIFRO_AREA_WITH_GTK2
  g_signal_connect (carea , "expose-event", G_CALLBACK (gtk_cifro_area_expose), NULL);
#else
  g_signal_connect (carea, "draw", G_CALLBACK (gtk_cifro_area_draw), NULL);
#endif
}

static void
gtk_cifro_area_object_finalize (GObject *object)
{
  GtkCifroArea *carea = GTK_CIFRO_AREA (object);
  GtkCifroAreaPrivate *priv = carea->priv;

  cairo_destroy (priv->visible_cairo);
  g_object_unref (priv->state);

#ifdef CIFRO_AREA_WITH_GTK2
  if (priv->point_cursor)
    gdk_cursor_unref (priv->point_cursor);
  if (priv->move_cursor)
    gdk_cursor_unref (priv->move_cursor);
#else
  if (priv->point_cursor)
    g_object_unref (priv->point_cursor);
  if (priv->move_cursor)
    g_object_unref (priv->move_cursor);
#endif

  G_OBJECT_CLASS (gtk_cifro_area_parent_class)->finalize (object);
}

/* Функция расчёта ширины видимой области для прямоугольника width x height, повёрнутого на угол angle. */
static gint
gtk_cifro_area_get_visible_width (gdouble width,
                                  gdouble height,
                                  gdouble angle)
{
  gint visible_width;

  angle = fabs (angle);
  if (angle > G_PI_2)
    angle = G_PI - angle;

  visible_width = round (fabs (cos (G_PI_2 - atan (width / height) - angle)) * sqrt (width * width + height * height));
  visible_width += (visible_width % 2);

  return visible_width;
}

/* Функция расчёта высоты видимой области для прямоугольника width x height, повёрнутого на угол angle. */
static gint
gtk_cifro_area_get_visible_height (gdouble width,
                                   gdouble height,
                                   gdouble angle)
{
  gint visible_height;

  angle = fabs (angle);
  if (angle > G_PI_2)
    angle = G_PI - angle;

  visible_height = round (fabs (cos (atan (width / height) - angle)) * sqrt (width * width + height * height));
  visible_height += (visible_height % 2);

  return visible_height;
}

/* Функция коррекции диапазонов масштабов при фиксированых соотношениях. */
static gboolean
gtk_cifro_area_fix_scale_aspect (gdouble  scale_aspect,
                                 gdouble *min_scale_x,
                                 gdouble *max_scale_x,
                                 gdouble *min_scale_y,
                                 gdouble *max_scale_y)
{
  gdouble new_min_scale_x;
  gdouble new_max_scale_x;
  gdouble new_min_scale_y;
  gdouble new_max_scale_y;

  if (scale_aspect <= 0.0)
    return TRUE;

  new_min_scale_y = CLAMP (*min_scale_x * scale_aspect, *min_scale_y, *max_scale_y);
  new_max_scale_y = CLAMP (*max_scale_x * scale_aspect, *min_scale_y, *max_scale_y);

  if (new_min_scale_y == new_max_scale_y)
    return FALSE;

  new_min_scale_x = CLAMP (new_min_scale_y / scale_aspect, *min_scale_x, *max_scale_x);
  new_max_scale_x = CLAMP (new_max_scale_y / scale_aspect, *min_scale_x, *max_scale_x);

  if (new_min_scale_x == new_max_scale_x)
    return FALSE;

  *min_scale_x = new_min_scale_x;
  *max_scale_x = new_max_scale_x;
  *min_scale_y = new_min_scale_y;
  *max_scale_y = new_max_scale_y;

  return TRUE;
}

/* Функция перерасчёта параметров отображения. */
static void
gtk_cifro_area_update_visible (GtkCifroAreaPrivate *priv)
{
  gdouble width;
  gdouble height;
  gdouble visible_width;
  gdouble visible_height;
  gdouble new_scale_x;
  gdouble new_scale_y;
  gdouble x_width;
  gdouble y_height;
  gdouble x, y;

  gint ivisible_width;
  gint ivisible_height;

  /* Размер видимой области отображения. */
  width = priv->widget_width - priv->border_left - priv->border_right;
  height = priv->widget_height - priv->border_top - priv->border_bottom;

  /* Проверяем размеры виджета. */
  if (width <= 0 || height <= 0)
    return;

  /* Размер видимой области отображения с учётом поворота. */
  visible_width = gtk_cifro_area_get_visible_width (width, height, priv->angle);
  visible_height = gtk_cifro_area_get_visible_height (width, height, priv->angle);

  /* Расчёт масштабов в начале и при задании области видимости. */
  if ((priv->cur_scale_x == 0.0) && (priv->cur_scale_y == 0.0))
    {
      priv->cur_scale_x = (priv->to_x - priv->from_x) / visible_width;
      priv->cur_scale_y = (priv->to_y - priv->from_y) / visible_height;
    }

  /* Коррекция масштабов в случае заданного соотношения. */
  if (priv->scale_aspect > 0.0 && priv->num_scales == 0)
    {
      new_scale_x = priv->cur_scale_y / priv->scale_aspect;
      new_scale_y = priv->cur_scale_x * priv->scale_aspect;
      new_scale_x = CLAMP (new_scale_x, priv->min_scale_x, priv->max_scale_x);
      new_scale_y = CLAMP (new_scale_y, priv->min_scale_y, priv->max_scale_y);

      if (new_scale_x < new_scale_y)
        {
          priv->cur_scale_x = new_scale_x;
          priv->cur_scale_y = new_scale_x / priv->scale_aspect;
        }
      else
        {
          priv->cur_scale_y = new_scale_y;
          priv->cur_scale_x = new_scale_y * priv->scale_aspect;
        }
    }

  /* Проверка на выход за границы, кроме случая фиксированных значений. */
  if (priv->num_scales == 0)
    {
      priv->cur_scale_x = CLAMP (priv->cur_scale_x, priv->min_scale_x, priv->max_scale_x);
      priv->cur_scale_y = CLAMP (priv->cur_scale_y, priv->min_scale_y, priv->max_scale_y);
    }

  /* Расчёт области видимости для получившихся масштабов и проверка
     на выход за границы допустимых значений. */
  x = priv->from_x + (priv->to_x - priv->from_x) / 2.0;
  y = priv->from_y + (priv->to_y - priv->from_y) / 2.0;

  x_width = priv->cur_scale_x * visible_width;
  y_height = priv->cur_scale_y * visible_height;

  if (x_width >= priv->max_x - priv->min_x)
    {
      priv->from_x = priv->min_x;
      priv->to_x = priv->max_x;
    }
  else
    {
      priv->from_x = x - x_width / 2.0;
      priv->to_x = x + x_width / 2.0;
      if (priv->from_x < priv->min_x)
        {
          priv->from_x = priv->min_x;
          priv->to_x = priv->min_x + x_width;
        }
      if (priv->to_x > priv->max_x)
        {
          priv->from_x = priv->max_x - x_width;
          priv->to_x = priv->max_x;
        }
    }

  if (y_height >= priv->max_y - priv->min_y)
    {
      priv->from_y = priv->min_y;
      priv->to_y = priv->max_y;
    }
  else
    {
      priv->from_y = y - y_height / 2.0;
      priv->to_y = y + y_height / 2.0;
      if (priv->from_y < priv->min_y)
        {
          priv->from_y = priv->min_y;
          priv->to_y = priv->min_y + y_height;
        }
      if (priv->to_y > priv->max_y)
        {
          priv->from_y = priv->max_y - y_height;
          priv->to_y = priv->max_y;
        }
    }

  /* Перерасчёт размеров видимой области в соответствии со скорректированной областью видимости. */
  visible_width = (priv->to_x - priv->from_x) / priv->cur_scale_x;
  visible_height = (priv->to_y - priv->from_y) / priv->cur_scale_y;
  ivisible_width = round (visible_width);
  ivisible_height = round (visible_height);

  /* Размеры видимой области изменились. */
  if ((priv->visible_width != ivisible_width) || (priv->visible_height != ivisible_height))
    {
      /* Если новые размеры стали больше, пересоздаём объекты рисования в видимой области. */
      if (ivisible_width > priv->visible_width || ivisible_height > priv->visible_height)
        {
          cairo_surface_t *surface;

          if (priv->visible_cairo != NULL)
            cairo_destroy (priv->visible_cairo);

          surface = cairo_image_surface_create (CAIRO_FORMAT_ARGB32, ivisible_width, ivisible_height);
          priv->visible_cairo = cairo_create (surface);
          cairo_surface_destroy (surface);

          priv->update_visible = TRUE;
        }

      /* Запоминаем новые размеры видимой области и ссобщаем их. */
      priv->visible_width = ivisible_width;
      priv->visible_height = ivisible_height;
      gtk_cifro_area_state_set_visible_size (priv->state, ivisible_width, ivisible_height);
    }

  /* Установка новых параметров состояния GtkCifroArea. */
  gtk_cifro_area_state_set_view (priv->state, priv->from_x, priv->to_x, priv->from_y, priv->to_y);
  gtk_cifro_area_state_set_angle (priv->state, priv->angle);

  /* Границы маски рисования слоёв с поворотом. */
  if (priv->border_left || priv->border_right || priv->border_top || priv->border_bottom)
    {
      priv->clip_x = priv->border_left;
      priv->clip_y = priv->border_top;
      priv->clip_width = priv->widget_width - priv->border_left - priv->border_right;
      priv->clip_height = priv->widget_height - priv->border_top - priv->border_bottom;
      priv->clip = TRUE;
    }
  else
    {
      priv->clip_width = priv->widget_width;
      priv->clip_height = priv->widget_height;
      priv->clip = FALSE;
    }
}

/* Обработчик нажатия кнопок клавиатуры. */
static gboolean
gtk_cifro_area_key_press (GtkWidget   *widget,
                          GdkEventKey *event,
                          gpointer     data)
{
  GtkCifroArea *carea = GTK_CIFRO_AREA (widget);
  GtkCifroAreaPrivate *priv = carea->priv;

  /* Перемещение области. */
  if (((event->keyval == GDK_KEY_Left) || (event->keyval == GDK_KEY_Right) ||
       (event->keyval == GDK_KEY_Up) || (event->keyval == GDK_KEY_Down)) &&
      !(event->state & GDK_SHIFT_MASK))
    {
      gdouble multiplier = (event->state & GDK_CONTROL_MASK) ? priv->move_multiplier : 1.0;
      gdouble step_x = 0;
      gdouble step_y = 0;

      if (event->keyval == GDK_KEY_Left)
        {
          step_x = multiplier;
        }
      if (event->keyval == GDK_KEY_Right)
        {
          step_x = -multiplier;
        }
      if (event->keyval == GDK_KEY_Up)
        {
          step_y = -multiplier;
        }
      if (event->keyval == GDK_KEY_Down)
        {
          step_y = multiplier;
        }

      gtk_cifro_area_move (GTK_CIFRO_AREA (widget), step_x, step_y);

      return TRUE;
    }

  /* Поворот области. */
  if ((event->state & GDK_SHIFT_MASK) && ((event->keyval == GDK_KEY_Left) || (event->keyval == GDK_KEY_Right)))
    {
      gdouble multiplier = (event->state & GDK_CONTROL_MASK) ? priv->rotate_multiplier : 1.0;
      gdouble angle = 0.0;

      if (event->keyval == GDK_KEY_Left)
        angle = -G_PI / 180.0 * multiplier;
      if (event->keyval == GDK_KEY_Right)
        angle = G_PI / 180.0 * multiplier;

      gtk_cifro_area_rotate (GTK_CIFRO_AREA (widget), angle);

      return TRUE;
    }

  /* Масштабирование области. */
  if ((event->keyval == GDK_KEY_KP_Add) || (event->keyval == GDK_KEY_KP_Subtract) ||
      (event->keyval == GDK_KEY_plus) || (event->keyval == GDK_KEY_minus))
    {
      gboolean zoom_x = TRUE;
      gboolean zoom_y = TRUE;
      gboolean zoom_in = TRUE;
      gdouble val_x, val_y;

      if (event->state & GDK_CONTROL_MASK)
        zoom_x = FALSE;
      if (event->state & GDK_MOD1_MASK)
        zoom_y = FALSE;
      if ((event->keyval == GDK_KEY_KP_Subtract) || (event->keyval == GDK_KEY_minus))
        zoom_in = FALSE;

      val_x = priv->from_x + (priv->to_x - priv->from_x) / 2.0;
      val_y = priv->from_y + (priv->to_y - priv->from_y) / 2.0;

      if (priv->num_scales)
        gtk_cifro_area_fixed_zoom (GTK_CIFRO_AREA (widget), val_x, val_y, zoom_in);
      else
        gtk_cifro_area_zoom (GTK_CIFRO_AREA (widget), zoom_x, zoom_y, val_x, val_y, zoom_in, priv->zoom_scale);

      return TRUE;
    }

  return FALSE;
}

/* Обработчик нажатия кнопок мышки. */
static gboolean
gtk_cifro_area_button_press_release (GtkWidget      *widget,
                                     GdkEventButton *event,
                                     gpointer        data)
{
  GtkCifroArea *carea = GTK_CIFRO_AREA (widget);
  GtkCifroAreaPrivate *priv = carea->priv;
  GdkWindow *window = gtk_widget_get_window (widget);

  if (event->type == GDK_BUTTON_PRESS && (event->button == 1))
    {
      gtk_widget_grab_focus (widget);

      /* Нажата левая клавиша мышки в области обрамления - переходим в режим перемещения. */
      if ((event->x > priv->clip_x) && (event->x < priv->clip_x + priv->clip_width) &&
          (event->y > priv->clip_y) && (event->y < priv->clip_x + priv->clip_height))
        {
          priv->move_area = TRUE;
          priv->move_start_x = event->x;
          priv->move_start_y = event->y;

          gdk_window_set_cursor (window, priv->move_cursor);

          return TRUE;
        }
    }

  /* Выключаем режим перемещения. */
  if (event->type == GDK_BUTTON_RELEASE && (event->button == 1))
    {
      priv->move_area = FALSE;

      gdk_window_set_cursor (window, priv->point_cursor);

      return TRUE;
    }

  return FALSE;
}

/* Обработчик перемещений курсора мыши. */
static gboolean
gtk_cifro_area_motion (GtkWidget      *widget,
                       GdkEventMotion *event,
                       gpointer        data)
{
  GtkCifroArea *carea = GTK_CIFRO_AREA (widget);
  GtkCifroAreaPrivate *priv = carea->priv;
  GdkWindow *window = gtk_widget_get_window (widget);

  gint x = event->x;
  gint y = event->y;

  /* Устанавливаем вид курсора в зависимости от области нахождения. */
  if (!priv->move_area)
    {
      if ((x > priv->clip_x) && (x < priv->clip_x + priv->clip_width) &&
          (y > priv->clip_y) && (y < priv->clip_x + priv->clip_height))
        {
          gdk_window_set_cursor (window, priv->point_cursor);
        }
      else
        {
          gdk_window_set_cursor (window, NULL);
        }
    }

  /* Режим перемещения - сдвигаем область. */
  if (priv->move_area)
    {
      gtk_cifro_area_move (GTK_CIFRO_AREA (widget), priv->move_start_x - x, y - priv->move_start_y);

      priv->move_start_x = x;
      priv->move_start_y = y;
    }

  gdk_event_request_motions (event);

  return FALSE;
}

/* Обработчик событий прокрутки колёсика мышки - масштабирование. */
static gboolean
gtk_cifro_area_scroll (GtkWidget      *widget,
                       GdkEventScroll *event,
                       gpointer        data)
{
  GtkCifroArea *carea = GTK_CIFRO_AREA (widget);
  GtkCifroAreaPrivate *priv = carea->priv;

  gboolean zoom_x;
  gboolean zoom_y;
  gboolean zoom_in;
  gdouble val_x, val_y;

  /* Обрабатываем только события внутри области отображения данных. */
  if ((event->x < priv->clip_x) || (event->x > priv->clip_x + priv->clip_width))
    return TRUE;
  if ((event->y < priv->clip_y) || (event->y > priv->clip_x + priv->clip_height))
    return TRUE;

  if (!(event->state & GDK_SHIFT_MASK) && !(event->state & GDK_CONTROL_MASK) && !(event->state & GDK_MOD1_MASK))
    return TRUE;

  /* Параметры масштабирования. */
  zoom_x = (event->state & GDK_SHIFT_MASK) || (event->state & GDK_MOD1_MASK);
  zoom_y = (event->state & GDK_SHIFT_MASK) || (event->state & GDK_CONTROL_MASK);
  zoom_in = (event->direction == GDK_SCROLL_UP) ? TRUE : FALSE;

  /* Точка, относительно которой будет производится масштабирование. */
  if (priv->zoom_on_center)
    {
      val_x = priv->from_x + (priv->to_x - priv->from_x) / 2.0;
      val_y = priv->from_y + (priv->to_y - priv->from_y) / 2.0;
    }
  else
    {
      gtk_cifro_area_state_visible_point_to_value (priv->state, event->x, event->y, &val_x, &val_y);
      if ((val_x < priv->min_x) || (val_x > priv->max_x))
        return TRUE;
      if ((val_y < priv->min_y) || (val_y > priv->max_y))
        return TRUE;
    }

  /* Масштабирование. */
  if (priv->num_scales)
    gtk_cifro_area_fixed_zoom (GTK_CIFRO_AREA (widget), val_x, val_y, zoom_in);
  else
    gtk_cifro_area_zoom (GTK_CIFRO_AREA (widget), zoom_x, zoom_y, val_x, val_y, zoom_in, priv->zoom_scale);

  return TRUE;
}

/* Обработчик изменения размеров виджета. */
static gboolean
gtk_cifro_area_configure (GtkWidget         *widget,
                          GdkEventConfigure *event,
                          gpointer           data)
{
  GtkCifroArea *carea = GTK_CIFRO_AREA (widget);
  GtkCifroAreaPrivate *priv = carea->priv;

  gint widget_width = event->width - (event->width % 2);
  gint widget_height = event->height - (event->height % 2);

  if (priv->widget_width == widget_width && priv->widget_height == widget_height)
    return FALSE;

  /* Новые размеры виджета. */
  priv->widget_width = widget_width;
  priv->widget_height = widget_height;
  gtk_cifro_area_state_set_area_size (priv->state, widget_width, widget_height);

  if (priv->scale_on_resize && (priv->scale_aspect < 0.0))
    gtk_cifro_area_set_view (GTK_CIFRO_AREA (widget), priv->from_x, priv->to_x, priv->from_y, priv->to_y);
  else
    gtk_cifro_area_update_visible (priv);

  return FALSE;
}

/* Обработчик рисования содержимого виджета GTK 3. */
static gboolean
gtk_cifro_area_draw (GtkWidget *widget,
                     cairo_t   *cairo,
                     gpointer   data)
{
  GtkCifroArea *carea = GTK_CIFRO_AREA (widget);
  GtkCifroAreaPrivate *priv = carea->priv;

  gdouble cairo_width = priv->widget_width;
  gdouble cairo_height = priv->widget_height;
  gdouble shift_width = ((priv->widget_width - priv->visible_width) / 2.0);
  gdouble shift_height = ((priv->widget_height - priv->visible_height) / 2.0);
  gdouble angle = priv->angle;

  if (priv->swap_x)
    angle = -angle;
  if (priv->swap_y)
    angle = -angle;

  /* Если необходима перерисовка видимой области, выполняем её. */
  if (priv->update_visible && priv->visible_cairo)
    {
      cairo_surface_t *surface = cairo_get_target (priv->visible_cairo);
      gpointer data = cairo_image_surface_get_data (surface);
      gssize dsize = cairo_image_surface_get_stride (surface);
      dsize *= cairo_image_surface_get_height (surface);

      /* Перед перерисовкой очищаем поверхность до прозрачного состояния
         и выполняем перерисовку видимой области. */
      cairo_surface_flush (surface);
      memset (data, 0, dsize);
      g_signal_emit (GTK_CIFRO_AREA (widget), gtk_cifro_area_signals[SIGNAL_VISIBLE_DRAW], 0, priv->visible_cairo);
      cairo_surface_mark_dirty (surface);

      priv->update_visible = FALSE;
    }

  /* Отображаем видимую область. */
  if (priv->visible_cairo)
    {
      cairo_save (cairo);

      cairo_set_operator (cairo, CAIRO_OPERATOR_OVER);

      if (priv->clip)
        {
          cairo_rectangle (cairo, priv->clip_x, priv->clip_y, priv->clip_width, priv->clip_height);
          cairo_clip (cairo);
        }

      if (priv->swap_x)
        {
          cairo_scale (cairo, -1.0, 1.0);
          cairo_translate (cairo, -cairo_width, 0);
        }

      if (priv->swap_y)
        {
          cairo_scale (cairo, 1.0, -1.0);
          cairo_translate (cairo, 0, -cairo_height);
        }

      if (priv->angle != 0.0)
        {
          cairo_translate (cairo, cairo_width / 2.0, cairo_height / 2.0);
          cairo_rotate (cairo, angle);
          cairo_translate (cairo, -cairo_width / 2.0, -cairo_height / 2.0);
        }

      cairo_set_source_surface (cairo, cairo_get_target (priv->visible_cairo), shift_width, shift_height);
      cairo_paint (cairo);

      cairo_restore (cairo);
    }

  /* Отображаем область всего виджета. */
  cairo_save (cairo);
  g_signal_emit (GTK_CIFRO_AREA (widget), gtk_cifro_area_signals[SIGNAL_AREA_DRAW], 0, cairo);
  cairo_restore (cairo);

  /* Рисуем рамку фокуса. */
#ifdef CIFRO_AREA_WITH_GTK2
  if (priv->draw_focus && gtk_widget_is_focus (widget))
    gtk_paint_focus (widget->style, widget->window, GTK_STATE_NORMAL, NULL, NULL, NULL,
                     0, 0, widget->allocation.width, widget->allocation.height);
#else
  if (priv->draw_focus && gtk_widget_is_focus (widget))
    gtk_render_focus (gtk_widget_get_style_context (widget), cairo, 0, 0,
                      gtk_widget_get_allocated_width (widget),
                      gtk_widget_get_allocated_height (widget));
#endif

  return TRUE;
}

#ifdef CIFRO_AREA_WITH_GTK2

/* Обработчик рисования содержимого виджета GTK 2. */
static gboolean
gtk_cifro_area_expose (GtkWidget      *widget,
                       GdkEventExpose *event,
                       gpointer        data)
{
  cairo_t *cairo = gdk_cairo_create (widget->window);
  gboolean status = gtk_cifro_area_draw (widget, cairo, NULL);

  cairo_destroy (cairo);

  return status;
}

#endif

/* Функция создания объекта CifroArea. */
GtkWidget *
gtk_cifro_area_new (void)
{
  return g_object_new (GTK_TYPE_CIFRO_AREA, NULL);
}

/* Функция возвращает указатель на объект GtkCifroAreaState. */
GtkCifroAreaState *
gtk_cifro_area_get_state (GtkCifroArea *carea)
{
  g_return_val_if_fail (GTK_IS_CIFRO_AREA (carea), NULL);

  return carea->priv->state;
}

/* Функция планирует перерисовку. */
void
gtk_cifro_area_update (GtkCifroArea *carea)
{
  g_return_if_fail (GTK_IS_CIFRO_AREA (carea));

  gboolean redraw = FALSE;

  /* Запрашиваем необходимость перерисовки видимой области. */
  g_signal_emit (carea, gtk_cifro_area_signals[SIGNAL_CHECK_VISIBLE_REDRAW], 0, &redraw);

  /* Если необходимо перерисовать видимую область - устанавливаем флаг обновления кэша ... */
  if (redraw)
    carea->priv->update_visible = TRUE;

  /* ... или проверяем необходимость перерисовки области виджета. */
  else
    g_signal_emit (carea, gtk_cifro_area_signals[SIGNAL_CHECK_AREA_REDRAW], 0, &redraw);

  /* Если необходима перерисовка - планируем её. */
  if (redraw)
    gtk_widget_queue_draw (GTK_WIDGET (carea));
}

/* Функция устанавливает необходимости рисования рамки при наличии фокуса ввода в объект CifroArea. */
void
gtk_cifro_area_set_draw_focus (GtkCifroArea *carea,
                               gboolean draw_focus)
{
  g_return_if_fail (GTK_IS_CIFRO_AREA (carea));

  carea->priv->draw_focus = draw_focus;
}

/* Функция возвращает необходимости рисования рамки при наличии фокуса ввода в объект CifroArea. */
gboolean
gtk_cifro_area_get_draw_focus (GtkCifroArea *carea)
{
  g_return_val_if_fail (GTK_IS_CIFRO_AREA (carea), FALSE);

  return carea->priv->draw_focus;
}

/* Функция задает курсор используемый при нахождении мышки в видимой области. */
void
gtk_cifro_area_set_point_cursor (GtkCifroArea *carea,
                                 GdkCursor    *cursor)
{
  g_return_if_fail (GTK_IS_CIFRO_AREA (carea));

#ifdef CIFRO_AREA_WITH_GTK2
  g_clear_pointer (&carea->priv->point_cursor, gdk_cursor_unref);
#else
  g_clear_object (&carea->priv->point_cursor);
#endif

  carea->priv->point_cursor = cursor;
}

/* Функция задает курсор используемый при перемещении видимой области. */
void
gtk_cifro_area_set_move_cursor (GtkCifroArea *carea,
                                GdkCursor    *cursor)
{
  g_return_if_fail (GTK_IS_CIFRO_AREA (carea));

#ifdef CIFRO_AREA_WITH_GTK2
  g_clear_pointer (&carea->priv->move_cursor, gdk_cursor_unref);
#else
  g_clear_object (&carea->priv->move_cursor);
#endif

  carea->priv->move_cursor = cursor;
}

/* Функция задаёт отражение по осям. */
void
gtk_cifro_area_set_swap (GtkCifroArea *carea,
                         gboolean      swap_x,
                         gboolean      swap_y)
{
  GtkCifroAreaPrivate *priv;

  g_return_if_fail (GTK_IS_CIFRO_AREA (carea));

  priv = carea->priv;

  priv->swap_x = swap_x;
  priv->swap_y = swap_y;

  gtk_cifro_area_state_set_swap (priv->state, swap_x, swap_y);

  gtk_cifro_area_update_visible (priv);
}

/* Функция возвращает текущее состояние отражения по осям. */
void
gtk_cifro_area_get_swap (GtkCifroArea *carea,
                         gboolean     *swap_x,
                         gboolean     *swap_y)
{
  g_return_if_fail (GTK_IS_CIFRO_AREA (carea));

  if (swap_x != NULL)
    *swap_x = carea->priv->swap_x;
  if (swap_y != NULL)
    *swap_y = carea->priv->swap_y;
}

/* Функция задаёт размер области для отрисовки элементов обрамления по периметру. */
gboolean
gtk_cifro_area_set_border (GtkCifroArea *carea,
                           gint          left,
                           gint          right,
                           gint          top,
                           gint          bottom)
{
  GtkCifroAreaPrivate *priv;

  g_return_val_if_fail (GTK_IS_CIFRO_AREA (carea), FALSE);

  g_return_val_if_fail (left >= 0 && left < 1024, FALSE);
  g_return_val_if_fail (right >= 0 && right < 1024, FALSE);
  g_return_val_if_fail (top >= 0 && top < 1024, FALSE);
  g_return_val_if_fail (bottom >= 0 && bottom < 1024, FALSE);

  priv = carea->priv;

  priv->border_left = left;
  priv->border_right = right;
  priv->border_top = top;
  priv->border_bottom = bottom;

  gtk_cifro_area_state_set_border (priv->state, left, right, top, bottom);

  gtk_cifro_area_update_visible (priv);

  return TRUE;
}

/* Функция возвращает текущий размер области для отрисовки элементов обрамления по периметру. */
void
gtk_cifro_area_get_border (GtkCifroArea *carea,
                           gint         *left,
                           gint         *right,
                           gint         *top,
                           gint         *bottom)
{
  GtkCifroAreaPrivate *priv;

  g_return_if_fail (GTK_IS_CIFRO_AREA (carea));

  priv = carea->priv;

  if (left != NULL)
    *left = priv->border_left;
  if (right != NULL)
    *right = priv->border_right;
  if (top != NULL)
    *top = priv->border_top;
  if (bottom != NULL)
    *bottom = priv->border_bottom;
}

/* Функция определяет: изменять масштаб при изменении размеров окна или изменять видимую область. */
void
gtk_cifro_area_set_scale_on_resize (GtkCifroArea *carea,
                                    gboolean      scale_on_resize)
{
  g_return_if_fail (GTK_IS_CIFRO_AREA (carea));

  carea->priv->scale_on_resize = scale_on_resize;
}

/* Функция возвращает состояние поведения масштабирования при изменении размеров окна. */
gboolean
gtk_cifro_area_get_scale_on_resize (GtkCifroArea *carea)
{
  g_return_val_if_fail (GTK_IS_CIFRO_AREA (carea), FALSE);

  return carea->priv->scale_on_resize;
}

/* Функция задаёт режим масштабирования: по центру или по курсору. */
void
gtk_cifro_area_set_zoom_on_center (GtkCifroArea *carea,
                                   gboolean      zoom_on_center)
{
  g_return_if_fail (GTK_IS_CIFRO_AREA (carea));

  carea->priv->zoom_on_center = zoom_on_center;
}

/* Функция возвращает текущий режим масштабирования. */
gboolean
gtk_cifro_area_get_zoom_on_center (GtkCifroArea *carea)
{
  g_return_val_if_fail (GTK_IS_CIFRO_AREA (carea), FALSE);

  return carea->priv->zoom_on_center;
}

/* Функция задаёт соотношение масштабов по осям при изменении размеров. */
gboolean
gtk_cifro_area_set_scale_aspect (GtkCifroArea *carea,
                                 gdouble       scale_aspect)
{
  GtkCifroAreaPrivate *priv;
  gboolean status;

  g_return_val_if_fail (GTK_IS_CIFRO_AREA (carea), FALSE);

  priv = carea->priv;

  status = gtk_cifro_area_fix_scale_aspect (scale_aspect,
                                            &priv->min_scale_x, &priv->max_scale_x,
                                            &priv->min_scale_y, &priv->max_scale_y);
  g_return_val_if_fail (status, FALSE);

  priv->scale_aspect = scale_aspect;

  gtk_cifro_area_update_visible (priv);

  return TRUE;
}

/* Функция возвращает текущее соотношение масштабов по осям при изменении размеров. */
gdouble
gtk_cifro_area_get_scale_aspect (GtkCifroArea *carea)
{
  g_return_val_if_fail (GTK_IS_CIFRO_AREA (carea), 0.0);

  return carea->priv->scale_aspect;
}

/* Функция задаёт значение умножителя скорости перемещения при нажатой клавише control. */
gboolean
gtk_cifro_area_set_move_multiplier (GtkCifroArea *carea,
                                    gdouble       move_multiplier)
{
  g_return_val_if_fail (GTK_IS_CIFRO_AREA (carea), FALSE);

  g_return_val_if_fail (move_multiplier > 0.0, FALSE);

  carea->priv->move_multiplier = move_multiplier;

  return TRUE;
}

/* Функция возвращает текущее значение умножителя скорости перемещения при нажатой клавише control. */
gdouble
gtk_cifro_area_get_move_multiplier (GtkCifroArea *carea)
{
  g_return_val_if_fail (GTK_IS_CIFRO_AREA (carea), 0.0);

  return carea->priv->move_multiplier;
}

/* Функция задаёт значение умножителя скорости вращения при нажатой клавише control. */
gboolean
gtk_cifro_area_set_rotate_multiplier (GtkCifroArea *carea,
                                      gdouble       rotate_multiplier)
{
  g_return_val_if_fail (GTK_IS_CIFRO_AREA (carea), FALSE);

  g_return_val_if_fail (rotate_multiplier > 0.0, FALSE);

  carea->priv->rotate_multiplier = rotate_multiplier;

  return TRUE;
}

/* Функция возвращает текущее значение умножителя скорости вращения при нажатой клавише control. */
gdouble
gtk_cifro_area_get_rotate_multiplier (GtkCifroArea *carea)
{
  g_return_val_if_fail (GTK_IS_CIFRO_AREA (carea), 0.0);

  return carea->priv->rotate_multiplier;
}

/* Функция задаёт возможность поворота изображения вокруг оси. */
void
gtk_cifro_area_set_rotation (GtkCifroArea *carea,
                             gboolean      rotation)
{
  g_return_if_fail (GTK_IS_CIFRO_AREA (carea));

  carea->priv->rotation = rotation;
}

/* Функция возвращает информацию о возможности поворота изображения вокруг оси. */
gboolean
gtk_cifro_area_get_rotation (GtkCifroArea *carea)
{
  g_return_val_if_fail (GTK_IS_CIFRO_AREA (carea), FALSE);

  return carea->priv->rotation;
}

/* Функция задаёт пределы возможных отображаемых значений. */
gboolean
gtk_cifro_area_set_view_limits (GtkCifroArea *carea,
                                gdouble       min_x,
                                gdouble       max_x,
                                gdouble       min_y,
                                gdouble       max_y)
{
  GtkCifroAreaPrivate *priv;

  g_return_val_if_fail (GTK_IS_CIFRO_AREA (carea), FALSE);

  g_return_val_if_fail (min_x < max_x, FALSE);
  g_return_val_if_fail (min_y < max_y, FALSE);

  priv = carea->priv;

  priv->min_x = min_x;
  priv->max_x = max_x;
  priv->min_y = min_y;
  priv->max_y = max_y;

  gtk_cifro_area_state_set_view_limits (priv->state, min_x, max_x, min_y, max_y);

  gtk_cifro_area_update_visible (priv);
  gtk_cifro_area_update (carea);

  return TRUE;
}

/* Функция возвращает пределы возможных отображаемых значений. */
void
gtk_cifro_area_get_view_limits (GtkCifroArea *carea,
                                gdouble      *min_x,
                                gdouble      *max_x,
                                gdouble      *min_y,
                                gdouble      *max_y)
{
  GtkCifroAreaPrivate *priv;

  g_return_if_fail (GTK_IS_CIFRO_AREA (carea));

  priv = carea->priv;

  if (min_x != NULL)
    *min_x = priv->min_x;
  if (max_x != NULL)
    *max_x = priv->max_x;
  if (min_y != NULL)
    *min_y = priv->min_y;
  if (max_y != NULL)
    *max_y = priv->max_y;
}

/* Функция задаёт пределы возможных коэффициентов масштабирования. */
gboolean
gtk_cifro_area_set_scale_limits (GtkCifroArea *carea,
                                 gdouble       min_scale_x,
                                 gdouble       max_scale_x,
                                 gdouble       min_scale_y,
                                 gdouble       max_scale_y)
{
  GtkCifroAreaPrivate *priv;
  gboolean status;

  g_return_val_if_fail (GTK_IS_CIFRO_AREA (carea), FALSE);

  priv = carea->priv;

  status = gtk_cifro_area_fix_scale_aspect (priv->scale_aspect,
                                            &min_scale_x, &max_scale_x,
                                            &min_scale_y, &max_scale_y);
  g_return_val_if_fail (status, FALSE);

  priv->min_scale_x = min_scale_x;
  priv->max_scale_x = max_scale_x;
  priv->min_scale_y = min_scale_y;
  priv->max_scale_y = max_scale_y;

  gtk_cifro_area_update_visible (priv);
  gtk_cifro_area_update (carea);

  return TRUE;
}

/* Функция возвращает пределы возможных коэффициентов масштабирования. */
void
gtk_cifro_area_get_scale_limits (GtkCifroArea *carea,
                                 gdouble      *min_scale_x,
                                 gdouble      *max_scale_x,
                                 gdouble      *min_scale_y,
                                 gdouble      *max_scale_y)
{
  GtkCifroAreaPrivate *priv;

  g_return_if_fail (GTK_IS_CIFRO_AREA (carea));

  priv = carea->priv;

  if (min_scale_x != NULL)
    *min_scale_x = priv->min_scale_x;
  if (max_scale_x != NULL)
    *max_scale_x = priv->max_scale_x;
  if (min_scale_y != NULL)
    *min_scale_y = priv->min_scale_y;
  if (max_scale_y != NULL)
    *max_scale_y = priv->max_scale_y;
}

/* Функция устанавливает параметры масштабирования. */
gboolean
gtk_cifro_area_set_zoom_scale (GtkCifroArea *carea,
                               gdouble       zoom_scale)
{
  g_return_val_if_fail (GTK_IS_CIFRO_AREA (carea), FALSE);

  g_return_val_if_fail (zoom_scale > 0.0, FALSE);

  carea->priv->zoom_scale = zoom_scale;

  return TRUE;
}

/* Функция возвращает параметры масштабирования. */
gdouble
gtk_cifro_area_get_zoom_scale (GtkCifroArea *carea)
{
  g_return_val_if_fail (GTK_IS_CIFRO_AREA (carea), 0.0);

  return carea->priv->zoom_scale;
}

/* Функция задаёт набор фиксированных значений масштабов. */
gboolean
gtk_cifro_area_set_fixed_zoom_scales (GtkCifroArea *carea,
                                      gdouble      *zoom_x_scales,
                                      gdouble      *zoom_y_scales,
                                      gint          num_scales)
{
  GtkCifroAreaPrivate *priv;
  gint i;

  g_return_val_if_fail (GTK_IS_CIFRO_AREA (carea), FALSE);

  priv = carea->priv;

  if (num_scales == 0)
    {
      g_free (priv->zoom_x_scales);
      g_free (priv->zoom_y_scales);
      priv->zoom_x_scales = NULL;
      priv->zoom_y_scales = NULL;
      priv->num_scales = 0;
      return TRUE;
    }

  for (i = 0; i < num_scales; i++)
    {
      g_return_val_if_fail (zoom_x_scales[ i ] > 0.0, FALSE);
      g_return_val_if_fail (zoom_y_scales[ i ] > 0.0, FALSE);
    }

  g_free (priv->zoom_x_scales);
  g_free (priv->zoom_y_scales);

  priv->zoom_x_scales = g_malloc (sizeof(gdouble) * num_scales);
  priv->zoom_y_scales = g_malloc (sizeof(gdouble) * num_scales);

  memcpy (priv->zoom_x_scales, zoom_x_scales, sizeof(gdouble) * num_scales);
  memcpy (priv->zoom_y_scales, zoom_y_scales, sizeof(gdouble) * num_scales);
  priv->num_scales = num_scales;

  gtk_cifro_area_set_view (carea, priv->from_x, priv->to_x, priv->from_y, priv->to_y);

  return TRUE;
}

/* Функция возвращает текущий набор фиксированых значений масштабов. */
void
gtk_cifro_area_get_fixed_zoom_scales (GtkCifroArea  *carea,
                                      gdouble      **zoom_x_scales,
                                      gdouble      **zoom_y_scales,
                                      gint          *num_scales)
{
  GtkCifroAreaPrivate *priv;

  g_return_if_fail (GTK_IS_CIFRO_AREA (carea));

  priv = carea->priv;

  if (priv->num_scales)
    {
      if (zoom_x_scales != NULL)
        {
          *zoom_x_scales = g_malloc (sizeof(gdouble) * priv->num_scales);
          memcpy (*zoom_x_scales, priv->zoom_x_scales, sizeof(gdouble) * priv->num_scales);
        }

      if (zoom_y_scales != NULL)
        {
          *zoom_y_scales = g_malloc (sizeof(gdouble) * priv->num_scales);
          memcpy (*zoom_y_scales, priv->zoom_y_scales, sizeof(gdouble) * priv->num_scales);
        }
    }
  else
    {
      if (zoom_x_scales != NULL)
        *zoom_x_scales = NULL;
      if (zoom_y_scales != NULL)
        *zoom_y_scales = NULL;
    }

  if (num_scales != NULL)
    *num_scales = priv->num_scales;
}

/* Функция задаёт границу текущей видимости. */
gboolean
gtk_cifro_area_set_view (GtkCifroArea *carea,
                         gdouble       from_x,
                         gdouble       to_x,
                         gdouble       from_y,
                         gdouble       to_y)
{
  GtkCifroAreaPrivate *priv;

  g_return_val_if_fail (GTK_IS_CIFRO_AREA (carea), FALSE);

  g_return_val_if_fail (from_x < to_x, FALSE);
  g_return_val_if_fail (from_y < to_y, FALSE);

  priv = carea->priv;

  if (from_x < priv->min_x)
    from_x = priv->min_x;
  if (to_x > priv->max_x)
    to_x = priv->max_x;

  if (from_y < priv->min_y)
    from_y = priv->min_y;
  if (to_y > priv->max_y)
    to_y = priv->max_y;

  priv->from_x = from_x;
  priv->to_x = to_x;
  priv->from_y = from_y;
  priv->to_y = to_y;

  if (priv->num_scales == 0)
    {
      priv->cur_scale_x = 0.0;
      priv->cur_scale_y = 0.0;
    }
  else
    {
      gdouble opt_scale_x;
      gdouble opt_scale_y;
      gdouble scale_diff;
      gdouble min_scale_diff;
      gint i, opt_i;

      if (priv->widget_width && priv->widget_height)
        {
          opt_scale_x = (priv->to_x - priv->from_x)
                        / gtk_cifro_area_get_visible_width (priv->widget_width, priv->widget_height, priv->angle);
          opt_scale_y = (priv->to_y - priv->from_y)
                        / gtk_cifro_area_get_visible_height (priv->widget_width, priv->widget_height, priv->angle);
        }
      else
        {
          opt_scale_x = 1.0;
          opt_scale_y = 1.0;
        }

      min_scale_diff = fabs (priv->zoom_x_scales[0] - opt_scale_x) + fabs (priv->zoom_y_scales[0] - opt_scale_y);
      opt_i = 0;

      for (i = 1; i < priv->num_scales; i++)
        {
          scale_diff = fabs (priv->zoom_x_scales[i] - opt_scale_x) + fabs (priv->zoom_y_scales[i] - opt_scale_y);
          if (scale_diff < min_scale_diff)
            {
              min_scale_diff = scale_diff;
              opt_i = i;
            }
        }

      priv->cur_scale_x = priv->zoom_x_scales[opt_i];
      priv->cur_scale_y = priv->zoom_y_scales[opt_i];
      priv->cur_zoom_index = opt_i;
    }

  gtk_cifro_area_update_visible (priv);
  gtk_cifro_area_update (carea);

  return TRUE;
}

/* Функция возвращает границу текущей видимости. */
void
gtk_cifro_area_get_view (GtkCifroArea *carea,
                         gdouble      *from_x,
                         gdouble      *to_x,
                         gdouble      *from_y,
                         gdouble      *to_y)
{
  GtkCifroAreaPrivate *priv;

  g_return_if_fail (GTK_IS_CIFRO_AREA (carea));

  priv = carea->priv;

  if (from_x != NULL)
    *from_x = priv->from_x;
  if (to_x != NULL)
    *to_x = priv->to_x;
  if (from_y != NULL)
    *from_y = priv->from_y;
  if (to_y != NULL)
    *to_y = priv->to_y;
}

/* Функция задаёт угол поворота изображения в радианах. */
void
gtk_cifro_area_set_angle (GtkCifroArea *carea,
                          gdouble       angle)
{
  GtkCifroAreaPrivate *priv;

  g_return_if_fail (GTK_IS_CIFRO_AREA (carea));

  priv = carea->priv;

  if (priv->rotation)
    {
      angle = fmodf (angle, 2.0 * G_PI);
      if (angle > G_PI)
        angle = angle - 2.0 * G_PI;
      if (angle < -G_PI)
        angle = angle + 2.0 * G_PI;
      priv->angle = angle;
      priv->angle_cos = cos (priv->angle);
      priv->angle_sin = sin (priv->angle);
      gtk_cifro_area_update_visible (priv);
      gtk_cifro_area_update (carea);
    }
}

/* Функция возвращает текущий угол поворота изображения в радианах. */
gdouble
gtk_cifro_area_get_angle (GtkCifroArea *carea)
{
  g_return_val_if_fail (GTK_IS_CIFRO_AREA (carea), 0.0);

  return carea->priv->angle;
}

/* Функция смещает видимую область на x_step, y_step. */
void
gtk_cifro_area_move (GtkCifroArea *carea,
                     gdouble       step_x,
                     gdouble       step_y)
{
  GtkCifroAreaPrivate *priv;
  gdouble shift_x, shift_y;

  g_return_if_fail (GTK_IS_CIFRO_AREA (carea));

  priv = carea->priv;

  shift_x = step_x * priv->angle_cos - step_y * priv->angle_sin;
  shift_y = step_y * priv->angle_cos + step_x * priv->angle_sin;

  shift_x *= priv->cur_scale_x;
  shift_y *= priv->cur_scale_y;

  if (priv->swap_x)
    shift_x = -shift_x;
  if (priv->swap_y)
    shift_y = -shift_y;

  if (shift_x < 0)
    {
      if (priv->from_x + shift_x < priv->min_x)
        shift_x = priv->min_x - priv->from_x;
    }
  else
    {
      if (priv->to_x + shift_x > priv->max_x)
        shift_x = priv->max_x - priv->to_x;
    }

  if (shift_y < 0)
    {
      if (priv->from_y + shift_y < priv->min_y)
        shift_y = priv->min_y - priv->from_y;
    }
  else
    {
      if (priv->to_y + shift_y > priv->max_y)
        shift_y = priv->max_y - priv->to_y;
    }

  priv->from_x += shift_x;
  priv->to_x += shift_x;
  priv->from_y += shift_y;
  priv->to_y += shift_y;

  gtk_cifro_area_update_visible (priv);
  gtk_cifro_area_update (carea);
}

/* Функция поворачивает видимую область на угол angle. */
void
gtk_cifro_area_rotate (GtkCifroArea *carea,
                       gdouble       angle)
{
  g_return_if_fail (GTK_IS_CIFRO_AREA (carea));

  angle += carea->priv->angle;

  gtk_cifro_area_set_angle (carea, angle);
}

/* Функция масштабирования видимой области. */
void
gtk_cifro_area_zoom (GtkCifroArea *carea,
                     gboolean      zoom_x,
                     gboolean      zoom_y,
                     gdouble       center_val_x,
                     gdouble       center_val_y,
                     gboolean      zoom_in,
                     gdouble       zoom_scale)
{
  GtkCifroAreaPrivate *priv;

  gdouble new_scale_x;
  gdouble new_scale_y;

  gdouble from_x, to_x;
  gdouble from_y, to_y;

  g_return_if_fail (GTK_IS_CIFRO_AREA (carea));

  priv = carea->priv;

  from_x = priv->from_x;
  to_x = priv->to_x;
  from_y = priv->from_y;
  to_y = priv->to_y;

  zoom_scale = 1.0 + (priv->zoom_scale / 100.0);
  if (!zoom_in)
    zoom_scale = 1.0 / zoom_scale;

  if (priv->scale_aspect > 0.0)
    {
      zoom_x = TRUE;
      zoom_y = TRUE;
    }

  new_scale_x = priv->cur_scale_x / zoom_scale;
  new_scale_y = priv->cur_scale_y / zoom_scale;

  if (new_scale_x < priv->min_scale_x || new_scale_x > priv->max_scale_x)
    zoom_x = FALSE;
  if (new_scale_y < priv->min_scale_y || new_scale_y > priv->max_scale_y)
    zoom_y = FALSE;

  if (priv->scale_aspect > 0.0 && (zoom_x == FALSE || zoom_y == FALSE))
    return;
  if (!zoom_x && !zoom_y)
    return;

  if (zoom_x)
    {
      from_x = center_val_x - (center_val_x - from_x) / zoom_scale;
      to_x = center_val_x + (to_x - center_val_x) / zoom_scale;
    }

  if (zoom_y)
    {
      from_y = center_val_y - (center_val_y - from_y) / zoom_scale;
      to_y = center_val_y + (to_y - center_val_y) / zoom_scale;
    }

  if (from_x < priv->min_x)
    from_x = priv->min_x;
  if (to_x > priv->max_x)
    to_x = priv->max_x;
  if (from_y < priv->min_y)
    from_y = priv->min_y;
  if (to_y > priv->max_y)
    to_y = priv->max_y;

  gtk_cifro_area_set_view (carea, from_x, to_x, from_y, to_y);
}

/* Функция переключения фиксированного масштаба. */
void
gtk_cifro_area_fixed_zoom (GtkCifroArea *carea,
                           gdouble       center_val_x,
                           gdouble       center_val_y,
                           gboolean      zoom_in)
{
  GtkCifroAreaPrivate *priv;

  gdouble length_x;
  gdouble length_y;
  gdouble from_x, to_x;
  gdouble from_y, to_y;

  g_return_if_fail (GTK_IS_CIFRO_AREA (carea));

  priv = carea->priv;

  if (priv->num_scales == 0)
    return;

  if (zoom_in && (priv->cur_zoom_index == priv->num_scales - 1))
    return;
  if (!zoom_in && (priv->cur_zoom_index == 0))
    return;

  if (zoom_in)
    priv->cur_zoom_index++;
  else
    priv->cur_zoom_index--;

  priv->cur_scale_x = priv->zoom_x_scales[priv->cur_zoom_index];
  priv->cur_scale_y = priv->zoom_y_scales[priv->cur_zoom_index];

  length_x = priv->to_x - priv->from_x;
  length_y = priv->to_y - priv->from_y;

  from_x = center_val_x - (priv->cur_scale_x * priv->visible_width * (center_val_x - priv->from_x) / length_x);
  to_x = center_val_x + (priv->cur_scale_x * priv->visible_width * (priv->to_x - center_val_x) / length_x);
  from_y = center_val_y - (priv->cur_scale_y * priv->visible_height * (center_val_y - priv->from_y) / length_y);
  to_y = center_val_y + (priv->cur_scale_y * priv->visible_height * (priv->to_y - center_val_y) / length_y);

  if (from_x < priv->min_x)
    from_x = priv->min_x;
  if (to_x > priv->max_x)
    to_x = priv->max_x;
  if (from_y < priv->min_y)
    from_y = priv->min_y;
  if (to_y > priv->max_y)
    to_y = priv->max_y;

  gtk_cifro_area_set_view (carea, from_x, to_x, from_y, to_y);
}
