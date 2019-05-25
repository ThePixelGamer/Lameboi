#pragma once

#include <QMainWindow>
#include <QFileDialog>
#include <QMenu>
#include <QMenuBar>
#include <QDesktopServices>

#include <iostream>
#include <fstream>
#include <cstdlib>

#include "GB.h"

using namespace std;

class MainWindow : public QMainWindow {
public:
    GB* core;

    MainWindow(QWidget *parent = 0);
    void setupMenuBar();
    void load();
    void pause();
    void resume();
    void fullscreen();
    void openVideoWindow();
    void openInputWindow();
    void openAudioWindow();
    void openGithubPage();
    void openAboutWindow();
};
