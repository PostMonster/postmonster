/* PostMonster, universal HTTP automation tool
 * Copyright (C) 2015 by Paul Artsishevsky <polter.rnd@gmail.com>
 *
 * This file is part of PostMonster.
 *
 * PostMonster is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * PostMonster is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with PostMonster.  If not, see <http://www.gnu.org/licenses/>.
 */
 
#ifndef CHECKBOXHEADER_H
#define CHECKBOXHEADER_H

#include <QHeaderView>
#include <QPainter>

class CheckBoxHeader : public QHeaderView
{
    Q_OBJECT

public:
    CheckBoxHeader(int checkableColumn, Qt::Orientation orientation, QWidget* parent = 0);

    bool isChecked() const { return m_checked; }
    void setChecked(bool checked);

signals:
    void checkBoxClicked(bool state);

protected:
    void paintSection(QPainter *painter, const QRect &rect, int logicalIndex) const;

private slots:
    void onSectionPressed(int logicalIndex);

private:
    int m_column;
    bool m_checked;
};
#endif // CHECKBOXHEADER_H
