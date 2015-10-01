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

#include "gtk-cifro-area.h"
#include "gtk-cifro-area-state.h"
#include "gtk-cifro-scope-renderer.h"

#include "cairo-sdline.h"
#include <glib/gprintf.h>
#include <string.h>
#include <math.h>


enum { PROP_0, PROP_AREA };


typedef struct GtkCifroScopeChannel {            // Канал осциллографа.

  gchar                     *name;               // Имя канала.
  GtkCifroScopeDrawType      draw_type;          // Тип отображения осциллограмм.
  guint32                    color;              // Цвета данных канала.
  gboolean                   show;               // "Выключатели" каналов осциллографа.
  gint                       num;                // Число данных для отображения.

  gfloat                     time_shift;         // Смещение данных по времени.
  gfloat                     time_step;          // Шаг времени.
  gfloat                     value_shift;        // Коэффициент смещения данных.
  gfloat                     value_scale;        // Коэффициент масштабирования данных.

  gfloat                    *data;               // Данные для отображения.
  gint                       size;               // Размер массива данных для отображения.

} GtkCifroScopeChannel;


typedef struct GtkCifroScopeRendererPriv {       // Внутренние данные объекта.

  GObject                   *area;               // Указатель на объект GtkCifroArea или GtkCifroImage.
  GtkCifroAreaState         *state;              // Объект хранения состояния GtkCifroArea.

  GHashTable                *channels;           // Данные каналов осциллографа.

  gboolean                   show_info;          // Показывать или нет информацию о значениях под курсором.

  gboolean                   visible_update;     // Признак необходимости перерисовки осциллограмм.
  gboolean                   area_update;        // Признак необходимости перерисовки оцифровки осей и информационного сообщения.

  gint                       pointer_x;          // Текущее местоположение курсора, x координата.
  gint                       pointer_y;          // Текущее местоположение курсора, y координата.

  gint32                     next_channel_id;    // Идентификатор для нового канала.

  PangoLayout               *font;               // Раскладка шрифта.

  gchar                     *x_axis_name;        // Подпись оси времени.
  gchar                     *y_axis_name;        // Подпись оси значений.

  guint32                    border_color;       // Цвет обрамления области отображения данных.
  guint32                    axis_color;         // Цвет осей.
  guint32                    zero_axis_color;    // Цвет осей для нулевых значений.
  guint32                    text_color;         // Цвет подписей.

  gint                       area_width;         // Ширина области вывода.
  gint                       area_height;        // Высота области вывода.

  gint                       visible_width;      // Видимая ширина.
  gint                       visible_height;     // Видимая высота.

  gint                       border_left;        // Размер области обрамления слева.
  gint                       border_right;       // Размер области обрамления справа.
  gint                       border_top;         // Размер области обрамления сверху.
  gint                       border_bottom;      // Размер области обрамления снизу.

  gboolean                   swap_x;             // TRUE - ось x направлена влево, FALSE - вправо.
  gboolean                   swap_y;             // TRUE - ось y направлена вниз, FALSE - вверх.

  gdouble                    angle;              // Угол поворота изображения в радианах.

  gdouble                    min_x;              // Минимально возможное значение по оси x.
  gdouble                    max_x;              // Максимально возможное значение по оси x.
  gdouble                    min_y;              // Минимально возможное значение по оси y.
  gdouble                    max_y;              // Максимально возможное значение по оси y.

  gdouble                    from_x;             // Минимальная граница отображения оси x.
  gdouble                    to_x;               // Максимальная граница отображения оси x.
  gdouble                    from_y;             // Минимальная граница отображения оси y.
  gdouble                    to_y;               // Максимальная граница отображения оси y.

  gdouble                    scale_x;            // Текущий коэффициент масштаба по оси x.
  gdouble                    scale_y;            // Текущий коэффициент масштаба по оси y.

} GtkCifroScopeRendererPriv;


#define GTK_CIFRO_SCOPE_RENDERER_GET_PRIVATE( obj ) ( G_TYPE_INSTANCE_GET_PRIVATE( ( obj ), GTK_TYPE_CIFRO_SCOPE_RENDERER, GtkCifroScopeRendererPriv ) )


static void gtk_cifro_scope_renderer_set_property( GObject *cifro_scope_renderer, guint prop_id, const GValue *value, GParamSpec *pspec );
static GObject* gtk_cifro_scope_renderer_object_constructor( GType g_type, guint n_construct_properties, GObjectConstructParam *construct_properties );
static void gtk_cifro_scope_renderer_object_finalize( GObject *cifro_scope_renderer );

static void gtk_cifro_scope_renderer_free_channel( GtkCifroScopeChannel *channel );

static void gtk_cifro_scope_renderer_area_changed( GtkCifroAreaState *state, GtkCifroAreaSize *size, GtkCifroScopeRendererPriv *priv );
static void gtk_cifro_scope_renderer_visible_changed( GtkCifroAreaState *state, GtkCifroAreaSize *size, GtkCifroScopeRendererPriv *priv );
static void gtk_cifro_scope_renderer_border_changed( GtkCifroAreaState *state, GtkCifroAreaBorder *border, GtkCifroScopeRendererPriv *priv );
static void gtk_cifro_scope_renderer_swap_changed( GtkCifroAreaState *state, GtkCifroAreaSwap *swap, GtkCifroScopeRendererPriv *priv );
static void gtk_cifro_scope_renderer_angle_changed( GtkCifroAreaState *state, gdouble angle, GtkCifroScopeRendererPriv *priv );
static void gtk_cifro_scope_renderer_view_limits_changed( GtkCifroAreaState *state, GtkCifroAreaView *view, GtkCifroScopeRendererPriv *priv );
static void gtk_cifro_scope_renderer_view_changed( GtkCifroAreaState *state, GtkCifroAreaView *view, GtkCifroScopeRendererPriv *priv );

static void gtk_cifro_scope_renderer_draw_hruler( cairo_t *cairo, GtkCifroScopeRendererPriv *priv );
static void gtk_cifro_scope_renderer_draw_vruler( cairo_t *cairo, GtkCifroScopeRendererPriv *priv );
static void gtk_cifro_scope_renderer_draw_x_pos( cairo_t *cairo, GtkCifroScopeRendererPriv *priv );
static void gtk_cifro_scope_renderer_draw_y_pos( cairo_t *cairo, GtkCifroScopeRendererPriv *priv );
static void gtk_cifro_scope_renderer_draw_info( cairo_t *cairo, GtkCifroScopeRendererPriv *priv );

static void gtk_cifro_scope_renderer_draw_axis( cairo_sdline_surface *surface, GtkCifroScopeRendererPriv *priv );
static void gtk_cifro_scope_renderer_draw_lined_data( cairo_sdline_surface *surface, GtkCifroScopeRendererPriv *priv, gpointer channel_id );
static void gtk_cifro_scope_renderer_draw_dotted_data( cairo_sdline_surface *surfaceo, GtkCifroScopeRendererPriv *priv, gpointer channel_id );

static gboolean gtk_cifro_scope_renderer_check_area_redraw( GtkWidget *widget, GtkCifroScopeRendererPriv *priv );
static gboolean gtk_cifro_scope_renderer_check_visible_redraw( GtkWidget *widget, GtkCifroScopeRendererPriv *priv );
static void gtk_cifro_scope_renderer_area_draw( GtkWidget *widget, cairo_t *cairo, GtkCifroScopeRendererPriv *priv );
static void gtk_cifro_scope_renderer_visible_draw( GtkWidget *widget, cairo_t *cairo, GtkCifroScopeRendererPriv *priv );


G_DEFINE_TYPE( GtkCifroScopeRenderer, gtk_cifro_scope_renderer, G_TYPE_OBJECT );


static void gtk_cifro_scope_renderer_class_init( GtkCifroScopeRendererClass *klass )
{

  GObjectClass *this_class = G_OBJECT_CLASS( klass );

  this_class->set_property = gtk_cifro_scope_renderer_set_property;

  this_class->constructor = gtk_cifro_scope_renderer_object_constructor;
  this_class->finalize = gtk_cifro_scope_renderer_object_finalize;

  g_object_class_install_property( this_class, PROP_AREA,
    g_param_spec_object( "area", "Area", "GtkCifroArea object", G_TYPE_OBJECT, G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY ) );

  g_type_class_add_private( klass, sizeof( GtkCifroScopeRendererPriv ) );

}


static void gtk_cifro_scope_renderer_init( GtkCifroScopeRenderer *cifro_scope_renderer ){ ; }


static void gtk_cifro_scope_renderer_set_property( GObject *cifro_scope_renderer, guint prop_id, const GValue *value, GParamSpec *pspec )
{

  GtkCifroScopeRendererPriv *priv = GTK_CIFRO_SCOPE_RENDERER_GET_PRIVATE( cifro_scope_renderer );

  switch ( prop_id )
    {

    case PROP_AREA:
      priv->area = g_value_get_object( value );
      if( !GTK_IS_CIFRO_AREA( priv->area ) )
        {
        G_OBJECT_WARN_INVALID_PROPERTY_ID( cifro_scope_renderer, prop_id, pspec );
        priv->area = NULL;
        }
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID( cifro_scope_renderer, prop_id, pspec );
      break;

    }

}


static GObject* gtk_cifro_scope_renderer_object_constructor( GType g_type, guint n_construct_properties, GObjectConstructParam *construct_properties )
{

  GObject *scope_renderer = G_OBJECT_CLASS( gtk_cifro_scope_renderer_parent_class )->constructor( g_type, n_construct_properties, construct_properties );
  GtkCifroScopeRendererPriv *priv = GTK_CIFRO_SCOPE_RENDERER_GET_PRIVATE( scope_renderer );

  // Объект состояния GtkCifroArea.
  if( GTK_IS_CIFRO_AREA( priv->area ) ) priv->state = gtk_cifro_area_get_state( GTK_CIFRO_AREA( priv->area ) );

  // Информация об осях.
  priv->x_axis_name = g_strdup( "X" );
  priv->y_axis_name = g_strdup( "Y" );

  // Каналы осциллографа.
  priv->channels = g_hash_table_new_full( g_direct_hash, g_direct_equal, NULL, (GDestroyNotify)gtk_cifro_scope_renderer_free_channel );

  // Координаты информационной точки.
  priv->pointer_x = -1;
  priv->pointer_y = -1;

  // Обработчики сигналов GtkCifroArea и GtkCifroImage.
  if( priv->area )
    {
    g_signal_connect( priv->area, "area-draw", G_CALLBACK( gtk_cifro_scope_renderer_area_draw ), priv );
    g_signal_connect( priv->area, "visible-draw", G_CALLBACK( gtk_cifro_scope_renderer_visible_draw ), priv );
    g_signal_connect( priv->area, "check-area-redraw", G_CALLBACK( gtk_cifro_scope_renderer_check_area_redraw ), priv );
    g_signal_connect( priv->area, "check-visible-redraw", G_CALLBACK( gtk_cifro_scope_renderer_check_visible_redraw ), priv );
    }

  // Обработчики сигналов состояния GtkCifroArea.
  if( priv->state )
    {
    g_signal_connect( priv->state, "area-changed", G_CALLBACK( gtk_cifro_scope_renderer_area_changed ), priv );
    g_signal_connect( priv->state, "visible-changed", G_CALLBACK( gtk_cifro_scope_renderer_visible_changed ), priv );
    g_signal_connect( priv->state, "border-changed", G_CALLBACK( gtk_cifro_scope_renderer_border_changed ), priv );
    g_signal_connect( priv->state, "swap-changed", G_CALLBACK( gtk_cifro_scope_renderer_swap_changed ), priv );
    g_signal_connect( priv->state, "angle-changed", G_CALLBACK( gtk_cifro_scope_renderer_angle_changed ), priv );
    g_signal_connect( priv->state, "view-limits-changed", G_CALLBACK( gtk_cifro_scope_renderer_view_limits_changed ), priv );
    g_signal_connect( priv->state, "view-changed", G_CALLBACK( gtk_cifro_scope_renderer_view_changed ), priv );
    }

  return scope_renderer;

}


static void gtk_cifro_scope_renderer_object_finalize( GObject *scope_renderer )
{

  GtkCifroScopeRendererPriv *priv = GTK_CIFRO_SCOPE_RENDERER_GET_PRIVATE( scope_renderer );

  if( priv->font ) g_object_unref( priv->font );
  g_hash_table_unref( priv->channels );
  g_free( priv->x_axis_name );
  g_free( priv->y_axis_name );

  G_OBJECT_CLASS( gtk_cifro_scope_renderer_parent_class )->finalize( scope_renderer );

}


// Функция удаляет структуру с данными канала.
static void gtk_cifro_scope_renderer_free_channel( GtkCifroScopeChannel *channel )
{

  if( !channel ) return;
  g_free( channel->data );
  g_free( channel->name );
  g_free( channel );

}


// Callback функция вызывается при изменении размера области виджета.
static void gtk_cifro_scope_renderer_area_changed( GtkCifroAreaState *state, GtkCifroAreaSize *size, GtkCifroScopeRendererPriv *priv )
{

  priv->area_width = size->width;
  priv->area_height = size->height;

  priv->visible_update = TRUE;

}


// Callback функция вызывается при изменении "видимой" области.
static void gtk_cifro_scope_renderer_visible_changed( GtkCifroAreaState *state, GtkCifroAreaSize *size, GtkCifroScopeRendererPriv *priv )
{

  priv->visible_width = size->width;
  priv->visible_height = size->height;
  gtk_cifro_area_state_get_scale( priv->state, &priv->scale_x, &priv->scale_y );

  priv->visible_update = TRUE;

}


// Callback функция вызывается при изменении размера окантовки вокруг видимой области.
static void gtk_cifro_scope_renderer_border_changed( GtkCifroAreaState *state, GtkCifroAreaBorder *border, GtkCifroScopeRendererPriv *priv )
{

  priv->border_left = border->left;
  priv->border_right = border->right;
  priv->border_top = border->top;
  priv->border_bottom = border->bottom;

  priv->visible_update = TRUE;

}


// Callback функция вызывается при изменении параметров зеркального отражения по осям.
static void gtk_cifro_scope_renderer_swap_changed( GtkCifroAreaState *state, GtkCifroAreaSwap *swap, GtkCifroScopeRendererPriv *priv )
{

  priv->swap_x = swap->swap_x;
  priv->swap_y = swap->swap_y;

  priv->visible_update = TRUE;

}


// Callback функция вызывается при изменении угла поворота изображения "видимой" области.
static void gtk_cifro_scope_renderer_angle_changed( GtkCifroAreaState *state, gdouble angle, GtkCifroScopeRendererPriv *priv )
{

  priv->angle = angle;

  priv->visible_update = TRUE;

}


// Callback функция вызывается при изменении пределов отображения.
static void gtk_cifro_scope_renderer_view_limits_changed( GtkCifroAreaState *state, GtkCifroAreaView *view, GtkCifroScopeRendererPriv *priv )
{

  priv->min_x = view->x1;
  priv->max_x = view->x2;
  priv->min_y = view->y1;
  priv->max_y = view->y2;

  priv->visible_update = TRUE;

}


// Callback функция вызывается при изменении текущей границы отображения.
static void gtk_cifro_scope_renderer_view_changed( GtkCifroAreaState *state, GtkCifroAreaView *view, GtkCifroScopeRendererPriv *priv )
{

  priv->from_x = view->x1;
  priv->to_x = view->x2;
  priv->from_y = view->y1;
  priv->to_y = view->y2;
  gtk_cifro_area_state_get_scale( priv->state, &priv->scale_x, &priv->scale_y );

  priv->visible_update = TRUE;

}


// Рисование оцифровки оси абсцисс.
static void gtk_cifro_scope_renderer_draw_hruler( cairo_t *cairo, GtkCifroScopeRendererPriv *priv )
{

  GtkCifroAreaState *state = priv->state;
  PangoLayout *font = priv->font;

  gint     area_width = priv->area_width;

  gint     border_left = priv->border_left;
  gint     border_right = priv->border_right;
  gint     border_top = priv->border_top;

  gdouble  from_x = priv->from_x;
  gdouble  to_x = priv->to_x;
  gdouble  from_y = priv->from_y;
  gdouble  to_y = priv->to_y;

  gdouble  scale_x = priv->scale_x;
  gdouble  scale_y = priv->scale_y;

  gdouble  angle = priv->angle;

  gdouble  step;
  gdouble  axis;
  gdouble  axis_to;
  gdouble  axis_pos;
  gdouble  axis_from;
  gdouble  axis_step;
  gdouble  axis_scale;
  gdouble  axis_mini_step;
  gint     axis_range;
  gint     axis_power;
  gint     axis_height;
  gint     axis_count;

  gchar    text_format[128];
  gchar    text_str[ 128 ];

  gint     text_width;
  gint     text_height;

  gboolean swap = FALSE;

  // Проверяем состояние объекта.
  if( !priv->state || !priv->font ) return;

  step = 4.0 * border_top;

  if( angle < -0.01 || angle > G_PI / 2.0 + 0.01 ) return;
  if( fabs( angle - G_PI / 2.0 ) < G_PI / 4.0 ) swap = TRUE;

  axis_to = swap ? to_y : to_x;
  axis_from = swap ? from_y : from_x;
  axis_scale = swap ? scale_y : scale_x;

  gtk_cifro_area_state_get_axis_step( axis_scale, step, &axis_from, &axis_step, &axis_range, &axis_power );

  if( axis_range == 1 ) axis_range = 10;
  if( axis_range == 2 ) axis_mini_step = axis_step / 2.0;
  else if( axis_range == 5 ) axis_mini_step = axis_step / 5.0;
  else axis_mini_step = axis_step / 10.0;

  // Рисуем засечки на осях.
  cairo_sdline_set_cairo_color( cairo, priv->zero_axis_color );
  cairo_set_line_width( cairo, 1.0 );

  axis_count = 0;
  axis = axis_from - axis_step;
  while( axis <= axis_to )
    {

    if( swap )
      gtk_cifro_area_state_value_to_point( state, &axis_pos, NULL, from_x, axis );
    else
      gtk_cifro_area_state_value_to_point( state, &axis_pos, NULL, axis, 0.0 );

    if( axis_count % axis_range == 0 ) axis_height = border_top / 4.0;
    else axis_height = border_top / 8.0;
    axis_count += 1;

    axis += axis_mini_step;
    if( axis_pos <= border_left + 1 ) continue;
    if( axis_pos >= area_width - border_right - 1 ) continue;

    cairo_move_to( cairo, (gint)axis_pos + 0.5, border_top + 0.5 );
    cairo_line_to( cairo, (gint)axis_pos + 0.5, border_top - axis_height + 0.5 );

    }

  cairo_stroke( cairo );

  // Рисуем подписи на оси.
  cairo_sdline_set_cairo_color( cairo, priv->text_color );

  if( axis_power > 0 ) axis_power = 0;
  g_snprintf( text_format, sizeof( text_format ), "%%.%df", (gint)fabs( axis_power ) );

  axis = axis_from;
  while( axis <= axis_to )
    {

    g_ascii_formatd( text_str, sizeof( text_str ), text_format, axis );
    pango_layout_set_text( font, text_str, -1 );
    pango_layout_get_size( font, &text_width, &text_height );
    text_width /= PANGO_SCALE;
    text_height /= PANGO_SCALE;

    if( swap )
      gtk_cifro_area_state_value_to_point( state, &axis_pos, NULL, from_x, axis );
    else
      gtk_cifro_area_state_value_to_point( state, &axis_pos, NULL, axis, 0.0 );
    axis_pos -= text_width / 2;
    axis += axis_step;

    if( axis_pos < border_left + 1 ) continue;
    if( axis_pos + text_width > area_width - border_right - 1 ) continue;

    cairo_move_to( cairo, axis_pos, ( ( 0.85 * border_top ) - text_height ) / 2.0 );
    pango_cairo_show_layout( cairo, font );

    }

  // Рисуем название оси.
  if( swap )
    pango_layout_set_text( font, priv->y_axis_name, -1 );
  else
    pango_layout_set_text( font, priv->x_axis_name, -1 );

  pango_layout_get_size( font, &text_width, &text_height );
  text_width /= PANGO_SCALE;
  text_height /= PANGO_SCALE;

  cairo_move_to( cairo, area_width - border_right / 2  - text_width / 2, border_top / 2 - text_height / 2 );
  pango_cairo_show_layout( cairo, font );

}


// Рисование оцифровки оси ординат.
static void gtk_cifro_scope_renderer_draw_vruler( cairo_t *cairo, GtkCifroScopeRendererPriv *priv )
{

  GtkCifroAreaState *state = priv->state;
  PangoLayout *font = priv->font;

  gint     area_height = priv->area_height;

  gint     border_left = priv->border_left;
  gint     border_top = priv->border_top;
  gint     border_bottom = priv->border_bottom;

  gdouble  from_x = priv->from_x;
  gdouble  to_x = priv->to_x;
  gdouble  from_y = priv->from_y;
  gdouble  to_y = priv->to_y;

  gdouble  scale_x = priv->scale_x;
  gdouble  scale_y = priv->scale_y;

  gdouble  angle = priv->angle;

  gdouble  step;
  gdouble  axis;
  gdouble  axis_to;
  gdouble  axis_pos;
  gdouble  axis_from;
  gdouble  axis_step;
  gdouble  axis_scale;
  gdouble  axis_mini_step;
  gint     axis_range;
  gint     axis_power;
  gint     axis_height;
  gint     axis_count;

  gchar    text_format[128];
  gchar    text_str[ 128 ];

  gint     text_width;
  gint     text_height;

  gboolean swap = FALSE;

  // Проверяем состояние объекта.
  if( !priv->state || !priv->font ) return;

  step = 4.0 * border_top;

  if( angle < -0.01 || angle > G_PI / 2.0 + 0.01 ) return;
  if( fabs( angle - G_PI / 2.0 ) < G_PI / 4.0 ) swap = TRUE;

  axis_to = swap ? to_x : to_y;
  axis_from = swap ? from_x : from_y;
  axis_scale = swap ? scale_x : scale_y;

  gtk_cifro_area_state_get_axis_step( axis_scale, step, &axis_from, &axis_step, &axis_range, &axis_power );

  if( axis_range == 1 ) axis_range = 10;
  if( axis_range == 2 ) axis_mini_step = axis_step / 2.0;
  else if( axis_range == 5 ) axis_mini_step = axis_step / 5.0;
  else axis_mini_step = axis_step / 10.0;

  // Рисуем засечки на осях.
  cairo_sdline_set_cairo_color( cairo, priv->zero_axis_color );
  cairo_set_line_width( cairo, 1.0 );

  axis_count = 0;
  axis = axis_from - axis_step;
  while( axis <= axis_to )
    {

    if( swap )
      gtk_cifro_area_state_value_to_point( state, NULL, &axis_pos, axis, to_y );
    else
      gtk_cifro_area_state_value_to_point( state, NULL, &axis_pos, 0.0, axis );

    if( axis_count % axis_range == 0 ) axis_height = border_top / 4.0;
    else axis_height = border_top / 8.0;
    axis_count += 1;

    axis += axis_mini_step;
    if( axis_pos <= border_top + 1) continue;
    if( axis_pos >= area_height - border_bottom - 1 ) continue;

    cairo_move_to( cairo, border_left - axis_height + 0.5, (gint)axis_pos + 0.5 );
    cairo_line_to( cairo, border_left + 0.5, (gint)axis_pos + 0.5 );

    }

  cairo_stroke( cairo );

  // Рисуем подписи на оси.
  cairo_sdline_set_cairo_color( cairo, priv->text_color );

  if( axis_power > 0 ) axis_power = 0;
  g_snprintf( text_format, sizeof( text_format ), "%%.%df", (gint)fabs( axis_power ) );

  axis = axis_from;
  while( axis <= axis_to )
    {

    g_ascii_formatd( text_str, sizeof( text_str ), text_format, axis );
    pango_layout_set_text( font, text_str, -1 );
    pango_layout_get_size( font, &text_width, &text_height );
    text_width /= PANGO_SCALE;
    text_height /= PANGO_SCALE;

    if( swap )
      gtk_cifro_area_state_value_to_point( state, NULL, &axis_pos, axis, to_y );
    else
      gtk_cifro_area_state_value_to_point( state, NULL, &axis_pos, 0.0, axis );
    axis_pos += text_width / 2;
    axis += axis_step;

    if( axis_pos - text_width <= border_top + 1) continue;
    if( axis_pos >= area_height - border_bottom - 1 ) continue;

    cairo_save( cairo );
    cairo_move_to( cairo, 0.15 * border_left, axis_pos );
    cairo_rotate( cairo, - G_PI / 2.0 );
    pango_cairo_show_layout( cairo, font );
    cairo_restore( cairo );

    }

  // Рисуем название оси.
  if( swap )
    pango_layout_set_text( font, priv->x_axis_name, -1 );
  else
    pango_layout_set_text( font, priv->y_axis_name, -1 );
  pango_layout_get_size( font, &text_width, &text_height );
  text_width /= PANGO_SCALE;
  text_height /= PANGO_SCALE;

  cairo_move_to( cairo, border_left / 2  - text_width / 2, area_height - border_top / 2 - text_height / 2 );
  pango_cairo_show_layout( cairo, font );

}


// Рисование "планшета" горизонтального местоположения видимой области.
static void gtk_cifro_scope_renderer_draw_x_pos( cairo_t *cairo, GtkCifroScopeRendererPriv *priv )
{

  gint     area_width = priv->area_width;
  gint     area_height = priv->area_height;

  gint     border_left = priv->border_left;
  gint     border_right = priv->border_right;
  gint     border_bottom = priv->border_bottom;

  gdouble  min_x = priv->min_x;
  gdouble  max_x = priv->max_x;
  gdouble  min_y = priv->min_y;
  gdouble  max_y = priv->max_y;

  gdouble  from_x = priv->from_x;
  gdouble  to_x = priv->to_x;
  gdouble  from_y = priv->from_y;
  gdouble  to_y = priv->to_y;

  gboolean swap_x = priv->swap_x;
  gboolean swap_y = priv->swap_y;

  gdouble  angle = priv->angle;

  gdouble  from;
  gdouble  to;

  gint     x;
  gint     y;
  gint     width;
  gint     height;

  gboolean swap = FALSE;

  // Проверяем состояние объекта.
  if( !priv->state ) return;

  if( fabs( angle - G_PI / 2.0 ) < G_PI / 4.0 ) swap = TRUE;

  if( swap )
    {
    from = ( from_y - min_y ) / ( max_y - min_y );
    to = ( to_y - min_y ) / ( max_y - min_y );
    if( swap_y ) { from = 1.0 - from; to = 1.0 - to; }
    }
  else
    {
    from = ( from_x - min_x ) / ( max_x - min_x );
    to = ( to_x - min_x ) / ( max_x - min_x );
    if( swap_x ) { from = 1.0 - from; to = 1.0 - to; }
    }

  x = border_left + 0.5;
  y = area_height - 0.75 * border_bottom + 0.5;
  width = area_width - border_left - border_right;
  height = border_bottom / 2;

  cairo_sdline_set_cairo_color( cairo, priv->axis_color );
  cairo_rectangle( cairo, x, y, width, height );
  cairo_fill( cairo );

  cairo_sdline_set_cairo_color( cairo, priv->zero_axis_color );
  cairo_rectangle( cairo, x + ( from * width ), y, ( ( to - from ) * width ), height );
  cairo_fill( cairo );

}


// Рисование "планшета" вертикального местоположения видимой области.
static void gtk_cifro_scope_renderer_draw_y_pos( cairo_t *cairo, GtkCifroScopeRendererPriv *priv )
{

  gint     area_width = priv->area_width;
  gint     area_height = priv->area_height;

  gint     border_left = priv->border_left;
  gint     border_top = priv->border_top;
  gint     border_bottom = priv->border_bottom;

  gdouble  min_x = priv->min_x;
  gdouble  max_x = priv->max_x;
  gdouble  min_y = priv->min_y;
  gdouble  max_y = priv->max_y;

  gdouble  from_x = priv->from_x;
  gdouble  to_x = priv->to_x;
  gdouble  from_y = priv->from_y;
  gdouble  to_y = priv->to_y;

  gboolean swap_x = priv->swap_x;
  gboolean swap_y = priv->swap_y;

  gdouble  angle = priv->angle;

  gdouble   from;
  gdouble   to;

  gint      x;
  gint      y;
  gint      width;
  gint      height;

  gboolean  swap = FALSE;

  if( fabs( angle - G_PI / 2.0 ) < G_PI / 4.0 ) swap = TRUE;

  if( swap )
    {
    from = 1.0 - ( from_x - min_x ) / ( max_x - min_x );
    to = 1.0 - ( to_x - min_x ) / ( max_x - min_x );
    if( !swap_x ) { from = 1.0 - from; to = 1.0 - to; }
    }
  else
    {
    from = 1.0 - ( from_y - min_y ) / ( max_y - min_y );
    to = 1.0 - ( to_y - min_y ) / ( max_y - min_y );
    if( swap_y ) { from = 1.0 - from; to = 1.0 - to; }
    }

  x = area_width - 0.75 * border_left + 0.5;
  y = border_top;
  width = border_left / 2;
  height = area_height - border_top - border_bottom;

  cairo_sdline_set_cairo_color( cairo, priv->axis_color );
  cairo_rectangle( cairo, x, y, width, height );
  cairo_fill( cairo );

  cairo_sdline_set_cairo_color( cairo, priv->zero_axis_color );
  cairo_rectangle( cairo, x, y + ( from * height ), width, ( ( to - from ) * height ) );
  cairo_fill( cairo );

}


// Рисование информационного блока.
static void gtk_cifro_scope_renderer_draw_info( cairo_t *cairo, GtkCifroScopeRendererPriv *priv )
{

  GHashTableIter channels_iter;
  GtkCifroScopeChannel *channel;

  PangoLayout *font = priv->font;

  gint     area_width = priv->area_width;
  gint     area_height = priv->area_height;

  gint     border_top = priv->border_top;

  gdouble  scale_x = priv->scale_x;
  gdouble  scale_y = priv->scale_y;

  gdouble  from_x = priv->from_x;
  gdouble  to_x = priv->to_x;
  gdouble  from_y = priv->from_y;
  gdouble  to_y = priv->to_y;

  gint     n_labels;

  gint     mark_width;
  gint     label_width;
  gint     font_height;

  gint     info_center;
  gint     info_width;
  gint     info_height;

  gint     x1, y1;

  gint     label_top;

  gdouble  value;
  gdouble  value_x;
  gdouble  value_y;

  gint     value_power;
  gchar    text_format[ 128 ];
  gchar    text_str[ 128 ];

  gint     text_width;
  gint     text_height;
  gint     text_spacing = border_top / 4;

  // Значения под курсором.
  if( priv->pointer_x < 0 || priv->pointer_y < 0 ) return;
  gtk_cifro_area_state_point_to_value( priv->state, priv->pointer_x, priv->pointer_y, &value_x, &value_y );

  // Вычисляем максимальную ширину и высоту строки с текстом.
  pango_layout_set_text( font, priv->x_axis_name, -1 );
  pango_layout_get_size( font, &text_width, &text_height );
  mark_width = text_width;
  font_height = text_height;

  pango_layout_set_text( font, priv->y_axis_name, -1 );
  pango_layout_get_size( font, &text_width, &text_height );
  if( text_width > mark_width ) mark_width = text_width;
  if( text_height > font_height ) font_height = text_height;

  n_labels = 2;
  label_width = 0;
  g_hash_table_iter_init( &channels_iter, priv->channels );
  while( g_hash_table_iter_next( &channels_iter, NULL, (gpointer)&channel ) )
    {

    if( channel->value_scale == 1.0 && !channel->name && label_width != 0 ) continue;

    value = value_y / channel->value_scale - channel->value_shift;
    gtk_cifro_area_state_get_axis_step( scale_y, 1, &value, NULL, NULL, &value_power );
    if( value_power > 0 ) value_power = 0;
    g_snprintf( text_format, sizeof( text_format ), "-%%.%df", (gint)fabs( value_power ) );
    value = MAX( ABS( from_y ), ABS( to_y ) );
    value = value / channel->value_scale - channel->value_shift;
    g_snprintf( text_str, sizeof( text_str ), text_format, value );

    if( channel->name )
      {
      pango_layout_set_text( font, channel->name, -1 );
      pango_layout_get_size( font, &text_width, &text_height );
      if( text_width > mark_width ) mark_width = text_width;
      if( text_height > font_height ) font_height = text_height;
      }

    pango_layout_set_text( font, text_str, -1 );
    pango_layout_get_size( font, &text_width, &text_height );
    if( text_width > label_width ) label_width = text_width;
    if( text_height > font_height ) font_height = text_height;

    if( channel->value_scale != 1.0 || channel->name ) n_labels += 1;

    }

  value = value_x;
  gtk_cifro_area_state_get_axis_step( scale_x, 1, &value, NULL, NULL, &value_power );
  if( value_power > 0 ) value_power = 0;
  g_snprintf( text_format, sizeof( text_format ), "-%%.%df", (gint)fabs( value_power ) );
  g_snprintf( text_str, sizeof( text_str ), text_format, MAX( ABS( from_x ), ABS( to_x ) ) );

  pango_layout_set_text( font, text_str, -1 );
  pango_layout_get_size( font, &text_width, &text_height );
  if( text_width > label_width ) label_width = text_width;
  if( text_height > font_height ) font_height = text_height;

  // Ширина текста с названием величины, с её значением и высота строки.
  mark_width /= PANGO_SCALE;
  label_width /= PANGO_SCALE;
  font_height /= PANGO_SCALE;

  // Размер места для отображения информации.
  info_width = 5 * text_spacing + label_width + mark_width;
  info_height = n_labels * ( font_height + text_spacing ) + 3 * text_spacing;
  if( n_labels > 2 ) info_height += text_spacing;

  // Проверяем размеры области отображения.
  if( info_width > area_width - 12 * text_spacing ) return;
  if( info_height > area_height - 12 * text_spacing ) return;

  // Место для отображения информации.
  if( priv->pointer_x > area_width - 8 * text_spacing - info_width &&
      priv->pointer_y < 8 * text_spacing + info_height )
    x1 = 6 * text_spacing;
  else
    x1 = area_width - 6 * text_spacing - info_width;
  y1 = 6 * text_spacing;

  cairo_set_line_width( cairo, 1.0 );

  cairo_set_source_rgba( cairo, 0.0, 0.0, 0.0, 0.25 );
  cairo_rectangle( cairo, x1 + 0.5, y1 + 0.5, info_width, info_height );
  cairo_fill( cairo );

  cairo_sdline_set_cairo_color( cairo, priv->axis_color );
  cairo_rectangle( cairo, x1 + 0.5, y1 + 0.5, info_width, info_height );
  cairo_stroke( cairo );

  cairo_sdline_set_cairo_color( cairo, priv->text_color );

  label_top = y1 + 2 * text_spacing;
  info_center = x1 + 3 * text_spacing + label_width;

  // Значение по оси абсцисс.
  value = value_x;
  gtk_cifro_area_state_get_axis_step( scale_x, 1, &value, NULL, NULL, &value_power );
  if( value_power > 0 ) value_power = 0;
  g_snprintf( text_format, sizeof( text_format ), "%%.%df", (gint)fabs( value_power ) );
  g_ascii_formatd( text_str, sizeof( text_str ), text_format, value_x );

  pango_layout_set_text( font, text_str, -1 );
  pango_layout_get_size( font, &text_width, &text_height );
  cairo_move_to( cairo, info_center - text_width / PANGO_SCALE - text_spacing / 2, label_top );
  pango_cairo_show_layout( cairo, font );

  pango_layout_set_text( font, priv->x_axis_name, -1 );
  cairo_move_to( cairo, info_center, label_top );
  pango_cairo_show_layout( cairo, font );
  label_top += font_height + text_spacing;

  // Значение по оси ординат.
  value = value_y;
  gtk_cifro_area_state_get_axis_step( scale_y, 1, &value, NULL, NULL, &value_power );
  if( value_power > 0 ) value_power = 0;
  g_snprintf( text_format, sizeof( text_format ), "%%.%df", (gint)fabs( value_power ) );
  g_ascii_formatd( text_str, sizeof( text_str ), text_format, value_y );

  pango_layout_set_text( font, text_str, -1 );
  pango_layout_get_size( font, &text_width, &text_height );
  cairo_move_to( cairo, info_center - text_width / PANGO_SCALE - text_spacing / 2, label_top );
  pango_cairo_show_layout( cairo, font );

  pango_layout_set_text( font, priv->y_axis_name, -1 );
  cairo_move_to( cairo, info_center, label_top );
  pango_cairo_show_layout( cairo, font );

  // Значения для каналов с отличным от 1 масштабом.
  if( n_labels > 2 )
    {

    label_top += font_height + text_spacing;
    cairo_sdline_set_cairo_color( cairo, priv->axis_color );
    cairo_move_to( cairo, x1 + 4.5, label_top + 0.5 );
    cairo_line_to( cairo, x1 + info_width - 7.5, label_top + 0.5 );
    cairo_stroke( cairo );

    label_top += text_spacing;

    g_hash_table_iter_init( &channels_iter, priv->channels );
    while( g_hash_table_iter_next( &channels_iter, NULL, (gpointer)&channel ) )
      {

      if( channel->value_scale == 1.0 && !channel->name ) continue;

      value = ( value_y - channel->value_shift ) / channel->value_scale;
      gtk_cifro_area_state_get_axis_step( scale_y, 1, &value, NULL, NULL, &value_power );
      if( value_power > 0 ) value_power = 0;
      g_snprintf( text_format, sizeof( text_format ), "%%.%df", (gint)fabs( value_power ) );
      g_ascii_formatd( text_str, sizeof( text_str ), text_format, ( value_y - channel->value_shift ) / channel->value_scale );

      cairo_sdline_set_cairo_color( cairo, channel->color );

      pango_layout_set_text( font, text_str, -1 );
      pango_layout_get_size( font, &text_width, &text_height );
      cairo_move_to( cairo, info_center - text_width / PANGO_SCALE - text_spacing / 2, label_top );
      pango_cairo_show_layout( cairo, font );

      pango_layout_set_text( font, channel->name ? channel->name : priv->y_axis_name, -1 );
      cairo_move_to( cairo, info_center, label_top );
      pango_cairo_show_layout( cairo, font );

      label_top += font_height + text_spacing;

      }

    }

}


// Рисование координатных линий в области осциллограмм.
static void gtk_cifro_scope_renderer_draw_axis( cairo_sdline_surface *surface, GtkCifroScopeRendererPriv *priv )
{

  GtkCifroAreaState *state = priv->state;

  gint     border_top = priv->border_top;

  gdouble  scale_x = priv->scale_x;
  gdouble  scale_y = priv->scale_y;

  gdouble  from_x = priv->from_x;
  gdouble  to_x = priv->to_x;
  gdouble  from_y = priv->from_y;
  gdouble  to_y = priv->to_y;

  gdouble  step;
  gdouble  axis;
  gdouble  axis_pos;
  gdouble  axis_step;

  step = 4.0 * border_top;

  // Проверяем граничные условия.
  if( border_top == 0 || scale_x == 0.0 || scale_y == 0.0 ) return;
  if( from_x == to_x || from_y == to_y ) return;

  // Сетка по оси X.
  axis = from_x;
  gtk_cifro_area_state_get_axis_step( scale_x, step, &axis, &axis_step, NULL, NULL );
  while( axis <= to_x )
    {
    gtk_cifro_area_state_visible_value_to_point( state, &axis_pos, NULL, axis, 0.0 );
    cairo_sdline_v( surface, axis_pos, 0, surface->height, priv->axis_color );
    axis += axis_step;
    }

  // "Нулевая" ось X.
  gtk_cifro_area_state_visible_value_to_point( state, &axis_pos, NULL, 0.0, 0.0 );
  cairo_sdline_v( surface, axis_pos, 0, surface->height, priv->zero_axis_color );

  // Сетка по оси Y.
  axis = from_y;
  gtk_cifro_area_state_get_axis_step( scale_y, step, &axis, &axis_step, NULL, NULL );
  while( axis <= to_y )
    {
    gtk_cifro_area_state_visible_value_to_point( state, NULL, &axis_pos, 0.0, axis );
    cairo_sdline_h( surface, 0, surface->width, axis_pos, priv->axis_color );
    axis += axis_step;
    }

  // "Нулевая" ось Y.
  gtk_cifro_area_state_visible_value_to_point( state, NULL, &axis_pos, 0.0, 0.0 );
  cairo_sdline_h( surface, 0, surface->width, axis_pos, priv->zero_axis_color );

  cairo_surface_mark_dirty( surface->cairo_surface );

}




// Функция рисования осциллограмм линиями.
static void gtk_cifro_scope_renderer_draw_lined_data( cairo_sdline_surface *surface, GtkCifroScopeRendererPriv *priv, gpointer channel_id )
{

  GtkCifroScopeChannel *channel = g_hash_table_lookup( priv->channels, channel_id );

  gint     visible_width = priv->visible_width;

  gdouble  from_x = priv->from_x;
  gdouble  to_x = priv->to_x;
  gdouble  to_y = priv->to_y;

  gdouble  scale_x = priv->scale_x;
  gdouble  scale_y = priv->scale_y;

  gfloat  *values_data;
  gint     values_num;
  gfloat   times_shift;
  gfloat   times_step;
  gfloat   values_scale;
  gfloat   values_shift;
  guint32  values_color;

#define VALUES_DATA(i) ( ( values_data[i] * values_scale ) + values_shift )

  gint     i, j;
  gint     i_range_begin, i_range_end;
  gfloat   x_range_begin, x_range_end;
  gfloat   y_start, y_end;
  gfloat   x1, x2, y1, y2;
  gboolean draw = FALSE;

  // Проверяем существование канала.
  if( !channel ) return;

  values_data = channel->data;
  values_num = channel->num;
  times_shift = channel->time_shift;
  times_step = channel->time_step;
  values_scale = channel->value_scale;
  values_shift = channel->value_shift;
  values_color = channel->color;

  x_range_begin = from_x - scale_x;
  x_range_end = from_x;

  x1 = -1.0; y1 = -1.0;
  y_start = y_end = 0.0;

  for( i = 0; i <= visible_width; i++ )
    {

    draw = FALSE;
    x2 = i;

    // Диапазон значений X между двумя точками осциллограммы.
    x_range_begin += scale_x;
    x_range_end += scale_x;

    // Диапазон индексов значений между двумя точками осциллограммы.
    i_range_begin = ( x_range_begin - times_shift ) / times_step;
    i_range_end = ( x_range_end - times_shift ) / times_step;

    // Проверка индексов на попадание в границы осциллограммы.
    if( ( i_range_begin < 0 ) && ( i_range_end <= 0 ) ) continue;
    if( i_range_begin >= values_num ) break;

    if( i_range_begin < 0 ) i_range_begin = 0;
    if( i_range_end >= values_num ) i_range_end = values_num - 1;

    if( ( i_range_end == i_range_begin ) && ( i != visible_width ) ) continue;

    // Последня точка не попала в границу осциллограммы.
    if( i_range_end == i_range_begin )
      {

      if( i_range_begin == 0 ) continue;

      if( i_range_end == values_num - 1 ) break;

      if( isnan( values_data[ i_range_begin ] ) || isnan( values_data[ i_range_end + 1 ] ) ) continue;

      // Предыдущей точки нет, расчитаем значение и расстояние до нее от начала видимой осциллограммы.
      // В этом случае все точки лежат за границей видимой области.
      if( x1 < 0 )
        {
        x1 = ( ( times_step * floor( from_x / times_step ) ) - from_x ) / scale_x;
        y1 = ( to_y - VALUES_DATA( i_range_begin ) ) / scale_y;
        x1 = CLAMP( x1, -32000.0, 32000.0 );
        y1 = CLAMP( y1, -32000.0, 32000.0 );
        }

      // Значение и расстояние до текущей точки от конца видимой осциллограммы.
      x2 = visible_width + ( ( times_step * ceil( to_x / times_step ) ) - to_x ) / scale_x;
      y2 = ( to_y - VALUES_DATA( i_range_end + 1 ) ) / scale_y;
      x2 = CLAMP( x2, -32000.0, 32000.0 );
      y2 = CLAMP( y2, -32000.0, 32000.0 );
      draw = TRUE;

      }

    // Индекс для этой точки осциллограммы изменился на 1.
    // Необходимо нарисовать линию из предыдущей точки в текущую.
    else if( ( i_range_end - i_range_begin ) == 1 )
      {

      // Предыдущей точки нет, расчитаеborderм значение и расстояние до нее от текущей позиции.
      if( ( x1 < 0 ) && ( i_range_begin >= 0 ) )
        {
        x1 = (gfloat)i - ( times_step / scale_x ) + 1.0;
        y1 = ( to_y - VALUES_DATA( i_range_begin ) ) / scale_y;
        x1 = CLAMP( x1, -32000.0, 32000.0 );
        y1 = CLAMP( y1, -32000.0, 32000.0 );
        }

      y2 = ( to_y - VALUES_DATA( i_range_end ) ) / scale_y;
      y2 = CLAMP( y2, -32000.0, 32000.0 );
      if( !isnan( values_data[ i_range_begin ] ) && !isnan( values_data[ i_range_end ] ) ) draw = TRUE;

      }

    // В одну точку осциллограммы попадает несколько значений.
    // Берем максимум и минимум из них и рисуем вертикальной линией.
    else
      {

      // Нарисуем линию от предыдущей точки.
      if( !isnan( values_data[ i_range_begin ] ) && !isnan( values_data[ i_range_begin + 1 ] ) )
        {
        y1 = ( to_y - VALUES_DATA( i_range_begin ) ) / scale_y;
        y2 = ( to_y - VALUES_DATA( i_range_begin + 1 ) ) / scale_y;
        y1 = CLAMP( y1, -32000.0, 32000.0 );
        y2 = CLAMP( y2, -32000.0, 32000.0 );
        cairo_sdline( surface, x2 - 1, y1, x2, y2, values_color );
        draw = TRUE;
        }

      for( j = i_range_begin + 1; j <= i_range_end; j++ )
        {
        if( isnan( values_data[j] ) ) continue;
        y_start = VALUES_DATA( j );
        y_end = y_start;
        draw = TRUE;
        break;
        }
      for( ; j <= i_range_end; j++ )
        {
        if( isnan( values_data[j] ) ) continue;
        if( VALUES_DATA( j ) < y_start ) y_start = VALUES_DATA( j );
        if( VALUES_DATA( j ) > y_end ) y_end = VALUES_DATA( j );
        draw = TRUE;
        }

      x1 = i;
      y1 = ( to_y - y_start ) / scale_y;
      y2 = ( to_y - y_end ) / scale_y;
      y1 = CLAMP( y1, -32000.0, 32000.0 );
      y2 = CLAMP( y2, -32000.0, 32000.0 );

      }

    if( draw )
      cairo_sdline( surface, x1, y1, x2, y2, values_color );

    if( ( i_range_end - i_range_begin ) == 1 )
      { x1 = x2; y1 = y2; }
    else
      { x1 = -1; y1 = -1; }

    }

  cairo_surface_mark_dirty( surface->cairo_surface );

}


// Функция рисования осциллограмм точками.
static void gtk_cifro_scope_renderer_draw_dotted_data( cairo_sdline_surface *surface, GtkCifroScopeRendererPriv *priv, gpointer channel_id )
{

  GtkCifroScopeChannel *channel = g_hash_table_lookup( priv->channels, GINT_TO_POINTER( channel_id ) );

  gdouble  from_x = priv->from_x;
  gdouble  to_x = priv->to_x;
  gdouble  to_y = priv->to_y;

  gdouble  scale_x = priv->scale_x;
  gdouble  scale_y = priv->scale_y;

  gfloat  *values_data;
  gint     values_num;
  gfloat   times_shift;
  gfloat   times_step;
  gfloat   values_scale;
  gfloat   values_shift;
  guint32  values_color;

#define VALUES_TIME(i) ( ( i * times_step ) + times_shift )
#define VALUES_DATA(i) ( ( values_data[i] * values_scale ) + values_shift )

  gint     i;
  gint     i_range_begin, i_range_end;
  gfloat   x, y;

  // Проверяем существование канала.
  if( !channel ) return;

  values_data = channel->data;
  values_num = channel->num;
  times_shift = channel->time_shift;
  times_step = channel->time_step;
  values_scale = channel->value_scale;
  values_shift = channel->value_shift;
  values_color = channel->color;

  i_range_begin = ( from_x - times_shift ) / times_step;
  i_range_end = ( to_x - times_shift ) / times_step;

  i_range_begin = CLAMP( i_range_begin, 0, values_num );
  i_range_end = CLAMP( i_range_end, 0, values_num );

  if( i_range_begin > i_range_end ) return;

  for( i = i_range_begin; i < i_range_end; i++ )
    {
    if( isnan( values_data[i] ) ) continue;
    x = VALUES_TIME( i );
    x = ( x - from_x ) / scale_x;
    y = VALUES_DATA( i );
    y = ( to_y - y ) / scale_y;
    cairo_sdline_dot( surface, x, y, values_color );
    }

  cairo_surface_mark_dirty( surface->cairo_surface );

}


// Callback функция проверки необходимости перерисовки области виджета.
static gboolean gtk_cifro_scope_renderer_check_area_redraw( GtkWidget *widget, GtkCifroScopeRendererPriv *priv )
{

  if( !priv->area_update && !priv->visible_update ) return FALSE;
  priv->area_update = FALSE;
  return TRUE;

}


// Callback функция проверки необходимости перерисовки видимой области.
static gboolean gtk_cifro_scope_renderer_check_visible_redraw( GtkWidget *widget, GtkCifroScopeRendererPriv *priv )
{

  if( !priv->visible_update ) return FALSE;
  priv->visible_update = FALSE;
  return TRUE;

}


// Функция рисования оцифровки осей и информации.
static void gtk_cifro_scope_renderer_area_draw( GtkWidget *widget, cairo_t *cairo, GtkCifroScopeRendererPriv *priv )
{

  gtk_cifro_scope_renderer_draw_hruler( cairo, priv );
  gtk_cifro_scope_renderer_draw_vruler( cairo, priv );
  gtk_cifro_scope_renderer_draw_x_pos( cairo, priv );
  gtk_cifro_scope_renderer_draw_y_pos( cairo, priv );
  if( priv->show_info ) gtk_cifro_scope_renderer_draw_info( cairo, priv );

}


// Функция рисования видимой области (осциллограмм).
static void gtk_cifro_scope_renderer_visible_draw( GtkWidget *widget, cairo_t *cairo, GtkCifroScopeRendererPriv *priv )
{

  GHashTableIter channels_iter;
  GtkCifroScopeChannel *channel;
  gpointer channel_id;

  gint width, height;

  cairo_sdline_surface *surface = cairo_sdline_surface_create_for( cairo_get_target( cairo ) );

  // Рисуем оси.
  gtk_cifro_scope_renderer_draw_axis( surface, priv );

  // Рисуем осциллограммы.
  g_hash_table_iter_init( &channels_iter, priv->channels );
  while( g_hash_table_iter_next( &channels_iter, &channel_id, (gpointer)&channel ) )
    {
    if( channel->show )
      {
      if( channel->draw_type == GTK_CIFRO_SCOPE_LINED )
        gtk_cifro_scope_renderer_draw_lined_data( surface, priv, channel_id );
      else if( channel->draw_type == GTK_CIFRO_SCOPE_DOTTED )
        gtk_cifro_scope_renderer_draw_dotted_data( surface, priv, channel_id );
      }
    }

  // Рисуем окантовку.
  gtk_cifro_area_state_get_visible_size( priv->state, &width, &height );
  cairo_sdline_h( surface, 0, width - 1, 0, priv->border_color );
  cairo_sdline_v( surface, 0, 0, height - 1, priv->border_color );
  cairo_sdline_h( surface, 0, width - 1, height - 1, priv->border_color );
  cairo_sdline_v( surface, width - 1, 0, height - 1, priv->border_color );

  cairo_sdline_surface_destroy( surface );

}


// Функция создаёт новый объект GtkCifroScopeRenderer.
GtkCifroScopeRenderer *gtk_cifro_scope_renderer_new( GObject *area )
{

  return g_object_new( GTK_TYPE_CIFRO_SCOPE_RENDERER, "area", area, NULL );

}


// Функция устанавливает раскладку шрифта для отображения текстовых надписей в осциллографе.
void gtk_cifro_scope_renderer_set_font( GtkCifroScopeRenderer *scope_renderer, PangoLayout *font )
{

  GtkCifroScopeRendererPriv *priv = GTK_CIFRO_SCOPE_RENDERER_GET_PRIVATE( scope_renderer );

  if( priv->font ) g_object_unref( priv->font );
  priv->font = font;

  priv->area_update = TRUE;

}


// Функция устанавливает основной цвет для отображения графических элементов в осциллографе.
void gtk_cifro_scope_renderer_set_fg_color( GtkCifroScopeRenderer *scope_renderer, gdouble red, gdouble green, gdouble blue, gdouble alpha )
{

  GtkCifroScopeRendererPriv *priv = GTK_CIFRO_SCOPE_RENDERER_GET_PRIVATE( scope_renderer );

  gdouble luminance;
  gdouble border_luminance;
  gdouble zero_axis_luminance;
  gdouble axis_luminance;

  // Яркость основного цвета.
  luminance = alpha * ( 0.2126 * red + 0.7152 * green + 0.0722 * blue );
  if( luminance < 0.2 ) luminance = 0.2;

  if( luminance < 0.5 ) // Светлая тема.
    {
    border_luminance = 0.8 * ( 1.0 - luminance );
    zero_axis_luminance = 0.9 * ( 1.0 - luminance );
    axis_luminance = 1.0 * ( 1.0 - luminance );
    }
  else // Тёмная тема.
    {
    border_luminance = 0.8 * luminance;
    zero_axis_luminance = 0.6 * luminance;
    axis_luminance = 0.4 * luminance;
    }

  priv->border_color = cairo_sdline_color( border_luminance, border_luminance, border_luminance, 1.0 );
  priv->zero_axis_color = cairo_sdline_color( zero_axis_luminance, zero_axis_luminance, zero_axis_luminance, 1.0 );
  priv->axis_color = cairo_sdline_color( axis_luminance, axis_luminance, axis_luminance, 1.0 );
  priv->text_color = cairo_sdline_color( red, green, blue, 1.0 );

  priv->visible_update = TRUE;

}


// Функция устанавливает координаты точки для отображения информации.
void gtk_cifro_scope_renderer_set_pointer( GtkCifroScopeRenderer *scope_renderer, gint pointer_x, gint pointer_y )
{

  GtkCifroScopeRendererPriv *priv = GTK_CIFRO_SCOPE_RENDERER_GET_PRIVATE( scope_renderer );

  priv->pointer_x = pointer_x;
  priv->pointer_y = pointer_y;

  priv->area_update = TRUE;

}


// Функция включает или выключает отображения блока с информацией о значениях под курсором.
void gtk_cifro_scope_renderer_set_info_show( GtkCifroScopeRenderer *scope_renderer, gboolean show )
{

  GtkCifroScopeRendererPriv *priv = GTK_CIFRO_SCOPE_RENDERER_GET_PRIVATE( scope_renderer );

  priv->show_info = show;

  priv->area_update = TRUE;

}


// Функция задаёт подписи к осям абсцисс и ординат.
void gtk_cifro_scope_renderer_set_axis_name( GtkCifroScopeRenderer *scope_renderer, const gchar *time_axis_name, const gchar *value_axis_name )
{

  GtkCifroScopeRendererPriv *priv = GTK_CIFRO_SCOPE_RENDERER_GET_PRIVATE( scope_renderer );

  g_free( priv->x_axis_name );
  g_free( priv->y_axis_name );

  priv->x_axis_name = g_strdup( time_axis_name );
  priv->y_axis_name = g_strdup( value_axis_name );

  priv->area_update = TRUE;

}


// Функция дабавляет канал отображения данных в осциллограф.
gpointer gtk_cifro_scope_renderer_add_channel( GtkCifroScopeRenderer *scope_renderer )
{

  GtkCifroScopeRendererPriv *priv = GTK_CIFRO_SCOPE_RENDERER_GET_PRIVATE( scope_renderer );
  GtkCifroScopeChannel* channel;
  gpointer channel_id;

  // Параметры канала по умолчанию.
  channel = g_new0( GtkCifroScopeChannel, 1 );
  channel->time_step = 1.0;
  channel->value_scale = 1.0;
  channel->color = cairo_sdline_color( g_random_double_range( 0.5, 1.0 ), g_random_double_range( 0.5, 1.0 ), g_random_double_range( 0.5, 1.0 ), 1.0 );
  channel->draw_type = GTK_CIFRO_SCOPE_LINED;

  // Генерируем новый идентификатор канала.
  do {
    channel_id = GINT_TO_POINTER( priv->next_channel_id++ );
  } while( !channel_id || g_hash_table_lookup( priv->channels, channel_id ) );

  g_hash_table_insert( priv->channels, channel_id, channel );

  return channel_id;

}


// Функция удаляет канал отображения данных из осциллографа.
void gtk_cifro_scope_renderer_remove_channel( GtkCifroScopeRenderer *scope_renderer, gpointer channel_id )
{

  GtkCifroScopeRendererPriv *priv = GTK_CIFRO_SCOPE_RENDERER_GET_PRIVATE( scope_renderer );
  g_hash_table_remove( priv->channels, channel_id );

  priv->visible_update = TRUE;

}


// Функция устанавливает имя канала.
void gtk_cifro_scope_renderer_set_channel_name( GtkCifroScopeRenderer *scope_renderer, gpointer channel_id, const gchar *axis_name )
{

  GtkCifroScopeRendererPriv *priv = GTK_CIFRO_SCOPE_RENDERER_GET_PRIVATE( scope_renderer );

  GHashTableIter channels_iter;
  GtkCifroScopeChannel *channel;
  gpointer cur_channel_id;

  g_hash_table_iter_init( &channels_iter, priv->channels );
  while( g_hash_table_iter_next( &channels_iter, &cur_channel_id, (gpointer)&channel ) )
    if( !channel_id || cur_channel_id == channel_id )
      {
      g_free( channel->name );
      channel->name = g_strdup( axis_name );
      }

  priv->visible_update = TRUE;

}


// Функция устанавливает с какого момента времени следует отображать данные и шаг между двумя соседними данными.
void gtk_cifro_scope_renderer_set_channel_time_param( GtkCifroScopeRenderer *scope_renderer, gpointer channel_id, gfloat time_shift, gfloat time_step )
{

  GtkCifroScopeRendererPriv *priv = GTK_CIFRO_SCOPE_RENDERER_GET_PRIVATE( scope_renderer );

  GHashTableIter channels_iter;
  GtkCifroScopeChannel *channel;
  gpointer cur_channel_id;

  g_hash_table_iter_init( &channels_iter, priv->channels );
  while( g_hash_table_iter_next( &channels_iter, &cur_channel_id, (gpointer)&channel ) )
    if( !channel_id || cur_channel_id == channel_id )
      {
      channel->time_shift = time_shift;
      channel->time_step = time_step;
      }

  priv->visible_update = TRUE;

}


// Функция устанавливает коэффициенты на которые умножаются и сдвигаются все данные в канале.
void gtk_cifro_scope_renderer_set_channel_value_param( GtkCifroScopeRenderer *scope_renderer, gpointer channel_id, gfloat value_shift, gfloat value_scale )
{

  GtkCifroScopeRendererPriv *priv = GTK_CIFRO_SCOPE_RENDERER_GET_PRIVATE( scope_renderer );

  GHashTableIter channels_iter;
  GtkCifroScopeChannel *channel;
  gpointer cur_channel_id;

  g_hash_table_iter_init( &channels_iter, priv->channels );
  while( g_hash_table_iter_next( &channels_iter, &cur_channel_id, (gpointer)&channel ) )
    if( !channel_id || cur_channel_id == channel_id )
      {
      channel->value_shift = value_shift;
      channel->value_scale = value_scale;
      }

  priv->visible_update = TRUE;

}


// Функция устанавливает типа отображения осциллограмм.
void gtk_cifro_scope_renderer_set_channel_draw_type( GtkCifroScopeRenderer *scope_renderer, gpointer channel_id, GtkCifroScopeDrawType draw_type )
{

  GtkCifroScopeRendererPriv *priv = GTK_CIFRO_SCOPE_RENDERER_GET_PRIVATE( scope_renderer );

  GHashTableIter channels_iter;
  GtkCifroScopeChannel *channel;
  gpointer cur_channel_id;

  g_hash_table_iter_init( &channels_iter, priv->channels );
  while( g_hash_table_iter_next( &channels_iter, &cur_channel_id, (gpointer)&channel ) )
    if( !channel_id || cur_channel_id == channel_id )
      channel->draw_type = draw_type;

  priv->visible_update = TRUE;

}


// Функция устанавливает цвет отображения данных канала.
void gtk_cifro_scope_renderer_set_channel_color( GtkCifroScopeRenderer *scope_renderer, gpointer channel_id, gdouble red, gdouble green, gdouble blue )
{

  GtkCifroScopeRendererPriv *priv = GTK_CIFRO_SCOPE_RENDERER_GET_PRIVATE( scope_renderer );

  GHashTableIter channels_iter;
  GtkCifroScopeChannel *channel;
  gpointer cur_channel_id;

  g_hash_table_iter_init( &channels_iter, priv->channels );
  while( g_hash_table_iter_next( &channels_iter, &cur_channel_id, (gpointer)&channel ) )
    if( !channel_id || cur_channel_id == channel_id )
      channel->color = cairo_sdline_color( red, green, blue, 1.0 );

  priv->visible_update = TRUE;

}


// Функция устанавливает данные канала для отображения.
void gtk_cifro_scope_renderer_set_channel_data( GtkCifroScopeRenderer *scope_renderer, gpointer channel_id, gint num, gfloat *values )
{

  GtkCifroScopeRendererPriv *priv = GTK_CIFRO_SCOPE_RENDERER_GET_PRIVATE( scope_renderer );
  GtkCifroScopeChannel* channel;

  channel = g_hash_table_lookup( priv->channels, channel_id );
  if( !channel ) return;

  if( num > channel->size )
    {
    channel->data = g_renew( float, channel->data, num );
    channel->size = num;
    }

  channel->num = num;
  if( num > 0 )
    {
    memcpy( channel->data, values, num * sizeof( gfloat ) );
    channel->show = TRUE;
    }

}


// Функция включает или выключает отображения данных канала.
void gtk_cifro_scope_renderer_set_channel_show( GtkCifroScopeRenderer *scope_renderer, gpointer channel_id, gboolean show )
{

  GtkCifroScopeRendererPriv *priv = GTK_CIFRO_SCOPE_RENDERER_GET_PRIVATE( scope_renderer );

  GHashTableIter channels_iter;
  GtkCifroScopeChannel *channel;
  gpointer cur_channel_id;

  g_hash_table_iter_init( &channels_iter, priv->channels );
  while( g_hash_table_iter_next( &channels_iter, &cur_channel_id, (gpointer)&channel ) )
    if( !channel_id || cur_channel_id == channel_id )
      channel->show = show;

  priv->visible_update = TRUE;

}


// Функция обновляет изображение осциллографа.
void gtk_cifro_scope_renderer_update( GtkCifroScopeRenderer *scope_renderer )
{

  GtkCifroScopeRendererPriv *priv = GTK_CIFRO_SCOPE_RENDERER_GET_PRIVATE( scope_renderer );

  priv->visible_update = TRUE;

}
