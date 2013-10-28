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

#include "glm.h"

#include <QMainWindow>
#include <QAction>
#include <QGLFormat>

class VoxelFile;
class VoxelEditor;
class Map;
class MapEditor;
class PaletteEditor;
class QActionGroup;
class ModelProperties;
class QMdiSubWindow;
class QMdiArea;
class PropEditor;
class QDockWidget;
class MapProperties;
class QCloseEvent;
class QGLWidget;

#define POINTER_EDIT_TOOL 0
#define BLOCK_EDIT_TOOL 1
#define PENCIL_EDIT_TOOL 2
#define BUCKET_EDIT_TOOL 3

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    PaletteEditor * palette_editor;
    ModelProperties * model_properties;
    QActionGroup * tool_group;
    QGLFormat gl_format;
    QGLWidget * shared_gl;

    QMdiArea * mdi;
    QMenu * file_menu;
    QMenu * model_menu;

    QAction * new_model_action;
    QAction * open_model_action;
    QAction * clone_model_action;
    QAction * save_action;
    QAction * save_as_action;
    QAction * exit_action;

    QAction * double_size_action;
    QAction * half_size_action;
    QAction * optimize_action;
    QAction * rotate_action;

    QDockWidget * model_dock;
    QDockWidget * palette_dock;

    MainWindow(QWidget * parent = 0);
    void closeEvent(QCloseEvent * event);
    void create_menus();
    void create_actions();
    VoxelFile * get_voxel();
    int get_palette_index();
    void set_palette_index(int i);
    int get_tool();
    QWidget * get_current_window();
    VoxelEditor * get_voxel_editor();
    bool test_current_window(QWidget * other);
    ~MainWindow();

    void model_changed();
    void set_status(const std::string & text);
    void set_animation_frame(bool forward);

private slots:
    void on_window_change(QMdiSubWindow * w);

    void new_model();
    void open_model();
    void clone_model();
    void save();
    void save_as();

    void double_size();
    void half_size();
    void optimize();
    void rotate();
};