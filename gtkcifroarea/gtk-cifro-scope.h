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
 * \file gtk-cifro-scope.h
 *
 * \brief Заголовочный файл GTK+ виджета осциллографа
 * \author Andrei Fadeev
 * \date 2013-2015
 * \license GNU General Public License version 3 или более поздняя<br>
 * Коммерческая лицензия - свяжитесь с автором
 *
 * \defgroup GtkCifroScope GtkCifroScope - GTK+ виджет осциллографа
 *
 * CifroScope позволяет реализовывать графическое представление данных как в осциллографе. Виджет создаётся функцией
 * #gtk_cifro_scope_new. Перед отображеним данных в осциллограф необходимо добавить каналы отображения.
 * Для это используется функция #gtk_cifro_scope_add_channel. Удалить неиспользуемые каналы можно
 * функцией #gtk_cifro_scope_remove_channel.
 *
 * Данный виджет является наследуемым от виджета \link GtkCifroArea \endlink и к нему могут
 * применяться все функции последнего. Управление виджетом осуществляется аналогично \link GtkCifroArea \endlink.
 *
 * По умолчанию ось времени направлена вправо, ось значений вверх. Направление ориентации осей можно изменить
 * функцией #gtk_cifro_scope_set_gravity.
 *
 * Подписи к осям задаются функцией #gtk_cifro_scope_set_axis_name.
 *
 * Тип отображения осциллограммы (линиями или точками) устанавливается функцией #gtk_cifro_scope_set_channel_draw_type.
 *
 * Параметры оцифровки данных канала устанавливаются функцией #gtk_cifro_scope_set_channel_time_param.
 *
 * Для каждого канала возможно задание индивидуальных параметров "усиления" и смещения сигнала функцией #gtk_cifro_scope_set_channel_value_param.
 *
 * Цвет осциллограммы канала задаётся функцией #gtk_cifro_scope_set_channel_color.
 *
 * Включение и выключение канала для отображения управляется функцией #gtk_cifro_scope_set_channel_show.
 *
 * Для каждого канала может быть задано имя, отличное от основного. Это имя будет отображаться в информационной области
 * как подпись к значению. Имя задаётся функцией #gtk_cifro_scope_set_channel_name.
 *
 * Данные для отображения задаются функцией #gtk_cifro_scope_set_channel_data. После того как данные для всех каналов
 * определены необходимо вызвать функцию #gtk_cifro_scope_update для обновления изображения.
 *
 * Виджет использует шрифт и цвета текущей темы GTK+.
 *
*/

#ifndef _gtk_cifro_scope_h
#define _gtk_cifro_scope_h

#include <gtk/gtk.h>

#include <gtk-cifro-area.h>
#include <gtk-cifro-scope-types.h>

G_BEGIN_DECLS


#define GTK_TYPE_CIFRO_SCOPE                     ( gtk_cifro_scope_get_type() )
#define GTK_CIFRO_SCOPE( obj )                   ( G_TYPE_CHECK_INSTANCE_CAST( ( obj ), GTK_TYPE_CIFRO_SCOPE, GtkCifroScope ) )
#define GTK_IS_CIFRO_SCOPE( obj )                ( G_TYPE_CHECK_INSTANCE_TYPE( ( obj ), GTK_TYPE_CIFRO_SCOPE ) )
#define GTK_CIFRO_SCOPE_CLASS( klass )           ( G_TYPE_CHECK_CLASS_CAST( ( klass ), GTK_TYPE_CIFRO_SCOPE, GtkCifroScopeClass ) )
#define GTK_IS_CIFRO_SCOPE_CLASS( klass )        ( G_TYPE_CHECK_CLASS_TYPE( ( klass ), GTK_TYPE_CIFRO_SCOPE ) )
#define GTK_CIFRO_SCOPE_GET_CLASS( obj )         ( G_TYPE_INSTANCE_GET_CLASS( ( obj ), GTK_TYPE_CIFRO_SCOPE, GtkCifroScopeClass ) )


typedef GtkCifroArea GtkCifroScope;
typedef GtkCifroAreaClass GtkCifroScopeClass;


GType gtk_cifro_scope_get_type( void );


/*!
 *
 * Функция создаёт виджет \link GtkCifroScope \endlink.
 *
 * \return Указатель на созданый виджет.
 *
 */
GtkWidget *gtk_cifro_scope_new( void );


/*!
 *
 * Функция изменяет ориентацию осей осциллографа.
 *
 * \param cscope указатель на виджет \link GtkCifroScope \endlink;
 * \param gravity ориентация осциллографа (\link GtkCifroScopeGravity \endlink).
 *
 * \return Нет.
 *
*/
void gtk_cifro_scope_set_gravity( GtkCifroScope *cscope, GtkCifroScopeGravity gravity );


/*!
 *
 * Функция включает или выключает отображения блока с информацией о значениях под курсором.
 *
 * \param cscope указатель на виджет \link GtkCifroScope \endlink;
 * \param show показывать - TRUE или нет - FALSE информацию о значениях под карсором.
 *
 * \return Нет.
 *
*/
void gtk_cifro_scope_set_info_show( GtkCifroScope *cscope, gboolean show );


/*!
 *
 * Функция задаёт подписи к осям абсцисс и ординат.
 *
 * \param cscope указатель на виджет \link GtkCifroScope \endlink;
 * \param time_axis_name подпись оси времени (абсцисса);
 * \param value_axis_name подпись оси данных (ордината).
 *
 * \return Нет.
 *
*/
void gtk_cifro_scope_set_axis_name( GtkCifroScope *cscope, const gchar *time_axis_name, const gchar *value_axis_name );


/*!
 *
 * Функция дабавляет канал отображения данных в осциллограф.
 *
 * \param cscope указатель на виджет \link GtkCifroScope \endlink.
 *
 * \return Идентификатор канала.
 *
*/
gpointer gtk_cifro_scope_add_channel( GtkCifroScope *cscope );


/*!
 *
 * Функция удаляет канал отображения данных из осциллографа.
 *
 * \param cscope указатель на виджет \link GtkCifroScope \endlink;
 * \param channel_id идентификатор канала осциллографа.
 *
 * \return Нет.
 *
*/
void gtk_cifro_scope_remove_channel( GtkCifroScope *cscope, gpointer channel_id );


/*!
 *
 * Функция устанавливает имя канала. Если идентификатор канала равен NULL имя устанавливается для всех каналов.
 *
 * \param scope указатель на виджет \link GtkCifroScope \endlink;
 * \param channel_id идентификатор канала осциллографа;
 * \param axis_name имя канала.
 *
 * \return Нет.
 *
*/
void gtk_cifro_scope_set_channel_name( GtkCifroScope *scope, gpointer channel_id, const gchar *axis_name );


/*!
 *
 * Функция устанавливает с какого момента времени следует отображать данные и
 * какой шаг между двумя соседними данными (частота оцифровки). Параметры задаются
 * индивидуально для каждого канала. Если идентификатор канала равен NULL параметры
 * устанавливаются для всех каналов.
 *
 * \param cscope указатель на виджет \link GtkCifroScope \endlink;
 * \param channel_id идентификатор канала осциллографа;
 * \param time_shift начальный момент времени;
 * \param time_step шаг смещения по оси времени.
 *
 * \return Нет.
 *
*/
void gtk_cifro_scope_set_channel_time_param( GtkCifroScope *cscope, gpointer channel_id, gfloat time_shift, gfloat time_step );


/*!
 *
 * Функция устанавливает коэффициенты на которые умножаются и сдвигаются все данные в
 * канале. Это позволяет отображать разнородные данные в одном пространстве и наглядно
 * сравнивать их друг с другом. Коэффициенты задаются индивидуально для каждого канала.
 * Если идентификатор канала равен NULL коэффициенты устанавливаются для всех каналов.
 *
 * \param cscope указатель на виджет \link GtkCifroScope \endlink;
 * \param channel_id идентификатор канала осциллографа;
 * \param value_shift коэффициент смещения данных;
 * \param value_scale коэффициент умножения данных.
 *
 * \return Нет.
 *
*/
void gtk_cifro_scope_set_channel_value_param( GtkCifroScope *cscope, gpointer channel_id, gfloat value_shift, gfloat value_scale );


/*!
 *
 * Функция устанавливает типа отображения осциллограмм. Если идентификатор канала равен NULL
 * тип отображения устанавливается для всех каналов.
 *
 * \param cscope указатель на виджет \link GtkCifroScope \endlink;
 * \param channel_id идентификатор канала осциллографа;
 * \param draw_type тип отображения осциллограмм (\link GtkCifroScopeDrawType \endlink).
 *
 * \return Нет.
 *
*/
void gtk_cifro_scope_set_channel_draw_type( GtkCifroScope *cscope, gpointer channel_id, GtkCifroScopeDrawType draw_type );


/*!
 *
 * Функция устанавливает цвет отображения данных канала. Если идентификатор канала равен NULL
 * цвет устанавливается для всех каналов.
 *
 * \param cscope указатель на виджет \link GtkCifroScope \endlink;
 * \param channel_id идентификатор канала осциллографа;
 * \param red значение красной составляющей цвета от 0 до 1;
 * \param green значение зелёной составляющей цвета от 0 до 1;
 * \param blue значение синей составляющей цвета от 0 до 1.
 *
 * \return Нет.
 *
*/
void gtk_cifro_scope_set_channel_color( GtkCifroScope *cscope, gpointer channel_id, gdouble red, gdouble green, gdouble blue );


/*!
 *
 * Функция устанавливает данные канала для отображения.
 *
 * \param cscope указатель на виджет \link GtkCifroScope \endlink;
 * \param channel_id идентификатор канала осциллографа;
 * \param num число значений для отображения;
 * \param values указатель на массив данных для отображения.
 *
 * \return Нет.
 *
*/
void gtk_cifro_scope_set_channel_data( GtkCifroScope *cscope, gpointer channel_id, gint num, gfloat *values );


/*!
 *
 * Функция включает или выключает отображения данных канала. Если идентификатор канала равен NULL
 * отображение устанавливается для всех каналов.
 *
 * \param cscope указатель на виджет \link GtkCifroScope \endlink;
 * \param channel_id идентификатор канала осциллографа;
 * \param show показывать - TRUE или нет - FALSE данные этого канала.
 *
 * \return Нет.
 *
*/
void gtk_cifro_scope_set_channel_show( GtkCifroScope *cscope, gpointer channel_id, gboolean show );


/*!
 *
 * Функция обновляет изображение осциллографа. Функцию следует вызывать после
 * изменения данных в каналах.
 *
 * \param cscope указатель на виджет \link GtkCifroScope \endlink.
 *
 * \return Нет.
 *
*/
void gtk_cifro_scope_update( GtkCifroScope *cscope );


G_END_DECLS

#endif /* _gtk_cifro_scope_h */
