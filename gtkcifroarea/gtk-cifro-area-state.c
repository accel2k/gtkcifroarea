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

#include "gtk-cifro-area-state.h"

#include <math.h>

enum
{
  SIGNAL_AREA_CHANGED,
  SIGNAL_VISIBLE_CHANGED,
  SIGNAL_BORDER_CHANGED,
  SIGNAL_SWAP_CHANGED,
  SIGNAL_ANGLE_CHANGED,
  SIGNAL_VIEW_LIMITS_CHANGED,
  SIGNAL_VIEW_CHANGED,
  SIGNAL_LAST
};

struct _GtkCifroAreaStatePrivate
{
  gint                 area_width;         /* Ширина окна объекта. */
  gint                 area_height;        /* Высота окна объекта. */

  gint                 visible_width;      /* Видимая ширина. */
  gint                 visible_height;     /* Видимая высота. */

  gint                 border_left;        /* Размер области обрамления слева. */
  gint                 border_right;       /* Размер области обрамления справа. */
  gint                 border_top;         /* Размер области обрамления сверху. */
  gint                 border_bottom;      /* Размер области обрамления снизу. */

  gboolean             swap_x;             /* TRUE - ось x направлена влево, FALSE - вправо. */
  gboolean             swap_y;             /* TRUE - ось y направлена вниз, FALSE - вверх. */

  gdouble              angle;              /* Угол поворота изображения в радианах. */
  gdouble              angle_cos;          /* Косинус угла поворота изображения. */
  gdouble              angle_sin;          /* Синус угла поворота изображения. */

  gdouble              min_x;              /* Минимально возможное значение по оси x. */
  gdouble              max_x;              /* Максимально возможное значение по оси x. */
  gdouble              min_y;              /* Минимально возможное значение по оси y. */
  gdouble              max_y;              /* Максимально возможное значение по оси y. */

  gdouble              from_x;             /* Минимальная граница отображения оси x. */
  gdouble              to_x;               /* Максимальная граница отображения оси x. */
  gdouble              from_y;             /* Минимальная граница отображения оси y. */
  gdouble              to_y;               /* Максимальная граница отображения оси y. */

  gdouble              scale_x;            /* Текущий коэффициент масштаба по оси x. */
  gdouble              scale_y;            /* Текущий коэффициент масштаба по оси y. */
};

static guint gtk_cifro_area_state_signals[ SIGNAL_LAST ] = { 0 };

G_DEFINE_TYPE_WITH_PRIVATE (GtkCifroAreaState, gtk_cifro_area_state, G_TYPE_OBJECT);

static void
gtk_cifro_area_state_class_init (GtkCifroAreaStateClass *klass)
{
  gtk_cifro_area_state_signals[SIGNAL_AREA_CHANGED] =
    g_signal_new ("area-changed", GTK_TYPE_CIFRO_AREA_STATE,
                  G_SIGNAL_RUN_LAST, 0,
                  NULL, NULL,
                  g_cclosure_marshal_VOID__POINTER,
                  G_TYPE_NONE,
                  1, G_TYPE_POINTER);

  gtk_cifro_area_state_signals[SIGNAL_VISIBLE_CHANGED] =
    g_signal_new ("visible-changed", GTK_TYPE_CIFRO_AREA_STATE,
                  G_SIGNAL_RUN_LAST, 0,
                  NULL, NULL,
                  g_cclosure_marshal_VOID__POINTER,
                  G_TYPE_NONE,
                  1, G_TYPE_POINTER);

  gtk_cifro_area_state_signals[SIGNAL_BORDER_CHANGED] =
    g_signal_new ("border-changed", GTK_TYPE_CIFRO_AREA_STATE,
                  G_SIGNAL_RUN_LAST, 0,
                  NULL, NULL,
                  g_cclosure_marshal_VOID__POINTER,
                  G_TYPE_NONE,
                  1, G_TYPE_POINTER);

  gtk_cifro_area_state_signals[SIGNAL_SWAP_CHANGED] =
    g_signal_new ("swap-changed", GTK_TYPE_CIFRO_AREA_STATE,
                  G_SIGNAL_RUN_LAST, 0,
                  NULL, NULL,
                  g_cclosure_marshal_VOID__POINTER,
                  G_TYPE_NONE,
                  1, G_TYPE_POINTER);

  gtk_cifro_area_state_signals[SIGNAL_ANGLE_CHANGED] =
    g_signal_new ("angle-changed", GTK_TYPE_CIFRO_AREA_STATE,
                  G_SIGNAL_RUN_LAST, 0,
                  NULL, NULL,
                  g_cclosure_marshal_VOID__DOUBLE,
                  G_TYPE_NONE,
                  1, G_TYPE_DOUBLE);

  gtk_cifro_area_state_signals[SIGNAL_VIEW_LIMITS_CHANGED] =
    g_signal_new ("view-limits-changed", GTK_TYPE_CIFRO_AREA_STATE,
                  G_SIGNAL_RUN_LAST, 0,
                  NULL, NULL,
                  g_cclosure_marshal_VOID__POINTER,
                  G_TYPE_NONE,
                  1, G_TYPE_POINTER);

  gtk_cifro_area_state_signals[SIGNAL_VIEW_CHANGED] =
    g_signal_new ("view-changed", GTK_TYPE_CIFRO_AREA_STATE,
                  G_SIGNAL_RUN_LAST, 0,
                  NULL, NULL,
                  g_cclosure_marshal_VOID__POINTER,
                  G_TYPE_NONE,
                  1, G_TYPE_POINTER);
}

static void
gtk_cifro_area_state_init (GtkCifroAreaState *state)
{
  state->priv = gtk_cifro_area_state_get_instance_private (state);
}

/* Функция создаёт новый объект GtkCifroAreaState. */
GtkCifroAreaState *
gtk_cifro_area_state_new (void)
{
  return g_object_new (GTK_TYPE_CIFRO_AREA_STATE, NULL);
}

/*  Функция задаёт новые значения размеров области вывода. */
void
gtk_cifro_area_state_set_area_size (GtkCifroAreaState *state,
                                    gint               width,
                                    gint               height)
{
  GtkCifroAreaStatePrivate *priv = state->priv;

  GtkCifroAreaSize area_size;
  gboolean emit = FALSE;

  g_return_if_fail (GTK_IS_CIFRO_AREA_STATE (state));

  if (priv->area_width != width || priv->area_height != height)
    {
      area_size.width = priv->area_width = width;
      area_size.height = priv->area_height = height;
      emit = TRUE;
    }

  if (emit)
    g_signal_emit (state, gtk_cifro_area_state_signals[SIGNAL_AREA_CHANGED], 0, &area_size);
}

/* Функция возвращает текущие значения размеров области вывода. */
void
gtk_cifro_area_state_get_area_size (GtkCifroAreaState *state,
                                    gint              *width,
                                    gint              *height)
{
  GtkCifroAreaStatePrivate *priv = state->priv;

  g_return_if_fail (GTK_IS_CIFRO_AREA_STATE (state));

  if (width != NULL)
    *width = priv->area_width;
  if (height != NULL)
    *height = priv->area_height;
}

/* Функция задаёт новые значения размеров видимой области. */
void
gtk_cifro_area_state_set_visible_size (GtkCifroAreaState *state,
                                       gint               width,
                                       gint               height)
{
  GtkCifroAreaStatePrivate *priv = state->priv;

  GtkCifroAreaSize visible_size;
  gboolean emit = FALSE;

  g_return_if_fail (GTK_IS_CIFRO_AREA_STATE (state));

  if (priv->visible_width != width || priv->visible_height != height)
    {
      visible_size.width = priv->visible_width = width;
      visible_size.height = priv->visible_height = height;

      /* Перерасчитываем текущий масштаб. */
      if (priv->visible_width > 0)
        priv->scale_x = (priv->to_x - priv->from_x) / priv->visible_width;
      if (priv->visible_height > 0)
        priv->scale_y = (priv->to_y - priv->from_y) / priv->visible_height;

      emit = TRUE;
    }

  if (emit)
    g_signal_emit (state, gtk_cifro_area_state_signals[SIGNAL_VISIBLE_CHANGED], 0, &visible_size);
}

/* Функция возвращает текущие значения размеров видимой области. */
void
gtk_cifro_area_state_get_visible_size (GtkCifroAreaState *state,
                                       gint              *width,
                                       gint              *height)
{
  GtkCifroAreaStatePrivate *priv = state->priv;

  g_return_if_fail (GTK_IS_CIFRO_AREA_STATE (state));

  if (width != NULL)
    *width = priv->visible_width;
  if (height != NULL)
    *height = priv->visible_height;
}

/* Функция задаёт новые значения размеров окантовки видимой области. */
void
gtk_cifro_area_state_set_border (GtkCifroAreaState *state,
                                 gint               left,
                                 gint               right,
                                 gint               top,
                                 gint               bottom)
{
  GtkCifroAreaStatePrivate *priv = state->priv;

  GtkCifroAreaBorder border;
  gboolean emit = FALSE;

  g_return_if_fail (GTK_IS_CIFRO_AREA_STATE (state));

  if (priv->border_left != left || priv->border_right != right ||
      priv->border_top != top || priv->border_bottom != bottom)
    {
      border.left = priv->border_left = left;
      border.right = priv->border_right = right;
      border.top = priv->border_top = top;
      border.bottom = priv->border_bottom = bottom;
      emit = TRUE;
    }

  if (emit)
    g_signal_emit (state, gtk_cifro_area_state_signals[SIGNAL_BORDER_CHANGED], 0, &border);
}

/* Функция возвращает текущие значения размеров окантовки видимой области. */
void
gtk_cifro_area_state_get_border (GtkCifroAreaState *state,
                                 gint              *left,
                                 gint              *right,
                                 gint              *top,
                                 gint              *bottom)
{
  GtkCifroAreaStatePrivate *priv = state->priv;

  g_return_if_fail (GTK_IS_CIFRO_AREA_STATE (state));

  if (left != NULL)
    *left = priv->border_left;
  if (right != NULL)
    *right = priv->border_right;
  if (top != NULL)
    *top = priv->border_top;
  if (bottom != NULL)
    *bottom = priv->border_bottom;
}

/* Функция задаёт новые значения параметров зеркального отражения по осям. */
void
gtk_cifro_area_state_set_swap (GtkCifroAreaState *state,
                               gboolean           swap_x,
                               gboolean           swap_y)
{
  GtkCifroAreaStatePrivate *priv = state->priv;

  GtkCifroAreaSwap swap;
  gboolean emit = FALSE;

  g_return_if_fail (GTK_IS_CIFRO_AREA_STATE (state));

  if (priv->swap_x != swap_x || priv->swap_x != swap_x)
    {
      swap.swap_x = priv->swap_x = swap_x;
      swap.swap_y = priv->swap_y = swap_y;
      emit = TRUE;
    }

  if (emit)
    g_signal_emit (state, gtk_cifro_area_state_signals[SIGNAL_SWAP_CHANGED], 0, &swap);
}

/* Функция возвращает текущие значения параметров зеркального отражения по осям. */
void
gtk_cifro_area_state_get_swap (GtkCifroAreaState *state,
                               gboolean          *swap_x,
                               gboolean          *swap_y)
{
  GtkCifroAreaStatePrivate *priv = state->priv;

  g_return_if_fail (GTK_IS_CIFRO_AREA_STATE (state));

  if (swap_x != NULL)
    *swap_x = priv->swap_x;
  if (swap_y != NULL)
    *swap_y = priv->swap_y;
}

/* Функция задаёт новое значение угла поворота изображения видимой области. */
void
gtk_cifro_area_state_set_angle (GtkCifroAreaState *state,
                                gdouble            angle)
{
  GtkCifroAreaStatePrivate *priv = state->priv;
  gboolean emit = FALSE;

  g_return_if_fail (GTK_IS_CIFRO_AREA_STATE (state));

  if (priv->angle != angle)
    {
      priv->angle = angle;
      priv->angle_cos = cos (angle);
      priv->angle_sin = sin (angle);
      emit = TRUE;
    }

  if (emit)
    g_signal_emit (state, gtk_cifro_area_state_signals[SIGNAL_ANGLE_CHANGED], 0, angle);
}

/* Функция возвращает текущее значение угла поворота изображения видимой области. */
gdouble
gtk_cifro_area_state_get_angle (GtkCifroAreaState *state)
{
  GtkCifroAreaStatePrivate *priv = state->priv;
  gdouble angle;

  g_return_val_if_fail (GTK_IS_CIFRO_AREA_STATE (state), 0.0);

  angle = priv->angle;

  return angle;
}

/* Функция задаёт новые значения пределов отображения. */
void
gtk_cifro_area_state_set_view_limits (GtkCifroAreaState *state,
                                      gdouble            min_x,
                                      gdouble            max_x,
                                      gdouble            min_y,
                                      gdouble            max_y)
{
  GtkCifroAreaStatePrivate *priv = state->priv;

  GtkCifroAreaView view;
  gboolean emit = FALSE;

  g_return_if_fail (GTK_IS_CIFRO_AREA_STATE (state));

  if (priv->min_x != min_x || priv->max_x != max_x ||
      priv->min_y != min_y || priv->max_y != max_y)
    {
      view.x1 = priv->min_x = min_x;
      view.x2 = priv->max_x = max_x;
      view.y1 = priv->min_y = min_y;
      view.y2 = priv->max_y = max_y;
      emit = TRUE;
    }

  if (emit)
    g_signal_emit (state, gtk_cifro_area_state_signals[SIGNAL_VIEW_LIMITS_CHANGED], 0, &view);
}

/* Функция возвращает значения пределов отображения. */
void
gtk_cifro_area_state_get_view_limits (GtkCifroAreaState *state,
                                      gdouble           *min_x,
                                      gdouble           *max_x,
                                      gdouble           *min_y,
                                      gdouble           *max_y)
{
  GtkCifroAreaStatePrivate *priv = state->priv;

  g_return_if_fail (GTK_IS_CIFRO_AREA_STATE (state));

  if (min_x != NULL)
    *min_x = priv->min_x;
  if (max_x != NULL)
    *max_x = priv->max_x;
  if (min_y != NULL)
    *min_y = priv->min_y;
  if (max_y != NULL)
    *max_y = priv->max_y;
}

/* Функция задаёт новые значения текущей границы отображения. */
void
gtk_cifro_area_state_set_view (GtkCifroAreaState *state,
                               gdouble            from_x,
                               gdouble            to_x,
                               gdouble            from_y,
                               gdouble            to_y)
{
  GtkCifroAreaStatePrivate *priv = state->priv;

  GtkCifroAreaView view;
  gboolean emit = FALSE;

  g_return_if_fail (GTK_IS_CIFRO_AREA_STATE (state));

  if (priv->from_x != from_x || priv->to_x != to_x ||
      priv->from_y != from_y || priv->to_y != to_y)
    {
      view.x1 = priv->from_x = from_x;
      view.x2 = priv->to_x = to_x;
      view.y1 = priv->from_y = from_y;
      view.y2 = priv->to_y = to_y;

      /* Перерасчитываем текущий масштаб. */
      if (priv->visible_width > 0)
        priv->scale_x = (priv->to_x - priv->from_x) / priv->visible_width;
      if (priv->visible_height > 0)
        priv->scale_y = (priv->to_y - priv->from_y) / priv->visible_height;

      emit = TRUE;
    }

  if (emit)
    g_signal_emit (state, gtk_cifro_area_state_signals[SIGNAL_VIEW_CHANGED], 0, &view);
}

/* Функция возвращает значения текущей границы отображения. */
void
gtk_cifro_area_state_get_view (GtkCifroAreaState *state,
                               gdouble           *from_x,
                               gdouble           *to_x,
                               gdouble           *from_y,
                               gdouble           *to_y)
{
  GtkCifroAreaStatePrivate *priv = state->priv;

  g_return_if_fail (GTK_IS_CIFRO_AREA_STATE (state));

  if (from_x != NULL)
    *from_x = priv->from_x;
  if (to_x != NULL)
    *to_x = priv->to_x;
  if (from_y != NULL)
    *from_y = priv->from_y;
  if (to_y != NULL)
    *to_y = priv->to_y;
}

/* Функция возвращает значения текущих масштабов отображения. */
void
gtk_cifro_area_state_get_scale (GtkCifroAreaState *state,
                                gdouble           *scale_x,
                                gdouble           *scale_y)
{
  GtkCifroAreaStatePrivate *priv = state->priv;

  g_return_if_fail (GTK_IS_CIFRO_AREA_STATE (state));

  if (scale_x != NULL)
    *scale_x = priv->scale_x;
  if (scale_y != NULL)
    *scale_y = priv->scale_y;
}

/* Функция преобразовает координаты из прямоугольной системы окна в логические координаты отображаемые в объекте. */
void
gtk_cifro_area_state_point_to_value (GtkCifroAreaState *state,
                                     gdouble            x,
                                     gdouble            y,
                                     gdouble           *x_val,
                                     gdouble           *y_val)
{
  GtkCifroAreaStatePrivate *priv = state->priv;

  gdouble x_val_tmp;
  gdouble y_val_tmp;

  g_return_if_fail (GTK_IS_CIFRO_AREA_STATE (state));

  /* Переносим систему координат в центр отображаемой области (виджета или изображения).
     Этот перенос необходим для корректного расчёта логических координат при повороте.*/
  x = x - priv->area_width / 2.0;
  y = priv->area_height / 2.0 - y;

  /* Расчитываем логическую координату с учётом возможного поворота изображения. */
  if (priv->angle != 0.0)
    {
      x_val_tmp = (x * priv->angle_cos - y * priv->angle_sin) * priv->scale_x;
      y_val_tmp = (y * priv->angle_cos + x * priv->angle_sin) * priv->scale_y;
    }
  else
    {
      x_val_tmp = x * priv->scale_x;
      y_val_tmp = y * priv->scale_y;
    }

  /* Если включено зеркальное отражение по осям - переносим координаты. */
  if (priv->swap_x)
    x_val_tmp = -x_val_tmp;
  if (priv->swap_y)
    y_val_tmp = -y_val_tmp;

  /* Выполняем обратный перенос системы координат. */
  x_val_tmp = ((priv->to_x - priv->from_x) / 2.0) + priv->from_x + x_val_tmp;
  y_val_tmp = ((priv->to_y - priv->from_y) / 2.0) + priv->from_y + y_val_tmp;

  if (x_val != NULL)
    *x_val = x_val_tmp;
  if (y_val != NULL)
    *y_val = y_val_tmp;
}

/* Функция преобразовает координаты из логических в прямоугольную систему координат окна. */
void
gtk_cifro_area_state_value_to_point (GtkCifroAreaState *state,
                                     gdouble           *x,
                                     gdouble           *y,
                                     gdouble            x_val,
                                     gdouble            y_val)
{
  GtkCifroAreaStatePrivate *priv = state->priv;

  gdouble x_tmp;
  gdouble y_tmp;

  g_return_if_fail (GTK_IS_CIFRO_AREA_STATE (state));

  /* Переносим систему координат в центр видимой области.
     Этот перенос необходим для корректного расчёта координат при повороте. */
  x_val = x_val - ((priv->to_x - priv->from_x) / 2.0) - priv->from_x;
  y_val = y_val - ((priv->to_y - priv->from_y) / 2.0) - priv->from_y;

  /* Перводим из логических координат в точки. */
  x_val = x_val / priv->scale_x;
  y_val = y_val / priv->scale_y;

  /* Если включено зеркальное отражение по осям - переносим координаты. */
  if (priv->swap_x)
    x_val = -x_val;
  if (priv->swap_y)
    y_val = -y_val;

  /* Расчитываем координаты с учётом возможного поворота изображения. */
  if (priv->angle != 0.0)
    {
      x_tmp = x_val * priv->angle_cos + y_val * priv->angle_sin;
      y_tmp = y_val * priv->angle_cos - x_val * priv->angle_sin;
    }
  else
    {
      x_tmp = x_val;
      y_tmp = y_val;
    }

  /* Выполняем обратный перенос системы координат. */
  x_tmp = x_tmp + priv->area_width / 2.0;
  y_tmp = priv->area_height / 2.0 - y_tmp;

  if (x != NULL)
    *x = x_tmp;
  if (y != NULL)
    *y = y_tmp;
}

/* Функция преобразовает координаты из прямоугольной системы видимой области в логические координаты. */
void
gtk_cifro_area_state_visible_point_to_value (GtkCifroAreaState *state,
                                             gdouble            x,
                                             gdouble            y,
                                             gdouble           *x_val,
                                             gdouble           *y_val)
{
  GtkCifroAreaStatePrivate *priv = state->priv;

  g_return_if_fail (GTK_IS_CIFRO_AREA_STATE (state));

  if (x_val != NULL)
    *x_val = priv->from_x + x * priv->scale_x;
  if (y_val != NULL)
    *y_val = priv->to_y - y * priv->scale_y;
}

/* Функция преобразовает координаты из логических в прямоугольную систему координат видимой области. */
void
gtk_cifro_area_state_visible_value_to_point (GtkCifroAreaState *state,
                                             gdouble           *x,
                                             gdouble           *y,
                                             gdouble            x_val,
                                             gdouble            y_val)
{
  GtkCifroAreaStatePrivate *priv = state->priv;

  g_return_if_fail (GTK_IS_CIFRO_AREA_STATE (state));

  if (x != NULL)
    *x = (x_val - priv->from_x) / priv->scale_x;
  if (y != NULL)
    *y = (priv->to_y - y_val) / priv->scale_y;
}

/* Функция расчитывает параметры прямоугольной координатной сетки. */
void
gtk_cifro_area_state_get_axis_step (gdouble  scale,
                                    gdouble  step_width,
                                    gdouble *from,
                                    gdouble *step,
                                    gint    *range,
                                    gint    *power)
{
  gdouble step_length;

  gdouble axis_1_width_delta;
  gdouble axis_2_width_delta;
  gdouble axis_5_width_delta;

  gdouble axis_1_score;
  gdouble axis_2_score;
  gdouble axis_5_score;

  gdouble from_ret;
  gdouble step_ret;
  gint range_ret;
  gint power_ret;

  /* Расстояние между соседними линиями координатной сетки в логических координатах. */
  step_length = scale * step_width;

  /* Нормируем расстояние в диапазон значений от 1 до 10. */
  power_ret = 0;
  while ((step_length > 10) || (step_length < 1))
    {
      if (step_length > 10)
        {
          step_length /= 10.0;
          power_ret = power_ret + 1;
        }
      if (step_length < 1)
        {
          step_length *= 10.0;
          power_ret = power_ret - 1;
        }
    }
  if (step_length > 5)
    {
      step_length /= 10.0;
      power_ret = power_ret + 1;
    }

  /* Выбираем с каким шагом рисовать сетку: 1, 2 или 5 (плюс их степени). */

  /* Расчитываем разность между размером ячейки для трёх возможных вариантов сетки и
     предпочтительным размером ячейки определёеным пользователем. */
  axis_1_width_delta = (1.0 * pow (10.0, power_ret) / scale) - step_width;
  axis_2_width_delta = (2.0 * pow (10.0, power_ret) / scale) - step_width;
  axis_5_width_delta = (5.0 * pow (10.0, power_ret) / scale) - step_width;

  /* Расчитываем "вес" каждого варианта. */
  axis_1_score = (axis_1_width_delta >= 0.0) ? 1.0 / axis_1_width_delta : -0.1 / axis_1_width_delta;
  axis_2_score = (axis_2_width_delta >= 0.0) ? 1.0 / axis_2_width_delta : -0.1 / axis_2_width_delta;
  axis_5_score = (axis_5_width_delta >= 0.0) ? 1.0 / axis_5_width_delta : -0.1 / axis_5_width_delta;

  /* Выбираем предпочтительный шаг координатной сетки. */
  if (axis_1_score > axis_2_score && axis_1_score > axis_5_score)
    range_ret = 1;
  else if (axis_2_score > axis_5_score)
    range_ret = 2;
  else
    range_ret = 5;

  /* Расчитываем истиный шаг координатной сетки с учётом нормировки. */
  step_ret = range_ret * pow (10.0, power_ret);

  /* Корректируем начало координатной сетки. */
  from_ret = step_ret * floor (*from / step_ret);
  if (from && from_ret < *from)
    from_ret += step_ret;

  /* Возвращаем результаты. */
  *from = from_ret;
  if (step != NULL)
    *step = step_ret;
  if (range != NULL)
    *range = range_ret;
  if (power != NULL)
    *power = power_ret;
}

/* Функция выравнивает координаты для использования в библиотеке cairo. */
gdouble
gtk_cifro_area_state_point_to_cairo (gdouble point)
{
  gdouble ipoint = (glong) point;

  return ((point - ipoint) > 0.5) ? ipoint + 0.5 : ipoint - 0.5;
}
