/*
Accountant - accounting program
Copyright (C) 2025  Cole Blakley

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/
#include "AboutDialog.hpp"
#include "ui_AboutDialog.h"

using namespace Qt::StringLiterals;

struct AboutDialog::Impl {
    Ui::AboutDialog ui;
};

AboutDialog::AboutDialog(QWidget* parent)
    : QDialog(parent), m_impl(new Impl)
{
    m_impl->ui.setupUi(this);
    m_impl->ui.text->setMarkdown(
        u"This program is licensed under the [GNU General Public License, version 3](https://www.gnu.org/licenses/gpl-3.0.html).\n\n"
        "It uses the following third-party libraries:\n"
        "- Qt Core (software version " QT_VERSION_STR ", LGPLv3 license)\n"
        "- Qt SQL (software version " QT_VERSION_STR ", LGPLv3 license)\n"
        "- Qt Widgets (software version " QT_VERSION_STR ", LGPLv3 license)\n"
        "- SQLite, (software version 3, Public Domain)\n\n"
        "For more information about Qt licensing and accessing Qt source code, visit https://www.qt.io/qt-licensing."_s
    );
}

AboutDialog::~AboutDialog() noexcept
{
    delete m_impl;
}
