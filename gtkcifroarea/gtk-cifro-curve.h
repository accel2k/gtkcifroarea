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
 * \file gtk-cifro-curve.h
 *
 * \brief Заголовочный файл GTK+ виджета осциллографа совмещённого с параметрической кривой
 * \author Andrei Fadeev
 * \date 2013-2016
 * \license GNU General Public License version 3 или более поздняя<br>
 * Коммерческая лицензия - свяжитесь с автором
 *
 * \defgroup GtkCifroCurve GtkCifroCurve - GTK+ виджет осциллографа совмещённого с параметрической кривой
 *
 * GtkCifroCurve позволяет отображать данные аналогично \link GtkCifroScope \endlink и, кроме этого,
 * обеспечивает отображение кривой с определением её параметров через контрольные точки.
 *
 * Функция расчёта кривой указывается пользователем при создании виджета функцией #gtk_cifro_curve_new.
 *
 * Данный виджет является наследуемым от виджета \link GtkCifroScope \endlink и к нему могут применяться
 * все функции последнего.
 *
 * Параметры кривой определяются местоположением контрольных точек. Точки можно перемещать при помощи мышки
 * при нажатой левой кнопке. Точка, выбранная для перемещения, выделяется окружностью.
 *
 * Точки можно добавлять или удалять. Для добавления точек необходимо нажать левую кнопку манипулятора при
 * нажатой клавише Ctrl на клавиатуре. Аналогично при нажатой клавише Ctrl можно удалить уже существующую точку.
 * Также можно удалить точку совместив её с одной из соседних.
 *
 * Точка описывается структурой типа \link GtkCifroCurvePoint \endlink. Массив точек передаётся через GArray в виде
 * массива структур \link GtkCifroCurvePoint \endlink.
 *
 * Аналитический вид кривой расчитывается функцией типа #GtkCifroCurveFunc, в неё передаются все точки существующие
 * на данный момент и указатель на пользовательские данные.
 *
 * Добавлением и удалением точек можно управлять при помощи функций #gtk_cifro_curve_clear_points, #gtk_cifro_curve_add_point
 * и #gtk_cifro_curve_set_points.
 *
 * Массив текущих точек можно получить функцией #gtk_cifro_curve_get_points.
 *
 * Цвет аналитической кривой и точек задаётся функциями #gtk_cifro_curve_set_curve_color и #gtk_cifro_curve_set_point_color
 * соответственно.
 *
 */

#ifndef __GTK_CIFRO_CURVE_H__
#define __GTK_CIFRO_CURVE_H__

#include <gtk/gtk.h>

#include <gtk-cifro-scope.h>
#include <gtk-cifro-curve-types.h>

G_BEGIN_DECLS

#define GTK_TYPE_CIFRO_CURVE             (gtk_cifro_curve_get_type ())
#define GTK_CIFRO_CURVE(obj)             (G_TYPE_CHECK_INSTANCE_CAST((obj), GTK_TYPE_CIFRO_CURVE, GtkCifroCurve))
#define GTK_IS_CIFRO_CURVE(obj)          (G_TYPE_CHECK_INSTANCE_TYPE((obj), GTK_TYPE_CIFRO_CURVE))
#define GTK_CIFRO_CURVE_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST((klass), GTK_TYPE_CIFRO_CURVE, GtkCifroCurveClass))
#define GTK_IS_CIFRO_CURVE_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE((klass), GTK_TYPE_CIFRO_CURVE))
#define GTK_CIFRO_CURVE_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS((obj), GTK_TYPE_CIFRO_CURVE, GtkCifroCurveClass))

typedef struct _GtkCifroCurve GtkCifroCurve;
typedef struct _GtkCifroCurvePrivate GtkCifroCurvePrivate;
typedef struct _GtkCifroCurveClass GtkCifroCurveClass;

struct _GtkCifroCurve
{
  GtkCifroScope parent_instance;

  GtkCifroCurvePrivate *priv;
};

struct _GtkCifroCurveClass
{
  GtkCifroScopeClass parent_class;
};

GTK_CIFROAREA_EXPORT
GType                  gtk_cifro_curve_get_type                (void);

/**
 *
 * Функция создаёт виджет \link GtkCifroCurve \endlink.
 *
 * \param curve_func функция расчёте кривой по заданным точкам (\link GtkCifroCurveFunc \endlink);
 * \param curve_data пользовательские данные для передачи в curve_func.
 *
 * \return Указатель на созданный виджет.
 *
 */
GTK_CIFROAREA_EXPORT
GtkWidget             *gtk_cifro_curve_new                     (GtkCifroCurveFunc      curve_func,
                                                                gpointer               curve_data);

/**
 *
 * Функция удаляет все контрольные точки.
 *
 * \param ccurve указатель на виджет \link GtkCifroCurve \endlink.
 *
 * \return Нет.
 *
 */
GTK_CIFROAREA_EXPORT
void                   gtk_cifro_curve_clear_points            (GtkCifroCurve         *ccurve);

/**
 *
 * Функция добавляет одну контрольную точку к существующим.
 *
 * \param ccurve указатель на виджет \link GtkCifroCurve \endlink;
 * \param x координата точки x;
 * \param y координата точки y.
 *
 * \return Нет.
 *
 */
GTK_CIFROAREA_EXPORT
void                   gtk_cifro_curve_add_point               (GtkCifroCurve         *ccurve,
                                                                gdouble                x,
                                                                gdouble                y);

/**
 *
 * Функция устанавливает новые контрольные точки взамен текущих.
 *
 * \param ccurve указатель на виджет \link GtkCifroCurve \endlink;
 * \param points массив новых точек.
 *
 * \return Нет.
 *
 */
GTK_CIFROAREA_EXPORT
void                   gtk_cifro_curve_set_points              (GtkCifroCurve         *ccurve,
                                                                GArray                *points);

/**
 *
 * Функция возвращает массив текущих контрольных точек.
 *
 * \param ccurve указатель на виджет \link GtkCifroCurve \endlink.
 *
 * \return Указатель на объект типа GArray с контрольными точками.
 *
 */
GTK_CIFROAREA_EXPORT
GArray                *gtk_cifro_curve_get_points              (GtkCifroCurve         *ccurve);

/**
 *
 * Функция устанавливает цвет кривой.
 *
 * \param ccurve указатель на виджет \link GtkCifroCurve \endlink;
 * \param red значение красной составляющей цвета от 0 до 1;
 * \param green значение зелёной составляющей цвета от 0 до 1;
 * \param blue значение синей составляющей цвета от 0 до 1.
 *
 * \return Нет.
 *
 */
GTK_CIFROAREA_EXPORT
void                   gtk_cifro_curve_set_curve_color         (GtkCifroCurve         *ccurve,
                                                                gdouble                red,
                                                                gdouble                green,
                                                                gdouble                blue);

/**
 *
 * Функция устанавливает цвет контрольных точек кривой.
 *
 * \param ccurve указатель на виджет \link GtkCifroCurve \endlink;
 * \param red значение красной составляющей цвета от 0 до 1;
 * \param green значение зелёной составляющей цвета от 0 до 1;
 * \param blue значение синей составляющей цвета от 0 до 1.
 *
 * \return Нет.
 *
 */
GTK_CIFROAREA_EXPORT
void                   gtk_cifro_curve_set_point_color         (GtkCifroCurve         *ccurve,
                                                                gdouble                red,
                                                                gdouble                green,
                                                                gdouble                blue);

G_END_DECLS

#endif /* __GTK_CIFRO_CURVE_H__ */
