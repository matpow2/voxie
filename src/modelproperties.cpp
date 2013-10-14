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

#include "voxel.h"
#include "modelproperties.h"
#include "mainwindow.h"
#include "editorcommon.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QSpinBox>
#include <QComboBox>
#include <QPushButton>

// ModelProperties

#define SPINBOX_RANGE 10000000

QSpinBox * ModelProperties::create_spinbox()
{
    QSpinBox * box = new QSpinBox(this);
    box->setRange(-SPINBOX_RANGE, SPINBOX_RANGE);
    connect(box, SIGNAL(valueChanged(int)), SLOT(on_change()));
    return box;
}

ModelProperties::ModelProperties(MainWindow * parent)
: QWidget(parent), window(parent), ignore_changes(true)
{
    QVBoxLayout * layout = new QVBoxLayout(this);

    QHBoxLayout * ref_list_layout = new QHBoxLayout;
    ref_list = new QComboBox(this);
    ref_list->setInsertPolicy(QComboBox::NoInsert);
    ref_list->setEditable(true);
    connect(ref_list, SIGNAL(currentIndexChanged(int)),
            SLOT(on_ref_change()));
    connect(ref_list, SIGNAL(editTextChanged(QString)),
            SLOT(on_ref_name(QString)));
    ref_list_layout->addWidget(ref_list);
    new_ref = new QPushButton("New");
    new_ref->setFixedWidth(50);
    ref_list_layout->addWidget(new_ref);
    connect(new_ref, SIGNAL(clicked(bool)), SLOT(on_new_ref()));
    del_ref = new QPushButton("Delete");
    del_ref->setFixedWidth(50);
    ref_list_layout->addWidget(del_ref);
    connect(del_ref, SIGNAL(clicked(bool)), SLOT(on_del_ref()));
    layout->addLayout(ref_list_layout);

    QHBoxLayout * ref_layout = new QHBoxLayout;
    ref_layout->addWidget(create_label("Ref"));
    ref_x = create_spinbox();
    ref_y = create_spinbox();
    ref_z = create_spinbox();
    ref_layout->addWidget(ref_x);
    ref_layout->addWidget(ref_y);
    ref_layout->addWidget(ref_z);
    layout->addLayout(ref_layout);

    QHBoxLayout * off_layout = new QHBoxLayout;
    off_layout->addWidget(create_label("Offset"));
    off_x = create_spinbox();
    off_y = create_spinbox();
    off_z = create_spinbox();
    off_layout->addWidget(off_x);
    off_layout->addWidget(off_y);
    off_layout->addWidget(off_z);
    layout->addLayout(off_layout);

    QHBoxLayout * size_layout = new QHBoxLayout;
    size_layout->addWidget(create_label("Size"));
    size_x = create_spinbox();
    size_x->setMinimum(1);
    size_y = create_spinbox();
    size_y->setMinimum(1);
    size_z = create_spinbox();
    size_z->setMinimum(1);
    size_layout->addWidget(size_x);
    size_layout->addWidget(size_y);
    size_layout->addWidget(size_z);
    layout->addLayout(size_layout);

/*    layout->addWidget(grid);
    layout->addWidget(color_space);
    layout->addWidget(color_slider);
*/
    setLayout(layout);
    ignore_changes = false;
}

void ModelProperties::update_controls()
{
    VoxelFile * voxel = window->get_voxel();
    if (voxel == NULL)
        return;
    ignore_changes = true;

    ref_list->clear();
    ReferencePoints::iterator it;
    for (it = voxel->points.begin(); it != voxel->points.end(); it++) {
        ReferencePoint & point = *it;
        ref_list->addItem(point.name);
    }

    off_x->setValue(voxel->x_offset);
    off_y->setValue(voxel->y_offset);
    off_z->setValue(voxel->z_offset);

    size_x->setValue(voxel->x_size);
    size_y->setValue(voxel->y_size);
    size_z->setValue(voxel->z_size);

    update_refs();

    ignore_changes = false;
}

ReferencePoint * ModelProperties::get_point()
{
    int i = ref_list->currentIndex();
    if (i == -1)
        return NULL;
    VoxelFile * voxel = window->get_voxel();
    if (voxel == NULL)
        return NULL;
    return window->get_voxel()->get_point(i);
}

#define DEFAULT_REFNAME "refname"

void ModelProperties::on_new_ref()
{
    ignore_changes = true;
    VoxelFile * voxel = window->get_voxel();
    if (voxel == NULL) {
        ignore_changes = false;
        return;
    }
    voxel->add_point(DEFAULT_REFNAME, 0, 0, 0);
    ref_list->addItem(DEFAULT_REFNAME);
    ref_list->setCurrentIndex(ref_list->count() - 1);
    ignore_changes = false;
    update_refs();
    window->model_changed();
}

void ModelProperties::on_del_ref()
{
    int i = ref_list->currentIndex();
    VoxelFile * voxel = window->get_voxel();
    if (voxel == NULL) {
        ignore_changes = false;
        return;
    }
    voxel->remove_point(i);
    ignore_changes = true;
    ref_list->removeItem(i);
    ignore_changes = false;
    update_refs();
    window->model_changed();
}

void ModelProperties::on_ref_change()
{
    if (ignore_changes)
        return;
    update_refs();
    window->model_changed();
}

void ModelProperties::on_ref_name(const QString & text)
{
    if (ignore_changes)
        return;
    ReferencePoint * p = get_point();
    if (p == NULL)
        return;
    p->name = text;
    ref_list->setItemText(ref_list->currentIndex(), text);
}

void ModelProperties::update_refs()
{
    bool disabled = ref_list->count() <= 0;
    del_ref->setDisabled(disabled);
    ref_list->setDisabled(disabled);
    ref_x->setDisabled(disabled);
    ref_y->setDisabled(disabled);
    ref_z->setDisabled(disabled);
    if (disabled)
        return;
    ignore_changes = true;
    ReferencePoint * p = get_point();
    if (p != NULL) {
        ref_x->setValue(p->x);
        ref_y->setValue(p->y);
        ref_z->setValue(p->z);
    }
    ignore_changes = false;
}

void ModelProperties::on_change()
{
    if (ignore_changes)
        return;
    VoxelFile * voxel = window->get_voxel();
    if (voxel == NULL)
        return;
    voxel->set_offset(off_x->value(), off_y->value(), off_z->value());
    voxel->resize(0, 0, 0, size_x->value(), size_y->value(), size_z->value());
    ReferencePoint * p = get_point();
    if (p != NULL) {
        p->x = ref_x->value();
        p->y = ref_y->value();
        p->z = ref_z->value();
    }
    window->model_changed();
}

QSize ModelProperties::sizeHint() const
{
    return minimumSize();
}