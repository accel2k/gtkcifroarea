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

#ifndef _gtk_cifro_scope_renderer_h
#define _gtk_cifro_scope_renderer_h

#include <glib-object.h>
#include <gtk-cifro-scope-types.h>

G_BEGIN_DECLS


#define GTK_TYPE_CIFRO_SCOPE_RENDERER                      ( gtk_cifro_scope_renderer_get_type() )
#define GTK_CIFRO_SCOPE_RENDERER( obj )                    ( G_TYPE_CHECK_INSTANCE_CAST( ( obj ), GTK_TYPE_CIFRO_SCOPE_RENDERER, GtkCifroScopeRenderer ) )
#define GTK_IS_CIFRO_SCOPE_RENDERER( obj )                 ( G_TYPE_CHECK_INSTANCE_TYPE( ( obj ), GTK_TYPE_CIFRO_SCOPE_RENDERER ) )
#define GTK_CIFRO_SCOPE_RENDERER_CLASS( klass )            ( G_TYPE_CHECK_CLASS_CAST( ( klass ), GTK_TYPE_CIFRO_SCOPE_RENDERER, GtkCifroScopeRendererClass ) )
#define GTK_IS_CIFRO_SCOPE_RENDERER_CLASS( klass )         ( G_TYPE_CHECK_CLASS_TYPE( ( klass ), GTK_TYPE_CIFRO_SCOPE_RENDERER ) )
#define GTK_CIFRO_SCOPE_RENDERER_GET_CLASS( obj )          ( G_TYPE_INSTANCE_GET_CLASS( ( obj ), GTK_TYPE_CIFRO_SCOPE_RENDERER, GtkCifroScopeRendererClass ) )


typedef GObject GtkCifroScopeRenderer;
typedef GObjectClass GtkCifroScopeRendererClass;


GType gtk_cifro_scope_renderer_get_type( void );


// Функция создаёт новый объект GtkCifroScopeRenderer.
GtkCifroScopeRenderer *gtk_cifro_scope_renderer_new( GObject *area );


// Функция устанавливает раскладку шрифта для отображения текстовых надписей в осциллографе.
void gtk_cifro_scope_renderer_set_font( GtkCifroScopeRenderer *scope_renderer, PangoLayout *font );


// Функция устанавливает основной цвет для отображения графических элементов в осциллографе.
void gtk_cifro_scope_renderer_set_fg_color( GtkCifroScopeRenderer *scope_renderer, gdouble red, gdouble green, gdouble blue, gdouble alpha );


// Функция устанавливает координаты точки для отображения информации.
void gtk_cifro_scope_renderer_set_pointer( GtkCifroScopeRenderer *scope_renderer, gint pointer_x, gint pointer_y );


// Функция включает или выключает отображения блока с информацией о значениях под курсором.
void gtk_cifro_scope_renderer_set_info_show( GtkCifroScopeRenderer *scope_renderer, gboolean show );


// Функция задаёт подписи к осям абсцисс и ординат.
void gtk_cifro_scope_renderer_set_axis_name( GtkCifroScopeRenderer *scope_renderer, const gchar *time_axis_name, const gchar *value_axis_name );


// Функция дабавляет канал отображения данных в осциллограф.
gpointer gtk_cifro_scope_renderer_add_channel( GtkCifroScopeRenderer *scope_renderer );


// Функция удаляет канал отображения данных из осциллографа.
void gtk_cifro_scope_renderer_remove_channel( GtkCifroScopeRenderer *scope_renderer, gpointer channel_id );


// Функция устанавливает имя канала.
void gtk_cifro_scope_renderer_set_channel_name( GtkCifroScopeRenderer *scope_renderer, gpointer channel_id, const gchar *axis_name );


// Функция устанавливает с какого момента времени следует отображать данные и шаг между двумя соседними данными.
void gtk_cifro_scope_renderer_set_channel_time_param( GtkCifroScopeRenderer *scope_renderer, gpointer channel_id, gfloat time_shift, gfloat time_step );


// Функция устанавливает коэффициенты на которые умножаются и сдвигаются все данные в канале.
void gtk_cifro_scope_renderer_set_channel_value_param( GtkCifroScopeRenderer *scope_renderer, gpointer channel_id, gfloat value_shift, gfloat value_scale );


// Функция устанавливает типа отображения осциллограмм.
void gtk_cifro_scope_renderer_set_channel_draw_type( GtkCifroScopeRenderer *scope_renderer, gpointer channel_id, GtkCifroScopeDrawType draw_type );


// Функция устанавливает цвет отображения данных канала.
void gtk_cifro_scope_renderer_set_channel_color( GtkCifroScopeRenderer *scope_renderer, gpointer channel_id, gdouble red, gdouble green, gdouble blue );


// Функция устанавливает данные канала для отображения.
void gtk_cifro_scope_renderer_set_channel_data( GtkCifroScopeRenderer *scope_renderer, gpointer channel_id, gint num, gfloat *values );


// Функция включает или выключает отображения данных канала.
void gtk_cifro_scope_renderer_set_channel_show( GtkCifroScopeRenderer *scope_renderer, gpointer channel_id, gboolean show );


// Функция обновляет изображение осциллографа.
void gtk_cifro_scope_renderer_update( GtkCifroScopeRenderer *scope_renderer );


G_END_DECLS

#endif // _gtk_cifro_scope_renderer_h
