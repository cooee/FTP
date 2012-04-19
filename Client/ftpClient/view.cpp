
/****************************************************************************
 **
 ** Copyright (C) 2011 Nokia Corporation and/or its subsidiary(-ies).
 ** All rights reserved.
 ** Contact: Nokia Corporation (qt-info@nokia.com)
 **
 ** This file is part of the demonstration applications of the Qt Toolkit.
 **
 ** $QT_BEGIN_LICENSE:LGPL$
 ** GNU Lesser General Public License Usage
 ** This file may be used under the terms of the GNU Lesser General Public
 ** License version 2.1 as published by the Free Software Foundation and
 ** appearing in the file LICENSE.LGPL included in the packaging of this
 ** file. Please review the following information to ensure the GNU Lesser
 ** General Public License version 2.1 requirements will be met:
 ** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
 **
 ** In addition, as a special exception, Nokia gives you certain additional
 ** rights. These rights are described in the Nokia Qt LGPL Exception
 ** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
 **
 ** GNU General Public License Usage
 ** Alternatively, this file may be used under the terms of the GNU General
 ** Public License version 3.0 as published by the Free Software Foundation
 ** and appearing in the file LICENSE.GPL included in the packaging of this
 ** file. Please review the following information to ensure the GNU General
 ** Public License version 3.0 requirements will be met:
 ** http://www.gnu.org/copyleft/gpl.html.
 **
 ** Other Usage
 ** Alternatively, this file may be used in accordance with the terms and
 ** conditions contained in a signed written agreement between you and Nokia.
 **
 **
 **
 **
 **
 ** $QT_END_LICENSE$
 **
 ****************************************************************************/

 #include "view.h"

 #include <QtGui>
 #ifndef QT_NO_OPENGL
 #endif

 #include <qmath.h>

 View::View(const QString &name, QWidget *parent)
     : QFrame(parent)
 {
     setFrameStyle(Sunken | StyledPanel);
     int size = style()->pixelMetric(QStyle::PM_ToolBarIconSize);
     QSize iconSize(size, size);
     QToolButton *zoomInIcon = new QToolButton;
     zoomInIcon->setAutoRepeat(true);
     zoomInIcon->setAutoRepeatInterval(33);
     zoomInIcon->setAutoRepeatDelay(0);
//     zoomInIcon->setIcon(QPixmap(":/zoomin.png"));
     zoomInIcon->setIconSize(iconSize);
     QToolButton *zoomOutIcon = new QToolButton;
     zoomOutIcon->setAutoRepeat(true);
     zoomOutIcon->setAutoRepeatInterval(33);
     zoomOutIcon->setAutoRepeatDelay(0);
//     zoomOutIcon->setIcon(QPixmap(":/zoomout.png"));
     zoomOutIcon->setIconSize(iconSize);
     zoomSlider = new QSlider;
     zoomSlider->setMinimum(0);
     zoomSlider->setMaximum(500);
     zoomSlider->setValue(250);
     zoomSlider->setTickPosition(QSlider::TicksRight);

     // Zoom slider layout
     QVBoxLayout *zoomSliderLayout = new QVBoxLayout;
     zoomSliderLayout->addWidget(zoomInIcon);
     zoomSliderLayout->addWidget(zoomSlider);
     zoomSliderLayout->addWidget(zoomOutIcon);

     QToolButton *rotateLeftIcon = new QToolButton;
//     rotateLeftIcon->setIcon(QPixmap(":/rotateleft.png"));
     rotateLeftIcon->setIconSize(iconSize);
     QToolButton *rotateRightIcon = new QToolButton;
//     rotateRightIcon->setIcon(QPixmap(":/rotateright.png"));
     rotateRightIcon->setIconSize(iconSize);
     rotateSlider = new QSlider;
     rotateSlider->setOrientation(Qt::Horizontal);
     rotateSlider->setMinimum(-360);
     rotateSlider->setMaximum(360);
     rotateSlider->setValue(0);
     rotateSlider->setTickPosition(QSlider::TicksBelow);

     // Rotate slider layout
     QHBoxLayout *rotateSliderLayout = new QHBoxLayout;
     rotateSliderLayout->addWidget(rotateLeftIcon);
     rotateSliderLayout->addWidget(rotateSlider);
     rotateSliderLayout->addWidget(rotateRightIcon);

     resetButton = new QToolButton;
     resetButton->setText(tr("0"));
     resetButton->setEnabled(false);

     // Label layout
     QHBoxLayout *labelLayout = new QHBoxLayout;
     label = new QLabel(name);
     antialiasButton = new QToolButton;
     antialiasButton->setText(tr("Antialiasing"));
     antialiasButton->setCheckable(true);
     antialiasButton->setChecked(false);
     openGlButton = new QToolButton;
     openGlButton->setText(tr("OpenGL"));
     openGlButton->setCheckable(true);
 #ifndef QT_NO_OPENGL
//     openGlButton->setEnabled(QGLFormat::hasOpenGL());
 #else
     openGlButton->setEnabled(false);
 #endif
     printButton = new QToolButton;
//     printButton->setIcon(QIcon(QPixmap(":/fileprint.png")));

     labelLayout->addWidget(label);
     labelLayout->addStretch();
     labelLayout->addWidget(antialiasButton);
     labelLayout->addWidget(openGlButton);
     labelLayout->addWidget(printButton);

     QGridLayout *topLayout = new QGridLayout;
     topLayout->addLayout(labelLayout, 0, 0);
     topLayout->addLayout(zoomSliderLayout, 1, 1);
     topLayout->addLayout(rotateSliderLayout, 2, 0);
     topLayout->addWidget(resetButton, 2, 1);
     setLayout(topLayout);
 }