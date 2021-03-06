/********************************************************************
KWin - the KDE window manager
This file is part of the KDE project.

Copyright (C) 2015 Martin Gräßlin <mgraesslin@kde.org>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*********************************************************************/
#ifndef KWIN_WAYLAND_TEST_H
#define KWIN_WAYLAND_TEST_H

#include "../../main.h"

// Qt
#include <QtTest/QtTest>

namespace KWayland
{
namespace Client
{
class ConnectionThread;
class Compositor;
class PlasmaShell;
class PlasmaWindowManagement;
class Seat;
class ServerSideDecorationManager;
class Shell;
class ShellSurface;
class ShmPool;
class Surface;
class XdgShellSurface;
}
}

namespace KWin
{

class AbstractClient;
class ShellClient;

class WaylandTestApplication : public Application
{
    Q_OBJECT
public:
    WaylandTestApplication(int &argc, char **argv);
    virtual ~WaylandTestApplication();

protected:
    void performStartup() override;

private:
    void createBackend();
    void createX11Connection();
    void continueStartupWithScreens();
    void continueStartupWithX();
    void startXwaylandServer();

    int m_xcbConnectionFd = -1;
    QProcess *m_xwaylandProcess = nullptr;
    QMetaObject::Connection m_xwaylandFailConnection;
};

namespace Test
{

enum class AdditionalWaylandInterface {
    Seat = 1 << 0,
    Decoration = 1 << 1,
    PlasmaShell = 1 << 2,
    WindowManagement = 1 << 3
};
Q_DECLARE_FLAGS(AdditionalWaylandInterfaces, AdditionalWaylandInterface)
/**
 * Creates a Wayland Connection in a dedicated thread and creates various
 * client side objects which can be used to create windows.
 * @param socketName The name of the Wayland socket to connect to.
 * @returns @c true if created successfully, @c false if there was an error
 * @see destroyWaylandConnection
 **/
bool setupWaylandConnection(const QString &socketName, AdditionalWaylandInterfaces flags = AdditionalWaylandInterfaces());

/**
 * Destroys the Wayland Connection created with @link{setupWaylandConnection}.
 * This can be called from cleanup in order to ensure that no Wayland Connection
 * leaks into the next test method.
 * @see setupWaylandConnection
 */
void destroyWaylandConnection();

KWayland::Client::ConnectionThread *waylandConnection();
KWayland::Client::Compositor *waylandCompositor();
KWayland::Client::Shell *waylandShell();
KWayland::Client::ShmPool *waylandShmPool();
KWayland::Client::Seat *waylandSeat();
KWayland::Client::ServerSideDecorationManager *waylandServerSideDecoration();
KWayland::Client::PlasmaShell *waylandPlasmaShell();
KWayland::Client::PlasmaWindowManagement *waylandWindowManagement();

bool waitForWaylandPointer();
bool waitForWaylandTouch();
bool waitForWaylandKeyboard();

void flushWaylandConnection();

KWayland::Client::Surface *createSurface(QObject *parent = nullptr);
enum class ShellSurfaceType {
    WlShell,
    XdgShellV5
};
QObject *createShellSurface(ShellSurfaceType type, KWayland::Client::Surface *surface, QObject *parent = nullptr);
KWayland::Client::ShellSurface *createShellSurface(KWayland::Client::Surface *surface, QObject *parent = nullptr);
KWayland::Client::XdgShellSurface *createXdgShellV5Surface(KWayland::Client::Surface *surface, QObject *parent = nullptr);

/**
 * Creates a shared memory buffer of @p size in @p color and attaches it to the @p surface.
 * The @p surface gets damaged and committed, thus it's rendered.
 **/
void render(KWayland::Client::Surface *surface, const QSize &size, const QColor &color, const QImage::Format &format = QImage::Format_ARGB32);

/**
 * Waits till a new ShellClient is shown and returns the created ShellClient.
 * If no ShellClient gets shown during @p timeout @c null is returned.
 **/
ShellClient *waitForWaylandWindowShown(int timeout = 5000);

/**
 * Combination of @link{render} and @link{waitForWaylandWindowShown}.
 **/
ShellClient *renderAndWaitForShown(KWayland::Client::Surface *surface, const QSize &size, const QColor &color, const QImage::Format &format = QImage::Format_ARGB32, int timeout = 5000);

/**
 * Waits for the @p client to be destroyed.
 **/
bool waitForWindowDestroyed(AbstractClient *client);

/**
 * Locks the screen and waits till the screen is locked.
 * @returns @c true if the screen could be locked, @c false otherwise
 **/
bool lockScreen();

/**
 * Unlocks the screen and waits till the screen is unlocked.
 * @returns @c true if the screen could be unlocked, @c false otherwise
 **/
bool unlockScreen();
}

}

Q_DECLARE_OPERATORS_FOR_FLAGS(KWin::Test::AdditionalWaylandInterfaces)
Q_DECLARE_METATYPE(KWin::Test::ShellSurfaceType)

#define WAYLANDTEST_MAIN_HELPER(TestObject, DPI) \
int main(int argc, char *argv[]) \
{ \
    setenv("QT_QPA_PLATFORM", "wayland-org.kde.kwin.qpa", true); \
    setenv("QT_QPA_PLATFORM_PLUGIN_PATH", KWINQPAPATH, true); \
    setenv("KWIN_FORCE_OWN_QPA", "1", true); \
    DPI; \
    KWin::WaylandTestApplication app(argc, argv); \
    app.setAttribute(Qt::AA_Use96Dpi, true); \
    TestObject tc; \
    return QTest::qExec(&tc, argc, argv); \
}

#if (QT_VERSION >= QT_VERSION_CHECK(5, 6, 0))
#define WAYLANDTEST_MAIN(TestObject) WAYLANDTEST_MAIN_HELPER(TestObject, QCoreApplication::setAttribute(Qt::AA_DisableHighDpiScaling) )
#else
#define WAYLANDTEST_MAIN(TestObject) WAYLANDTEST_MAIN_HELPER(TestObject,)
#endif

#endif
