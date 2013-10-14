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

#include "mainwindow.h"
#include "voxeditor.h"
#include "palette.h"
#include "modelproperties.h"
#include "voxel.h"
#include "editorcommon.h"

#include <QToolBar>
#include <QMenuBar>
#include <QApplication>
#include <QDockWidget>
#include <QMdiArea>
#include <QTabBar>
#include <QMdiSubWindow>
#include <QClipboard>
#include <QMimeData>
#include <QStatusBar>
#include <QCloseEvent>
#include <QGLWidget>
#include <QFileDialog>

QAction * create_tool_icon(const QString & name, const QString & v,
                           QActionGroup * group, int id)
{
    QAction * action = new QAction(name, group);
    QPixmap img(v);
    img = img.scaled(img.size() * 2);
    action->setIcon(QIcon(img));
    action->setCheckable(true);
    if (id == BLOCK_EDIT_TOOL)
        action->setChecked(true);
    action->setData(id);
    return action;
}

MainWindow::MainWindow(QWidget * parent)
: QMainWindow(parent)
{
    gl_format = QGLFormat::defaultFormat();
    gl_format.setSampleBuffers(true);
    gl_format.setSamples(4);

    shared_gl = new QGLWidget(gl_format, this);

    mdi = new QMdiArea(this);
    mdi->setViewMode(QMdiArea::TabbedView);
    mdi->setTabsClosable(true);
    mdi->setTabsMovable(true);
    setCentralWidget(mdi);
    connect(mdi, SIGNAL(subWindowActivated(QMdiSubWindow*)), 
        SLOT(on_window_change(QMdiSubWindow*)));

    create_actions();
    create_menus();

    QToolBar * tool = new QToolBar("Tools", this);
    tool_group = new QActionGroup(this);
    tool->addAction(create_tool_icon("Pointer", "editor/pointer_tool.png",
        tool_group, POINTER_EDIT_TOOL));
    tool->addAction(create_tool_icon("Block", "editor/block_tool.png",
        tool_group, BLOCK_EDIT_TOOL));
    tool->addAction(create_tool_icon("Pencil", "editor/pencil_tool.png",
        tool_group, PENCIL_EDIT_TOOL));
    tool->addAction(create_tool_icon("Bucket", "editor/bucket_tool.png",
        tool_group, BUCKET_EDIT_TOOL));
    addToolBar(Qt::LeftToolBarArea, tool);

    model_dock = new QDockWidget("Model");
    model_properties = new ModelProperties(this);
    model_dock->setWidget(model_properties);
    addDockWidget(Qt::RightDockWidgetArea, model_dock);

    palette_dock = new QDockWidget("Palette");
    palette_editor = new PaletteEditor(this);
    palette_dock->setWidget(palette_editor);
    addDockWidget(Qt::RightDockWidgetArea, palette_dock);

    on_window_change(0);

    set_status("Ready...");
}

void MainWindow::create_menus()
{
    file_menu = menuBar()->addMenu(tr("&File"));
    file_menu->addAction(new_model_action);
    file_menu->addAction(open_model_action);
    file_menu->addSeparator();
    file_menu->addAction(save_action);
    file_menu->addAction(save_as_action);
    file_menu->addSeparator();
    file_menu->addAction(exit_action);

    model_menu = menuBar()->addMenu(tr("&Model"));
    model_menu->addAction(double_size_action);
    model_menu->addAction(half_size_action);
    model_menu->addAction(optimize_action);
    model_menu->addAction(rotate_action);
}

bool MainWindow::test_current_window(QWidget * other)
{
    return get_current_window() == other;
}

QWidget * MainWindow::get_current_window()
{
    QMdiSubWindow * w = mdi->currentSubWindow();
    if (!w)
        return NULL;
    return w->widget();
}

VoxelEditor * MainWindow::get_voxel_editor()
{
    return qobject_cast<VoxelEditor*>(get_current_window());
}

VoxelFile * MainWindow::get_voxel()
{
    VoxelEditor * ed = get_voxel_editor();
    if (ed)
        return ed->voxel;
    return NULL;
}

int MainWindow::get_palette_index()
{
    return palette_editor->grid->palette_index;
}

void MainWindow::set_palette_index(int i)
{
    palette_editor->grid->palette_index = i;
    palette_editor->set_current();
}

int MainWindow::get_tool()
{
    return tool_group->checkedAction()->data().toInt();
}

void MainWindow::model_changed()
{
    get_voxel_editor()->on_changed();
}

void MainWindow::create_actions()
{
    // file menu

    new_model_action = new QAction(tr("New voxel model"), this);
    connect(new_model_action, SIGNAL(triggered()), this, SLOT(new_model()));
 
    open_model_action = new QAction(tr("Open voxel model"), this);
    connect(open_model_action, SIGNAL(triggered()), this, SLOT(open_model()));
 
    save_action = new QAction(tr("&Save"), this);
    save_action->setShortcuts(QKeySequence::Save);
    connect(save_action, SIGNAL(triggered()), this, SLOT(save()));
 
    save_as_action = new QAction(tr("Save &As..."), this);
    save_as_action->setShortcuts(QKeySequence::SaveAs);
    connect(save_as_action, SIGNAL(triggered()), this, SLOT(save_as()));
 
    exit_action = new QAction(tr("E&xit"), this);
    exit_action->setShortcuts(QKeySequence::Quit);
    exit_action->setStatusTip(tr("Exit the application"));
    connect(exit_action, SIGNAL(triggered()), qApp, SLOT(closeAllWindows()));
 
/*        cut_action = new QAction(tr("Cu&t"), this);
    cut_action->setShortcuts(QKeySequence::Cut);
    cut_action->setStatusTip(tr("Cut the current selection's contents to the "
                            "clipboard"));
    connect(cut_action, SIGNAL(triggered()), this, SLOT(cut()));
 
    copy_action = new QAction(tr("&Copy"), this);
    copy_action->setShortcuts(QKeySequence::Copy);
    copy_action->setStatusTip(tr("Copy the current selection's contents to the "
                             "clipboard"));
    connect(copy_action, SIGNAL(triggered()), this, SLOT(copy()));
 
    paste_action = new QAction(tr("&Paste"), this);
    paste_action->setShortcuts(QKeySequence::Paste);
    paste_action->setStatusTip(tr("Paste the clipboard's contents into the current "
                              "selection"));
    connect(paste_action, SIGNAL(triggered()), this, SLOT(paste()));*/

    // model menu

    double_size_action = new QAction(tr("Double size"), this);
    // new_action->setShortcuts(QKeySequence::New);
    connect(double_size_action, SIGNAL(triggered()), this, 
        SLOT(double_size()));
 
    half_size_action = new QAction(tr("Half size"), this);
    // open_action->setShortcuts(QKeySequence::Open);
    connect(half_size_action, SIGNAL(triggered()), this, 
        SLOT(half_size()));
 
    optimize_action = new QAction(tr("Optimize dimensions"), this);
    // save_action->setShortcuts(QKeySequence::Save);
    connect(optimize_action, SIGNAL(triggered()), this,
        SLOT(optimize()));

    rotate_action = new QAction(tr("Rotate 90 degrees"), this);
    // save_action->setShortcuts(QKeySequence::Save);
    connect(rotate_action, SIGNAL(triggered()), this,
        SLOT(rotate()));

}

void MainWindow::closeEvent(QCloseEvent * event)
{

    while (true) {
        QMdiSubWindow * w = mdi->activeSubWindow();
        if (w == NULL)
            break;
        if (w->close())
            continue;
        event->ignore();
        return;
    }
    event->accept();
}

MainWindow::~MainWindow()
{

}

void MainWindow::set_status(const std::string & text)
{
    statusBar()->showMessage(text.c_str());
}

void MainWindow::on_window_change(QMdiSubWindow * w)
{
    VoxelEditor * v = get_voxel_editor();
    bool model_visible = v != 0;

    model_dock->setVisible(model_visible);
    palette_dock->setVisible(model_visible);
    model_menu->setEnabled(model_visible);

    if (model_visible) {
        model_properties->update_controls();
        palette_editor->set_current();
    }
}

void MainWindow::new_model()
{
    VoxelEditor * ed = new VoxelEditor(this);
    QMdiSubWindow * w = mdi->addSubWindow(ed);
    ed->reset();
    w->showMaximized();
}

void MainWindow::open_model()
{
    QString name = get_model_name(this, false);
    if (name.isEmpty())
        return;
    VoxelEditor * ed = new VoxelEditor(this);
    QMdiSubWindow * w = mdi->addSubWindow(ed);
    ed->load(name);
    w->showMaximized();
}

void MainWindow::save()
{
    QWidget * w = get_current_window();
    if (!w)
        return;
    QMetaObject::invokeMethod(w, "save");
}

void MainWindow::save_as()
{
    QWidget * w = get_current_window();
    if (!w)
        return;
    QMetaObject::invokeMethod(w, "save_as");
}

void MainWindow::double_size()
{
    VoxelFile * voxel = get_voxel();
    voxel->scale(2.0f, 2.0f, 2.0f);
    model_properties->update_controls();
    model_changed();
}

void MainWindow::half_size()
{
    VoxelFile * voxel = get_voxel();
    voxel->scale(0.5f, 0.5f, 0.5f);
    model_properties->update_controls();
    model_changed();
}

void MainWindow::optimize()
{
    VoxelFile * voxel = get_voxel();
    voxel->optimize();
    model_properties->update_controls();
    model_changed();
}

void MainWindow::rotate()
{
    VoxelFile * voxel = get_voxel();
    voxel->rotate();
    model_properties->update_controls();
    model_changed();
}

void MainWindow::set_animation_frame(bool forward)
{
    VoxelEditor * old = get_voxel_editor();
    if (forward)
        mdi->activateNextSubWindow();
    else
        mdi->activatePreviousSubWindow();
    if (old == NULL)
        return;
    VoxelEditor * v = get_voxel_editor();
    v->pos = old->pos;
    v->scale = old->scale;
    v->rotate_x = old->rotate_x;
    v->rotate_z = old->rotate_z;
    v->update();
    v->update_hit();
}
