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
 * SECTION: gtk-cifro-curve
 * @Short_description: GTK+ виджет осциллографа с параметрической кривой
 * @Title: GtkCifroCurve
 * @See_also: #GtkCifroScope, #GtkCifroArea, #GtkCifroAreaControl
 *
 * GtkCifroCurve позволяет отображать данные аналогично #GtkCifroScope и, кроме этого,
 * обеспечивает отображение кривой с определением её параметров через контрольные точки.
 * Функция расчёта кривой указывается пользователем при создании виджета.
 *
 * Параметры кривой определяются местоположением контрольных точек. Точки можно перемещать
 * при помощи мышки при нажатой левой кнопке. Точка, выбранная для перемещения, выделяется
 * окружностью. Точки можно добавлять или удалять. Для добавления точек необходимо нажать
 * левую кнопку манипулятора при нажатой клавише Ctrl на клавиатуре. Аналогично при нажатой
 * клавише Ctrl можно удалить уже существующую точку. Также можно удалить точку совместив её
 * с одной из соседних.
 *
 * Точка описывается структурой типа #GtkCifroCurvePoint. Массив точек передаётся через #GArray
 * в виде массива структур #GtkCifroCurvePoint.
 *
 * Аналитический вид кривой расчитывается функцией типа #GtkCifroCurveFunc, в неё передаются
 * все точки существующие на данный момент и указатель на пользовательские данные.
 *
 */

#include "gtk-cifro-curve.h"
#include "cairo-sdline.h"

#include <math.h>

enum
{
  PROP_0,
  PROP_CURVE_FUNC,
  PROP_CURVE_DATA
};

struct _GtkCifroCurvePrivate
{
  GArray                      *curve_points;                   /* Точки кривой. */
  GtkCifroCurveFunc            curve_func;                     /* Функция расчёта значений кривой. */
  gpointer                     curve_data;                     /* Пользовательские данные для функции расчёта значений кривой. */

  guint32                      curve_color;                    /* Цвет кривой. */
  guint32                      point_color;                    /* Цвет точек. */

  gint                         selected_point;                 /* Индекс текущей выбранной точки; */
  gboolean                     move_point;                     /* Перемещать или нет выбранную точку; */
  gboolean                     remove_point;                   /* Удалить выбранную точку. */
};

static void            gtk_cifro_curve_set_property            (GObject               *ccurve,
                                                                guint                  prop_id,
                                                                const GValue          *value,
                                                                GParamSpec            *pspec);

static void            gtk_cifro_curve_object_finalize         (GObject               *object);

static void            gtk_cifro_curve_select_point            (GtkCifroCurve         *ccurve,
                                                                gint                   pointer_x,
                                                                gint                   pointer_y);

static void            gtk_cifro_curve_visible_draw            (GtkWidget             *widget,
                                                                cairo_t               *cairo);

static gboolean        gtk_cifro_curve_button_press_event      (GtkWidget             *widget,
                                                                GdkEventButton        *event);

static gboolean        gtk_cifro_curve_button_release_event    (GtkWidget             *widget,
                                                                GdkEventButton        *event);

static gboolean        gtk_cifro_curve_motion_notify_event     (GtkWidget             *widget,
                                                                GdkEventMotion        *event);

G_DEFINE_TYPE_WITH_PRIVATE (GtkCifroCurve, gtk_cifro_curve, GTK_TYPE_CIFRO_SCOPE)

static void
gtk_cifro_curve_init (GtkCifroCurve *ccurve)
{
  GtkCifroCurvePrivate *priv = gtk_cifro_curve_get_instance_private (ccurve);
  ccurve->priv = priv;

  /* Контрольные точки кривой. */
  priv->curve_points = g_array_new (FALSE, FALSE, sizeof (GtkCifroCurvePoint));

  /* Цвета по умолчанию. */
  priv->curve_color = cairo_sdline_color (g_random_double_range (0.5, 1.0),
                                          g_random_double_range (0.5, 1.0),
                                          g_random_double_range (0.5, 1.0),
                                          1.0);
  priv->point_color = cairo_sdline_color (g_random_double_range (0.5, 1.0),
                                          g_random_double_range (0.5, 1.0),
                                          g_random_double_range (0.5, 1.0),
                                          1.0);

  /* Обработчики сигналов. */
  g_signal_connect (ccurve, "visible-draw", G_CALLBACK (gtk_cifro_curve_visible_draw), NULL);
  g_signal_connect (ccurve, "button-press-event", G_CALLBACK (gtk_cifro_curve_button_press_event), NULL);
  g_signal_connect (ccurve, "button-release-event", G_CALLBACK (gtk_cifro_curve_button_release_event), NULL);
  g_signal_connect (ccurve, "motion-notify-event", G_CALLBACK (gtk_cifro_curve_motion_notify_event), NULL);
}

static void
gtk_cifro_curve_class_init (GtkCifroCurveClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->set_property = gtk_cifro_curve_set_property;
  object_class->finalize = gtk_cifro_curve_object_finalize;

  g_object_class_install_property (object_class, PROP_CURVE_FUNC,
    g_param_spec_pointer ("curve-func", "Curve func", "Curve function",
                          G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY));

  g_object_class_install_property (object_class, PROP_CURVE_DATA,
    g_param_spec_pointer ("curve-data", "Curve data", "Curve function data",
                          G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY));
}

static void
gtk_cifro_curve_set_property (GObject      *object,
                              guint         prop_id,
                              const GValue *value,
                              GParamSpec   *pspec)
{
  GtkCifroCurve *ccurve = GTK_CIFRO_CURVE (object);
  GtkCifroCurvePrivate *priv = ccurve->priv;

  switch (prop_id)
    {
      case PROP_CURVE_FUNC:
        priv->curve_func = g_value_get_pointer (value);
        break;

      case PROP_CURVE_DATA:
        priv->curve_data = g_value_get_pointer (value);
        break;

      default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID( ccurve, prop_id, pspec);
        break;
    }
}

static void
gtk_cifro_curve_object_finalize (GObject *object)
{
  GtkCifroCurve *ccurve = GTK_CIFRO_CURVE (object);

  g_array_unref (ccurve->priv->curve_points);

  G_OBJECT_CLASS( gtk_cifro_curve_parent_class )->finalize (object);
}

/* Функция выбирает точку по указанным координатам. */
static void
gtk_cifro_curve_select_point (GtkCifroCurve *ccurve,
                              gint           pointer_x,
                              gint           pointer_y)
{
  GtkCifroArea *carea = GTK_CIFRO_AREA (ccurve);
  GtkCifroCurvePrivate *priv = ccurve->priv;

  gint selected_point;

  guint area_width;
  guint area_height;
  guint border_top;

  gdouble x, y;
  gdouble point_distance;
  gdouble prev_point_distance;
  gdouble point_radius;

  guint i;

  if (priv->move_point)
    return;

  gtk_cifro_area_get_size (carea, &area_width, &area_height);
  gtk_cifro_area_get_border (carea, NULL, NULL, &border_top, NULL);
  point_radius = 0.4 * border_top;

  /* Ищем ближающую рядом с курсором точку, которая находится в радиусе "размера" точки. */
  selected_point = -1;
  prev_point_distance = G_MAXDOUBLE;
  for (i = 0; i < priv->curve_points->len; i++)
    {
      GtkCifroCurvePoint *point = &g_array_index( priv->curve_points, GtkCifroCurvePoint, i );
      gtk_cifro_area_value_to_point (carea, &x, &y, point->x, point->y);
      if (x < -point_radius || x >= area_width + point_radius || y < -point_radius
          || y >= area_height + point_radius)
        continue;

      /* Расстояние от курсора до текущей проверяемой точки. */
      point_distance = sqrt ((x - pointer_x) * (x - pointer_x) + (y - pointer_y) * (y - pointer_y));

      /* Если расстояние слишком большое пропускаем эту точку. */
      if (point_distance > 2 * point_radius)
        continue;

      /* Сравниваем с расстоянием до предыдущей ближайшей точки. */
      if (point_distance < prev_point_distance)
        {
          selected_point = i;
          prev_point_distance = point_distance;
        }
    }

  if (selected_point != priv->selected_point)
    priv->selected_point = selected_point;
}

/* Функция рисования параметрической кривой. */
static void
gtk_cifro_curve_visible_draw (GtkWidget *widget,
                              cairo_t   *cairo)
{
  GtkCifroArea *carea = GTK_CIFRO_AREA (widget);
  GtkCifroCurve *ccurve = GTK_CIFRO_CURVE (widget);
  GtkCifroCurvePrivate *priv = ccurve->priv;

  cairo_sdline_surface *surface;

  guint visible_width;
  guint visible_height;
  guint border_top;

  gdouble x_value;
  gdouble y_value;
  gdouble y1, y2;
  gint i;

  surface = cairo_sdline_surface_create_for (cairo_get_target (cairo));
  g_return_if_fail (surface != NULL);

  gtk_cifro_area_get_visible_size (carea, &visible_width, &visible_height);
  gtk_cifro_area_get_border (carea, NULL, NULL, &border_top, NULL);

  /* Рисуем кривую. */
  for (i = 0; i < visible_width; i++)
    {
      gtk_cifro_area_visible_point_to_value (carea, i, 0, &x_value, NULL);
      y_value = priv->curve_func (x_value, priv->curve_points, priv->curve_data);
      gtk_cifro_area_visible_value_to_point (carea, NULL, &y2, x_value, y_value);

      if (i == 0)
        {
          y1 = y2;
          continue;
        }
      cairo_sdline (surface, i - 1, y1, i, y2, priv->curve_color);
      y1 = y2;
    }

  cairo_surface_mark_dirty (surface->cairo_surface);
  cairo_sdline_set_cairo_color (cairo, priv->point_color);
  cairo_set_line_width (cairo, 1.0);

  /* Рисуем точки. */
  for (i = 0; i < priv->curve_points->len; i++)
    {
      gdouble x, y;
      gdouble point_radius = 0.5 * border_top;
      GtkCifroCurvePoint *point = &g_array_index( priv->curve_points, GtkCifroCurvePoint, i );

      gtk_cifro_area_visible_value_to_point (carea, &x, &y, point->x, point->y);

      if (x < -point_radius || x >= visible_width + point_radius ||
          y < -point_radius || y >= visible_height + point_radius)
        continue;

      x = gtk_cifro_area_point_to_cairo (x);
      y = gtk_cifro_area_point_to_cairo (y);

      cairo_arc (cairo, x, y, point_radius / 4.0, 0.0, 2 * G_PI);
      cairo_fill (cairo);

      if (priv->selected_point != i)
        continue;

      cairo_arc (cairo, x, y, point_radius, 0.0, 2 * G_PI);
      cairo_stroke (cairo);
    }

  cairo_sdline_surface_destroy (surface);
}

/* Функция обработки сигнала нажатия кнопки мыши. */
gboolean
gtk_cifro_curve_button_press_event (GtkWidget      *widget,
                                    GdkEventButton *event)
{
  GtkCifroArea *carea = GTK_CIFRO_AREA (widget);
  GtkCifroCurve *ccurve = GTK_CIFRO_CURVE (widget);
  GtkCifroCurvePrivate *priv = ccurve->priv;

  /* Обрабатываем только нажатия левой кнопки манипулятора. */
  if (event->button != 1)
    return FALSE;

  priv->remove_point = FALSE;
  priv->move_point = FALSE;

  if (priv->selected_point >= 0)
    {
      /* Выбрана точка и нажата кнопка Ctrl - нужно удалить эту точку. */
      if (event->state & GDK_CONTROL_MASK)
        {
          g_array_remove_index (priv->curve_points, priv->selected_point);
          priv->selected_point = -1;
        }
      /* Выбрана точка для перемещения. */
      else
        priv->move_point = TRUE;

      return TRUE;
    }
  else
    {
      /* Точка не выбрана, но нажата кнопка Ctrl - нужно добавить новую точку. */
      if (event->state & GDK_CONTROL_MASK)
        {
          gdouble value_x, value_y;
          gtk_cifro_area_point_to_value (carea, event->x, event->y, &value_x, &value_y);
          gtk_cifro_curve_add_point (ccurve, value_x, value_y);

          return TRUE;
        }
    }

  return FALSE;
}

/* Функция обработки сигнала отпускания кнопки мыши. */
gboolean
gtk_cifro_curve_button_release_event (GtkWidget      *widget,
                                      GdkEventButton *event)
{
  GtkCifroCurve *ccurve = GTK_CIFRO_CURVE (widget);
  GtkCifroCurvePrivate *priv = ccurve->priv;

  /* Обрабатываем только нажатия левой кнопки манипулятора. */
  if (event->button != 1)
    return FALSE;

  /* Обрабатываем удаление точки при её совмещении с другой точкой. */
  if (priv->move_point && priv->remove_point && priv->selected_point >= 0)
    g_array_remove_index (priv->curve_points, priv->selected_point);

  priv->move_point = FALSE;

  return FALSE;
}

/* Функция обработки сигнала движения мышки. */
gboolean
gtk_cifro_curve_motion_notify_event (GtkWidget      *widget,
                                     GdkEventMotion *event)
{
  GtkCifroArea *carea = GTK_CIFRO_AREA (widget);
  GtkCifroCurve *ccurve = GTK_CIFRO_CURVE (widget);
  GtkCifroCurvePrivate *priv = ccurve->priv;

  GtkCifroCurvePoint *point;
  GtkCifroCurvePoint *near_point;

  guint border_top;

  gdouble min_x, max_x;
  gdouble min_y, max_y;
  gdouble from_x, to_x;
  gdouble from_y, to_y;
  gdouble value_x, value_y;
  gdouble point_radius;

  gtk_cifro_area_get_border (carea, NULL, NULL, &border_top, NULL);

  /* Выделяем точку для перемещения. */
  gtk_cifro_curve_select_point (ccurve, event->x, event->y);

  /* Если мы не находимся в режиме перемещения точки. */
  if (!priv->move_point)
    return FALSE;

  priv->remove_point = FALSE;

  /* Вышли за границу окна. */
  if (priv->selected_point < 0)
    return FALSE;

  /* Текущая граница отображения. */
  gtk_cifro_area_get_view (carea, &from_x, &to_x, &from_y, &to_y);
  gtk_cifro_area_get_limits (carea, &min_x, &max_x, &min_y, &max_y);

  /* Расчитываем новое местоположение точки. */
  gtk_cifro_area_point_to_value (carea, event->x, event->y, &value_x, &value_y);

  point = &g_array_index( priv->curve_points, GtkCifroCurvePoint, priv->selected_point );
  point_radius = 0.4 * border_top;

  /* Определяем границы перемещения точки и расстояние до соседних точек.
   * Если расстояние до одной из соседних точек меньше чем "радиус" точки,
   * помечаем точку на удаление. Само удаление будет произведено при отпускании
   * кнопки манипулятора. */
  if (priv->selected_point > 0)
    {
      near_point = &g_array_index (priv->curve_points, GtkCifroCurvePoint, priv->selected_point - 1);
      min_x = MAX (from_x, near_point->x);
      if (near_point->x > from_x)
        {
          gdouble x1, y1, x2, y2;
          gtk_cifro_area_value_to_point (carea, &x1, &y1, value_x, value_y);
          gtk_cifro_area_value_to_point (carea, &x2, &y2, near_point->x, near_point->y);
          if (sqrt ((x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2)) < point_radius)
            priv->remove_point = TRUE;
        }
    }

  if (priv->selected_point < priv->curve_points->len - 1)
    {
      near_point = &g_array_index (priv->curve_points, GtkCifroCurvePoint, priv->selected_point + 1);
      max_x = MIN (to_x, near_point->x);
      if (near_point->x < to_x)
        {
          gdouble x1, y1, x2, y2;
          gtk_cifro_area_value_to_point (carea, &x1, &y1, value_x, value_y);
          gtk_cifro_area_value_to_point (carea, &x2, &y2, near_point->x, near_point->y);
          if (sqrt ((x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2)) < point_radius)
            priv->remove_point = TRUE;
        }
    }

  if (value_x < min_x)
    value_x = min_x;
  if (value_x > max_x)
    value_x = max_x;
  if (value_y < min_y)
    value_y = min_y;
  if (value_y > max_y)
    value_y = max_y;

  /* Задаём новое положение точки. */
  point->x = value_x;
  point->y = value_y;

  return FALSE;
}

/**
 * gtk_cifro_curve_new:
 * @curve_func: функция расчёта кривой по заданным точкам
 * @curve_data: пользовательские данные для передачи в curve_func
 *
 * Функция создаёт виджет #GtkCifroCurve.
 *
 * Returns: #GtkCifroCurve.
 *
 */
GtkWidget *
gtk_cifro_curve_new (GtkCifroCurveFunc curve_func,
                     gpointer          curve_data)
{
  return g_object_new (GTK_TYPE_CIFRO_CURVE, "curve-func", curve_func, "curve-data", curve_data, NULL);
}

/**
 * gtk_cifro_curve_clear_points:
 * @ccurve: указатель на #GtkCifroCurve
 *
 * Функция удаляет все установленные точки.
 *
 */
void
gtk_cifro_curve_clear_points (GtkCifroCurve *ccurve)
{
  g_return_if_fail (GTK_IS_CIFRO_CURVE (ccurve));

  g_array_set_size (ccurve->priv->curve_points, 0);

  gtk_widget_queue_draw (GTK_WIDGET (ccurve));
}

/**
 * gtk_cifro_curve_add_point:
 * @ccurve: указатель на #GtkCifroCurve
 * @x: X координата точки, логические координаты
 * @y: Y координата точки, логические координаты
 *
 * Функция добавляет точку к существующим.
 *
 */
void
gtk_cifro_curve_add_point (GtkCifroCurve *ccurve,
                           gdouble        x,
                           gdouble        y)
{
  GtkCifroCurvePrivate *priv;

  GtkCifroCurvePoint new_point;
  GtkCifroCurvePoint *point;
  guint i;

  g_return_if_fail (GTK_IS_CIFRO_CURVE (ccurve));

  priv = ccurve->priv;

  /* Ищем место для добавления точки. */
  for (i = 0; i < priv->curve_points->len; i++)
    {
      point = &g_array_index (priv->curve_points, GtkCifroCurvePoint, i);
      if (point->x > x)
        break;
    }

  new_point.x = x;
  new_point.y = y;
  g_array_insert_val( priv->curve_points, i, new_point);

  gtk_widget_queue_draw (GTK_WIDGET (ccurve));
}

/**
 * gtk_cifro_curve_set_points:
 * @ccurve: указатель на #GtkCifroCurve
 * @points: массив точек (#GtkCifroCurvePoint) параметров функции
 *
 * Функция удаляет текущие точки и устанавливает взамен них новые.
 *
 */
void
gtk_cifro_curve_set_points (GtkCifroCurve *ccurve,
                            GArray        *points)
{
  GtkCifroCurvePrivate *priv;

  GtkCifroCurvePoint *point;
  guint i;

  g_return_if_fail (GTK_IS_CIFRO_CURVE (ccurve));

  priv = ccurve->priv;

  g_array_set_size (priv->curve_points, 0);
  for (i = 0; i < points->len; i++)
    {
      point = &g_array_index (points, GtkCifroCurvePoint, i);
      gtk_cifro_curve_add_point (ccurve, point->x, point->y);
    }

  gtk_widget_queue_draw (GTK_WIDGET (ccurve));
}

/**
 * gtk_cifro_curve_get_points:
 * @ccurve: указатель на #GtkCifroCurve
 *
 * Функция возвращает массив текущих точек (#GtkCifroCurvePoint).
 *
 * Returns: Массив точек (#GtkCifroCurvePoint) параметров функции.
 *
 */
GArray *
gtk_cifro_curve_get_points (GtkCifroCurve *ccurve)
{
  GArray *points;

  g_return_val_if_fail (GTK_IS_CIFRO_CURVE (ccurve), NULL);

  points = g_array_new (FALSE, FALSE, sizeof(GtkCifroCurvePoint));
  g_array_insert_vals (points, 0,
                       ccurve->priv->curve_points->data,
                       ccurve->priv->curve_points->len);

  return points;
}

/**
 * gtk_cifro_curve_set_curve_color:
 * @ccurve: указатель на #GtkCifroCurve
 * @red: значение красной составляющей цвета от 0 до 1
 * @green: значение зелёной составляющей цвета от 0 до 1
 * @blue: значение синей составляющей цвета от 0 до 1
 *
 * Функция устанавливает цвет кривой.
 *
 */
void
gtk_cifro_curve_set_curve_color (GtkCifroCurve *ccurve,
                                 gdouble        red,
                                 gdouble        green,
                                 gdouble        blue)
{
  g_return_if_fail (GTK_IS_CIFRO_CURVE (ccurve));

  ccurve->priv->curve_color = cairo_sdline_color (red, green, blue, 1.0);

  gtk_widget_queue_draw (GTK_WIDGET (ccurve));
}

/**
 * gtk_cifro_curve_set_point_color:
 * @ccurve: указатель на #GtkCifroCurve
 * @red: значение красной составляющей цвета от 0 до 1
 * @green: значение зелёной составляющей цвета от 0 до 1
 * @blue: значение синей составляющей цвета от 0 до 1
 *
 * Функция устанавливает цвет контрольных точек кривой.
 *
 */
void
gtk_cifro_curve_set_point_color (GtkCifroCurve *ccurve,
                                 gdouble        red,
                                 gdouble        green,
                                 gdouble        blue)
{
  g_return_if_fail (GTK_IS_CIFRO_CURVE (ccurve));

  ccurve->priv->point_color = cairo_sdline_color (red, green, blue, 1.0);

  gtk_widget_queue_draw (GTK_WIDGET (ccurve));
}
