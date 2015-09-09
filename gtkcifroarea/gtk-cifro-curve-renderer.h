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

#ifndef _gtk_cifro_curve_renderer_h
#define _gtk_cifro_curve_renderer_h

#include <glib-object.h>
#include <gtk-cifro-curve-types.h>

G_BEGIN_DECLS


#define GTK_TYPE_CIFRO_CURVE_RENDERER                      ( gtk_cifro_curve_renderer_get_type() )
#define GTK_CIFRO_CURVE_RENDERER( obj )                    ( G_TYPE_CHECK_INSTANCE_CAST( ( obj ), GTK_TYPE_CIFRO_CURVE_RENDERER, GtkCifroCurveRenderer ) )
#define GTK_IS_CIFRO_CURVE_RENDERER( obj )                 ( G_TYPE_CHECK_INSTANCE_TYPE( ( obj ), GTK_TYPE_CIFRO_CURVE_RENDERER ) )
#define GTK_CIFRO_CURVE_RENDERER_CLASS( klass )            ( G_TYPE_CHECK_CLASS_CAST( ( klass ), GTK_TYPE_CIFRO_CURVE_RENDERER, GtkCifroCurveRendererClass ) )
#define GTK_IS_CIFRO_CURVE_RENDERER_CLASS( klass )         ( G_TYPE_CHECK_CLASS_TYPE( ( klass ), GTK_TYPE_CIFRO_CURVE_RENDERER ) )
#define GTK_CIFRO_CURVE_RENDERER_GET_CLASS( obj )          ( G_TYPE_INSTANCE_GET_CLASS( ( obj ), GTK_TYPE_CIFRO_CURVE_RENDERER, GtkCifroCurveRendererClass ) )


typedef GObject GtkCifroCurveRenderer;
typedef GObjectClass GtkCifroCurveRendererClass;


GType gtk_cifro_curve_renderer_get_type( void );


// Функция создаёт новый объект GtkCifroCurveRenderer.
GtkCifroCurveRenderer *gtk_cifro_curve_renderer_new( GObject *area, GtkCifroCurveFunc curve_func, gpointer curve_data );


// Функция удаляет все контрольные точки.
void gtk_cifro_curve_renderer_clear_points( GtkCifroCurveRenderer *curve_renderer );


// Функция добавляет одну контрольную точку к существующим.
void gtk_cifro_curve_renderer_add_point( GtkCifroCurveRenderer *curve_renderer, gdouble x, gdouble y );


// Функция устанавливает новые контрольные точки взамен текущих.
void gtk_cifro_curve_renderer_set_points( GtkCifroCurveRenderer *curve_renderer, GArray *points );


// Функция возвращает массив текущих контрольных точек.
GArray *gtk_cifro_curve_renderer_get_points( GtkCifroCurveRenderer *curve_renderer );


// Функция устанавливает цвет кривой.
void gtk_cifro_curve_renderer_set_curve_color( GtkCifroCurveRenderer *curve_renderer, gdouble red, gdouble green, gdouble blue );


// Функция устанавливает цвет контрольных точек кривой.
void gtk_cifro_curve_renderer_set_point_color( GtkCifroCurveRenderer *curve_renderer, gdouble red, gdouble green, gdouble blue );


// Функция обработки сигнала нажатия кнопки мыши. Должна быть подключена к сигналу "button-press-event".
gboolean gtk_cifro_curve_renderer_button_press_event( GtkWidget *widget, GdkEventButton *event, GtkCifroCurveRenderer *curve_renderer );


// Функция обработки сигнала отпускания кнопки мыши. Должна быть подключена к сигналу "button-release-event".
gboolean gtk_cifro_curve_renderer_button_release_event( GtkWidget *widget, GdkEventButton *event, GtkCifroCurveRenderer *curve_renderer );


// Функция обработки сигнала движения мышки. Должна быть подключена к сигналу "motion-notify-event".
gboolean gtk_cifro_curve_renderer_motion_notify_event( GtkWidget *widget, GdkEventMotion *event, GtkCifroCurveRenderer *curve_renderer );


G_END_DECLS

#endif // _gtk_cifro_curve_renderer_h
