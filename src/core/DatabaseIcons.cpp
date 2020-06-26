/*
 *  Copyright (C) 2010 Felix Geyer <debfx@fobos.de>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 2 or (at your option)
 *  version 3 of the License.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "DatabaseIcons.h"

#include "core/Config.h"
#include "core/Global.h"
#include "core/Resources.h"
#include "gui/MainWindow.h"

#include <QDir>
#include <QImageReader>
#include <QPainter>
#include <QPixmapCache>

DatabaseIcons* DatabaseIcons::m_instance(nullptr);

namespace
{
    const QString iconDir = QStringLiteral(":/icons/database/");
    QStringList iconList;

    const QString badgeDir = QStringLiteral(":/icons/badges/");
    QStringList badgeList;
} // namespace

DatabaseIcons::DatabaseIcons()
{
    // Set the pixmap cache limit to 20 MB
    QPixmapCache::setCacheLimit(20480);

    iconList = QDir(iconDir).entryList(QDir::NoFilter, QDir::Name);
    badgeList = QDir(badgeDir).entryList(QDir::NoFilter, QDir::Name);

    m_compactMode = config()->get(Config::GUI_ApplicationDensity).toInt() != 0;
}

DatabaseIcons* DatabaseIcons::instance()
{
    if (!m_instance) {
        m_instance = new DatabaseIcons();
    }

    return m_instance;
}

QPixmap DatabaseIcons::icon(int index, IconSize size)
{
    if (index < 0 || index >= count()) {
        qWarning("DatabaseIcons::icon: invalid icon index %d", index);
        return {};
    }

    auto cacheKey = QString::number(index);
    auto icon = m_iconCache.value(cacheKey);
    if (icon.isNull()) {
        icon.addFile(iconDir + iconList[index]);
        icon.addPixmap(icon.pixmap(iconSize(IconSize::Default)));
        icon.addPixmap(icon.pixmap(iconSize(IconSize::Medium)));
        icon.addPixmap(icon.pixmap(iconSize(IconSize::Large)));
        m_iconCache.insert(cacheKey, icon);
    }

    return icon.pixmap(iconSize(size));
}

QPixmap DatabaseIcons::applyBadge(const QPixmap& basePixmap, Badges badgeIndex)
{
    const auto cacheKey = QStringLiteral("badgedicon-%1-%2").arg(basePixmap.cacheKey()).arg(badgeIndex);
    QPixmap pixmap = basePixmap;
    if (badgeIndex < 0 || badgeIndex >= badgeList.size()) {
        qWarning("DatabaseIcons: Out-of-range badge index given to applyBadge: %d", badgeIndex);
    } else if (!QPixmapCache::find(cacheKey, &pixmap)) {
        int baseSize = basePixmap.width();
        int badgeSize = baseSize <= iconSize(IconSize::Default) * basePixmap.devicePixelRatio() ? baseSize * 0.6 : baseSize * 0.5;
        QPoint badgePos(baseSize - badgeSize, baseSize - badgeSize);
        badgePos /= basePixmap.devicePixelRatio();

        QImageReader reader(badgeDir + badgeList[badgeIndex]);
        reader.setScaledSize({badgeSize, badgeSize});
        auto badge = QPixmap::fromImageReader(&reader);
        badge.setDevicePixelRatio(basePixmap.devicePixelRatio());

        QPainter painter(&pixmap);
        painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
        painter.drawPixmap(badgePos, badge);

        QPixmapCache::insert(cacheKey, pixmap);
    }

    return pixmap;
}

int DatabaseIcons::count()
{
    return iconList.size();
}

int DatabaseIcons::iconSize(IconSize size) {
    switch (size) {
    case Medium:
        return m_compactMode ? 26 : 30;
    case Large:
        return m_compactMode ? 30 : 36;
    default:
        return m_compactMode ? 16 : 22;
    }
}
