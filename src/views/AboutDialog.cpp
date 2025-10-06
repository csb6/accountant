/*
QAccountant - personal accounting software
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
#include <QFile>
#include <stdexcept>
#include "ui_AboutDialog.h"

using namespace Qt::StringLiterals;

struct AboutDialog::Impl {
    Ui::AboutDialog ui;
};

AboutDialog::AboutDialog(QWidget* parent)
    : QDialog(parent), m_impl(new Impl)
{
    m_impl->ui.setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose);
    QFile sbom_file{u":/qaccountant/about.md"_s};
    if(!sbom_file.open(QFile::ReadOnly | QFile::Text)) {
        throw std::runtime_error("Unable to read contents of About page");
    }
    m_impl->ui.text->setMarkdown(sbom_file.readAll());
}

AboutDialog::~AboutDialog() noexcept
{
    delete m_impl;
}
