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
 * \file gtk-cifro-area.h
 *
 * \brief Заголовочный файл GTK+ виджета показа многослойных изображений
 * \author Andrei Fadeev
 * \date 2013-2016
 * \license GNU General Public License version 3 или более поздняя<br>
 * Коммерческая лицензия - свяжитесь с автором
 *
 * \defgroup GtkCifroArea GtkCifroArea - GTK+ виджет показа многослойных изображений
 *
 * Виджет предназначен для показа многослойных изображений сформированных внешними модулями.
 * Графическая структура многослойного изображения приведена в \ref index "главном разделе".
 *
 * Виджет реализует следующие возможности:
 * - сведение нескольких изображений в одно и отображение его на экране;
 * - зеркальное отражение изображений по вертикальной и(или) горизонтальной осям;
 * - поворот изображений на определенный пользователем угол;
 * - масштабирование изображений, включая совместное и раздельное по обеим осям, а также с фиксированными коэффициентами;
 * - управление отображением с использованием клавиатуры и мышки.
 *
 * Находясь в фокусе виджет воспринимает следующие команды управления от клавиатуры и мышки:
 * - перемещение изображения (в пределах возможных отображаемых значений) клавишами вверх, вниз, вправо, влево;
 * - перемещение изображения (в пределах возможных отображаемых значений) мышкой при нажатой левой клавише;
 * - поворот изображения клавишами вправо, влево и нажатой клавише shift;
 * - масштабирование изображения клавишами "+" - плюс, "-" - минус, при этом если дополнительно будет нажата
 *   клавиша Ctrl - масштабирование будет производиться только по вертикальной оси, Alt - масштабирование
 *   будет производиться по горизонтальной оси;
 * - масштабирование изображения вращением колесика мышки, с нажатой клавишей Shift - по обеим осям,
 *   Ctrl - по вертикальной оси, Alt - по горизонтальной оси.
 *
 * Создание виджета производится функцией #gtk_cifro_area_new. В процессе работы виджет отслеживает состояние области
 * отображения и передаёт эту информацию в объект \link GtkCifroAreaState \endlink. Получить указатель на объект
 * можно с помощью функции #gtk_cifro_area_get_state.
 *
 * Функции #gtk_cifro_area_set_draw_focus и #gtk_cifro_area_get_draw_focus используются для задания необходимости рисовать
 * рамку вокруг виджета, если он находится в фокусе.
 *
 * При нахождении курсора мышки в видимой области его изображение меняется на GdkCursor типа GDK_CROSSHAIR,
 * а при перемещении видимой области на GDK_FLEUR. Это поведение можно изменить функциями #gtk_cifro_area_set_point_cursor
 * и #gtk_cifro_area_set_move_cursor. При удалении виджета все используемые курсоры будут удалены, соответственно
 * можно просто использовать функции gdk_cursor_new_* без обработки ссылок на курсор. Если в качестве курсора
 * передать NULL будет использовать курсор оконной системы по умолчанию.
 *
 * Зеркальное отражение изображений по осям задается функцией #gtk_cifro_area_set_swap, его текущее состояние
 * можно определить функцией #gtk_cifro_area_get_swap.
 *
 * Размер окантовки устанавливается функцией #gtk_cifro_area_set_border, текущее его значение возвращает
 * функция #gtk_cifro_area_get_border.
 *
 * При изменении размеров окна может происходить изменение коэффициента масштабирования или изменение
 * видимой области. Поведение виджета в этом случае задается функцией #gtk_cifro_area_set_scale_on_resize.
 * Парная ей функция #gtk_cifro_area_get_scale_on_resize возвращает текущее заданное поведение виджета.
 * Изменение коэффициента масштабирования производится только при произвольном изменении масштабов
 * без сохранения пропорций.
 *
 * Масштабирование, вращением колесика мышки, может осуществляться относительно центра изображения или
 * относительно текущего положения курсора, при этом точка изображения под курсором всегда остается на
 * своем месте. Поведение виджета в этом случае задается функцией #gtk_cifro_area_set_zoom_on_center.
 * Парная ей функция #gtk_cifro_area_get_zoom_on_center возвращает текущее заданное поведение виджета.
 *
 * В случае произвольного изменения масштабов можно поддерживать заданную пропорцию между масштабом
 * по вертикальной и горизонтальной оси. Величина этой пропорции задается функцией #gtk_cifro_area_set_scale_aspect.
 * Задаваемая величина должна иметь значение большее нуля, в противном случае поддержание пропорции
 * в масштабах отменяется. Текущая заданная величина пропорции между масштабами может быть получена
 * функцией #gtk_cifro_area_get_scale_aspect.
 *
 * Перемещение изображения клавишами вверх, вниз, вправо, влево происходит на один экранный пиксель. Если при
 * этом одновременно удерживать нажатой клавишу Ctrl, перемещение происходит на число пикселей указанное
 * функцией #gtk_cifro_area_set_move_multiplier. Текущая величина смещения может быть получена
 * функцией #gtk_cifro_area_get_move_multiplier.
 *
 * Поворот изображения клавишами вправо, влево происходит на 1 градус, если при этом одновременно удерживать
 * нажатой клавишу Ctrl, поворот происходит на величину угла указанную функцией #gtk_cifro_area_set_rotate_multiplier.
 * Текущая величина изменения угла поворота может быть получена функцией #gtk_cifro_area_get_rotate_multiplier.
 *
 * Поворот изображения может быть разрешен или отменен функцией #gtk_cifro_area_set_rotation в любой момент
 * времени. Однако запрещение поворота не влияет на уже установленный угол поворота. Если пользователь хочет
 * вернуть прежнее состояние он должен самостоятельно установить угол поворота до запрещения поворота.
 * Текущее состояние разрешения поворота можно получить функцией #gtk_cifro_area_get_rotation.
 *
 * Пределы возможных отображаемых значений до которых можно производить перемещение изображения задаются
 * функцией #gtk_cifro_area_set_view_limits, текущие пределы можно узнать функцией #gtk_cifro_area_get_view_limits.
 *
 * Границы изменения масштабов определяются функцией #gtk_cifro_area_set_scale_limits, текущие границы
 * можно узнать функцией #gtk_cifro_area_get_scale_limits.
 *
 * Масштабирование изображения производится на величину, определенную функцией #gtk_cifro_area_set_zoom_scale.
 * Величина изменения масштаба задается в процентах. Текущая величина изменения масштаба может быть
 * получена функцией #gtk_cifro_area_get_zoom_scale.
 *
 * Помимо произвольного изменения масштаба можно определить фиксированный набор масштабов, которые будут
 * применяться последовательно при выполнении соответствующей операции. Набор масштабов можно задать
 * или получить его текущее значение функциями #gtk_cifro_area_set_fixed_zoom_scales и
 * #gtk_cifro_area_get_fixed_zoom_scales соответственно. Если набор фиксированных масштабов определен,
 * будут использоваться только они. Для отмены этого поведения необходимо зарегистрировать пустой набор.
 *
 * Текущие границы изображения задаются функцией #gtk_cifro_area_set_view, возвращаются #gtk_cifro_area_get_view.
 *
 * Угол поворота изображения задается функцией #gtk_cifro_area_set_angle, возвращается #gtk_cifro_area_get_angle.
 *
 * Функции #gtk_cifro_area_move, #gtk_cifro_area_rotate, #gtk_cifro_area_zoom, #gtk_cifro_area_fixed_zoom используются для
 * программного управления перемещением, поворотом и масштабированием изображения соответственно.
 *
 */

#ifndef __GTK_CIFRO_AREA_H__
#define __GTK_CIFRO_AREA_H__

#include <gtk/gtk.h>
#include <gtk-cifro-area-state.h>

G_BEGIN_DECLS

#define GTK_TYPE_CIFRO_AREA             (gtk_cifro_area_get_type ())
#define GTK_CIFRO_AREA(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), GTK_TYPE_CIFRO_AREA, GtkCifroArea))
#define GTK_IS_CIFRO_AREA(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GTK_TYPE_CIFRO_AREA))
#define GTK_CIFRO_AREA_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), GTK_TYPE_CIFRO_AREA, GtkCifroAreaClass))
#define GTK_IS_CIFRO_AREA_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), GTK_TYPE_CIFRO_AREA))
#define GTK_CIFRO_AREA_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), GTK_TYPE_CIFRO_AREA, GtkCifroAreaClass))

typedef struct _GtkCifroArea GtkCifroArea;
typedef struct _GtkCifroAreaPrivate GtkCifroAreaPrivate;
typedef struct _GtkCifroAreaClass GtkCifroAreaClass;

struct _GtkCifroArea
{
  GtkDrawingArea parent_instance;

  GtkCifroAreaPrivate *priv;
};

struct _GtkCifroAreaClass
{
  GtkDrawingAreaClass parent_class;
};

GTK_CIFROAREA_EXPORT
GType                  gtk_cifro_area_get_type                 (void);

/**
 *
 * Функция создаёт виджет \link GtkCifroArea \endlink.
 *
 * \return Указатель на созданый виджет.
 *
 */
GTK_CIFROAREA_EXPORT
GtkWidget              *gtk_cifro_area_new                     (void);

/**
 *
 * Функция возвращает указатель на объект \link GtkCifroAreaState \endlink. Объект
 * \link GtkCifroAreaState \endlink принадлежит \link GtkCifroArea \endlink и не должен
 * удаляться после использования.
 *
 * \param carea указатель на виджет \link GtkCifroArea \endlink.
 *
 * \return Указатель на объект \link GtkCifroAreaState \endlink или NULL.
 *
 */
GTK_CIFROAREA_EXPORT
GtkCifroAreaState     *gtk_cifro_area_get_state                (GtkCifroArea          *carea);

/**
 *
 * Функция опрашивает модули формирования изображения на предмет необходимости
 * перерисовки изображения на экране. Если перерисовка необходима, она выполняется.
 *
 * \param carea указатель на виджет \link GtkCifroArea \endlink.
 *
 * \return Нет.
 *
 */
GTK_CIFROAREA_EXPORT
void                   gtk_cifro_area_update                   (GtkCifroArea          *carea);

/**
 *
 * Функция устанавливает необходимость рисования рамки при нахождении виджетаа в фокусе.
 *
 * \param carea указатель на виджет \link GtkCifroArea \endlink;
 * \param draw_focus рисовать - TRUE или нет - FALSE рамку при нахождении в фокусе.
 *
 * \return Нет.
 *
 */
GTK_CIFROAREA_EXPORT
void                   gtk_cifro_area_set_draw_focus           (GtkCifroArea          *carea,
                                                                gboolean               draw_focus);
/**
 *
 * Функция возвращает текущие параметры рисования рамки при нахождении виджета в фокусе.
 *
 * \param carea указатель на виджет \link GtkCifroArea \endlink;
 *
 * \return Рисовать - TRUE или нет - FALSE рамку при нахождении в фокусе.
 *
 */
GTK_CIFROAREA_EXPORT
gboolean               gtk_cifro_area_get_draw_focus           (GtkCifroArea          *carea);

/**
 *
 * Функция устанавливает вид курсора, используемый при нахождении мышки в видимой области.
 *
 *
 * \param carea указатель на виджет \link GtkCifroArea \endlink;
 * \param cursor указатель на GdkCursor или NULL.
 *
 * \return Нет.
 *
 */
GTK_CIFROAREA_EXPORT
void                   gtk_cifro_area_set_point_cursor         (GtkCifroArea          *carea,
                                                                GdkCursor             *cursor);

/**
 *
 * Функция устанавливает вид курсора, используемый при перемещении видимой области.
 *
 *
 * \param carea указатель на виджет \link GtkCifroArea \endlink;
 * \param cursor указатель на GdkCursor или NULL.
 *
 * \return Нет.
 *
 */
GTK_CIFROAREA_EXPORT
void                   gtk_cifro_area_set_move_cursor          (GtkCifroArea          *carea,
                                                                GdkCursor             *cursor);

/**
 *
 * Функция устанавливает параметры зеркального отражения изображения по осям.
 *
 * \param carea указатель на виджет \link GtkCifroArea \endlink;
 * \param swap_x отражать зеркально изображение относительно вертикальной оси - TRUE или нет - FALSE;
 * \param swap_y отражать зеркально изображение относительно горизонтальной оси - TRUE или нет - FALSE.
 *
 * \return Нет.
 *
 */
GTK_CIFROAREA_EXPORT
void                   gtk_cifro_area_set_swap                 (GtkCifroArea          *carea,
                                                                gboolean               swap_x,
                                                                gboolean               swap_y);

/**
 *
 * Функция возвращает текущие параметры зеркального отражения изображения по осям.
 *
 * \param carea указатель на виджет \link GtkCifroArea \endlink;
 * \param swap_x отражать зеркально изображение относительно вертикальной оси - TRUE или нет - FALSE или NULL;
 * \param swap_y отражать зеркально изображение относительно горизонтальной оси - TRUE или нет - FALSE или NULL.
 *
 * \return Нет.
 *
 */
GTK_CIFROAREA_EXPORT
void                   gtk_cifro_area_get_swap                 (GtkCifroArea          *carea,
                                                                gboolean              *swap_x,
                                                                gboolean              *swap_y);

/**
 *
 * Функция устанавливает размеры области окантовки виджета.
 *
 * Отступы с каждой стороны должны находится в пределах 0 - 1024 точек.
 *
 * \param carea указатель на виджет \link GtkCifroArea \endlink;
 * \param left отсуп с левой стороны;
 * \param right отступ с правой стороны;
 * \param top отступ сверху;
 * \param bottom отступ снизу.
 *
 * \return TRUE если размеры области окантовки установлены, FALSE в случае ошибки.
 *
 */
GTK_CIFROAREA_EXPORT
gboolean               gtk_cifro_area_set_border               (GtkCifroArea          *carea,
                                                                gint                   left,
                                                                gint                   right,
                                                                gint                   top,
                                                                gint                   bottom);

/**
 *
 * Функция возвращает текущие размеры области окантовки виджета.
 *
 * \param carea указатель на виджет \link GtkCifroArea \endlink;
 * \param left отсуп с левой стороны или NULL;
 * \param right отступ с правой стороны или NULL;
 * \param top отступ сверху или NULL;
 * \param bottom отступ снизу или NULL.
 *
 * \return Нет.
 *
 */
GTK_CIFROAREA_EXPORT
void                   gtk_cifro_area_get_border               (GtkCifroArea          *carea,
                                                                gint                  *left,
                                                                gint                  *right,
                                                                gint                  *top,
                                                                gint                  *bottom);

/**
 *
 * Функция устанавливает поведение масштаба при изменении размеров виджета.
 *
 * При изменении размеров виджета возможно изменение коэффициента масштабирования или
 * изменение видимой области изображения. Если есть определенные функцией #gtk_cifro_area_set_fixed_zoom_scales
 * фиксированные масштабы при изменении размеров виджета всегда будет изменяться видимая область изображения.
 *
 * \param carea указатель на виджет \link GtkCifroArea \endlink;
 * \param scale_on_resize изменять - TRUE или нет - FALSE масштаб при изменении размеров виджета.
 *
 * \return Нет.
 *
 */
GTK_CIFROAREA_EXPORT
void                   gtk_cifro_area_set_scale_on_resize      (GtkCifroArea          *carea,
                                                                gboolean               scale_on_resize);

/**
 *
 * Функция возвращает текущее поведение масштаба при изменении размеров виджета.
 *
 * \param carea указатель на виджет \link GtkCifroArea \endlink.
 *
 * \return Изменять - TRUE или нет - FALSE масштаб при изменении размеров виджета.
 *
 */
GTK_CIFROAREA_EXPORT
gboolean               gtk_cifro_area_get_scale_on_resize      (GtkCifroArea          *carea);

/**
 *
 * Функция устанавливает режим масштабирования: относительно центра или текущего положения курсора.
 *
 * Данный параметр влияет только на изменение масштаба с использованием колёсика мышки.
 *
 * \param carea указатель на виджет \link GtkCifroArea \endlink;
 * \param zoom_on_center масштабировать изображение относительно центра - TRUE или относительно текущего местоположения курсора - FALSE.
 *
 * \return Нет.
 *
 */
GTK_CIFROAREA_EXPORT
void                   gtk_cifro_area_set_zoom_on_center       (GtkCifroArea          *carea,
                                                                gboolean               zoom_on_center);

/**
 *
 * Функция возвращает текущий режим масштабирования.
 *
 * \param carea указатель на виджет \link GtkCifroArea \endlink.
 *
 * \return Масштабировать изображение относительно центра - TRUE или относительно текущего местоположения курсора - FALSE.
 *
 */
GTK_CIFROAREA_EXPORT
gboolean               gtk_cifro_area_get_zoom_on_center       (GtkCifroArea          *carea);

/**
 *
 * Функция устанавливает величину пропорциональности между масштабами по разным осям.
 *
 * Если величина пропорциональности определена, соотношение масштабов по разным осям будет оставаться
 * неизменным. Если задана отрицательная величина поддержание пропорции в масштабах отменяется.
 *
 * \param carea указатель на виджет \link GtkCifroArea \endlink;
 * \param scale_aspect величина пропорциональности между масштабами.
 *
 * \return TRUE если величина пропорциональности между масштабами установлена, FALSE в случае ошибки.
 *
 */
GTK_CIFROAREA_EXPORT
gboolean               gtk_cifro_area_set_scale_aspect         (GtkCifroArea          *carea,
                                                                gdouble                scale_aspect);

/**
 *
 * Функция возвращает текущую величину пропорциональности между масштабами.
 *
 * \param carea указатель на виджет \link GtkCifroArea \endlink.
 *
 * \return Величина пропорциональности между масштабами.
 *
 */
GTK_CIFROAREA_EXPORT
gdouble                gtk_cifro_area_get_scale_aspect         (GtkCifroArea          *carea);

/**
 *
 * Функция устанавливает величину перемещения изображения при нажатой клавише Ctrl.
 *
 * Величина перемещения должна быть больше нуля.
 *
 * \param carea указатель на виджет \link GtkCifroArea \endlink;
 * \param move_multiplier велечина перемещения в точках (может быть дробной).
 *
 * \return TRUE если величина перемещения установлена, FALSE в случае ошибки.
 *
 */
GTK_CIFROAREA_EXPORT
gboolean               gtk_cifro_area_set_move_multiplier      (GtkCifroArea          *carea,
                                                                gdouble                move_multiplier);

/**
 *
 * Функция возвращает текущую величину перемещения изображения при нажатой клавише Ctrl.
 *
 * \param carea указатель на виджет \link GtkCifroArea \endlink.
 *
 * \return Велечина перемещения в точках.
 *
 */
GTK_CIFROAREA_EXPORT
gdouble                gtk_cifro_area_get_move_multiplier      (GtkCifroArea          *carea);

/**
 *
 * Функция устанавливает величину поворота изображения при нажатой клавише Ctrl.
 *
 * Величина поворота должна быть больше нуля.
 *
 * \param carea указатель на виджет \link GtkCifroArea \endlink;
 * \param rotate_multiplier величина поворота в радианах.
 *
 * \return TRUE если величина поворота установлена, FALSE в случае ошибки.
 *
 */
GTK_CIFROAREA_EXPORT
gboolean               gtk_cifro_area_set_rotate_multiplier    (GtkCifroArea          *carea,
                                                                gdouble                rotate_multiplier);

/**
 *
 * Функция возвращает текущую величину поворота изображения при нажатой клавише Ctrl.
 *
 * \param carea указатель на виджет \link GtkCifroArea \endlink.
 *
 * \return Величина поворота в радианах.
 *
 */
GTK_CIFROAREA_EXPORT
gdouble                gtk_cifro_area_get_rotate_multiplier    (GtkCifroArea          *carea);

/**
 *
 * Функция разрешает или запрещает поворот изображения.
 *
 * Изменение параметра разрешения поворота изображения не изменяет
 * текущий угол поворота.
 *
 * \param carea указатель на виджет \link GtkCifroArea \endlink;
 * \param rotation резрешать - TRUE или нет - FALSE изменение угла поворота изображения.
 *
 * \return Нет.
 *
 */
GTK_CIFROAREA_EXPORT
void                   gtk_cifro_area_set_rotation             (GtkCifroArea          *carea,
                                                                gboolean               rotation);

/**
 *
 * Функция возвращает текущее значение параметра разрешения поворота изображения.
 *
 * \param carea указатель на виджет \link GtkCifroArea \endlink.
 *
 * \return Разрешать - TRUE или нет - FALSE изменение угла поворота изображения.
 *
 */
GTK_CIFROAREA_EXPORT
gboolean               gtk_cifro_area_get_rotation             (GtkCifroArea          *carea);

/**
 *
 * Функция устанавливает пределы перемещения изображения. Изображение может перемещаться только
 * в этих пределах. Формирование изображение, также требуется только в этих пределах. Пределы
 * задаются в условных единицах, реальные размеры в точках зависят от коэффициента масштабирования
 * и представления данных модулем формирования изображения.
 *
 * \param carea указатель на виджет \link GtkCifroArea \endlink;
 * \param min_x минимально возможное значение по оси x;
 * \param max_x максимально возможное значение по оси x;
 * \param min_y минимально возможное значение по оси y;
 * \param max_y максимально возможное значение по оси y.
 *
 * \return TRUE если пределы установлены, FALSE в случае ошибки.
 *
 */
GTK_CIFROAREA_EXPORT
gboolean               gtk_cifro_area_set_view_limits          (GtkCifroArea          *carea,
                                                                gdouble                min_x,
                                                                gdouble                max_x,
                                                                gdouble                min_y,
                                                                gdouble                max_y);

/**
 *
 * Функция возвращает текущие значения пределов перемещения изображения.
 *
 * \param carea указатель на виджет \link GtkCifroArea \endlink;
 * \param min_x минимально возможное значение по оси x или NULL;
 * \param max_x максимально возможное значение по оси x или NULL;
 * \param min_y минимально возможное значение по оси y или NULL;
 * \param max_y максимально возможное значение по оси y или NULL.
 *
 * \return Нет.
 *
 */
GTK_CIFROAREA_EXPORT
void                   gtk_cifro_area_get_view_limits          (GtkCifroArea          *carea,
                                                                gdouble               *min_x,
                                                                gdouble               *max_x,
                                                                gdouble               *min_y,
                                                                gdouble               *max_y);

/**
 *
 * Функция устанавливает границы изменения масштабов.
 *
 * \param carea указатель на виджет \link GtkCifroArea \endlink;
 * \param min_scale_x минимально возможный коэффициент масштаба по оси x (приближение);
 * \param max_scale_x максимально возможный коэффициент масштаба по оси x (отдаление);
 * \param min_scale_y минимально возможный коэффициент масштаба по оси y (приближение);
 * \param max_scale_y максимально возможный коэффициент масштаба по оси y (отдаление).
 *
 * \return TRUE если границы изменения масштабов установлены, FALSE в случае ошибки.
 *
 */
GTK_CIFROAREA_EXPORT
gboolean               gtk_cifro_area_set_scale_limits         (GtkCifroArea          *carea,
                                                                gdouble                min_scale_x,
                                                                gdouble                max_scale_x,
                                                                gdouble                min_scale_y,
                                                                gdouble                max_scale_y);

/**
 *
 * Функция возвращает текущие границы изменения масштабов.
 *
 * \param carea указатель на виджет \link GtkCifroArea \endlink;
 * \param min_scale_x минимально возможный коэффициент масштаба по оси x (приближение) или NULL;
 * \param max_scale_x максимально возможный коэффициент масштаба по оси x (отдаление) или NULL;
 * \param min_scale_y минимально возможный коэффициент масштаба по оси y (приближение) или NULL;
 * \param max_scale_y максимально возможный коэффициент масштаба по оси y (отдаление) или NULL.
 *
 * \return Нет.
 *
 */
GTK_CIFROAREA_EXPORT
void                   gtk_cifro_area_get_scale_limits         (GtkCifroArea          *carea,
                                                                gdouble               *min_scale_x,
                                                                gdouble               *max_scale_x,
                                                                gdouble               *min_scale_y,
                                                                gdouble               *max_scale_y);

/**
 *
 * Функция устанавливает величину изменения масштаба.
 *
 * Величина изменения масштаба определяет на сколько измениться масштаб за один шаг.
 * Величина изменения масштаба задается в процентах и должна быть больше нуля.
 *
 * \param carea указатель на виджет \link GtkCifroArea \endlink;
 * \param zoom_scale величина изменения масштаба.
 *
 * \return TRUE если величина изменения масштаба установлена, FALSE в случае ошибки.
 *
 */
GTK_CIFROAREA_EXPORT
gboolean               gtk_cifro_area_set_zoom_scale           (GtkCifroArea          *carea,
                                                                gdouble                zoom_scale);

/**
 *
 * Функция возвращает текущую величину изменения масштаба.
 *
 * \param carea указатель на виджет \link GtkCifroArea \endlink.
 *
 * \return Величина изменения масштаба.
 *
 */
GTK_CIFROAREA_EXPORT
gdouble                gtk_cifro_area_get_zoom_scale           (GtkCifroArea          *carea);

/**
 *
 * Функция устанавливает фиксированный набор масштабов.
 *
 * Если фиксированный набор масштабов определен используются только коэффициенты масштабирования
 * из этого набора. Если задать пустой набор (num_scales = 0) использование фиксированного набора
 * масштабов отменяется.
 *
 * \param carea указатель на виджет \link GtkCifroArea \endlink;
 * \param zoom_x_scales указатель на массив коэффициентов масштабирования по оси x;
 * \param zoom_y_scales указатель на массив коэффициентов масштабирования по оси y;
 * \param num_scales число коэффициентов масштабирования.
 *
 * \return TRUE если фиксированный набор масштабов установлен, FALSE в случае ошибки.
 *
 */
GTK_CIFROAREA_EXPORT
gboolean               gtk_cifro_area_set_fixed_zoom_scales    (GtkCifroArea          *carea,
                                                                gdouble               *zoom_x_scales,
                                                                gdouble               *zoom_y_scales,
                                                                gint                   num_scales);

/**
 *
 * Функция возвращает текущий фиксированный набор масштабов.
 *
 * Функция сама выделит память под массивы и вернет указатели на них. Пользователь должен
 * самостоятельно освободить память массивов функцией g_free.
 *
 * \param carea указатель на виджет \link GtkCifroArea \endlink;
 * \param zoom_x_scales указатель на массив коэффициентов масштабирования по оси x или NULL;
 * \param zoom_y_scales указатель на массив коэффициентов масштабирования по оси y или NULL;
 * \param num_scales число коэффициентов масштабирования или NULL.
 *
 * \return Нет.
 *
 */
GTK_CIFROAREA_EXPORT
void                   gtk_cifro_area_get_fixed_zoom_scales    (GtkCifroArea          *carea,
                                                                gdouble              **zoom_x_scales,
                                                                gdouble              **zoom_y_scales,
                                                                gint                  *num_scales);

/**
 *
 * Функция устанавливает границы текущей видимости изображения.
 *
 * \param carea указатель на виджет \link GtkCifroArea \endlink;
 * \param from_x минимальная граница изображения по оси x;
 * \param to_x максимальная граница изображения по оси x;
 * \param from_y минимальная граница изображения по оси y;
 * \param to_y максимальная граница изображения по оси y.
 *
 * \return TRUE если границы текущей видимости изображения установлены, FALSE в случае ошибки.
 *
 */
GTK_CIFROAREA_EXPORT
gboolean               gtk_cifro_area_set_view                 (GtkCifroArea          *carea,
                                                                gdouble                from_x,
                                                                gdouble                to_x,
                                                                gdouble                from_y,
                                                                gdouble                to_y);

/**
 *
 * Функция возвращает границы текущей видимости изображения.
 *
 * \param carea указатель на виджет \link GtkCifroArea \endlink;
 * \param from_x минимальная граница изображения по оси x или NULL;
 * \param to_x максимальная граница изображения по оси x или NULL;
 * \param from_y минимальная граница изображения по оси y или NULL;
 * \param to_y максимальная граница изображения по оси y или NULL.
 *
 * \return Нет.
 *
 */
GTK_CIFROAREA_EXPORT
void                   gtk_cifro_area_get_view                 (GtkCifroArea          *carea,
                                                                gdouble               *from_x,
                                                                gdouble               *to_x,
                                                                gdouble               *from_y,
                                                                gdouble               *to_y);
/**
 *
 * Функция устанавливает угол поворота изображения.
 *
 * \param carea указатель на виджет \link GtkCifroArea \endlink;
 * \param angle угол поворота изображения в радианах.
 *
 * \return Нет.
 *
 */
GTK_CIFROAREA_EXPORT
void                   gtk_cifro_area_set_angle                (GtkCifroArea          *carea,
                                                                gdouble                angle);

/**
 *
 * Функция возвращает текущее значение угола поворота изображения.
 *
 * \param carea указатель на виджет \link GtkCifroArea \endlink.
 *
 * \return Угол текущего поворота изображения в радианах.
 *
 */
GTK_CIFROAREA_EXPORT
gdouble                gtk_cifro_area_get_angle                (GtkCifroArea          *carea);

/**
 *
 * Функция смещает изображение на определенный шаг.
 *
 * \param carea указатель на виджет \link GtkCifroArea \endlink;
 * \param step_x шаг смещений по оси x ;
 * \param step_y шаг смещений по оси y.
 *
 * \return Нет.
 *
 */
GTK_CIFROAREA_EXPORT
void                   gtk_cifro_area_move                     (GtkCifroArea          *carea,
                                                                gdouble                step_x,
                                                                gdouble                step_y);

/**
 *
 * Функция поворачивает изображение на определенный угол.
 *
 * Угол добавляется к текущему углу на который повернуто изображение.
 *
 * \param carea указатель на виджет \link GtkCifroArea \endlink;
 * \param angle угол поворота изображения.
 *
 * \return Нет.
 *
 */
GTK_CIFROAREA_EXPORT
void                   gtk_cifro_area_rotate                   (GtkCifroArea          *carea,
                                                                gdouble                angle);

/**
 *
 * Функция изменяет текущий масштаб изображения.
 *
 * \param carea указатель на виджет \link GtkCifroArea \endlink;
 * \param zoom_x масштабировать по оси x;
 * \param zoom_y масштабировать по оси y;
 * \param center_val_x центр масштабирования по оси x;
 * \param center_val_y центр масштабирования по оси y;
 * \param zoom_in TRUE - приближение, FALSE - отдаление;
 * \param zoom_scale величина изменения масштаба в процентах.
 *
 * \return Нет.
 *
 */
GTK_CIFROAREA_EXPORT
void                   gtk_cifro_area_zoom                     (GtkCifroArea          *carea,
                                                                gboolean               zoom_x,
                                                                gboolean               zoom_y,
                                                                gdouble                center_val_x,
                                                                gdouble                center_val_y,
                                                                gboolean               zoom_in,
                                                                gdouble                zoom_scale);

/**
 *
 * Функция переключает фиксированый масштаб.
 *
 * \param carea указатель на виджет \link GtkCifroArea \endlink;
 * \param center_val_x центр масштабирования по оси x;
 * \param center_val_y центр масштабирования по оси y;
 * \param zoom_in TRUE - приближение, FALSE - отдаление.
 *
 * \return Нет.
 *
 */
GTK_CIFROAREA_EXPORT
void                   gtk_cifro_area_fixed_zoom               (GtkCifroArea          *carea,
                                                                gdouble                center_val_x,
                                                                gdouble                center_val_y,
                                                                gboolean               zoom_in);

G_END_DECLS

#endif /* __GTK_CIFRO_AREA_H__ */
