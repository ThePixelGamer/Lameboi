#pragma once

#include <QMainWindow>
#include <QFileDialog>
#include <QMenu>
#include <QMenuBar>
#include <QDesktopServices>
#include <QLabel>
#include <QtGui>
#include <QVBoxLayout>

#include <iostream>
#include <fstream>
#include <cstdlib>

#include "Gameboy/GB.h"
#include "emulatorscreen.h"

using namespace std;

class MainWindow : public QMainWindow {
public:
    EmulatorScreen* glWidget;
    GB* core;

    QMenu context;
    QMenu* emulation;
    QMenu* debug;

    bool fullscreenV = false;

    MainWindow(QWidget* parent = nullptr);
    virtual void contextMenuEvent(QContextMenuEvent*);
    virtual void resizeEvent(QResizeEvent*);

    void setupMenus();
    void load();
    void togglePause();
    void fullscreen();
    void openDebuggerWindow();
    void openVRAMWindow();
    void openOptionsWindow();
    void openGithubPage();
    void openAboutWindow();
};
