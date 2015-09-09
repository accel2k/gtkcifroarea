/*
 * GtkCifroArea - 2D layers image management library.
 *
 * Copyright 2013-2015 Andrei Fadeev (andrei@webcontrol.ru)
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

/*!
 * \file gtk-cifro-curve.c
 *
 * \brief Исходный файл GTK+ виджета осциллографа совмещённого с параметрической кривой
 * \author Andrei Fadeev
 * \date 2013-2015
 * \license GNU General Public License version 3 или более поздняя<br>
 * Коммерческая лицензия - свяжитесь с автором
 *
*/

#include "gtk-cifro-curve.h"
#include "gtk-cifro-curve-renderer.h"


enum { PROP_0, PROP_CURVE_FUNC, PROP_CURVE_DATA };


typedef struct GtkCifroCurvePriv {

  GtkCifroCurveFunc          curve_func;         // Функция расчёта значений кривой.
  gpointer                   curve_data;         // Пользовательские данные для функции расчёта значений кривой.

  GtkCifroCurveRenderer     *curve_renderer;     // Объект рисования кривой.

  GtkCifroAreaState         *state;              // Объект хранения состояния GtkCifroArea.

} GtkCifroCurvePriv;


#define GTK_CIFRO_CURVE_GET_PRIVATE( obj ) ( G_TYPE_INSTANCE_GET_PRIVATE( ( obj ), GTK_TYPE_CIFRO_CURVE, GtkCifroCurvePriv ) )


static void gtk_cifro_curve_set_property( GObject *ccurve, guint prop_id, const GValue *value, GParamSpec *pspec );
static GObject* gtk_cifro_curve_object_constructor( GType type, guint n_construct_properties, GObjectConstructParam *construct_properties );
static void gtk_cifro_curve_object_finalize( GObject *ccurve );


G_DEFINE_TYPE( GtkCifroCurve, gtk_cifro_curve, GTK_TYPE_CIFRO_SCOPE )


static void gtk_cifro_curve_init( GtkCifroCurve *ccurve ){ ; }


// Настройка класса.
static void gtk_cifro_curve_class_init( GtkCifroCurveClass *klass )
{

  GObjectClass *this_class = G_OBJECT_CLASS( klass );

  this_class->set_property = gtk_cifro_curve_set_property;

  this_class->constructor = (void*)gtk_cifro_curve_object_constructor;
  this_class->finalize = (void*)gtk_cifro_curve_object_finalize;

  g_object_class_install_property( this_class, PROP_CURVE_FUNC,
    g_param_spec_pointer( "curve-func", "Curve func", "Curve function", G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY ) );

  g_object_class_install_property( this_class, PROP_CURVE_DATA,
    g_param_spec_pointer( "curve-data", "Curve data", "Curve function data", G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY ) );

  g_type_class_add_private( klass, sizeof( GtkCifroCurvePriv ) );

}


// Функция установки параметров.
static void gtk_cifro_curve_set_property( GObject *ccurve, guint prop_id, const GValue *value, GParamSpec *pspec )
{

  GtkCifroCurvePriv *priv = GTK_CIFRO_CURVE_GET_PRIVATE( ccurve );

  switch ( prop_id )
    {

    case PROP_CURVE_FUNC:
      priv->curve_func = g_value_get_pointer( value );
      break;

    case PROP_CURVE_DATA:
      priv->curve_data = g_value_get_pointer( value );
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID( ccurve, prop_id, pspec );
      break;

    }

}


static GObject* gtk_cifro_curve_object_constructor( GType g_type, guint n_construct_properties, GObjectConstructParam *construct_properties )
{

  GObject *ccurve = G_OBJECT_CLASS( gtk_cifro_curve_parent_class )->constructor( g_type, n_construct_properties, construct_properties );
  GtkCifroCurvePriv *priv = GTK_CIFRO_CURVE_GET_PRIVATE( ccurve );

  // Объект рисования осциллографа.
  priv->curve_renderer = gtk_cifro_curve_renderer_new( ccurve, priv->curve_func, priv->curve_data );

  // Объект состояния GtkCifroArea.
  priv->state = gtk_cifro_area_get_state( GTK_CIFRO_AREA( ccurve ) );

  // Обработчики сигналов от мышки.
  g_signal_connect( ccurve, "button-press-event",   G_CALLBACK( gtk_cifro_curve_renderer_button_press_event ), priv->curve_renderer );
  g_signal_connect( ccurve, "button-release-event", G_CALLBACK( gtk_cifro_curve_renderer_button_release_event ), priv->curve_renderer );
  g_signal_connect( ccurve, "motion-notify-event",  G_CALLBACK( gtk_cifro_curve_renderer_motion_notify_event ), priv->curve_renderer );

  // По умолчанию отключаем отображение информационного блока.
  gtk_cifro_scope_set_info_show( GTK_CIFRO_SCOPE( ccurve ), FALSE );

  return ccurve;

}


static void gtk_cifro_curve_object_finalize( GObject *ccurve )
{

  GtkCifroCurvePriv *priv = GTK_CIFRO_CURVE_GET_PRIVATE( ccurve );

  g_object_unref( priv->curve_renderer );

  G_OBJECT_CLASS( gtk_cifro_curve_parent_class )->finalize( G_OBJECT( ccurve ) );

}


// Функция создаёт виджет GtkCifroCurve.
GtkWidget *gtk_cifro_curve_new( GtkCifroCurveFunc curve_func, gpointer curve_data )
{

  return g_object_new( GTK_TYPE_CIFRO_CURVE, "curve-func", curve_func, "curve-data", curve_data, NULL );

}


// Функция удаляет все контрольные точки.
void gtk_cifro_curve_clear_points( GtkCifroCurve *ccurve )
{

  GtkCifroCurvePriv *priv = GTK_CIFRO_CURVE_GET_PRIVATE( ccurve );

  gtk_cifro_curve_renderer_clear_points( priv->curve_renderer );

  gtk_cifro_area_update( GTK_CIFRO_AREA( ccurve ) );

}


// Функция добавляет одну контрольную точку к существующим.
void gtk_cifro_curve_add_point( GtkCifroCurve *ccurve, gdouble x, gdouble y )
{

  GtkCifroCurvePriv *priv = GTK_CIFRO_CURVE_GET_PRIVATE( ccurve );

  gtk_cifro_curve_renderer_add_point( priv->curve_renderer, x, y );

  gtk_cifro_area_update( GTK_CIFRO_AREA( ccurve ) );

}


// Функция устанавливает новые контрольные точки взамен текущих.
void gtk_cifro_curve_set_points( GtkCifroCurve *ccurve, GArray *points )
{

  GtkCifroCurvePriv *priv = GTK_CIFRO_CURVE_GET_PRIVATE( ccurve );

  gtk_cifro_curve_renderer_set_points( priv->curve_renderer, points );

  gtk_cifro_area_update( GTK_CIFRO_AREA( ccurve ) );

}


// Функция возвращает массив текущих контрольных точек.
GArray *gtk_cifro_curve_get_points( GtkCifroCurve *ccurve )
{

  GtkCifroCurvePriv *priv = GTK_CIFRO_CURVE_GET_PRIVATE( ccurve );

  return gtk_cifro_curve_renderer_get_points( priv->curve_renderer );

}


// Функция устанавливает цвет кривой.
void gtk_cifro_curve_set_curve_color( GtkCifroCurve *ccurve, gdouble red, gdouble green, gdouble blue )
{

  GtkCifroCurvePriv *priv = GTK_CIFRO_CURVE_GET_PRIVATE( ccurve );

  gtk_cifro_curve_renderer_set_curve_color( priv->curve_renderer, red, green, blue );

  gtk_cifro_area_update( GTK_CIFRO_AREA( ccurve ) );

}


// Функция устанавливает цвет контрольных точек кривой.
void gtk_cifro_curve_set_point_color( GtkCifroCurve *ccurve, gdouble red, gdouble green, gdouble blue )
{

  GtkCifroCurvePriv *priv = GTK_CIFRO_CURVE_GET_PRIVATE( ccurve );

  gtk_cifro_curve_renderer_set_point_color( priv->curve_renderer, red, green, blue );

  gtk_cifro_area_update( GTK_CIFRO_AREA( ccurve ) );

}
