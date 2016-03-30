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

/*!
 * \file gtk-cifro-scope-types.h
 *
 * \brief Заголовочный файл общих типов GTK+ виджета осциллографа
 * \author Andrei Fadeev
 * \date 2013-2016
 * \license GNU General Public License version 3 или более поздняя<br>
 * Коммерческая лицензия - свяжитесь с автором
 *
*/

#ifndef _gtk_cifro_scope_types_h
#define _gtk_cifro_scope_types_h

#include <glib.h>

G_BEGIN_DECLS


/*! \brief Типы ориентации осей осциллографа */
typedef enum {

  GTK_CIFRO_SCOPE_GRAVITY_RIGHT_UP = 1,          /*!< Ось X - вправо, ось Y - вверх */
  GTK_CIFRO_SCOPE_GRAVITY_LEFT_UP,               /*!< Ось X - влево,  ось Y - вверх */
  GTK_CIFRO_SCOPE_GRAVITY_RIGHT_DOWN,            /*!< Ось X - вправо, ось Y - вниз */
  GTK_CIFRO_SCOPE_GRAVITY_LEFT_DOWN,             /*!< Ось X - влево,  ось Y - вниз */
  GTK_CIFRO_SCOPE_GRAVITY_UP_RIGHT,              /*!< Ось X - вверх,  ось Y - вправо */
  GTK_CIFRO_SCOPE_GRAVITY_UP_LEFT,               /*!< Ось X - вверх,  ось Y - влево */
  GTK_CIFRO_SCOPE_GRAVITY_DOWN_RIGHT,            /*!< Ось X - вниз,   ось Y - вправо */
  GTK_CIFRO_SCOPE_GRAVITY_DOWN_LEFT              /*!< Ось X - вниз,   ось Y - влево */

} GtkCifroScopeGravity;


/*! \brief Тип изображения осциллографа */
typedef enum {

  GTK_CIFRO_SCOPE_LINED = 1,                     /*!< Данные соединяются линиями */
  GTK_CIFRO_SCOPE_DOTTED                         /*!< Данные рисуются отдельными точками */

} GtkCifroScopeDrawType;


G_END_DECLS

#endif /* _gtk_cifro_scope_types_h */
