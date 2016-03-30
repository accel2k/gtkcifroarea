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
 * \file gtk-cifro-curve.c
 *
 * \brief Исходный файл GTK+ виджета осциллографа совмещённого с параметрической кривой
 * \author Andrei Fadeev
 * \date 2013-2016
 * \license GNU General Public License version 3 или более поздняя<br>
 * Коммерческая лицензия - свяжитесь с автором
 *
 */

#include "gtk-cifro-curve.h"
#include "gtk-cifro-curve-renderer.h"


enum
{
  PROP_0,
  PROP_CURVE_FUNC,
  PROP_CURVE_DATA
};

struct _GtkCifroCurvePrivate
{
  GtkCifroCurveFunc            curve_func;                     /* Функция расчёта значений кривой. */
  gpointer                     curve_data;                     /* Пользовательские данные для функции расчёта значений кривой. */

  GtkCifroCurveRenderer       *curve_renderer;                 /* Объект рисования кривой. */

  GtkCifroAreaState           *state;                          /* Объект хранения состояния GtkCifroArea. */
};

static void            gtk_cifro_curve_set_property            (GObject               *ccurve,
                                                                guint                  prop_id,
                                                                const GValue          *value,
                                                                GParamSpec            *pspec);
static void            gtk_cifro_curve_object_constructed      (GObject               *object);
static void            gtk_cifro_curve_object_finalize         (GObject               *object);

G_DEFINE_TYPE_WITH_PRIVATE (GtkCifroCurve, gtk_cifro_curve, GTK_TYPE_CIFRO_SCOPE)

static void
gtk_cifro_curve_init (GtkCifroCurve *ccurve)
{
  ccurve->priv = gtk_cifro_curve_get_instance_private (ccurve);
}

static void
gtk_cifro_curve_class_init (GtkCifroCurveClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->set_property = gtk_cifro_curve_set_property;

  object_class->constructed = gtk_cifro_curve_object_constructed;
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
gtk_cifro_curve_object_constructed (GObject *object)
{
  GtkCifroCurve *ccurve = GTK_CIFRO_CURVE (object);
  GtkCifroCurvePrivate *priv = ccurve->priv;

  G_OBJECT_CLASS (gtk_cifro_curve_parent_class)->constructed (object);

  /* Объект рисования осциллографа. */
  priv->curve_renderer = gtk_cifro_curve_renderer_new (GTK_CIFRO_AREA (ccurve),
                                                                 priv->curve_func,
                                                                 priv->curve_data);

  /* Объект состояния GtkCifroArea. */
  priv->state = gtk_cifro_area_get_state (GTK_CIFRO_AREA (ccurve));

  /* Обработчики сигналов от мышки. */
  g_signal_connect (ccurve, "button-press-event",
                    G_CALLBACK( gtk_cifro_curve_renderer_button_press_event ), priv->curve_renderer);
  g_signal_connect (ccurve, "button-release-event",
                    G_CALLBACK( gtk_cifro_curve_renderer_button_release_event ), priv->curve_renderer);
  g_signal_connect (ccurve, "motion-notify-event",
                    G_CALLBACK( gtk_cifro_curve_renderer_motion_notify_event ), priv->curve_renderer);

  /* По умолчанию отключаем отображение информационного блока. */
  gtk_cifro_scope_set_info_show (GTK_CIFRO_SCOPE( ccurve ), FALSE);
}

static void
gtk_cifro_curve_object_finalize (GObject *object)
{
  GtkCifroCurve *ccurve = GTK_CIFRO_CURVE (object);

  g_object_unref (ccurve->priv->curve_renderer);

  G_OBJECT_CLASS( gtk_cifro_curve_parent_class )->finalize (object);
}

/* Функция создаёт виджет GtkCifroCurve. */
GtkWidget *
gtk_cifro_curve_new (GtkCifroCurveFunc curve_func,
                     gpointer          curve_data)
{
  return g_object_new (GTK_TYPE_CIFRO_CURVE, "curve-func", curve_func, "curve-data", curve_data, NULL);
}

/* Функция удаляет все контрольные точки. */
void
gtk_cifro_curve_clear_points (GtkCifroCurve *ccurve)
{
  g_return_if_fail (GTK_IS_CIFRO_CURVE (ccurve));

  gtk_cifro_curve_renderer_clear_points (ccurve->priv->curve_renderer);

  gtk_cifro_area_update (GTK_CIFRO_AREA (ccurve));
}

/* Функция добавляет одну контрольную точку к существующим. */
void
gtk_cifro_curve_add_point (GtkCifroCurve *ccurve,
                           gdouble        x,
                           gdouble        y)
{
  g_return_if_fail (GTK_IS_CIFRO_CURVE (ccurve));

  gtk_cifro_curve_renderer_add_point (ccurve->priv->curve_renderer, x, y);

  gtk_cifro_area_update (GTK_CIFRO_AREA (ccurve));
}

/* Функция устанавливает новые контрольные точки взамен текущих. */
void
gtk_cifro_curve_set_points (GtkCifroCurve *ccurve,
                            GArray        *points)
{
  g_return_if_fail (GTK_IS_CIFRO_CURVE (ccurve));

  gtk_cifro_curve_renderer_set_points (ccurve->priv->curve_renderer, points);

  gtk_cifro_area_update (GTK_CIFRO_AREA (ccurve));
}

/* Функция возвращает массив текущих контрольных точек. */
GArray *
gtk_cifro_curve_get_points (GtkCifroCurve *ccurve)
{
  g_return_val_if_fail (GTK_IS_CIFRO_CURVE (ccurve), NULL);

  return gtk_cifro_curve_renderer_get_points (ccurve->priv->curve_renderer);
}

/* Функция устанавливает цвет кривой. */
void
gtk_cifro_curve_set_curve_color (GtkCifroCurve *ccurve,
                                 gdouble        red,
                                 gdouble        green,
                                 gdouble        blue)
{
  g_return_if_fail (GTK_IS_CIFRO_CURVE (ccurve));

  gtk_cifro_curve_renderer_set_curve_color (ccurve->priv->curve_renderer, red, green, blue);

  gtk_cifro_area_update (GTK_CIFRO_AREA (ccurve));
}

/* Функция устанавливает цвет контрольных точек кривой. */
void
gtk_cifro_curve_set_point_color (GtkCifroCurve *ccurve,
                                 gdouble        red,
                                 gdouble        green,
                                 gdouble        blue)
{
  g_return_if_fail (GTK_IS_CIFRO_CURVE (ccurve));

  gtk_cifro_curve_renderer_set_point_color (ccurve->priv->curve_renderer, red, green, blue);

  gtk_cifro_area_update (GTK_CIFRO_AREA (ccurve));
}
