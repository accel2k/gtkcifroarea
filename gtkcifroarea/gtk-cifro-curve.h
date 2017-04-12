/*
 * GtkCifroArea - 2D layers image management library.
 *
 * Copyright 2013-2017 Andrei Fadeev (andrei@webcontrol.ru)
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

#ifndef __GTK_CIFRO_CURVE_H__
#define __GTK_CIFRO_CURVE_H__

#include <gtk-cifro-scope.h>

G_BEGIN_DECLS

typedef struct _GtkCifroCurvePoint GtkCifroCurvePoint;

/**
 * GtkCifroCurvePoint:
 * @x: X координата точки
 * @y: Y координата точки
 *
 * Структура с описанием точки.
 *
 */
struct _GtkCifroCurvePoint
{
  gdouble              x;
  gdouble              y;
};

/**
 * GtkCifroCurveFunc:
 * @param: переменная функции
 * @points: массив точек (#GtkCifroCurvePoint) параметров функции
 * @curve_data: данные пользователя
 *
 * Функция для расчета аналитической кривой.
 *
 * Returns: значение функции.
 *
 */
typedef gdouble        (*GtkCifroCurveFunc)                    (gdouble        param,
                                                                GArray        *points,
                                                                gpointer       curve_data);

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

GTK_CIFROAREA_EXPORT
GtkWidget             *gtk_cifro_curve_new                     (GtkCifroScopeGravity   gravity,
                                                                GtkCifroCurveFunc      curve_func,
                                                                gpointer               curve_data);

GTK_CIFROAREA_EXPORT
void                   gtk_cifro_curve_clear_points            (GtkCifroCurve         *ccurve);

GTK_CIFROAREA_EXPORT
void                   gtk_cifro_curve_add_point               (GtkCifroCurve         *ccurve,
                                                                gdouble                x,
                                                                gdouble                y);

GTK_CIFROAREA_EXPORT
void                   gtk_cifro_curve_set_points              (GtkCifroCurve         *ccurve,
                                                                GArray                *points);

GTK_CIFROAREA_EXPORT
GArray                *gtk_cifro_curve_get_points              (GtkCifroCurve         *ccurve);

GTK_CIFROAREA_EXPORT
void                   gtk_cifro_curve_set_curve_color         (GtkCifroCurve         *ccurve,
                                                                gdouble                red,
                                                                gdouble                green,
                                                                gdouble                blue);

GTK_CIFROAREA_EXPORT
void                   gtk_cifro_curve_set_point_color         (GtkCifroCurve         *ccurve,
                                                                gdouble                red,
                                                                gdouble                green,
                                                                gdouble                blue);

G_END_DECLS

#endif /* __GTK_CIFRO_CURVE_H__ */
