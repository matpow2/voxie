/*
Copyright (c) 2013 Mathias Kaerlev

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

#include "color.h"

#include <QWidget>
#include <QLineEdit>

class MainWindow;
class PaletteGrid;
class QSpinBox;

class ColorSpace : public QWidget
{
    Q_OBJECT

public:
    QPixmap * pix;
    float hue, sat, val;

    ColorSpace(QWidget * parent = 0);
    void set_hue(float h);
    void set_hsv(float h, float s, float v);
    void set_mouse_pos(const QPoint & p);
    void mousePressEvent(QMouseEvent * event);
    void mouseMoveEvent(QMouseEvent* event);
    void paintEvent(QPaintEvent * event);
};

class ColorSlider : public QWidget
{
    Q_OBJECT

public:
    QPixmap * pix;
    float value;

    ColorSlider(QWidget * parent = 0);
    void set(float v);
    void set_mouse_pos(const QPoint & p);
    void mousePressEvent(QMouseEvent * event);
    void mouseMoveEvent(QMouseEvent* event);
    void paintEvent(QPaintEvent * event);
};

class PaletteGrid : public QWidget
{
    Q_OBJECT

public:
    MainWindow * window;
    int palette_index;
    QPoint drag_start;

    PaletteGrid(MainWindow * parent);
    int get_index(const QPoint & p);
    void paintEvent(QPaintEvent * event);
    void mousePressEvent(QMouseEvent * event);
    void mouseMoveEvent(QMouseEvent * event);
    void dragEnterEvent(QDragEnterEvent * event);
    void dropEvent(QDropEvent * event);
};

class PaletteEditor : public QWidget
{
    Q_OBJECT

public:
    MainWindow * window;
    PaletteGrid * grid;
    ColorSpace * color_space;
    ColorSlider * color_slider;
    QSpinBox * r_edit;
    QSpinBox * g_edit;
    QSpinBox * b_edit;
    QLineEdit * name;
    bool ignore_rgb;

    PaletteEditor(MainWindow * parent);
    int get_palette_index();
    void set_current();
    void set_palette();
    RGBColor & get_palette_color();
    QSpinBox * create_color_spinbox();
    void reset_palette();

public slots:
    void rgb_changed();
    void name_changed();
};