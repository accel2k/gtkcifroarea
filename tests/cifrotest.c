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

#include <gtk-cifro-curve.h>
#include <math.h>

#define MAX_N_CHANNELS 8                               /* Максимальное число каналов, от него зависит список цветов. */

static gboolean        quit = FALSE;

static gchar          *draw_type = "lined";            /* Тип отображения данных. */
static guint           n_channels = 4;                 /* Число каналов осциллографа. */
static guint           n_points = 1000;                /* Число точек осциллограммы. */

static gdouble         frequency = 10.0;               /* Частота сигнала, Гц. */
static gdouble         max_time = 1000.0;              /* Максимальное время отображения, мс. */
static gdouble         max_range = 1.0;                /* Максимальный размах амплитуды, В. */

static guint           channels[MAX_N_CHANNELS];
static gfloat         *data[MAX_N_CHANNELS];

/* Цвета каналов.*/
static gchar *channel_colors[MAX_N_CHANNELS + 2] =
{
  "red",
  "green",
  "blue",
  "orange",
  "magenta",
  "lime",
  "cyan",
  "goldenrod",
  "purple",
  NULL
};

typedef struct
{
  gdouble a, b, c, d, x;
} spline_tuple;

gdouble
curve_func_akima (gdouble   param,
                  GArray   *points,
                  gpointer  curve_data)
{
  guint i, j, k, n;

  GtkCifroCurvePoint *point;
  GtkCifroCurvePoint *point2;

  spline_tuple *spline;
  spline_tuple *splines;
  gfloat *alpha, *beta, dx, dy, y;

  if (points->len < 2)
    return param;

  if (points->len == 2)
    {
      point = &g_array_index (points, GtkCifroCurvePoint, 0);
      point2 = &g_array_index (points, GtkCifroCurvePoint, 1);
      dy = (point2->y - point->y) / (point2->x - point->x);
      y = param * dy + (point->y - point->x * dy);
      return y;
    }

  n = points->len;
  splines = g_malloc0 (n * sizeof(spline_tuple));
  alpha = g_malloc0 ((n - 1) * sizeof(gfloat));
  beta = g_malloc0 ((n - 1) * sizeof(gfloat));

  for (i = 0; i < n; i++)
    {
      point = &g_array_index (points, GtkCifroCurvePoint, i);
      splines[i].x = point->x;
      splines[i].a = point->y;
    }
  splines[0].c = splines[n - 1].c = 0.0;

  alpha[0] = beta[0] = 0.;
  for (i = 1; i < n - 1; i++)
    {
      gfloat h_i = splines[i].x - splines[i - 1].x;
      gfloat h_i1 = splines[i + 1].x - splines[i].x;
      gfloat A = h_i;
      gfloat B = h_i1;
      gfloat C = 2.0 * (h_i + h_i1);
      gfloat F = 6.0 * ((splines[i + 1].a - splines[i].a) / h_i1 - (splines[i].a - splines[i - 1].a) / h_i);
      gfloat z = A * alpha[i - 1] + C;
      alpha[i] = -B / z;
      beta[i] = (F - A * beta[i - 1]) / z;
    }

  for (i = n - 2; i > 0; i--)
    splines[i].c = alpha[i] * splines[i + 1].c + beta[i];

  for (i = n - 1; i > 0; i--)
    {
      gfloat h_i = splines[i].x - splines[i - 1].x;
      splines[i].d = (splines[i].c - splines[i - 1].c) / h_i;
      splines[i].b = h_i * (2.0 * splines[i].c + splines[i - 1].c) / 6.0 + (splines[i].a - splines[i - 1].a) / h_i;
    }

  if (param <= splines[0].x)
    spline = splines + 1;
  else if (param >= splines[n - 1].x)
    spline = splines + n - 1;
  else
    {
      i = 0;
      j = n - 1;
      while (i + 1 < j)
        {
          k = i + (j - i) / 2;
          if (param <= splines[k].x)
            j = k;
          else
            i = k;
        }
      spline = splines + j;
    }

  dx = (param - spline->x);
  y = spline->a + (spline->b + (spline->c / 2.0 + spline->d * dx / 6.0) * dx) * dx;

  g_free (splines);
  g_free (alpha);
  g_free (beta);

  return y;
}

void
destroy_callback (GtkWidget *widget,
                  gpointer   user_data)
{
  GArray *points;
  GtkCifroCurvePoint *point;
  guint i;

  points = gtk_cifro_curve_get_points (GTK_CIFRO_CURVE( user_data ));

  g_message( "Control points: %d", points->len);
  for (i = 0; i < points->len; i++)
    {
      point = &g_array_index( points, GtkCifroCurvePoint, i );
      g_message( "%3d: %3.6lf %3.6lf", i, point->x, point->y);
    }
  g_array_unref (points);

  for (i = 0; i < n_channels; i++)
    g_free (data[i]);

  quit = TRUE;
  gtk_main_quit ();
}

gboolean
update_data (gpointer user_data)
{
  GtkCifroScope *cscope = user_data;

  static guint i, j, k;

  if (quit)
    return FALSE;

  /* Выделяем память для данных. */
  for (k = 0; k < n_channels; k++)
    if (data[k] == NULL)
      data[k] = g_new0( gfloat, n_points );

  for (k = 0; k < n_channels - 1; k++)
    {
      for (i = 0; i < n_points; i++)
        {
          gdouble t = ((gdouble) i / (gdouble) (n_points - 1)) * (max_time / 1000.0);
          gdouble dt = ((gdouble) k / (gdouble) (n_channels - 1))
                       + ((gdouble) (j * (n_channels - k - 1)) / (4.0 * max_time));
          gdouble df = 0.25 * ((gdouble) k / (gdouble) (n_channels - 1)) * frequency;
          gdouble ph = 2.0 * G_PI * (frequency - df) * (t - dt);
          data[k][i] = sin (ph);
        }

      if (k == 0)
        {
          for (i = n_points / 3; i < 2 * n_points / 3; i++)
            if (i % (n_points / 32) > (n_points / 64))
              data[0][i] = NAN;
          data[0][j % n_points] = 10.0;
        }

      gtk_cifro_scope_set_channel_data (cscope, channels[k], n_points, data[k]);
    }

  for (i = 0; i < n_points; i++)
    data[k][i] = g_random_double_range (-1.0, 1.0);

  gtk_cifro_scope_set_channel_data (cscope, channels[k], n_points, data[k]);

  j++;

  gtk_widget_queue_draw (GTK_WIDGET (cscope));

  return TRUE;
}

int
main (int    argc,
      char **argv)
{
  GtkWidget *window;
  GtkWidget *area;

  guint i;

  gtk_init (&argc, &argv);

  /* Разбор командной строки. */
  {
    GError *error = NULL;
    GOptionContext *context;
    GOptionEntry entries[] =
      {
        { "draw-type", 'd', 0, G_OPTION_ARG_STRING, &draw_type, "Osciloscope data draw type (lined, dotted, dotted2, dotted-line, crossed, crossed-line)", NULL },
        { "channels", 'c', 0, G_OPTION_ARG_INT, &n_channels, "Number of osciloscope channels", NULL },
        { "points", 'n', 0, G_OPTION_ARG_INT, &n_points, "Number of points per channel", NULL },
        { "time", 't', 0, G_OPTION_ARG_DOUBLE, &max_time, "Maximum sampling time, ms", NULL },
        { "range", 'r', 0, G_OPTION_ARG_DOUBLE, &max_range, "Maximum signal range, V", NULL },
        { NULL }
      };

    context = g_option_context_new ("");
    g_option_context_set_help_enabled (context, TRUE);
    g_option_context_add_main_entries (context, entries, NULL);
    g_option_context_set_ignore_unknown_options (context, FALSE);

    if (!g_option_context_parse (context, &argc, &argv, &error))
      {
        g_message( error->message);
        return -1;
      }

    g_option_context_free (context);
  }

  /* Ограничения по числу каналов. */
  if (n_channels < 2)
    n_channels = 2;
  if (n_channels > MAX_N_CHANNELS)
    n_channels = MAX_N_CHANNELS;

  /* Ограничения по числу точек в канале. */
  if (n_points < 32)
    n_points = 32;
  if (n_points > 100000)
    n_points = 100000;

  /* Основное окно программы. */
  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title ( GTK_WINDOW (window), "CifroScope");
  gtk_window_set_default_size (GTK_WINDOW (window), 800, 400);

  /* Осциллограф. */
  area = gtk_cifro_curve_new (GTK_CIFRO_SCOPE_GRAVITY_RIGHT_UP, curve_func_akima, NULL);

  /* Цвета каналов и кривой. */
  for (i = 0; i <= n_channels; i++)
    {
      gdouble red, green, blue;

#ifdef CIFRO_AREA_WITH_GTK2
      GdkColor color;
      gdk_color_parse (channel_colors[i], &color);
      red = color.red / 65535.0;
      green = color.green / 65535.0;
      blue = color.blue / 65535.0;
#else
      GdkRGBA color;
      gdk_rgba_parse (&color, channel_colors[i]);
      red = color.red;
      green = color.green;
      blue = color.blue;
#endif

      channels[i] = gtk_cifro_scope_add_channel (GTK_CIFRO_SCOPE (area));

      if (i == n_channels)
        {
          gtk_cifro_curve_set_curve_color (GTK_CIFRO_CURVE (area), red, green, blue);
          gtk_cifro_curve_set_point_color (GTK_CIFRO_CURVE (area), red, green, blue);
        }
      else
        {
          gtk_cifro_scope_set_channel_color (GTK_CIFRO_SCOPE (area), channels[i], red, green, blue);
          gtk_cifro_scope_set_channel_name (GTK_CIFRO_SCOPE (area), channels[i], channel_colors[i]);
        }
    }

  /* Параметры осциллографа. */
  gtk_cifro_area_set_view (GTK_CIFRO_AREA (area), 0.0, 1000.0, -1.0, 1.0);
  gtk_cifro_area_control_set_scroll_mode (GTK_CIFRO_AREA_CONTROL (area), GTK_CIFRO_AREA_SCROLL_MODE_COMBINED);
  gtk_cifro_area_control_set_scroll_mode (GTK_CIFRO_AREA_CONTROL (area), GTK_CIFRO_AREA_SCROLL_MODE_ZOOM);
  gtk_cifro_area_control_set_move_step (GTK_CIFRO_AREA_CONTROL (area), 20);
  gtk_cifro_area_set_scale_on_resize (GTK_CIFRO_AREA (area), FALSE);
  gtk_cifro_scope_set_limits (GTK_CIFRO_SCOPE (area),
                              -(max_time / 100.0), max_time + max_time / 100.0,
                              -(max_range + max_range / 10.0), max_range + max_range / 10.0);
  gtk_cifro_scope_set_axis_name (GTK_CIFRO_SCOPE (area), "ms", "V");
  gtk_cifro_scope_set_channel_time_param (GTK_CIFRO_SCOPE (area), 0, 0.0, max_time / (n_points - 1));
  gtk_cifro_scope_set_info_show (GTK_CIFRO_SCOPE (area), TRUE);

  if (g_strcmp0 (draw_type, "dotted") == 0)
    gtk_cifro_scope_set_channel_draw_type (GTK_CIFRO_SCOPE (area), 0, GTK_CIFRO_SCOPE_DOTTED);
  else if (g_strcmp0 (draw_type, "dotted2") == 0)
    gtk_cifro_scope_set_channel_draw_type (GTK_CIFRO_SCOPE (area), 0, GTK_CIFRO_SCOPE_DOTTED2);
  else if (g_strcmp0 (draw_type, "dotted-line") == 0)
    gtk_cifro_scope_set_channel_draw_type (GTK_CIFRO_SCOPE (area), 0, GTK_CIFRO_SCOPE_DOTTED_LINE);
  else if (g_strcmp0 (draw_type, "crossed") == 0)
    gtk_cifro_scope_set_channel_draw_type (GTK_CIFRO_SCOPE (area), 0, GTK_CIFRO_SCOPE_CROSSED);
  else if (g_strcmp0 (draw_type, "crossed-line") == 0)
    gtk_cifro_scope_set_channel_draw_type (GTK_CIFRO_SCOPE (area), 0, GTK_CIFRO_SCOPE_CROSSED_LINE);

  gtk_cifro_curve_add_point (GTK_CIFRO_CURVE (area), 0.0, -max_range);
  gtk_cifro_curve_add_point (GTK_CIFRO_CURVE (area), max_time / 2.0, 0.0);
  gtk_cifro_curve_add_point (GTK_CIFRO_CURVE (area), max_time - 1.0, max_range);

  /* Масштабы каналов. */
  for (i = 0; i < n_channels - 1; i++)
    {
      gtk_cifro_scope_set_channel_value_param (GTK_CIFRO_SCOPE (area),
                                               channels[i],
                                               0.0,
                                               0.25 + 0.75 * ((gdouble) (i + 1) / (gdouble) (n_channels - 1)));
    }
  gtk_cifro_scope_set_channel_value_param (GTK_CIFRO_SCOPE (area), channels[i], 0.0, 0.05);

  gtk_container_add (GTK_CONTAINER (window), GTK_WIDGET (area));
  gtk_widget_show_all (window);

  gtk_cifro_area_set_view (GTK_CIFRO_AREA (area), 0.0, max_time, -max_range, max_range);

  g_signal_connect( G_OBJECT (window), "destroy", G_CALLBACK (destroy_callback), area);
  g_timeout_add (100, update_data, area);

  gtk_main ();

  return 0;
}
