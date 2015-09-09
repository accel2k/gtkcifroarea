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
#include "gtk-cifro-curve-renderer.h"

#include "cairo-sdline.h"
#include <math.h>


enum { PROP_O, PROP_AREA, PROP_CURVE_FUNC, PROP_CURVE_DATA };


typedef struct GtkCifroCurveRendererPriv {       // Внутренние данные объекта.

  GObject                   *area;               // Указатель на объект GtkCifroArea или GtkCifroImage.
  GtkCifroAreaState         *state;              // Объект хранения состояния GtkCifroArea.

  GArray                    *curve_points;       // Точки кривой.
  GtkCifroCurveFunc          curve_func;         // Функция расчёта значений кривой.
  gpointer                   curve_data;         // Пользовательские данные для функции расчёта значений кривой.

  guint32                    curve_color;        // Цвет кривой.
  guint32                    point_color;        // Цвет точек.

  gint                       selected_point;     // Индекс текущей выбранной точки;
  gboolean                   move_point;         // Перемещать или нет выбранную точку;
  gboolean                   remove_point;       // Удалить выбранную точку.

  gboolean                   visible_update;     // Признак необходимости перерисовки осциллограмм.

  gint                       area_width;         // Ширина области вывода.
  gint                       area_height;        // Высота области вывода.

  gint                       visible_width;      // Видимая ширина.
  gint                       visible_height;     // Видимая высота.

  gint                       border_top;         // Размер области обрамления сверху.

} GtkCifroCurveRendererPriv;


#define GTK_CIFRO_CURVE_RENDERER_GET_PRIVATE( obj ) ( G_TYPE_INSTANCE_GET_PRIVATE( ( obj ), GTK_TYPE_CIFRO_CURVE_RENDERER, GtkCifroCurveRendererPriv ) )


static void gtk_cifro_curve_renderer_set_property( GObject *curve_renderer, guint prop_id, const GValue *value, GParamSpec *pspec );
static GObject* gtk_cifro_curve_renderer_object_constructor( GType g_type, guint n_construct_properties, GObjectConstructParam *construct_properties );
static void gtk_cifro_curve_renderer_object_finalize( GObject *curve_renderer );

static void gtk_cifro_curve_renderer_area_changed( GtkCifroAreaState *state, GtkCifroAreaSize *size, GtkCifroCurveRendererPriv *priv );
static void gtk_cifro_curve_renderer_visible_changed( GtkCifroAreaState *state, GtkCifroAreaSize *size, GtkCifroCurveRendererPriv *priv );
static void gtk_cifro_curve_renderer_border_changed( GtkCifroAreaState *state, GtkCifroAreaBorder *border, GtkCifroCurveRendererPriv *priv );

static gboolean gtk_cifro_curve_renderer_check_visible_redraw( GtkWidget *widget, GtkCifroCurveRendererPriv *priv );
static void gtk_cifro_curve_renderer_visible_draw( GtkWidget *widget, cairo_t *cairo, GtkCifroCurveRendererPriv *priv );


G_DEFINE_TYPE( GtkCifroCurveRenderer, gtk_cifro_curve_renderer, G_TYPE_OBJECT );


static void gtk_cifro_curve_renderer_class_init( GtkCifroCurveRendererClass *klass )
{

  GObjectClass *this_class = G_OBJECT_CLASS( klass );

  this_class->set_property = gtk_cifro_curve_renderer_set_property;

  this_class->constructor = gtk_cifro_curve_renderer_object_constructor;
  this_class->finalize = gtk_cifro_curve_renderer_object_finalize;

  g_object_class_install_property( this_class, PROP_AREA,
    g_param_spec_object( "area", "Area", "GtkCifroArea object", G_TYPE_OBJECT, G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY ) );

  g_object_class_install_property( this_class, PROP_CURVE_FUNC,
    g_param_spec_pointer( "curve-func", "Curve func", "Curve function", G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY ) );

  g_object_class_install_property( this_class, PROP_CURVE_DATA,
    g_param_spec_pointer( "curve-data", "Curve data", "Curve function data", G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY ) );

  g_type_class_add_private( klass, sizeof( GtkCifroCurveRendererPriv ) );

}


static void gtk_cifro_curve_renderer_init( GtkCifroCurveRenderer *cifro_curve_renderer ){ ; }


static void gtk_cifro_curve_renderer_set_property( GObject *curve_renderer, guint prop_id, const GValue *value, GParamSpec *pspec )
{

  GtkCifroCurveRendererPriv *priv = GTK_CIFRO_CURVE_RENDERER_GET_PRIVATE( curve_renderer );

  switch ( prop_id )
    {

    case PROP_AREA:
      priv->area = g_value_get_object( value );
      if( !GTK_IS_CIFRO_AREA( priv->area ) )
        {
        G_OBJECT_WARN_INVALID_PROPERTY_ID( curve_renderer, prop_id, pspec );
        priv->area = NULL;
        }
      break;

    case PROP_CURVE_FUNC:
      priv->curve_func = g_value_get_pointer( value );
      if( !priv->curve_func )
        G_OBJECT_WARN_INVALID_PROPERTY_ID( curve_renderer, prop_id, pspec );
      break;

    case PROP_CURVE_DATA:
      priv->curve_data = g_value_get_pointer( value );
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID( curve_renderer, prop_id, pspec );
      break;

    }

}


static GObject* gtk_cifro_curve_renderer_object_constructor( GType g_type, guint n_construct_properties, GObjectConstructParam *construct_properties )
{

  GObject *curve_renderer = G_OBJECT_CLASS( gtk_cifro_curve_renderer_parent_class )->constructor( g_type, n_construct_properties, construct_properties );

  GtkCifroCurveRendererPriv *priv = GTK_CIFRO_CURVE_RENDERER_GET_PRIVATE( curve_renderer );

  // Объект состояния GtkCifroArea.
  if( GTK_IS_CIFRO_AREA( priv->area ) ) priv->state = gtk_cifro_area_get_state( GTK_CIFRO_AREA( priv->area ) );

  // Контрольные точки кривой.
  priv->curve_points = g_array_new( FALSE, FALSE, sizeof( GtkCifroCurvePoint ) );

  // Цвета по умолчанию.
  priv->curve_color = cairo_sdline_color( g_random_double_range( 0.5, 1.0 ), g_random_double_range( 0.5, 1.0 ), g_random_double_range( 0.5, 1.0 ), 1.0 );
  priv->point_color = cairo_sdline_color( g_random_double_range( 0.5, 1.0 ), g_random_double_range( 0.5, 1.0 ), g_random_double_range( 0.5, 1.0 ), 1.0 );

  // Обработчики сигналов GtkCifroArea и GtkCifroImage.
  if( priv->area )
    {
    g_signal_connect( priv->area, "visible-draw", G_CALLBACK( gtk_cifro_curve_renderer_visible_draw ), priv );
    g_signal_connect( priv->area, "check-visible-redraw", G_CALLBACK( gtk_cifro_curve_renderer_check_visible_redraw ), priv );
    }

  // Обработчики сигналов состояния GtkCifroArea.
  if( priv->state )
    {
    g_signal_connect( priv->state, "area-changed", G_CALLBACK( gtk_cifro_curve_renderer_area_changed ), priv );
    g_signal_connect( priv->state, "visible-changed", G_CALLBACK( gtk_cifro_curve_renderer_visible_changed ), priv );
    g_signal_connect( priv->state, "border-changed", G_CALLBACK( gtk_cifro_curve_renderer_border_changed ), priv );
    }

  return curve_renderer;

}


static void gtk_cifro_curve_renderer_object_finalize( GObject *curve_renderer )
{

  GtkCifroCurveRendererPriv *priv = GTK_CIFRO_CURVE_RENDERER_GET_PRIVATE( curve_renderer );

  g_array_unref( priv->curve_points );

  G_OBJECT_CLASS( gtk_cifro_curve_renderer_parent_class )->finalize( curve_renderer );

}



// Функция выбирает точку по указанным координатам.
static void gtk_cifro_curve_renderer_select_point( GtkCifroCurveRendererPriv *priv, gint pointer_x, gint pointer_y )
{

  guint i;

  gint selected_point;

  gdouble x, y;
  gdouble point_distance;
  gdouble prev_point_distance;
  gdouble point_radius = 0.4 * priv->border_top;

  if( priv->move_point ) return;

  // Ищем ближающую рядом с курсором точку, которая находится в радиусе "размера" точки.
  selected_point = -1;
  prev_point_distance = G_MAXDOUBLE;
  for( i = 0; i < priv->curve_points->len; i++ )
    {

    GtkCifroCurvePoint *point = &g_array_index( priv->curve_points, GtkCifroCurvePoint, i );
    gtk_cifro_area_state_value_to_point( priv->state, &x, &y, point->x, point->y );
    if( x < -point_radius || x >= priv->area_width + point_radius || y < -point_radius || y >= priv->area_height + point_radius ) continue;

    // Расстояние от курсора до текущей проверяемой точки.
    point_distance = sqrt( ( x - pointer_x ) * ( x - pointer_x ) + ( y - pointer_y ) * ( y - pointer_y ) );

    // Если расстояние слишком большое пропускаем эту точку.
    if( point_distance > 2 * point_radius ) continue;

    // Сравниваем с расстоянием до предыдущей ближайшей точки.
    if( point_distance < prev_point_distance )
      {
      selected_point = i;
      prev_point_distance = point_distance;
      }

    }

  if( selected_point != priv->selected_point )
    {
    priv->selected_point = selected_point;
    priv->visible_update = TRUE;
    }

}


// Callback функция вызывается при изменении размера области виджета.
static void gtk_cifro_curve_renderer_area_changed( GtkCifroAreaState *state, GtkCifroAreaSize *size, GtkCifroCurveRendererPriv *priv )
{

  priv->area_width = size->width;
  priv->area_height = size->height;
  priv->visible_update = TRUE;

}


// Callback функция вызывается при изменении "видимой" области.
static void gtk_cifro_curve_renderer_visible_changed( GtkCifroAreaState *state, GtkCifroAreaSize *size, GtkCifroCurveRendererPriv *priv )
{

  priv->visible_width = size->width;
  priv->visible_height = size->height;
  priv->visible_update = TRUE;

}


// Callback функция вызывается при изменении размера окантовки вокруг видимой области.
static void gtk_cifro_curve_renderer_border_changed( GtkCifroAreaState *state, GtkCifroAreaBorder *border, GtkCifroCurveRendererPriv *priv )
{

  priv->border_top = border->top;
  priv->visible_update = TRUE;

}


// Callback функция проверки необходимости перерисовки видимой области.
static gboolean gtk_cifro_curve_renderer_check_visible_redraw( GtkWidget *widget, GtkCifroCurveRendererPriv *priv )
{

  if( !priv->visible_update ) return FALSE;
  priv->visible_update = FALSE;
  return TRUE;

}


// Функция рисования видимой области (параметрической кривой).
static void gtk_cifro_curve_renderer_visible_draw( GtkWidget *widget, cairo_t *cairo, GtkCifroCurveRendererPriv *priv )
{

  cairo_sdline_surface *surface = cairo_sdline_surface_create_for( cairo_get_target( cairo ) );

  gint i;

  // Рисуем кривую.
  for( i = 0; i < priv->visible_width; i++ )
    {

    gdouble x_value;
    gdouble y_value;
    gdouble y1, y2;

    gtk_cifro_area_state_visible_point_to_value( priv->state, i, 0, &x_value, NULL );
    y_value = priv->curve_func( x_value, priv->curve_points, priv->curve_data );
    gtk_cifro_area_state_visible_value_to_point( priv->state, NULL, &y2, x_value, y_value );

    if( i == 0 ) { y1 = y2; continue; }
    cairo_sdline( surface, i - 1, y1, i, y2, priv->curve_color );
    y1 = y2;

    }

  cairo_surface_mark_dirty( surface->cairo_surface );
  cairo_sdline_set_cairo_color( cairo, priv->point_color );
  cairo_set_line_width( cairo, 1.0 );

  // Рисуем точки.
  for( i = 0; i < priv->curve_points->len; i++ )
    {

    gdouble x, y;
    gdouble point_radius = 0.5 * priv->border_top;

    GtkCifroCurvePoint *point = &g_array_index( priv->curve_points, GtkCifroCurvePoint, i );
    gtk_cifro_area_state_visible_value_to_point( priv->state, &x, &y, point->x, point->y );
    if( x < -point_radius || x >= priv->visible_width + point_radius || y < -point_radius || y >= priv->visible_height + point_radius ) continue;

    cairo_arc( cairo, gtk_cifro_area_state_point_to_cairo( x ), gtk_cifro_area_state_point_to_cairo( y ), point_radius / 4.0, 0.0, 2*G_PI );
    cairo_fill( cairo );

    if( priv->selected_point != i ) continue;

    cairo_arc( cairo, gtk_cifro_area_state_point_to_cairo( x ), gtk_cifro_area_state_point_to_cairo( y ), point_radius, 0.0, 2*G_PI );
    cairo_stroke( cairo );

    }

  cairo_sdline_surface_destroy( surface );

}


// Функция создаёт новый объект GtkCifroCurveRenderer.
GtkCifroCurveRenderer *gtk_cifro_curve_renderer_new( GObject *area, GtkCifroCurveFunc curve_func, gpointer curve_data )
{

  return g_object_new( GTK_TYPE_CIFRO_CURVE_RENDERER, "area", area, "curve-func", curve_func, "curve-data", curve_data, NULL );

}


// Функция удаляет все контрольные точки.
void gtk_cifro_curve_renderer_clear_points( GtkCifroCurveRenderer *curve_renderer )
{

  GtkCifroCurveRendererPriv *priv = GTK_CIFRO_CURVE_RENDERER_GET_PRIVATE( curve_renderer );

  g_array_set_size( priv->curve_points, 0 );

  priv->visible_update = TRUE;

}


// Функция добавляет одну контрольную точку к существующим.
void gtk_cifro_curve_renderer_add_point( GtkCifroCurveRenderer *curve_renderer, gdouble x, gdouble y )
{

  GtkCifroCurveRendererPriv *priv = GTK_CIFRO_CURVE_RENDERER_GET_PRIVATE( curve_renderer );

  GtkCifroCurvePoint new_point;
  GtkCifroCurvePoint *point;
  gint i;

  // Ищем место для добавления точки.
  for( i = 0; i < priv->curve_points->len; i++ )
    {
    point = &g_array_index( priv->curve_points, GtkCifroCurvePoint, i );
    if( point->x > x ) break;
    }

  new_point.x = x;
  new_point.y = y;
  g_array_insert_val( priv->curve_points, i, new_point );

  priv->visible_update = TRUE;

}


// Функция устанавливает новые контрольные точки взамен текущих.
void gtk_cifro_curve_renderer_set_points( GtkCifroCurveRenderer *curve_renderer, GArray *points )
{

  GtkCifroCurveRendererPriv *priv = GTK_CIFRO_CURVE_RENDERER_GET_PRIVATE( curve_renderer );

  GtkCifroCurvePoint *point;
  guint i;

  g_array_set_size( priv->curve_points, 0 );
  for( i = 0; i < points->len; i++ )
    {
    point = &g_array_index( points, GtkCifroCurvePoint, i );
    gtk_cifro_curve_renderer_add_point( curve_renderer, point->x, point->y );
    }

  priv->visible_update = TRUE;

}


// Функция возвращает массив текущих контрольных точек.
GArray *gtk_cifro_curve_renderer_get_points( GtkCifroCurveRenderer *curve_renderer )
{

  GtkCifroCurveRendererPriv *priv = GTK_CIFRO_CURVE_RENDERER_GET_PRIVATE( curve_renderer );

  GArray *points = g_array_new( FALSE, FALSE, sizeof( GtkCifroCurvePoint ) );

  g_array_insert_vals( points, 0, priv->curve_points->data, priv->curve_points->len );

  return points;


}


// Функция устанавливает цвет кривой.
void gtk_cifro_curve_renderer_set_curve_color( GtkCifroCurveRenderer *curve_renderer, gdouble red, gdouble green, gdouble blue )
{

  GtkCifroCurveRendererPriv *priv = GTK_CIFRO_CURVE_RENDERER_GET_PRIVATE( curve_renderer );

  priv->curve_color = cairo_sdline_color( red, green, blue, 1.0 );

  priv->visible_update = TRUE;

}


// Функция устанавливает цвет контрольных точек кривой.
void gtk_cifro_curve_renderer_set_point_color( GtkCifroCurveRenderer *curve_renderer, gdouble red, gdouble green, gdouble blue )
{

  GtkCifroCurveRendererPriv *priv = GTK_CIFRO_CURVE_RENDERER_GET_PRIVATE( curve_renderer );

  priv->point_color = cairo_sdline_color( red, green, blue, 1.0 );

  priv->visible_update = TRUE;

}




// Функция обработки сигнала нажатия кнопки мыши. Должна быть подключена к сигналу "button-press-event".
gboolean gtk_cifro_curve_renderer_button_press_event( GtkWidget *widget, GdkEventButton *event, GtkCifroCurveRenderer *curve_renderer )
{

  GtkCifroCurveRendererPriv *priv = GTK_CIFRO_CURVE_RENDERER_GET_PRIVATE( curve_renderer );

  // Обрабатываем только нажатия левой кнопки манипулятора.
  if( event->button != 1 ) return FALSE;

  priv->remove_point = FALSE;
  priv->move_point = FALSE;

  if( priv->selected_point >= 0 )
    {
    // Выбрана точка для перемещения и нажата кнопка Ctrl - нужно удалить эту точку.
    if( event->state & GDK_CONTROL_MASK )
      {
      g_array_remove_index( priv->curve_points, priv->selected_point );
      priv->selected_point = -1;
      priv->visible_update = TRUE;
      }
    // Выбрана точка для перемещения.
    else
      priv->move_point = TRUE;
    }
  else
    {
    // Точка для перемещения не выбрана, но нажата кнопка Ctrl - нужно добавить новую точку.
    if( event->state & GDK_CONTROL_MASK )
      {
      gdouble value_x, value_y;
      gtk_cifro_area_state_point_to_value( priv->state, event->x, event->y, &value_x, &value_y );
      gtk_cifro_curve_renderer_add_point( curve_renderer, value_x, value_y );
      }
    }

  return FALSE;

}


// Функция обработки сигнала отпускания кнопки мыши. Должна быть подключена к сигналу "button-release-event".
gboolean gtk_cifro_curve_renderer_button_release_event( GtkWidget *widget, GdkEventButton *event, GtkCifroCurveRenderer *curve_renderer )
{

  GtkCifroCurveRendererPriv *priv = GTK_CIFRO_CURVE_RENDERER_GET_PRIVATE( curve_renderer );

  // Обрабатываем только нажатия левой кнопки манипулятора.
  if( event->button != 1 ) return FALSE;

  // Обрабатываем удаление точки при её совмещении с другой точкой.
  if( priv->move_point && priv->remove_point && priv->selected_point >= 0 )
    g_array_remove_index( priv->curve_points, priv->selected_point );

  priv->move_point = FALSE;

  return FALSE;

}


// Функция обработки сигнала движения мышки. Должна быть подключена к сигналу "motion-notify-event".
gboolean gtk_cifro_curve_renderer_motion_notify_event( GtkWidget *widget, GdkEventMotion *event, GtkCifroCurveRenderer *curve_renderer )
{

  GtkCifroCurveRendererPriv *priv = GTK_CIFRO_CURVE_RENDERER_GET_PRIVATE( curve_renderer );

  GtkCifroCurvePoint *point;
  GtkCifroCurvePoint *near_point;

  gdouble from_x, to_x;
  gdouble from_y, to_y;

  gdouble value_x, value_y;
  gdouble min_x, max_x;

  gdouble point_radius = 0.4 * priv->border_top;

  // Выделяем точку для перемещения.
  gtk_cifro_curve_renderer_select_point( priv, event->x, event->y );

  // Если мы не находимся в режиме перемещения точки.
  if( !priv->move_point ) return FALSE;

  priv->remove_point = FALSE;

  // Вышли за границу окна.
  if( priv->selected_point < 0 ) return TRUE;

  // Текущая граница отображения.
  gtk_cifro_area_state_get_view( priv->state, &from_x, &to_x, &from_y, &to_y );

  // Расчитываем новое местоположение точки.
  gtk_cifro_area_state_point_to_value( priv->state, event->x, event->y, &value_x, &value_y );

  point = &g_array_index( priv->curve_points, GtkCifroCurvePoint, priv->selected_point );

  // Определяем границы перемещения точки и расстояние до соседних точек.
  // Если расстояние до одной из соседних точек меньше чем "радиус" точки - помечаем точку на удаление.
  // Само удаление будет произведено при отпускании кнопки манипулятора.
  if( priv->selected_point > 0 )
    {
    near_point = &g_array_index( priv->curve_points, GtkCifroCurvePoint, priv->selected_point - 1 );
    min_x = MAX( from_x, near_point->x );
    if( near_point->x > from_x )
      {
      gdouble x1, y1, x2, y2;
      gtk_cifro_area_state_value_to_point( priv->state, &x1, &y1, value_x, value_y );
      gtk_cifro_area_state_value_to_point( priv->state, &x2, &y2, near_point->x, near_point->y );
      if( sqrt( ( x1 - x2 ) * ( x1 - x2 ) + ( y1 - y2 ) * ( y1 - y2 ) ) < point_radius ) priv->remove_point = TRUE;
      }
    }
  else
    min_x = from_x;

  if( priv->selected_point < priv->curve_points->len - 1 )
    {
    near_point = &g_array_index( priv->curve_points, GtkCifroCurvePoint, priv->selected_point + 1 );
    max_x = MIN( to_x, near_point->x );
    if( near_point->x < to_x )
      {
      gdouble x1, y1, x2, y2;
      gtk_cifro_area_state_value_to_point( priv->state, &x1, &y1, value_x, value_y );
      gtk_cifro_area_state_value_to_point( priv->state, &x2, &y2, near_point->x, near_point->y );
      if( sqrt( ( x1 - x2 ) * ( x1 - x2 ) + ( y1 - y2 ) * ( y1 - y2 ) ) < point_radius ) priv->remove_point = TRUE;
      }
    }
  else
    max_x = to_x;

  if( value_x < min_x ) value_x = min_x;
  if( value_x > max_x ) value_x = max_x;
  if( value_y < from_y ) value_y = from_y;
  if( value_y > to_y ) value_y = to_y;

  // Задаём новое положение точки.
  point->x = value_x;
  point->y = value_y;

  priv->visible_update = TRUE;

  return TRUE;

}
