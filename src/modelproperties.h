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

#include <QWidget>

class MainWindow;
class QSpinBox;
class QComboBox;
class QPushButton;
class ReferencePoint;

class ModelProperties : public QWidget
{
    Q_OBJECT

public:
    bool ignore_changes;

    QComboBox * ref_list;
    QPushButton * new_ref;
    QPushButton * del_ref;

    QSpinBox * off_x;
    QSpinBox * off_y;
    QSpinBox * off_z;

    QSpinBox * size_x;
    QSpinBox * size_y;
    QSpinBox * size_z;

    QSpinBox * ref_x;
    QSpinBox * ref_y;
    QSpinBox * ref_z;

    MainWindow * window;

    ModelProperties(MainWindow * parent);
    QSpinBox * create_spinbox();
    void update_refs();
    void update_controls();
    ReferencePoint * get_point();
    QSize sizeHint() const;

public slots:
    void on_change();
    void on_new_ref();
    void on_del_ref();
    void on_ref_name(const QString & text);
    void on_ref_change();
};