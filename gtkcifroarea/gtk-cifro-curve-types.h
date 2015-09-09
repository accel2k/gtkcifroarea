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
 * \file gtk-cifro-curve-types.h
 *
 * \brief Заголовочный файл общих типов GTK+ виджета осциллографа совмещённого с параметрической кривой
 * \author Andrei Fadeev
 * \date 2013-2015
 * \license GNU General Public License version 3 или более поздняя<br>
 * Коммерческая лицензия - свяжитесь с автором
 *
*/

#ifndef _gtk_cifro_curve_types_h
#define _gtk_cifro_curve_types_h

#include <glib.h>

G_BEGIN_DECLS


/*! \brief Структура с описанием точки. */
typedef struct GtkCifroCurvePoint {

  gdouble                    x;                  /*!< X координата точки; */
  gdouble                    y;                  /*!< Y координата точки. */

} GtkCifroCurvePoint;


/*! Callback функция для расчета аналитической кривой.
 *
 * \param param - переменная функции;
 * \param points - массив точек параметров функции;
 * \param curve_data - данные пользователя.
 *
 * \return значение функции.
 *
 */
typedef gdouble (*GtkCifroCurveFunc)( gdouble param, GArray *points, gpointer curve_data );


G_END_DECLS

#endif /* _gtk_cifro_curve_types_h */
