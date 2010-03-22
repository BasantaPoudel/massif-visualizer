/*
   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef MASSIF_DOTGRAPHGENERATOR_H
#define MASSIF_DOTGRAPHGENERATOR_H

#include <QTextStream>
#include <QThread>

namespace Massif {

class SnapshotItem;
class TreeLeafItem;

class DotGraphGenerator : public QThread
{
    Q_OBJECT

public:
    /**
     * Generates a Dot graph file representing @p snapshot
     * and writes it to a temporary file.
     * NOTE: The file will not get deleted automatically, you have to do that!
     * @see outputFile()
     */
    DotGraphGenerator(const SnapshotItem* snapshot, QObject* parent = 0);
    ~DotGraphGenerator();

    /**
     * Stops generating the Dot graph file and deletes the temp file.
     */
    void cancel();

    virtual void run();
    /**
     * @return A path to the generated Dot graph file. Path might be empty if errors occurred.
     */
    QString outputFile() const;

private:
    void nodeToDot(TreeLeafItem* node, QTextStream& out, const QString& parent);
    const SnapshotItem* m_snapshot;
    QString m_file;
    bool m_canceled;
};

}

#endif // MASSIF_DOTGRAPHGENERATOR_H