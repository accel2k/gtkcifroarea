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
 * \file gtk-cifro-scope.c
 *
 * \brief Исходный файл GTK+ виджета осциллографа
 * \author Andrei Fadeev
 * \date 2013-2015
 * \license GNU General Public License version 3 или более поздняя<br>
 * Коммерческая лицензия - свяжитесь с автором
 *
*/

#include "gtk-cifro-scope.h"
#include "gtk-cifro-scope-renderer.h"

#include "cairo-sdline.h"
#include <glib/gprintf.h>
#include <string.h>
#include <math.h>


typedef struct GtkCifroScopePriv {

  GtkCifroScopeRenderer     *scope_renderer;     // Объект рисования осциллографа.

  GtkCifroScopeGravity       gravity;            // Направление осей осциллографа.

  GtkCifroAreaState         *state;              // Объект хранения состояния GtkCifroArea.

} GtkCifroScopePriv;


#define GTK_CIFRO_SCOPE_GET_PRIVATE( obj ) ( G_TYPE_INSTANCE_GET_PRIVATE( ( obj ), GTK_TYPE_CIFRO_SCOPE, GtkCifroScopePriv ) )


static GObject* gtk_cifro_scope_object_constructor( GType type, guint n_construct_properties, GObjectConstructParam *construct_properties );
static void gtk_cifro_scope_object_finalize( GObject *cscope );

static gboolean gtk_cifro_scope_configure( GtkWidget *widget, GdkEventConfigure *event, GtkCifroScopePriv *priv );
gboolean gtk_cifro_scope_motion_notify( GtkWidget *widget, GdkEventMotion *event, GtkCifroScopePriv *priv );
gboolean gtk_cifro_scope_leave_notify( GtkWidget *widget, GdkEventCrossing *event, GtkCifroScopePriv *priv );


G_DEFINE_TYPE( GtkCifroScope, gtk_cifro_scope, GTK_TYPE_CIFRO_AREA )


static void gtk_cifro_scope_init( GtkCifroScope *cscope ){ ; }


static void gtk_cifro_scope_class_init( GtkCifroScopeClass *klass )
{

  GObjectClass *this_class = G_OBJECT_CLASS( klass );

  this_class->constructor = gtk_cifro_scope_object_constructor;
  this_class->finalize = gtk_cifro_scope_object_finalize;

  g_type_class_add_private( klass, sizeof( GtkCifroScopePriv ) );

}


static GObject* gtk_cifro_scope_object_constructor( GType g_type, guint n_construct_properties, GObjectConstructParam *construct_properties )
{

  GObject *cscope = G_OBJECT_CLASS( gtk_cifro_scope_parent_class )->constructor( g_type, n_construct_properties, construct_properties );
  GtkCifroScopePriv *priv = GTK_CIFRO_SCOPE_GET_PRIVATE( cscope );

  // Объект рисования осциллографа.
  priv->scope_renderer = gtk_cifro_scope_renderer_new( cscope );

  // Объект состояния GtkCifroArea.
  priv->state = gtk_cifro_area_get_state( GTK_CIFRO_AREA( cscope ) );

  // Устанавливаем направления осей.
  gtk_cifro_scope_set_gravity( GTK_CIFRO_SCOPE( cscope ), GTK_CIFRO_SCOPE_GRAVITY_RIGHT_UP );

  // Параметры GtkCifroArea по умолчанию.
  gtk_cifro_area_set_scale_limits( GTK_CIFRO_AREA( cscope ), 0.01, 100.0, 0.01, 100.0 );
  gtk_cifro_area_set_scale_on_resize( GTK_CIFRO_AREA( cscope ), TRUE );
  gtk_cifro_area_set_rotation( GTK_CIFRO_AREA( cscope ), FALSE );
  gtk_cifro_area_set_scale_aspect( GTK_CIFRO_AREA( cscope ), -1.0 );
  gtk_cifro_area_set_zoom_on_center( GTK_CIFRO_AREA( cscope ), FALSE );
  gtk_cifro_area_set_zoom_scale( GTK_CIFRO_AREA( cscope ), 10 );
  gtk_cifro_area_set_view_limits( GTK_CIFRO_AREA( cscope ), -100, 100, -100, 100 );
  gtk_cifro_area_set_view( GTK_CIFRO_AREA( cscope ), -100, 100, -100, 100 );

  // Обработчики сигналов.
  g_signal_connect( cscope, "configure-event", G_CALLBACK( gtk_cifro_scope_configure ), priv );
  g_signal_connect( cscope, "motion-notify-event",  G_CALLBACK( gtk_cifro_scope_motion_notify ), priv );
  g_signal_connect( cscope, "leave-notify-event", G_CALLBACK( gtk_cifro_scope_leave_notify ), priv );

  return cscope;

}


static void gtk_cifro_scope_object_finalize( GObject *cscope )
{

  GtkCifroScopePriv *priv = GTK_CIFRO_SCOPE_GET_PRIVATE( cscope );

  g_object_unref( priv->scope_renderer );

  G_OBJECT_CLASS( gtk_cifro_scope_parent_class )->finalize( G_OBJECT( cscope ) );

}


static gboolean gtk_cifro_scope_configure( GtkWidget *widget, GdkEventConfigure *event, GtkCifroScopePriv *priv )
{

  PangoLayout *font;
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

  // Текущий шрифт приложения.
  font = gtk_widget_create_pango_layout( widget, NULL );
  gtk_cifro_scope_renderer_set_font( priv->scope_renderer, font );

  pango_layout_set_text( font, "0123456789ABCDEFGHIJKLMNOPQRSTUWXYZ.,", -1 );
  pango_layout_get_size( font, NULL, &border );

  // Устанавливаем размер линеек оцифровки осей.
  border *= 1.6;
  border /= PANGO_SCALE;
  gtk_cifro_area_set_border( GTK_CIFRO_AREA( widget ), border, border, border, border );

  // Основной цвет темы.
#ifdef CIFRO_AREA_WITH_GTK2
  style = gtk_rc_get_style_by_paths( gtk_settings_get_default(), NULL, NULL, GTK_TYPE_WINDOW );
  if( style )
    {
    color = &style->fg[GTK_STATE_NORMAL];
    red = color->red / 65535.0;
    green = color->green / 65535.0;
    blue = color->blue / 65535.0;
    }
#else
  gtk_style_context_get_color( gtk_widget_get_style_context( widget ), GTK_STATE_FLAG_NORMAL, &color );
  red = color.red;
  green = color.green;
  blue = color.blue;
  alpha = color.alpha;
#endif

  gtk_cifro_scope_renderer_set_fg_color( priv->scope_renderer, red, green, blue, alpha );

  return FALSE;

}


// Функция обработки сигнала движения мышки.
gboolean gtk_cifro_scope_motion_notify( GtkWidget *widget, GdkEventMotion *event, GtkCifroScopePriv *priv )
{

  gint area_width;
  gint area_height;

  gint border_left;
  gint border_right;
  gint border_top;
  gint border_bottom;

  gint x = event->x;
  gint y = event->y;

  // Проверяем, что координаты курсора находятся в рабочей области.
  gtk_cifro_area_state_get_area_size( priv->state, &area_width, &area_height );
  gtk_cifro_area_state_get_border( priv->state, &border_left, &border_right, &border_top, &border_bottom );
  if( x < border_left ) { x = -1; y = -1; }
  if( y < border_top ) { x = -1; y = -1; }
  if( x > area_width - border_right ) { x = -1; y = -1; }
  if( y > area_height - border_bottom ) { x = -1; y = -1; }

  // Запоминаем координаты, устанавливаем флаг перерисовки слоя с информацией.
  gtk_cifro_scope_renderer_set_pointer( priv->scope_renderer, x, y );

  gtk_cifro_area_update( GTK_CIFRO_AREA( widget ) );

  return FALSE;

}


// Функция обработки сигнала выхода курсора за пределы окна.
gboolean gtk_cifro_scope_leave_notify( GtkWidget *widget, GdkEventCrossing *event, GtkCifroScopePriv *priv )
{

  gtk_cifro_scope_renderer_set_pointer( priv->scope_renderer, -1, -1 );

  gtk_cifro_area_update( GTK_CIFRO_AREA( widget ) );

  return FALSE;

}


// Функция создаёт виджет GtkCifroScope.
GtkWidget *gtk_cifro_scope_new( void )
{

  return g_object_new( GTK_TYPE_CIFRO_SCOPE, NULL );

}


// Функция изменяет ориентацию осей осциллографа.
void gtk_cifro_scope_set_gravity( GtkCifroScope *cscope, GtkCifroScopeGravity gravity )
{

  GtkCifroScopePriv *priv = GTK_CIFRO_SCOPE_GET_PRIVATE( cscope );

  gtk_cifro_area_set_rotation( GTK_CIFRO_AREA( cscope ), TRUE );

  // Устанавливаем направления осей.
  switch( gravity )
    {

    case GTK_CIFRO_SCOPE_GRAVITY_RIGHT_UP:
      gtk_cifro_area_set_angle( GTK_CIFRO_AREA( cscope ), 0.0 );
      gtk_cifro_area_set_swap( GTK_CIFRO_AREA( cscope ), FALSE, FALSE );
      break;

    case GTK_CIFRO_SCOPE_GRAVITY_LEFT_UP:
      gtk_cifro_area_set_angle( GTK_CIFRO_AREA( cscope ), 0.0 );
      gtk_cifro_area_set_swap( GTK_CIFRO_AREA( cscope ), TRUE, FALSE );
      break;

    case GTK_CIFRO_SCOPE_GRAVITY_RIGHT_DOWN:
      gtk_cifro_area_set_angle( GTK_CIFRO_AREA( cscope ), 0.0 );
      gtk_cifro_area_set_swap( GTK_CIFRO_AREA( cscope ), FALSE, TRUE );
      break;

    case GTK_CIFRO_SCOPE_GRAVITY_LEFT_DOWN:
      gtk_cifro_area_set_angle( GTK_CIFRO_AREA( cscope ), 0.0 );
      gtk_cifro_area_set_swap( GTK_CIFRO_AREA( cscope ), TRUE, TRUE );
      break;

    case GTK_CIFRO_SCOPE_GRAVITY_UP_RIGHT:
      gtk_cifro_area_set_angle( GTK_CIFRO_AREA( cscope ), G_PI/2.0 );
      gtk_cifro_area_set_swap( GTK_CIFRO_AREA( cscope ), TRUE, FALSE );
      break;

    case GTK_CIFRO_SCOPE_GRAVITY_UP_LEFT:
      gtk_cifro_area_set_angle( GTK_CIFRO_AREA( cscope ), G_PI/2.0 );
      gtk_cifro_area_set_swap( GTK_CIFRO_AREA( cscope ), TRUE, TRUE );
      break;

    case GTK_CIFRO_SCOPE_GRAVITY_DOWN_RIGHT:
      gtk_cifro_area_set_angle( GTK_CIFRO_AREA( cscope ), G_PI/2.0 );
      gtk_cifro_area_set_swap( GTK_CIFRO_AREA( cscope ), FALSE, FALSE );
      break;

    case GTK_CIFRO_SCOPE_GRAVITY_DOWN_LEFT:
      gtk_cifro_area_set_angle( GTK_CIFRO_AREA( cscope ), G_PI/2.0 );
      gtk_cifro_area_set_swap( GTK_CIFRO_AREA( cscope ), FALSE, TRUE );
      break;

    default:
      gtk_cifro_area_set_rotation( GTK_CIFRO_AREA( cscope ), FALSE );
      return;

    }

  gtk_cifro_area_set_rotation( GTK_CIFRO_AREA( cscope ), FALSE );

  priv->gravity = gravity;

  gtk_cifro_area_update( GTK_CIFRO_AREA( cscope ) );

}


// Функция включает или выключает отображения блока с информацией о значениях под курсором.
void gtk_cifro_scope_set_info_show( GtkCifroScope *cscope, gboolean show )
{

  GtkCifroScopePriv *priv = GTK_CIFRO_SCOPE_GET_PRIVATE( cscope );

  gtk_cifro_scope_renderer_set_info_show( priv->scope_renderer, show );
  gtk_cifro_area_update( GTK_CIFRO_AREA( cscope ) );

}


// Функция задаёт подписи к осям абсцисс и ординат.
void gtk_cifro_scope_set_axis_name( GtkCifroScope *cscope, const gchar *time_axis_name, const gchar *value_axis_name )
{

  GtkCifroScopePriv *priv = GTK_CIFRO_SCOPE_GET_PRIVATE( cscope );

  gtk_cifro_scope_renderer_set_axis_name( priv->scope_renderer, time_axis_name, value_axis_name );
  gtk_cifro_area_update( GTK_CIFRO_AREA( cscope ) );

}


// Функция дабавляет канал отображения данных в осциллограф.
gpointer gtk_cifro_scope_add_channel( GtkCifroScope *cscope )
{

  GtkCifroScopePriv *priv = GTK_CIFRO_SCOPE_GET_PRIVATE( cscope );

  return gtk_cifro_scope_renderer_add_channel( priv->scope_renderer );

}


// Функция удаляет канал отображения данных из осциллографа.
void gtk_cifro_scope_remove_channel( GtkCifroScope *cscope, gpointer channel_id )
{

  GtkCifroScopePriv *priv = GTK_CIFRO_SCOPE_GET_PRIVATE( cscope );

  gtk_cifro_scope_renderer_remove_channel( priv->scope_renderer, channel_id );
  gtk_cifro_area_update( GTK_CIFRO_AREA( cscope ) );

}


// Функция устанавливает имя канала.
void gtk_cifro_scope_set_channel_name( GtkCifroScope *cscope, gpointer channel_id, const gchar *axis_name )
{

  GtkCifroScopePriv *priv = GTK_CIFRO_SCOPE_GET_PRIVATE( cscope );

  gtk_cifro_scope_renderer_set_channel_name( priv->scope_renderer, channel_id, axis_name );
  gtk_cifro_area_update( GTK_CIFRO_AREA( cscope ) );

}


// Функция устанавливает с какого момента времени следует отображать данные и шаг между двумя соседними данными.
void gtk_cifro_scope_set_channel_time_param( GtkCifroScope *cscope, gpointer channel_id, gfloat time_shift, gfloat time_step )
{

  GtkCifroScopePriv *priv = GTK_CIFRO_SCOPE_GET_PRIVATE( cscope );

  gtk_cifro_scope_renderer_set_channel_time_param( priv->scope_renderer, channel_id, time_shift, time_step );
  gtk_cifro_area_update( GTK_CIFRO_AREA( cscope ) );

}


// Функция устанавливает коэффициенты на которые умножаются и сдвигаются все данные в канале.
void gtk_cifro_scope_set_channel_value_param( GtkCifroScope *cscope, gpointer channel_id, gfloat value_shift, gfloat value_scale )
{

  GtkCifroScopePriv *priv = GTK_CIFRO_SCOPE_GET_PRIVATE( cscope );

  gtk_cifro_scope_renderer_set_channel_value_param( priv->scope_renderer, channel_id, value_shift, value_scale );
  gtk_cifro_area_update( GTK_CIFRO_AREA( cscope ) );

}


// Функция устанавливает типа отображения осциллограмм.
void gtk_cifro_scope_set_channel_draw_type( GtkCifroScope *cscope, gpointer channel_id, GtkCifroScopeDrawType draw_type )
{

  GtkCifroScopePriv *priv = GTK_CIFRO_SCOPE_GET_PRIVATE( cscope );

  gtk_cifro_scope_renderer_set_channel_draw_type( priv->scope_renderer, channel_id, draw_type );
  gtk_cifro_area_update( GTK_CIFRO_AREA( cscope ) );

}


// Функция устанавливает цвет отображения данных канала.
void gtk_cifro_scope_set_channel_color( GtkCifroScope *cscope, gpointer channel_id, gdouble red, gdouble green, gdouble blue )
{

  GtkCifroScopePriv *priv = GTK_CIFRO_SCOPE_GET_PRIVATE( cscope );

  gtk_cifro_scope_renderer_set_channel_color( priv->scope_renderer, channel_id, red, green, blue );
  gtk_cifro_area_update( GTK_CIFRO_AREA( cscope ) );

}


// Функция устанавливает данные канала для отображения.
void gtk_cifro_scope_set_channel_data( GtkCifroScope *cscope, gpointer channel_id, gint num, gfloat *values )
{

  GtkCifroScopePriv *priv = GTK_CIFRO_SCOPE_GET_PRIVATE( cscope );

  gtk_cifro_scope_renderer_set_channel_data( priv->scope_renderer, channel_id, num, values );

}


// Функция включает или выключает отображения данных канала.
void gtk_cifro_scope_set_channel_show( GtkCifroScope *cscope, gpointer channel_id, gboolean show )
{

  GtkCifroScopePriv *priv = GTK_CIFRO_SCOPE_GET_PRIVATE( cscope );

  gtk_cifro_scope_renderer_set_channel_show( priv->scope_renderer, channel_id, show );
  gtk_cifro_area_update( GTK_CIFRO_AREA( cscope ) );

}


// ункция обновляет изображение осциллографа.
void gtk_cifro_scope_update( GtkCifroScope *cscope )
{

  GtkCifroScopePriv *priv = GTK_CIFRO_SCOPE_GET_PRIVATE( cscope );

  gtk_cifro_scope_renderer_update( priv->scope_renderer );
  gtk_cifro_area_update( GTK_CIFRO_AREA( cscope ) );

}
