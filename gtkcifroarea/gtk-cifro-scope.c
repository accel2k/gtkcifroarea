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
 * SECTION: gtk-cifro-scope
 * @Short_description: GTK+ виджет осциллографа
 * @Title: GtkCifroScope
 * @See_also: #GtkCifroArea, #GtkCifroAreaControl
 *
 * CifroScope позволяет реализовывать графическое представление данных как в осциллографе.
 * Данный виджет является наследуемым от виджета #GtkCifroAreaControl и к нему могут
 * применяться все функции последнего. Управление виджетом осуществляется аналогично
 * #GtkCifroAreaControl.
 *
 * Перед отображеним данных в осциллограф необходимо добавить каналы отображения, установить
 * параметры отображения данных и задать сами данные. После того как данные для всех каналов
 * определены необходимо вызвать функцию gtk_widget_queue_draw() для обновления изображения.
 *
 */

#include "gtk-cifro-scope.h"
#include "cairo-sdline.h"

#include <glib/gprintf.h>
#include <string.h>
#include <math.h>

enum
{
  PROP_O,
  PROP_GRAVITY
};

typedef struct
{
  gchar                       *name;                           /* Имя канала. */
  GtkCifroScopeDrawType        draw_type;                      /* Тип отображения осциллограмм. */
  guint32                      color;                          /* Цвета данных канала. */
  gboolean                     show;                           /* "Выключатели" каналов осциллографа. */
  gint                         num;                            /* Число данных для отображения. */

  gfloat                       time_shift;                     /* Смещение данных по времени. */
  gfloat                       time_step;                      /* Шаг времени. */
  gfloat                       value_shift;                    /* Коэффициент смещения данных. */
  gfloat                       value_scale;                    /* Коэффициент масштабирования данных. */

  gfloat                      *data;                           /* Данные для отображения. */
  gint                         size;                           /* Размер массива данных для отображения. */
} GtkCifroScopeChannel;

struct _GtkCifroScopePrivate
{
  GtkCifroScopeGravity         gravity;                        /* Направление осей осциллографа. */
  gboolean                     show_info;                      /* Показывать или нет информацию о значениях под курсором. */
  gint32                       next_channel_id;                /* Идентификатор для нового канала. */

  gdouble                      min_x;                          /* Минимально возможное значение по оси x. */
  gdouble                      max_x;                          /* Максимально возможное значение по оси x. */
  gdouble                      min_y;                          /* Минимально возможное значение по оси y. */
  gdouble                      max_y;                          /* Максимально возможное значение по оси y. */

  GHashTable                  *channels;                       /* Данные каналов осциллографа. */

  gint                         pointer_x;                      /* Текущее местоположение курсора, x координата. */
  gint                         pointer_y;                      /* Текущее местоположение курсора, y координата. */

  PangoLayout                 *font;                           /* Раскладка шрифта. */

  gchar                       *x_axis_name;                    /* Подпись оси времени. */
  gchar                       *y_axis_name;                    /* Подпись оси значений. */

  guint                        border_size;                    /* Размер области обрамления. */
  guint32                      border_color;                   /* Цвет обрамления области отображения данных. */
  guint32                      axis_color;                     /* Цвет осей. */
  guint32                      zero_axis_color;                /* Цвет осей для нулевых значений. */
  guint32                      text_color;                     /* Цвет подписей. */
};

static void            gtk_cifro_scope_set_property            (GObject                       *object,
                                                                guint                          prop_id,
                                                                const GValue                  *value,
                                                                GParamSpec                    *pspec);
static void            gtk_cifro_scope_object_constructed      (GObject                       *object);
static void            gtk_cifro_scope_object_finalize         (GObject                       *object);

static void            gtk_cifro_scope_free_channel            (gpointer                       data);

static gboolean        gtk_cifro_scope_get_rotate              (GtkCifroArea                  *carea);

static void            gtk_cifro_scope_get_swap                (GtkCifroArea                  *carea,
                                                                gboolean                      *swap_x,
                                                                gboolean                      *swap_y);

static void            gtk_cifro_scope_get_border              (GtkCifroArea                  *carea,
                                                                guint                         *top,
                                                                guint                         *bottom,
                                                                guint                         *left,
                                                                guint                         *right);

static void            gtk_cifro_scope_get_limits              (GtkCifroArea                  *carea,
                                                                gdouble                       *min_x,
                                                                gdouble                       *max_x,
                                                                gdouble                       *min_y,
                                                                gdouble                       *max_y);

static void            gtk_cifro_scope_set_fg_color            (GtkCifroScopePrivate          *priv,
                                                                gdouble                        red,
                                                                gdouble                        green,
                                                                gdouble                        blue,
                                                                gdouble                        alpha);

static void            gtk_cifro_scope_draw_hruler             (GtkWidget                     *widget,
                                                                cairo_t                       *cairo);
static void            gtk_cifro_scope_draw_vruler             (GtkWidget                     *widget,
                                                                cairo_t                       *cairo);
static void            gtk_cifro_scope_draw_x_pos              (GtkWidget                     *widget,
                                                                cairo_t                       *cairo);
static void            gtk_cifro_scope_draw_y_pos              (GtkWidget                     *widget,
                                                                cairo_t                       *cairo);
static void            gtk_cifro_scope_draw_info               (GtkWidget                     *widget,
                                                                cairo_t                       *cairo);

static void            gtk_cifro_scope_draw_axis               (GtkWidget                     *widget,
                                                                cairo_sdline_surface          *surface);
static void            gtk_cifro_scope_draw_lined_data         (GtkWidget                     *widget,
                                                                cairo_sdline_surface          *surface,
                                                                gpointer                       channel_id);
static void            gtk_cifro_scope_draw_dotted_data        (GtkWidget                     *widget,
                                                                cairo_sdline_surface          *surface,
                                                                gpointer                       channel_id,
                                                                guint                          size);
static void            gtk_cifro_scope_draw_crossed_data       (GtkWidget                     *widget,
                                                                cairo_sdline_surface          *surface,
                                                                gpointer                       channel_id,
                                                                guint                          size);

static void            gtk_cifro_scope_area_draw               (GtkWidget                     *widget,
                                                                cairo_t                       *cairo);
static void            gtk_cifro_scope_visible_draw            (GtkWidget                     *widget,
                                                                cairo_t                       *cairo);

static gboolean        gtk_cifro_scope_configure               (GtkWidget                     *widget,
                                                                GdkEventConfigure             *event);
static gboolean        gtk_cifro_scope_motion_notify           (GtkWidget                     *widget,
                                                                GdkEventMotion                *event);
static gboolean        gtk_cifro_scope_leave_notify            (GtkWidget                     *widget,
                                                                GdkEventCrossing              *event);

G_DEFINE_TYPE_WITH_PRIVATE (GtkCifroScope, gtk_cifro_scope, GTK_TYPE_CIFRO_AREA_CONTROL)

static void
gtk_cifro_scope_init (GtkCifroScope *cscope)
{
  cscope->priv =  gtk_cifro_scope_get_instance_private (cscope);
}

static void
gtk_cifro_scope_class_init (GtkCifroScopeClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkCifroAreaClass *carea_class = GTK_CIFRO_AREA_CLASS (klass);

  object_class->set_property = gtk_cifro_scope_set_property;

  object_class->constructed = gtk_cifro_scope_object_constructed;
  object_class->finalize = gtk_cifro_scope_object_finalize;

  carea_class->get_rotate = gtk_cifro_scope_get_rotate;
  carea_class->get_swap = gtk_cifro_scope_get_swap;
  carea_class->get_border = gtk_cifro_scope_get_border;
  carea_class->get_limits = gtk_cifro_scope_get_limits;

  g_object_class_install_property (object_class, PROP_GRAVITY,
    g_param_spec_int ("gravity", "Gravity", "Gravity",
                      GTK_CIFRO_SCOPE_GRAVITY_RIGHT_UP, GTK_CIFRO_SCOPE_GRAVITY_DOWN_LEFT,
                      GTK_CIFRO_SCOPE_GRAVITY_RIGHT_UP,
                      G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY));
}

static void
gtk_cifro_scope_set_property (GObject      *object,
                              guint         prop_id,
                              const GValue *value,
                              GParamSpec   *pspec)
{
  GtkCifroScope *cscope = GTK_CIFRO_SCOPE (object);
  GtkCifroScopePrivate *priv = cscope->priv;

  switch (prop_id)
    {
    case PROP_GRAVITY:
      priv->gravity = g_value_get_int (value);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

static void
gtk_cifro_scope_object_constructed (GObject *object)
{
  GtkCifroScope *cscope = GTK_CIFRO_SCOPE (object);
  GtkCifroScopePrivate *priv = cscope->priv;

  G_OBJECT_CLASS (gtk_cifro_scope_parent_class)->constructed (object);

  /* Параметры GtkCifroArea по умолчанию. */
  gtk_cifro_area_set_scale_on_resize (GTK_CIFRO_AREA (cscope), TRUE);

  /* Информация об осях. */
  priv->x_axis_name = g_strdup ("X");
  priv->y_axis_name = g_strdup ("Y");

  /* Каналы осциллографа. */
  priv->channels = g_hash_table_new_full (g_direct_hash, g_direct_equal, NULL,
                                          gtk_cifro_scope_free_channel);

  /* Координаты информационной точки. */
  priv->pointer_x = -1;
  priv->pointer_y = -1;

  /* Обработчики сигналов. */
  g_signal_connect (cscope, "area-draw", G_CALLBACK (gtk_cifro_scope_area_draw), NULL);
  g_signal_connect (cscope, "visible-draw", G_CALLBACK (gtk_cifro_scope_visible_draw), NULL);
  g_signal_connect (cscope, "configure-event", G_CALLBACK (gtk_cifro_scope_configure), NULL);
  g_signal_connect (cscope, "motion-notify-event", G_CALLBACK (gtk_cifro_scope_motion_notify), NULL);
  g_signal_connect (cscope, "leave-notify-event", G_CALLBACK (gtk_cifro_scope_leave_notify), NULL);
}

static void
gtk_cifro_scope_object_finalize (GObject *object)
{
  GtkCifroScope *cscope = GTK_CIFRO_SCOPE (object);
  GtkCifroScopePrivate *priv = cscope->priv;

  g_clear_pointer (&priv->font, g_object_unref);
  g_hash_table_unref (priv->channels);

  g_free (priv->x_axis_name);
  g_free (priv->y_axis_name);

  G_OBJECT_CLASS (gtk_cifro_scope_parent_class)->finalize (object);
}

/* Функция удаляет структуру с данными канала. */
static void
gtk_cifro_scope_free_channel (gpointer data)
{
  GtkCifroScopeChannel *channel = data;

  if (channel == NULL)
    return;

  g_free (channel->data);
  g_free (channel->name);
  g_free (channel);
}

/* Виртуальная функция для определения разрешения поворота изображения. */
static gboolean
gtk_cifro_scope_get_rotate (GtkCifroArea *carea)
{
  return FALSE;
}

/* Виртуальная функция для определения необходимости отражения изображения. */
static void
gtk_cifro_scope_get_swap (GtkCifroArea *carea,
                          gboolean     *swap_x,
                          gboolean     *swap_y)
{
  GtkCifroScope *cscope;
  gboolean new_swap_x;
  gboolean new_swap_y;
  gdouble new_angle;

  cscope = GTK_CIFRO_SCOPE (carea);

  switch (cscope->priv->gravity)
    {
    case GTK_CIFRO_SCOPE_GRAVITY_RIGHT_UP:
      new_swap_x = FALSE;
      new_swap_y = FALSE;
      new_angle = 0;
      break;

    case GTK_CIFRO_SCOPE_GRAVITY_LEFT_UP:
      new_swap_x = TRUE;
      new_swap_y = FALSE;
      new_angle = 0;
      break;

    case GTK_CIFRO_SCOPE_GRAVITY_RIGHT_DOWN:
      new_swap_x = FALSE;
      new_swap_y = TRUE;
      new_angle = 0;
      break;

    case GTK_CIFRO_SCOPE_GRAVITY_LEFT_DOWN:
      new_swap_x = TRUE;
      new_swap_y = TRUE;
      new_angle = 0;
      break;

    case GTK_CIFRO_SCOPE_GRAVITY_UP_RIGHT:
      new_swap_x = TRUE;
      new_swap_y = FALSE;
      new_angle = G_PI / 2.0;
      break;

    case GTK_CIFRO_SCOPE_GRAVITY_UP_LEFT:
      new_swap_x = TRUE;
      new_swap_y = TRUE;
      new_angle = G_PI / 2.0;
      break;

    case GTK_CIFRO_SCOPE_GRAVITY_DOWN_RIGHT:
      new_swap_x = FALSE;
      new_swap_y = FALSE;
      new_angle = G_PI / 2.0;
      break;

    case GTK_CIFRO_SCOPE_GRAVITY_DOWN_LEFT:
      new_swap_x = FALSE;
      new_swap_y = TRUE;
      new_angle = G_PI / 2.0;
      break;

    default:
      new_swap_x = FALSE;
      new_swap_y = FALSE;
      new_angle = 0;
      break;
    }

  gtk_cifro_area_set_angle (carea, new_angle);

  (swap_x != NULL) ? *swap_x = new_swap_x : 0;
  (swap_y != NULL) ? *swap_y = new_swap_y : 0;
}

/* Виртуальная функция для определения размеров окантовки. */
static void
gtk_cifro_scope_get_border (GtkCifroArea *carea,
                            guint        *top,
                            guint        *bottom,
                            guint        *left,
                            guint        *right)
{
  GtkCifroScope *cscope;
  guint border;

  cscope = GTK_CIFRO_SCOPE (carea);
  border = cscope->priv->border_size;

  (top != NULL) ? *top = border : 0;
  (bottom != NULL) ? *bottom = border : 0;
  (left != NULL) ? *left = border : 0;
  (right != NULL) ? *right = border : 0;
}

/* Виртуальная функция для определения текущих значений пределов перемещения изображения. */
static void
gtk_cifro_scope_get_limits (GtkCifroArea *carea,
                            gdouble      *min_x,
                            gdouble      *max_x,
                            gdouble      *min_y,
                            gdouble      *max_y)
{
  GtkCifroScopePrivate *priv;
  GtkCifroScope *cscope;

  cscope = GTK_CIFRO_SCOPE (carea);
  priv = cscope->priv;

  (min_x != NULL) ? *min_x = priv->min_x : 0;
  (max_x != NULL) ? *max_x = priv->max_x : 0;
  (min_y != NULL) ? *min_y = priv->min_y : 0;
  (max_y != NULL) ? *max_y = priv->max_y : 0;
}

/* Функция устанавливает основной цвет для отображения графических элементов в осциллографе. */
static void
gtk_cifro_scope_set_fg_color (GtkCifroScopePrivate *priv,
                              gdouble               red,
                              gdouble               green,
                              gdouble               blue,
                              gdouble               alpha)
{
  gdouble luminance;
  gdouble border_luminance;
  gdouble zero_axis_luminance;
  gdouble axis_luminance;

  /* Яркость основного цвета. */
  luminance = alpha * (0.2126 * red + 0.7152 * green + 0.0722 * blue);
  if (luminance < 0.2)
    luminance = 0.2;

  /* Светлая тема. */
  if (luminance < 0.5)
    {
      border_luminance = 0.8 * (1.0 - luminance);
      zero_axis_luminance = 0.9 * (1.0 - luminance);
      axis_luminance = 1.0 * (1.0 - luminance);
    }
  /* Тёмная тема. */
  else
    {
      border_luminance = 0.8 * luminance;
      zero_axis_luminance = 0.6 * luminance;
      axis_luminance = 0.4 * luminance;
    }

  priv->border_color = cairo_sdline_color (border_luminance, border_luminance, border_luminance, 1.0);
  priv->zero_axis_color = cairo_sdline_color (zero_axis_luminance, zero_axis_luminance, zero_axis_luminance, 1.0);
  priv->axis_color = cairo_sdline_color (axis_luminance, axis_luminance, axis_luminance, 1.0);
  priv->text_color = cairo_sdline_color (red, green, blue, 1.0);
}

/* Рисование оцифровки оси абсцисс. */
static void
gtk_cifro_scope_draw_hruler (GtkWidget *widget,
                             cairo_t   *cairo)
{
  GtkCifroArea *carea = GTK_CIFRO_AREA (widget);
  GtkCifroScope *cscope = GTK_CIFRO_SCOPE (widget);
  GtkCifroScopePrivate *priv = cscope->priv;
  PangoLayout *font = priv->font;
  guint border_size = priv->border_size;

  guint area_width;
  guint area_height;


  gdouble from_x;
  gdouble to_x;
  gdouble from_y;
  gdouble to_y;

  gdouble scale_x;
  gdouble scale_y;

  gdouble angle;

  gdouble step;
  gdouble axis;
  gdouble axis_to;
  gdouble axis_pos;
  gdouble axis_from;
  gdouble axis_step;
  gdouble axis_scale;
  gdouble axis_mini_step;
  guint axis_range;
  gint axis_power;
  gint axis_height;
  gint axis_count;

  gchar text_format[128];
  gchar text_str[128];

  gint text_width;
  gint text_height;

  gboolean swap = FALSE;

  gtk_cifro_area_get_size (carea, &area_width, &area_height);
  gtk_cifro_area_get_view (carea, &from_x, &to_x, &from_y, &to_y);
  gtk_cifro_area_get_scale (carea, &scale_x, &scale_y);
  angle = gtk_cifro_area_get_angle (carea);

  /* Проверяем состояние объекта. */
  if (priv->font == NULL)
    return;

  step = 4.0 * border_size;

  if ((angle < -0.01) || (angle > (G_PI / 2.0 + 0.01)))
    return;
  if (fabs (angle - G_PI / 2.0) < (G_PI / 4.0))
    swap = TRUE;

  axis_to = swap ? to_y : to_x;
  axis_from = swap ? from_y : from_x;
  axis_scale = swap ? scale_y : scale_x;

  /* Шаг оцифровки. */
  if (!gtk_cifro_area_get_axis_step (axis_scale, step, &axis_from,
                                     &axis_step, &axis_range, &axis_power))
    {
      return;
    }

  if (axis_range == 1)
    axis_range = 10;
  if (axis_range == 2)
    axis_mini_step = axis_step / 2.0;
  else if (axis_range == 5)
    axis_mini_step = axis_step / 5.0;
  else
    axis_mini_step = axis_step / 10.0;

  /* Рисуем засечки на осях. */
  cairo_sdline_set_cairo_color (cairo, priv->zero_axis_color);
  cairo_set_line_width (cairo, 1.0);

  axis_count = 0;
  axis = axis_from - axis_step;
  while (axis <= axis_to)
    {
      if (swap)
        gtk_cifro_area_value_to_point (carea, &axis_pos, NULL, from_x, axis);
      else
        gtk_cifro_area_value_to_point (carea, &axis_pos, NULL, axis, 0.0);

      if (axis_count % axis_range == 0)
        axis_height = border_size / 4.0;
      else
        axis_height = border_size / 8.0;
      axis_count += 1;

      axis += axis_mini_step;
      if (axis_pos <= (border_size + 1))
        continue;
      if (axis_pos >= (area_width - border_size - 1))
        continue;

      cairo_move_to (cairo, (gint) axis_pos + 0.5, border_size + 0.5);
      cairo_line_to (cairo, (gint) axis_pos + 0.5, border_size - axis_height + 0.5);
    }

  cairo_stroke (cairo);

  /* Рисуем подписи на оси. */
  cairo_sdline_set_cairo_color (cairo, priv->text_color);

  if (axis_power > 0)
    axis_power = 0;
  g_snprintf (text_format, sizeof(text_format), "%%.%df", (gint) fabs (axis_power));

  axis = axis_from;
  while (axis <= axis_to)
    {
      g_ascii_formatd (text_str, sizeof(text_str), text_format, axis);
      pango_layout_set_text (font, text_str, -1);
      pango_layout_get_size (font, &text_width, &text_height);
      text_width /= PANGO_SCALE;
      text_height /= PANGO_SCALE;

      if (swap)
        gtk_cifro_area_value_to_point (carea, &axis_pos, NULL, from_x, axis);
      else
        gtk_cifro_area_value_to_point (carea, &axis_pos, NULL, axis, 0.0);
      axis_pos -= text_width / 2;
      axis += axis_step;

      if (axis_pos < (border_size + 1))
        continue;
      if ((axis_pos + text_width) > (area_width - border_size - 1))
        continue;

      cairo_move_to (cairo, axis_pos, ((0.85 * border_size) - text_height) / 2.0);
      pango_cairo_show_layout (cairo, font);
    }

  /* Рисуем название оси. */
  if (swap)
    pango_layout_set_text (font, priv->y_axis_name, -1);
  else
    pango_layout_set_text (font, priv->x_axis_name, -1);

  pango_layout_get_size (font, &text_width, &text_height);
  text_width /= PANGO_SCALE;
  text_height /= PANGO_SCALE;

  cairo_move_to (cairo, area_width - border_size / 2 - text_width / 2, border_size / 2 - text_height / 2);
  pango_cairo_show_layout (cairo, font);
}

/* Рисование оцифровки оси ординат. */
static void
gtk_cifro_scope_draw_vruler (GtkWidget *widget,
                             cairo_t   *cairo)
{
  GtkCifroArea *carea = GTK_CIFRO_AREA (widget);
  GtkCifroScope *cscope = GTK_CIFRO_SCOPE (widget);
  GtkCifroScopePrivate *priv = cscope->priv;
  PangoLayout *font = priv->font;
  guint border_size = priv->border_size;

  guint area_width;
  guint area_height;

  gdouble from_x;
  gdouble to_x;
  gdouble from_y;
  gdouble to_y;

  gdouble scale_x;
  gdouble scale_y;

  gdouble angle;

  gdouble step;
  gdouble axis;
  gdouble axis_to;
  gdouble axis_pos;
  gdouble axis_from;
  gdouble axis_step;
  gdouble axis_scale;
  gdouble axis_mini_step;
  guint axis_range;
  gint axis_power;
  gint axis_height;
  gint axis_count;

  gchar text_format[128];
  gchar text_str[128];

  gint text_width;
  gint text_height;

  gboolean swap = FALSE;

  gtk_cifro_area_get_size (carea, &area_width, &area_height);
  gtk_cifro_area_get_view (carea, &from_x, &to_x, &from_y, &to_y);
  gtk_cifro_area_get_scale (carea, &scale_x, &scale_y);
  angle = gtk_cifro_area_get_angle (carea);

  /* Проверяем состояние объекта. */
  if (priv->font == NULL)
    return;

  step = 4.0 * border_size;

  if ((angle < -0.01) || (angle > (G_PI / 2.0 + 0.01)))
    return;
  if (fabs (angle - G_PI / 2.0) < (G_PI / 4.0))
    swap = TRUE;

  axis_to = swap ? to_x : to_y;
  axis_from = swap ? from_x : from_y;
  axis_scale = swap ? scale_x : scale_y;

  /* Шаг оцифровки. */
  if (!gtk_cifro_area_get_axis_step (axis_scale, step, &axis_from,
                                     &axis_step, &axis_range, &axis_power))
    {
      return;
    }

  if (axis_range == 1)
    axis_range = 10;
  if (axis_range == 2)
    axis_mini_step = axis_step / 2.0;
  else if (axis_range == 5)
    axis_mini_step = axis_step / 5.0;
  else
    axis_mini_step = axis_step / 10.0;

  /* Рисуем засечки на осях. */
  cairo_sdline_set_cairo_color (cairo, priv->zero_axis_color);
  cairo_set_line_width (cairo, 1.0);

  axis_count = 0;
  axis = axis_from - axis_step;
  while (axis <= axis_to)
    {
      if (swap)
        gtk_cifro_area_value_to_point (carea, NULL, &axis_pos, axis, to_y);
      else
        gtk_cifro_area_value_to_point (carea, NULL, &axis_pos, 0.0, axis);

      if (axis_count % axis_range == 0)
        axis_height = border_size / 4.0;
      else
        axis_height = border_size / 8.0;
      axis_count += 1;

      axis += axis_mini_step;
      if (axis_pos <= (border_size + 1))
        continue;
      if (axis_pos >= (area_height - border_size - 1))
        continue;

      cairo_move_to (cairo, border_size - axis_height + 0.5, (gint) axis_pos + 0.5);
      cairo_line_to (cairo, border_size + 0.5, (gint) axis_pos + 0.5);
    }

  cairo_stroke (cairo);

  /* Рисуем подписи на оси. */
  cairo_sdline_set_cairo_color (cairo, priv->text_color);

  if (axis_power > 0)
    axis_power = 0;
  g_snprintf (text_format, sizeof(text_format), "%%.%df", (gint) fabs (axis_power));

  axis = axis_from;
  while (axis <= axis_to)
    {
      g_ascii_formatd (text_str, sizeof(text_str), text_format, axis);
      pango_layout_set_text (font, text_str, -1);
      pango_layout_get_size (font, &text_width, &text_height);
      text_width /= PANGO_SCALE;
      text_height /= PANGO_SCALE;

      if (swap)
        gtk_cifro_area_value_to_point (carea, NULL, &axis_pos, axis, to_y);
      else
        gtk_cifro_area_value_to_point (carea, NULL, &axis_pos, 0.0, axis);
      axis_pos += text_width / 2;
      axis += axis_step;

      if ((axis_pos - text_width) <= (border_size + 1))
        continue;
      if (axis_pos >= (area_height - border_size - 1))
        continue;

      cairo_save (cairo);
      cairo_move_to (cairo, 0.15 * border_size, axis_pos);
      cairo_rotate (cairo, -G_PI / 2.0);
      pango_cairo_show_layout (cairo, font);
      cairo_restore (cairo);
    }

  /* Рисуем название оси. */
  if (swap)
    pango_layout_set_text (font, priv->x_axis_name, -1);
  else
    pango_layout_set_text (font, priv->y_axis_name, -1);
  pango_layout_get_size (font, &text_width, &text_height);
  text_width /= PANGO_SCALE;
  text_height /= PANGO_SCALE;

  cairo_move_to (cairo, border_size / 2 - text_width / 2, area_height - border_size / 2 - text_height / 2);
  pango_cairo_show_layout (cairo, font);
}

/* Рисование "планшета" горизонтального местоположения видимой области. */
static void
gtk_cifro_scope_draw_x_pos (GtkWidget *widget,
                            cairo_t   *cairo)
{
  GtkCifroArea *carea = GTK_CIFRO_AREA (widget);
  GtkCifroScope *cscope = GTK_CIFRO_SCOPE (widget);
  GtkCifroScopePrivate *priv = cscope->priv;
  guint border_size = priv->border_size;

  guint area_width;
  guint area_height;

  gdouble min_x;
  gdouble max_x;
  gdouble min_y;
  gdouble max_y;

  gdouble from_x;
  gdouble to_x;
  gdouble from_y;
  gdouble to_y;

  gboolean swap_x;
  gboolean swap_y;

  gdouble angle;

  gdouble from;
  gdouble to;

  gint x;
  gint y;
  gint width;
  gint height;

  gboolean swap = FALSE;

  gtk_cifro_area_get_size (carea, &area_width, &area_height);
  gtk_cifro_area_get_limits (carea, &min_x, &max_x, &min_y, &max_y);
  gtk_cifro_area_get_view (carea, &from_x, &to_x, &from_y, &to_y);
  gtk_cifro_area_get_swap (carea, &swap_x, &swap_y);
  angle = gtk_cifro_area_get_angle (carea);

  from_x = MAX (from_x, min_x);
  to_x = MIN (to_x, max_x);
  from_y = MAX (from_y, min_y);
  to_y = MIN (to_y, max_y);

  if (fabs (angle - G_PI / 2.0) < (G_PI / 4.0))
    swap = TRUE;

  if (swap)
    {
      from = (from_y - min_y) / (max_y - min_y);
      to = (to_y - min_y) / (max_y - min_y);
      if (swap_y)
        {
          from = 1.0 - from;
          to = 1.0 - to;
        }
    }
  else
    {
      from = (from_x - min_x) / (max_x - min_x);
      to = (to_x - min_x) / (max_x - min_x);
      if (swap_x)
        {
          from = 1.0 - from;
          to = 1.0 - to;
        }
    }

  x = border_size + 0.5;
  y = area_height - 0.75 * border_size + 0.5;
  width = area_width - 2 * border_size;
  height = border_size / 2;

  cairo_sdline_set_cairo_color (cairo, priv->axis_color);
  cairo_rectangle (cairo, x, y, width, height);
  cairo_fill (cairo);

  cairo_sdline_set_cairo_color (cairo, priv->zero_axis_color);
  cairo_rectangle (cairo, x + (from * width), y, ((to - from) * width), height);
  cairo_fill (cairo);
}

/* Рисование "планшета" вертикального местоположения видимой области. */
static void
gtk_cifro_scope_draw_y_pos (GtkWidget *widget,
                            cairo_t   *cairo)
{
  GtkCifroArea *carea = GTK_CIFRO_AREA (widget);
  GtkCifroScope *cscope = GTK_CIFRO_SCOPE (widget);
  GtkCifroScopePrivate *priv = cscope->priv;
  guint border_size = priv->border_size;

  guint area_width;
  guint area_height;

  gdouble min_x;
  gdouble max_x;
  gdouble min_y;
  gdouble max_y;

  gdouble from_x;
  gdouble to_x;
  gdouble from_y;
  gdouble to_y;

  gboolean swap_x;
  gboolean swap_y;

  gdouble angle;

  gdouble from;
  gdouble to;

  gint x;
  gint y;
  gint width;
  gint height;

  gboolean swap = FALSE;

  gtk_cifro_area_get_size (carea, &area_width, &area_height);
  gtk_cifro_area_get_limits (carea, &min_x, &max_x, &min_y, &max_y);
  gtk_cifro_area_get_view (carea, &from_x, &to_x, &from_y, &to_y);
  gtk_cifro_area_get_swap (carea, &swap_x, &swap_y);
  angle = gtk_cifro_area_get_angle (carea);

  from_x = MAX (from_x, min_x);
  to_x = MIN (to_x, max_x);
  from_y = MAX (from_y, min_y);
  to_y = MIN (to_y, max_y);

  if (fabs (angle - G_PI / 2.0) < (G_PI / 4.0))
    swap = TRUE;

  if (swap)
    {
      from = 1.0 - (from_x - min_x) / (max_x - min_x);
      to = 1.0 - (to_x - min_x) / (max_x - min_x);
      if (!swap_x)
        {
          from = 1.0 - from;
          to = 1.0 - to;
        }
    }
  else
    {
      from = 1.0 - (from_y - min_y) / (max_y - min_y);
      to = 1.0 - (to_y - min_y) / (max_y - min_y);
      if (swap_y)
        {
          from = 1.0 - from;
          to = 1.0 - to;
        }
    }

  x = area_width - 0.75 * border_size + 0.5;
  y = border_size;
  width = border_size / 2;
  height = area_height - 2 * border_size;

  cairo_sdline_set_cairo_color (cairo, priv->axis_color);
  cairo_rectangle (cairo, x, y, width, height);
  cairo_fill (cairo);

  cairo_sdline_set_cairo_color (cairo, priv->zero_axis_color);
  cairo_rectangle (cairo, x, y + (from * height), width, ((to - from) * height));
  cairo_fill (cairo);
}

/* Рисование информационного блока. */
static void
gtk_cifro_scope_draw_info (GtkWidget *widget,
                           cairo_t   *cairo)
{
  GtkCifroArea *carea = GTK_CIFRO_AREA (widget);
  GtkCifroScope *cscope = GTK_CIFRO_SCOPE (widget);
  GtkCifroScopePrivate *priv = cscope->priv;

  GHashTableIter channels_iter;
  GtkCifroScopeChannel *channel;

  PangoLayout *font = priv->font;

  guint area_width;
  guint area_height;

  gdouble scale_x;
  gdouble scale_y;

  gdouble from_x;
  gdouble to_x;
  gdouble from_y;
  gdouble to_y;

  gint n_labels;

  gint mark_width;
  gint label_width;
  gint font_height;

  gint info_center;
  gint info_width;
  gint info_height;

  gint x1, y1;

  gint label_top;

  gdouble value;
  gdouble value_x;
  gdouble value_y;

  gint value_power;
  gchar text_format[128];
  gchar text_str[128];

  gint text_width;
  gint text_height;
  gint text_spacing;

  /* Значения под курсором. */
  if ((priv->pointer_x < 0) || (priv->pointer_y < 0))
    return;

  gtk_cifro_area_get_size (carea, &area_width, &area_height);
  gtk_cifro_area_get_scale (carea, &scale_x, &scale_y);
  gtk_cifro_area_get_view (carea, &from_x, &to_x, &from_y, &to_y);

  gtk_cifro_area_point_to_value (carea, priv->pointer_x, priv->pointer_y, &value_x, &value_y);

  text_spacing = priv->border_size / 4;

  /* Вычисляем максимальную ширину и высоту строки с текстом. */
  pango_layout_set_text (font, priv->x_axis_name, -1);
  pango_layout_get_size (font, &text_width, &text_height);
  mark_width = text_width;
  font_height = text_height;

  pango_layout_set_text (font, priv->y_axis_name, -1);
  pango_layout_get_size (font, &text_width, &text_height);
  if (text_width > mark_width)
    mark_width = text_width;
  if (text_height > font_height)
    font_height = text_height;

  n_labels = 2;
  label_width = 0;
  g_hash_table_iter_init (&channels_iter, priv->channels);
  while (g_hash_table_iter_next (&channels_iter, NULL, (gpointer) &channel))
    {
      if ((channel->value_scale == 1.0) && (channel->name == NULL) && (label_width != 0))
        continue;

      value = value_y / channel->value_scale - channel->value_shift;
      gtk_cifro_area_get_axis_step (scale_y, 1, &value, NULL, NULL, &value_power);
      if (value_power > 0)
        value_power = 0;
      g_snprintf (text_format, sizeof(text_format), "-%%.%df", (gint) fabs (value_power));
      value = MAX( ABS( from_y ), ABS( to_y ) );
      value = value / channel->value_scale - channel->value_shift;
      g_snprintf (text_str, sizeof(text_str), text_format, value);

      if (channel->name != NULL)
        {
          pango_layout_set_text (font, channel->name, -1);
          pango_layout_get_size (font, &text_width, &text_height);
          if (text_width > mark_width)
            mark_width = text_width;
          if (text_height > font_height)
            font_height = text_height;
        }

      pango_layout_set_text (font, text_str, -1);
      pango_layout_get_size (font, &text_width, &text_height);
      if (text_width > label_width)
        label_width = text_width;
      if (text_height > font_height)
        font_height = text_height;

      if ((channel->value_scale != 1.0) || (channel->name != NULL))
        n_labels += 1;
    }

  value = value_x;
  gtk_cifro_area_get_axis_step (scale_x, 1, &value, NULL, NULL, &value_power);
  if (value_power > 0)
    value_power = 0;
  g_snprintf (text_format, sizeof(text_format), "-%%.%df", (gint) fabs (value_power));
  g_snprintf (text_str, sizeof(text_str), text_format, MAX( ABS( from_x ), ABS( to_x ) ));

  pango_layout_set_text (font, text_str, -1);
  pango_layout_get_size (font, &text_width, &text_height);
  if (text_width > label_width)
    label_width = text_width;
  if (text_height > font_height)
    font_height = text_height;

  /* Ширина текста с названием величины, с её значением и высота строки. */
  mark_width /= PANGO_SCALE;
  label_width /= PANGO_SCALE;
  font_height /= PANGO_SCALE;

  /* Размер места для отображения информации. */
  info_width = 5 * text_spacing + label_width + mark_width;
  info_height = n_labels * (font_height + text_spacing) + 3 * text_spacing;
  if (n_labels > 2)
    info_height += text_spacing;

  /* Проверяем размеры области отображения. */
  if (info_width > (area_width - 12 * text_spacing))
    return;
  if (info_height > (area_height - 12 * text_spacing))
    return;

  /* Место для отображения информации. */
  if ((priv->pointer_x > (area_width - 8 * text_spacing - info_width)) &&
      (priv->pointer_y < (8 * text_spacing + info_height)))
    {
      x1 = 6 * text_spacing;
    }
  else
    {
      x1 = area_width - 6 * text_spacing - info_width;
    }
  y1 = 6 * text_spacing;

  cairo_set_line_width (cairo, 1.0);

  cairo_set_source_rgba (cairo, 0.0, 0.0, 0.0, 0.25);
  cairo_rectangle (cairo, x1 + 0.5, y1 + 0.5, info_width, info_height);
  cairo_fill (cairo);

  cairo_sdline_set_cairo_color (cairo, priv->axis_color);
  cairo_rectangle (cairo, x1 + 0.5, y1 + 0.5, info_width, info_height);
  cairo_stroke (cairo);

  cairo_sdline_set_cairo_color (cairo, priv->text_color);

  label_top = y1 + 2 * text_spacing;
  info_center = x1 + 3 * text_spacing + label_width;

  /* Значение по оси абсцисс. */
  value = value_x;
  gtk_cifro_area_get_axis_step (scale_x, 1, &value, NULL, NULL, &value_power);
  if (value_power > 0)
    value_power = 0;
  g_snprintf (text_format, sizeof(text_format), "%%.%df", (gint) fabs (value_power));
  g_ascii_formatd (text_str, sizeof(text_str), text_format, value_x);

  pango_layout_set_text (font, text_str, -1);
  pango_layout_get_size (font, &text_width, &text_height);
  cairo_move_to (cairo, info_center - text_width / PANGO_SCALE - text_spacing / 2, label_top);
  pango_cairo_show_layout (cairo, font);

  pango_layout_set_text (font, priv->x_axis_name, -1);
  cairo_move_to (cairo, info_center, label_top);
  pango_cairo_show_layout (cairo, font);
  label_top += font_height + text_spacing;

  /* Значение по оси ординат. */
  value = value_y;
  gtk_cifro_area_get_axis_step (scale_y, 1, &value, NULL, NULL, &value_power);
  if (value_power > 0)
    value_power = 0;
  g_snprintf (text_format, sizeof(text_format), "%%.%df", (gint) fabs (value_power));
  g_ascii_formatd (text_str, sizeof(text_str), text_format, value_y);

  pango_layout_set_text (font, text_str, -1);
  pango_layout_get_size (font, &text_width, &text_height);
  cairo_move_to (cairo, info_center - text_width / PANGO_SCALE - text_spacing / 2, label_top);
  pango_cairo_show_layout (cairo, font);

  pango_layout_set_text (font, priv->y_axis_name, -1);
  cairo_move_to (cairo, info_center, label_top);
  pango_cairo_show_layout (cairo, font);

  /* Значения для каналов с отличным от 1 масштабом. */
  if (n_labels > 2)
    {
      label_top += font_height + text_spacing;
      cairo_sdline_set_cairo_color (cairo, priv->axis_color);
      cairo_move_to (cairo, x1 + 4.5, label_top + 0.5);
      cairo_line_to (cairo, x1 + info_width - 7.5, label_top + 0.5);
      cairo_stroke (cairo);

      label_top += text_spacing;

      g_hash_table_iter_init (&channels_iter, priv->channels);
      while (g_hash_table_iter_next (&channels_iter, NULL, (gpointer) &channel))
        {
          if ((channel->value_scale == 1.0) && (channel->name == NULL))
            continue;

          value = (value_y - channel->value_shift) / channel->value_scale;
          gtk_cifro_area_get_axis_step (scale_y, 1, &value, NULL, NULL, &value_power);
          if (value_power > 0)
            value_power = 0;
          g_snprintf (text_format, sizeof(text_format), "%%.%df", (gint) fabs (value_power));
          g_ascii_formatd (text_str, sizeof(text_str), text_format,
                           (value_y - channel->value_shift) / channel->value_scale);

          cairo_sdline_set_cairo_color (cairo, channel->color);

          pango_layout_set_text (font, text_str, -1);
          pango_layout_get_size (font, &text_width, &text_height);
          cairo_move_to (cairo, info_center - text_width / PANGO_SCALE - text_spacing / 2, label_top);
          pango_cairo_show_layout (cairo, font);

          pango_layout_set_text (font, channel->name != NULL ? channel->name : priv->y_axis_name, -1);
          cairo_move_to (cairo, info_center, label_top);
          pango_cairo_show_layout (cairo, font);

          label_top += font_height + text_spacing;
        }
    }
}

/* Рисование координатных линий в области осциллограмм. */
static void
gtk_cifro_scope_draw_axis (GtkWidget            *widget,
                           cairo_sdline_surface *surface)
{
  GtkCifroArea *carea = GTK_CIFRO_AREA (widget);
  GtkCifroScope *cscope = GTK_CIFRO_SCOPE (widget);
  GtkCifroScopePrivate *priv = cscope->priv;

  gdouble scale_x;
  gdouble scale_y;

  gdouble from_x;
  gdouble to_x;
  gdouble from_y;
  gdouble to_y;

  gdouble step;
  gdouble axis;
  gdouble axis_pos;
  gdouble axis_step;

  gtk_cifro_area_get_scale (carea, &scale_x, &scale_y);
  gtk_cifro_area_get_view (carea, &from_x, &to_x, &from_y, &to_y);

  /* Проверяем граничные условия. */
  if ((priv->border_size == 0) || (scale_x <= 0.0) || (scale_y <= 0.0))
    return;
  if ((from_x == to_x) || (from_y == to_y))
    return;

  step = 4.0 * priv->border_size;

  /* Сетка по оси X. */
  axis = from_x;
  gtk_cifro_area_get_axis_step (scale_x, step, &axis, &axis_step, NULL, NULL);
  while (axis <= to_x)
    {
      gtk_cifro_area_visible_value_to_point (carea, &axis_pos, NULL, axis, 0.0);
      cairo_sdline_v (surface, axis_pos, 0, surface->height, priv->axis_color);
      axis += axis_step;
    }

  /* "Нулевая" ось X. */
  gtk_cifro_area_visible_value_to_point (carea, &axis_pos, NULL, 0.0, 0.0);
  cairo_sdline_v (surface, axis_pos, 0, surface->height, priv->zero_axis_color);

  /* Сетка по оси Y. */
  axis = from_y;
  gtk_cifro_area_get_axis_step (scale_y, step, &axis, &axis_step, NULL, NULL);
  while (axis <= to_y)
    {
      gtk_cifro_area_visible_value_to_point (carea, NULL, &axis_pos, 0.0, axis);
      cairo_sdline_h (surface, 0, surface->width, axis_pos, priv->axis_color);
      axis += axis_step;
    }

  /* "Нулевая" ось Y. */
  gtk_cifro_area_visible_value_to_point (carea, NULL, &axis_pos, 0.0, 0.0);
  cairo_sdline_h (surface, 0, surface->width, axis_pos, priv->zero_axis_color);

  cairo_surface_mark_dirty (surface->cairo_surface);
}

/* Функция рисования осциллограмм линиями. */
static void
gtk_cifro_scope_draw_lined_data (GtkWidget            *widget,
                                 cairo_sdline_surface *surface,
                                 gpointer              channel_id)
{
  GtkCifroArea *carea = GTK_CIFRO_AREA (widget);
  GtkCifroScope *cscope = GTK_CIFRO_SCOPE (widget);
  GtkCifroScopePrivate *priv = cscope->priv;
  GtkCifroScopeChannel *channel = g_hash_table_lookup (priv->channels, channel_id);

  guint visible_width;
  guint visible_height;

  gdouble from_x;
  gdouble to_x;
  gdouble from_y;
  gdouble to_y;

  gdouble scale_x;
  gdouble scale_y;

  gfloat *values_data;
  gint values_num;
  gfloat times_shift;
  gfloat times_step;
  gfloat values_scale;
  gfloat values_shift;
  guint32 values_color;

#define VALUES_DATA(i) ((values_data[i] * values_scale) + values_shift)

  gint i, j;
  gint i_range_begin, i_range_end;
  gfloat x_range_begin, x_range_end;
  gfloat y_start, y_end;
  gfloat x1, x2, y1, y2;
  gboolean draw = FALSE;

  /* Проверяем существование канала. */
  if (channel == NULL)
    return;

  gtk_cifro_area_get_visible_size (carea, &visible_width, &visible_height);
  gtk_cifro_area_get_scale (carea, &scale_x, &scale_y);
  gtk_cifro_area_get_view (carea, &from_x, &to_x, &from_y, &to_y);

  values_data = channel->data;
  values_num = channel->num;
  times_shift = channel->time_shift;
  times_step = channel->time_step;
  values_scale = channel->value_scale;
  values_shift = channel->value_shift;
  values_color = channel->color;

  x_range_begin = from_x - scale_x;
  x_range_end = from_x;

  x1 = -1.0;
  y1 = -1.0;
  y_start = y_end = 0.0;

  for (i = 0; i <= visible_width; i++)
    {
      draw = FALSE;
      x2 = i;

      /* Диапазон значений X между двумя точками осциллограммы. */
      x_range_begin += scale_x;
      x_range_end += scale_x;

      /* Диапазон индексов значений между двумя точками осциллограммы. */
      i_range_begin = (x_range_begin - times_shift) / times_step;
      i_range_end = (x_range_end - times_shift) / times_step;

      /* Проверка индексов на попадание в границы осциллограммы. */
      if ((i_range_begin < 0) && (i_range_end <= 0))
        continue;
      if (i_range_begin >= values_num)
        break;

      if (i_range_begin < 0)
        i_range_begin = 0;
      if (i_range_end >= values_num)
        i_range_end = values_num - 1;

      if ((i_range_end == i_range_begin) && (i != visible_width))
        continue;

      /* Последняя точка не попала в границу осциллограммы. */
      if (i_range_end == i_range_begin)
        {
          if (i_range_begin == 0)
            continue;

          if (i_range_end == (values_num - 1))
            break;

          if (isnan (values_data[i_range_begin]) || isnan (values_data[i_range_end + 1]))
            continue;

          /* Предыдущей точки нет, расчитаем значение и расстояние до нее от начала видимой осциллограммы.
             В этом случае все точки лежат за границей видимой области. */
          if (x1 < 0)
            {
              x1 = ((times_step * floor (from_x / times_step)) - from_x) / scale_x;
              y1 = (to_y - VALUES_DATA (i_range_begin)) / scale_y;
              x1 = CLAMP( x1, -32000.0, 32000.0 );
              y1 = CLAMP( y1, -32000.0, 32000.0 );
            }

          /* Значение и расстояние до текущей точки от конца видимой осциллограммы. */
          x2 = visible_width + ((times_step * ceil (to_x / times_step)) - to_x) / scale_x;
          y2 = (to_y - VALUES_DATA (i_range_end + 1)) / scale_y;
          x2 = CLAMP( x2, -32000.0, 32000.0 );
          y2 = CLAMP( y2, -32000.0, 32000.0 );
          draw = TRUE;
        }

      /* Индекс для этой точки осциллограммы изменился на 1.
         Необходимо нарисовать линию из предыдущей точки в текущую. */
      else if ((i_range_end - i_range_begin) == 1)
        {
          /* Предыдущей точки нет, расчитаем значение и расстояние до нее от текущей позиции. */
          if ((x1 < 0) && (i_range_begin >= 0))
            {
              x1 = (gfloat) i - (times_step / scale_x) + 1.0;
              y1 = (to_y - VALUES_DATA (i_range_begin)) / scale_y;
              x1 = CLAMP (x1, -32000.0, 32000.0);
              y1 = CLAMP (y1, -32000.0, 32000.0);
            }

          y2 = (to_y - VALUES_DATA (i_range_end)) / scale_y;
          y2 = CLAMP (y2, -32000.0, 32000.0);
          if (!isnan (values_data[i_range_begin]) && !isnan (values_data[i_range_end]))
            draw = TRUE;
        }

      /* В одну точку осциллограммы попадает несколько значений.
         Берем максимум и минимум из них и рисуем вертикальной линией. */
      else
        {
          /* Нарисуем линию от предыдущей точки. */
          if (!isnan (values_data[i_range_begin]) && !isnan (values_data[i_range_begin + 1]))
            {
              y1 = (to_y - VALUES_DATA (i_range_begin)) / scale_y;
              y2 = (to_y - VALUES_DATA (i_range_begin + 1)) / scale_y;
              y1 = CLAMP (y1, -32000.0, 32000.0);
              y2 = CLAMP (y2, -32000.0, 32000.0);
              cairo_sdline (surface, x2 - 1, y1, x2, y2, values_color);
              draw = TRUE;
            }

          for (j = i_range_begin + 1; j <= i_range_end; j++)
            {
              if (isnan (values_data[j]))
                continue;
              y_start = VALUES_DATA (j);
              y_end = y_start;
              draw = TRUE;
              break;
            }

          for (; j <= i_range_end; j++)
            {
              if (isnan (values_data[j]))
                continue;
              if ( VALUES_DATA (j) < y_start)
                y_start = VALUES_DATA (j);
              if ( VALUES_DATA (j) > y_end)
                y_end = VALUES_DATA (j);
              draw = TRUE;
            }

          x1 = i;
          y1 = (to_y - y_start) / scale_y;
          y2 = (to_y - y_end) / scale_y;
          y1 = CLAMP (y1, -32000.0, 32000.0);
          y2 = CLAMP (y2, -32000.0, 32000.0);
        }

      if (draw)
        cairo_sdline (surface, x1, y1, x2, y2, values_color);

      if ((i_range_end - i_range_begin) == 1)
        {
          x1 = x2;
          y1 = y2;
        }
      else
        {
          x1 = -1;
          y1 = -1;
        }
    }

  cairo_surface_mark_dirty (surface->cairo_surface);
}

/* Функция рисования осциллограмм точками. */
static void
gtk_cifro_scope_draw_dotted_data (GtkWidget            *widget,
                                  cairo_sdline_surface *surface,
                                  gpointer              channel_id,
                                  guint                 size)
{
  GtkCifroArea *carea = GTK_CIFRO_AREA (widget);
  GtkCifroScope *cscope = GTK_CIFRO_SCOPE (widget);
  GtkCifroScopePrivate *priv = cscope->priv;
  GtkCifroScopeChannel *channel = g_hash_table_lookup (priv->channels, GINT_TO_POINTER( channel_id ));

  gdouble from_x;
  gdouble to_x;
  gdouble from_y;
  gdouble to_y;

  gdouble scale_x;
  gdouble scale_y;

  gfloat *values_data;
  gint values_num;
  gfloat times_shift;
  gfloat times_step;
  gfloat values_scale;
  gfloat values_shift;
  guint32 values_color;

#define VALUES_TIME(i) ((i * times_step) + times_shift)
#define VALUES_DATA(i) ((values_data[i] * values_scale) + values_shift)

  gint i;
  gint i_range_begin, i_range_end;
  gfloat x, y;

  /* Проверяем существование канала. */
  if (channel == NULL)
    return;

  gtk_cifro_area_get_scale (carea, &scale_x, &scale_y);
  gtk_cifro_area_get_view (carea, &from_x, &to_x, &from_y, &to_y);

  values_data = channel->data;
  values_num = channel->num;
  times_shift = channel->time_shift;
  times_step = channel->time_step;
  values_scale = channel->value_scale;
  values_shift = channel->value_shift;
  values_color = channel->color;

  i_range_begin = (from_x - times_shift) / times_step;
  i_range_end = (to_x - times_shift) / times_step;

  i_range_begin = CLAMP (i_range_begin, 0, values_num);
  i_range_end = CLAMP (i_range_end, 0, values_num);

  if (i_range_begin > i_range_end)
    return;

  for (i = i_range_begin; i < i_range_end; i++)
    {
      if (isnan (values_data[i]))
        continue;
      x = VALUES_TIME (i);
      x = (x - from_x) / scale_x;
      y = VALUES_DATA (i);
      y = (to_y - y) / scale_y;
      if (size == 0)
        cairo_sdline_dot (surface, x, y, values_color);
      else
        cairo_sdline_bar (surface, x - size, y - size, x + size, y + size, values_color);
    }

  cairo_surface_mark_dirty (surface->cairo_surface);
}

/* Функция рисования осциллограмм перекрестиями. */
static void
gtk_cifro_scope_draw_crossed_data (GtkWidget            *widget,
                                   cairo_sdline_surface *surface,
                                   gpointer              channel_id,
                                   guint                 size)
{
  GtkCifroArea *carea = GTK_CIFRO_AREA (widget);
  GtkCifroScope *cscope = GTK_CIFRO_SCOPE (widget);
  GtkCifroScopePrivate *priv = cscope->priv;
  GtkCifroScopeChannel *channel = g_hash_table_lookup (priv->channels, GINT_TO_POINTER( channel_id ));

  gdouble from_x;
  gdouble to_x;
  gdouble from_y;
  gdouble to_y;

  gdouble scale_x;
  gdouble scale_y;

  gfloat *values_data;
  gint values_num;
  gfloat times_shift;
  gfloat times_step;
  gfloat values_scale;
  gfloat values_shift;
  guint32 values_color;

#define VALUES_TIME(i) ((i * times_step) + times_shift)
#define VALUES_DATA(i) ((values_data[i] * values_scale) + values_shift)

  gint i;
  gint i_range_begin, i_range_end;
  gfloat x, y;

  /* Проверяем существование канала. */
  if (channel == NULL)
    return;

  gtk_cifro_area_get_scale (carea, &scale_x, &scale_y);
  gtk_cifro_area_get_view (carea, &from_x, &to_x, &from_y, &to_y);

  values_data = channel->data;
  values_num = channel->num;
  times_shift = channel->time_shift;
  times_step = channel->time_step;
  values_scale = channel->value_scale;
  values_shift = channel->value_shift;
  values_color = channel->color;

  i_range_begin = (from_x - times_shift) / times_step;
  i_range_end = (to_x - times_shift) / times_step;

  i_range_begin = CLAMP (i_range_begin, 0, values_num);
  i_range_end = CLAMP (i_range_end, 0, values_num);

  if (i_range_begin > i_range_end)
    return;

  for (i = i_range_begin; i < i_range_end; i++)
    {
      if (isnan (values_data[i]))
        continue;
      x = VALUES_TIME (i);
      x = (x - from_x) / scale_x;
      y = VALUES_DATA (i);
      y = (to_y - y) / scale_y;
      cairo_sdline_h (surface, x - size, x + size, y, values_color);
      cairo_sdline_v (surface, x, y - size, y + size, values_color);
    }

  cairo_surface_mark_dirty (surface->cairo_surface);
}

/* Функция рисования оцифровки осей и информации. */
static void
gtk_cifro_scope_area_draw (GtkWidget *widget,
                           cairo_t   *cairo)
{
  GtkCifroScope *cscope = GTK_CIFRO_SCOPE (widget);

  gtk_cifro_scope_draw_hruler (widget, cairo);
  gtk_cifro_scope_draw_vruler (widget, cairo);
  gtk_cifro_scope_draw_x_pos (widget, cairo);
  gtk_cifro_scope_draw_y_pos (widget, cairo);
  if (cscope->priv->show_info)
    gtk_cifro_scope_draw_info (widget, cairo);
}

/* Функция рисования видимой области (осциллограмм). */
static void
gtk_cifro_scope_visible_draw (GtkWidget *widget,
                              cairo_t   *cairo)
{
  GtkCifroArea *carea = GTK_CIFRO_AREA (widget);
  GtkCifroScope *cscope = GTK_CIFRO_SCOPE (widget);
  GtkCifroScopePrivate *priv = cscope->priv;

  cairo_sdline_surface *surface;

  GHashTableIter channels_iter;
  GtkCifroScopeChannel *channel;
  gpointer channel_id;

  guint width, height;

  if (cairo == NULL)
    return;

  surface = cairo_sdline_surface_create_for (cairo_get_target (cairo));

  /* Рисуем оси. */
  gtk_cifro_scope_draw_axis (widget, surface);

  /* Рисуем осциллограммы. */
  g_hash_table_iter_init (&channels_iter, priv->channels);
  while (g_hash_table_iter_next (&channels_iter, &channel_id, (gpointer) &channel))
    {
      if (channel->show)
        {
          switch (channel->draw_type)
            {
            case GTK_CIFRO_SCOPE_DOTTED:
              gtk_cifro_scope_draw_dotted_data (widget, surface, channel_id, 0);
              break;

            case GTK_CIFRO_SCOPE_DOTTED2:
              gtk_cifro_scope_draw_dotted_data (widget, surface, channel_id, 1);
              break;

            case GTK_CIFRO_SCOPE_DOTTED_LINE:
              gtk_cifro_scope_draw_dotted_data (widget, surface, channel_id, 1);
              gtk_cifro_scope_draw_lined_data (widget, surface, channel_id);
              break;

            case GTK_CIFRO_SCOPE_CROSSED:
              gtk_cifro_scope_draw_crossed_data (widget, surface, channel_id, 3);
              break;

            case GTK_CIFRO_SCOPE_CROSSED_LINE:
              gtk_cifro_scope_draw_crossed_data (widget, surface, channel_id, 3);
              gtk_cifro_scope_draw_lined_data (widget, surface, channel_id);
              break;

            default:
              gtk_cifro_scope_draw_lined_data (widget, surface, channel_id);
            }
        }
    }

  /* Рисуем окантовку. */
  gtk_cifro_area_get_visible_size (carea, &width, &height);
  cairo_sdline_h (surface, 0, width - 1, 0, priv->border_color);
  cairo_sdline_v (surface, 0, 0, height - 1, priv->border_color);
  cairo_sdline_h (surface, 0, width - 1, height - 1, priv->border_color);
  cairo_sdline_v (surface, width - 1, 0, height - 1, priv->border_color);

  cairo_sdline_surface_destroy (surface);
}

/* Функция обработки сигнала изменения параметров дисплея. */
static gboolean
gtk_cifro_scope_configure (GtkWidget            *widget,
                           GdkEventConfigure    *event)
{
  GtkCifroScope *cscope = GTK_CIFRO_SCOPE (widget);
  GtkCifroScopePrivate *priv = cscope->priv;

  gint border;

#ifdef CIFRO_AREA_WITH_GTK2
  GtkStyle *style;
  GdkColor *color;
#else
  GdkRGBA color;
#endif

  gdouble red = 0.0;
  gdouble green = 0.0;
  gdouble blue = 0.0;
  gdouble alpha = 1.0;

  /* Текущий шрифт приложения. */
  g_clear_object (&priv->font);
  priv->font = gtk_widget_create_pango_layout (widget, NULL);

  pango_layout_set_text (priv->font, "0123456789ABCDEFGHIJKLMNOPQRSTUWXYZ.,", -1);
  pango_layout_get_size (priv->font, NULL, &border);

  /* Устанавливаем размер линеек оцифровки осей. */
  border *= 1.6;
  priv->border_size = border / PANGO_SCALE;

  /* Основной цвет темы. */
#ifdef CIFRO_AREA_WITH_GTK2
  style = gtk_rc_get_style_by_paths (gtk_settings_get_default (), NULL, NULL, GTK_TYPE_WINDOW);
  if (style)
    {
      color = &style->fg[GTK_STATE_NORMAL];
      red = color->red / 65535.0;
      green = color->green / 65535.0;
      blue = color->blue / 65535.0;
    }
#else
  gtk_style_context_get_color (gtk_widget_get_style_context (widget), GTK_STATE_FLAG_NORMAL, &color);
  red = color.red;
  green = color.green;
  blue = color.blue;
  alpha = color.alpha;
#endif

  gtk_cifro_scope_set_fg_color (priv, red, green, blue, alpha);

  return FALSE;
}

/* Функция обработки сигнала движения мышки. */
static gboolean
gtk_cifro_scope_motion_notify (GtkWidget            *widget,
                               GdkEventMotion       *event)
{
  GtkCifroArea *carea = GTK_CIFRO_AREA (widget);
  GtkCifroScope *cscope = GTK_CIFRO_SCOPE (widget);
  GtkCifroScopePrivate *priv = cscope->priv;
  guint border_size = priv->border_size;

  guint area_width;
  guint area_height;

  gint x = event->x;
  gint y = event->y;

  /* Проверяем, что координаты курсора находятся в рабочей области. */
  gtk_cifro_area_get_size (carea, &area_width, &area_height);
  if (x < border_size)
    {
      x = -1;
      y = -1;
    }
  if (y < border_size)
    {
      x = -1;
      y = -1;
    }
  if (x > area_width - border_size)
    {
      x = -1;
      y = -1;
    }
  if (y > area_height - border_size)
    {
      x = -1;
      y = -1;
    }

  /* Запоминаем координаты, устанавливаем флаг перерисовки. */
  priv->pointer_x = x;
  priv->pointer_y = y;

  gtk_widget_queue_draw (widget);

  return FALSE;
}

/* Функция обработки сигнала выхода курсора за пределы окна. */
static gboolean
gtk_cifro_scope_leave_notify (GtkWidget            *widget,
                              GdkEventCrossing     *event)
{
  GtkCifroScope *cscope = GTK_CIFRO_SCOPE (widget);
  GtkCifroScopePrivate *priv = cscope->priv;

  priv->pointer_x = -1;
  priv->pointer_y = -1;

  gtk_widget_queue_draw (widget);

  return FALSE;
}

/**
 * gtk_cifro_scope_new:
 * @gravity: ориентация осей осциллографа
 *
 * Функция создаёт виджет #GtkCifroScope.
 *
 * Returns: #GtkCifroScope.
 *
 */
GtkWidget *
gtk_cifro_scope_new (GtkCifroScopeGravity gravity)
{
  return g_object_new (GTK_TYPE_CIFRO_SCOPE, "gravity", gravity, NULL);
}

/**
 * gtk_cifro_scope_set_limits:
 * @cscope: указатель на #GtkCifroScope
 * @min_x: минимально возможное значение по оси X
 * @max_x: максимально возможное значение по оси X
 * @min_y: минимально возможное значение по оси Y
 * @max_y: максимально возможное значение по оси Y
 *
 * Функция устанавливает диапазон значений отображаемых данных.
 *
 */
void
gtk_cifro_scope_set_limits (GtkCifroScope *cscope,
                            gdouble        min_x,
                            gdouble        max_x,
                            gdouble        min_y,
                            gdouble        max_y)
{
  g_return_if_fail (GTK_IS_CIFRO_SCOPE (cscope));

  g_return_if_fail (min_x < max_x);
  g_return_if_fail (min_y < max_y);

  cscope->priv->min_x = min_x;
  cscope->priv->max_x = max_x;
  cscope->priv->min_y = min_y;
  cscope->priv->max_y = max_y;

  gtk_widget_queue_draw (GTK_WIDGET (cscope));
}

/**
 * gtk_cifro_scope_set_info_show:
 * @cscope: указатель на #GtkCifroScope
 * @show: признак отображения информационного блока
 *
 * Функция включает (@show = %TRUE) или выключает (@show = %FALSE) отображение
 * блока с информацией о значениях под курсором.
 *
 */
void
gtk_cifro_scope_set_info_show (GtkCifroScope *cscope,
                               gboolean       show)
{
  g_return_if_fail (GTK_IS_CIFRO_SCOPE (cscope));

  cscope->priv->show_info = show;

  gtk_widget_queue_draw (GTK_WIDGET (cscope));
}

/**
 * gtk_cifro_scope_set_axis_name:
 * @cscope: указатель на #GtkCifroScope
 * @time_axis_name: название оси времени (абсцисса)
 * @value_axis_name: название оси данных (ордината)
 *
 * Функция задаёт названия к осям абсцисс и ординат.
 *
 */
void
gtk_cifro_scope_set_axis_name (GtkCifroScope *cscope,
                               const gchar   *time_axis_name,
                               const gchar   *value_axis_name)
{
  GtkCifroScopePrivate *priv;

  g_return_if_fail (GTK_IS_CIFRO_SCOPE (cscope));

  priv = cscope->priv;

  g_free (priv->x_axis_name);
  g_free (priv->y_axis_name);

  priv->x_axis_name = g_strdup (time_axis_name);
  priv->y_axis_name = g_strdup (value_axis_name);

  gtk_widget_queue_draw (GTK_WIDGET (cscope));
}

/**
 * gtk_cifro_scope_add_channel:
 * @cscope: указатель на #GtkCifroScope
 *
 * Функция дабавляет канал отображения данных в осциллограф.
 *
 * Returns: Идентификатор нового канала.
 *
 */
guint
gtk_cifro_scope_add_channel (GtkCifroScope *cscope)
{
  GtkCifroScopePrivate *priv;
  GtkCifroScopeChannel* channel;
  guint channel_id = 0;

  g_return_val_if_fail (GTK_IS_CIFRO_SCOPE (cscope), 0);

  priv = cscope->priv;

  /* Параметры канала по умолчанию. */
  channel = g_new0 (GtkCifroScopeChannel, 1);
  channel->time_step = 1.0;
  channel->value_scale = 1.0;
  channel->draw_type = GTK_CIFRO_SCOPE_LINED;
  channel->color = cairo_sdline_color (g_random_double_range (0.5, 1.0),
                                       g_random_double_range (0.5, 1.0),
                                       g_random_double_range (0.5, 1.0),
                                       1.0);

  /* Генерируем новый идентификатор канала. */
  while ((channel_id == 0) || g_hash_table_contains (priv->channels, GUINT_TO_POINTER (channel_id)))
    channel_id = g_random_int ();

  g_hash_table_insert (priv->channels, GUINT_TO_POINTER (channel_id), channel);

  return channel_id;
}

/**
 * gtk_cifro_scope_remove_channel:
 * @cscope: указатель на #GtkCifroScope
 * @channel_id: идентификатор канала данных
 *
 * Функция удаляет канал отображения данных из осциллографа.
 *
 */
void
gtk_cifro_scope_remove_channel (GtkCifroScope *cscope,
                                guint          channel_id)
{
  g_return_if_fail (GTK_IS_CIFRO_SCOPE (cscope));

  g_hash_table_remove (cscope->priv->channels, GUINT_TO_POINTER (channel_id));

  gtk_widget_queue_draw (GTK_WIDGET (cscope));
}

/**
 * gtk_cifro_scope_set_channel_name:
 * @cscope: указатель на #GtkCifroScope
 * @channel_id: идентификатор канала данных
 * @axis_name: название канала
 *
 * Функция устанавливает название канала. Если идентификатор канала равен 0 название
 * устанавливается для всех каналов.
 *
 */
void
gtk_cifro_scope_set_channel_name (GtkCifroScope *cscope,
                                  guint          channel_id,
                                  const gchar   *axis_name)
{
  GHashTableIter channels_iter;
  GtkCifroScopeChannel *channel;
  guint cur_channel_id;

  g_return_if_fail (GTK_IS_CIFRO_SCOPE (cscope));

  g_hash_table_iter_init (&channels_iter, cscope->priv->channels);
  while (g_hash_table_iter_next (&channels_iter, (gpointer) &cur_channel_id, (gpointer) &channel))
    if ((channel_id == 0) || (cur_channel_id == channel_id))
      {
        g_free (channel->name);
        channel->name = g_strdup (axis_name);
      }

  gtk_widget_queue_draw (GTK_WIDGET (cscope));
}

/**
 * gtk_cifro_scope_set_channel_time_param:
 * @cscope: указатель на #GtkCifroScope
 * @channel_id: идентификатор канала данных
 * @time_shift: начальный момент времени
 * @time_step: шаг смещения по оси времени
 *
 * Функция устанавливает с какого момента времени следует отображать данные и
 * какой шаг между двумя соседними данными (частота оцифровки). Параметры задаются
 * индивидуально для каждого канала. Если идентификатор канала равен 0 параметры
 * устанавливаются для всех каналов.
 *
 */
void
gtk_cifro_scope_set_channel_time_param (GtkCifroScope *cscope,
                                        guint          channel_id,
                                        gfloat         time_shift,
                                        gfloat         time_step)
{
  GHashTableIter channels_iter;
  GtkCifroScopeChannel *channel;
  guint cur_channel_id;

  g_return_if_fail (GTK_IS_CIFRO_SCOPE (cscope));

  g_hash_table_iter_init (&channels_iter, cscope->priv->channels);
  while (g_hash_table_iter_next (&channels_iter, (gpointer) &cur_channel_id, (gpointer) &channel))
    if ((channel_id == 0) || (cur_channel_id == channel_id))
      {
        channel->time_shift = time_shift;
        channel->time_step = time_step;
      }

  gtk_widget_queue_draw (GTK_WIDGET (cscope));
}

/**
 * gtk_cifro_scope_set_channel_value_param:
 * @cscope: указатель на #GtkCifroScope
 * @channel_id: идентификатор канала данных
 * @value_shift: коэффициент смещения данных
 * @value_scale: коэффициент умножения данных
 *
 * Функция устанавливает коэффициенты на которые умножаются и сдвигаются все данные в
 * канале. Это позволяет отображать разнородные данные в одном пространстве и наглядно
 * сравнивать их друг с другом. Коэффициенты задаются индивидуально для каждого канала.
 * Если идентификатор канала равен 0 коэффициенты устанавливаются для всех каналов.
 *
 */
void
gtk_cifro_scope_set_channel_value_param (GtkCifroScope *cscope,
                                         guint          channel_id,
                                         gfloat         value_shift,
                                         gfloat         value_scale)
{
  GHashTableIter channels_iter;
  GtkCifroScopeChannel *channel;
  guint cur_channel_id;

  g_return_if_fail (GTK_IS_CIFRO_SCOPE (cscope));

  g_hash_table_iter_init (&channels_iter, cscope->priv->channels);
  while (g_hash_table_iter_next (&channels_iter, (gpointer) &cur_channel_id, (gpointer) &channel))
    if ((channel_id == 0) || (cur_channel_id == channel_id))
      {
        channel->value_shift = value_shift;
        channel->value_scale = value_scale;
      }

  gtk_widget_queue_draw (GTK_WIDGET (cscope));
}

/**
 * gtk_cifro_scope_set_channel_draw_type:
 * @cscope: указатель на #GtkCifroScope
 * @channel_id: идентификатор канала данных
 * @draw_type: тип отображения осциллограмм
 *
 * Функция устанавливает типа отображения осциллограмм. Если идентификатор канала равен 0
 * тип отображения устанавливается для всех каналов.
 *
 */
void
gtk_cifro_scope_set_channel_draw_type (GtkCifroScope         *cscope,
                                       guint                  channel_id,
                                       GtkCifroScopeDrawType  draw_type)
{
  GHashTableIter channels_iter;
  GtkCifroScopeChannel *channel;
  guint cur_channel_id;

  g_return_if_fail (GTK_IS_CIFRO_SCOPE (cscope));

  g_hash_table_iter_init (&channels_iter, cscope->priv->channels);
  while (g_hash_table_iter_next (&channels_iter, (gpointer) &cur_channel_id, (gpointer) &channel))
    if ((channel_id == 0) || (cur_channel_id == channel_id))
      channel->draw_type = draw_type;

  gtk_widget_queue_draw (GTK_WIDGET (cscope));
}

/**
 * gtk_cifro_scope_set_channel_color:
 * @cscope: указатель на #GtkCifroScope
 * @channel_id: идентификатор канала данных
 * @red: значение красной составляющей цвета от 0 до 1
 * @green: значение зелёной составляющей цвета от 0 до 1
 * @blue: значение синей составляющей цвета от 0 до 1
 *
 * Функция устанавливает цвет отображения данных канала. Если идентификатор канала равен 0
 * цвет устанавливается для всех каналов.
 *
 */
void
gtk_cifro_scope_set_channel_color (GtkCifroScope *cscope,
                                   guint          channel_id,
                                   gdouble        red,
                                   gdouble        green,
                                   gdouble        blue)
{
  GHashTableIter channels_iter;
  GtkCifroScopeChannel *channel;
  guint cur_channel_id;

  g_return_if_fail (GTK_IS_CIFRO_SCOPE (cscope));

  g_hash_table_iter_init (&channels_iter, cscope->priv->channels);
  while (g_hash_table_iter_next (&channels_iter, (gpointer) &cur_channel_id, (gpointer) &channel))
    if ((channel_id == 0) || (cur_channel_id == channel_id))
      channel->color = cairo_sdline_color (red, green, blue, 1.0);

  gtk_widget_queue_draw (GTK_WIDGET (cscope));
}

/**
 * gtk_cifro_scope_set_channel_data:
 * @cscope: указатель на #GtkCifroScope
 * @channel_id: идентификатор канала данных
 * @n_values: число значений для отображения
 * @values: указатель на массив данных для отображения
 *
 * Функция устанавливает данные канала для отображения.
 *
 */
void
gtk_cifro_scope_set_channel_data (GtkCifroScope *cscope,
                                  guint          channel_id,
                                  guint          n_values,
                                  gfloat        *values)
{
  GtkCifroScopeChannel* channel;

  g_return_if_fail (GTK_IS_CIFRO_SCOPE (cscope));

  channel = g_hash_table_lookup (cscope->priv->channels, GUINT_TO_POINTER (channel_id));
  if (channel == NULL)
    return;

  if (n_values > channel->size)
    {
      channel->data = g_renew (float, channel->data, n_values);
      channel->size = n_values;
    }

  channel->num = n_values;
  if (n_values > 0)
    {
      memcpy (channel->data, values, n_values * sizeof(gfloat));
      channel->show = TRUE;
    }
}

/**
 * gtk_cifro_scope_set_channel_show:
 * @cscope: указатель на #GtkCifroScope
 * @channel_id: идентификатор канала данных
 * @show: признак отображения канала данных
 *
 * Функция включает (@show = %TRUE) или выключает (@show = %FALSE) отображения данных канала.
 * Если идентификатор канала равен 0 отображение устанавливается для всех каналов.
 *
 */
void
gtk_cifro_scope_set_channel_show (GtkCifroScope *cscope,
                                  guint          channel_id,
                                  gboolean       show)
{
  GHashTableIter channels_iter;
  GtkCifroScopeChannel *channel;
  guint cur_channel_id;

  g_return_if_fail (GTK_IS_CIFRO_SCOPE (cscope));

  g_hash_table_iter_init (&channels_iter, cscope->priv->channels);
  while (g_hash_table_iter_next (&channels_iter, (gpointer) &cur_channel_id, (gpointer) &channel))
    if ((channel_id == 0) || (cur_channel_id == channel_id))
      channel->show = show;

  gtk_widget_queue_draw (GTK_WIDGET (cscope));
}
