#include "mainwindow.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    setObjectName("GameboyQt");
    setWindowTitle("Gameboy Emulator");

    setupMenuBar();

    fstream config("config.ini");
    string line;
    int w = 432, h = 480;
    if(config.is_open()) {
        while(getline(config, line)) {
            if(line.find("Width: "))
                w = atoi(line.substr(8).c_str());
            if(line.find("Height: "))
                h = atoi(line.substr(9).c_str());
        }
    } else {
        config << "Height: 480\n";
        config << "Width: 432\n";
        config.close();
    }
    resize(w, h);
}
void MainWindow::setupMenuBar() {
    QMenu* file = menuBar()->addMenu(tr("&File"));
    file->addAction(tr("Load rom"), this, &MainWindow::load);
    file->addSeparator();
    file->addAction(tr("&Quit"), this, &QWidget::close);

    QMenu* emulation = menuBar()->addMenu(tr("&Emulation"));
    emulation->addAction(tr("Pause"), this, &MainWindow::pause);
    emulation->addAction(tr("Resume"), this, &MainWindow::resume);
    emulation->addSeparator();
    emulation->addAction(tr("Fullscreen"), this, &MainWindow::fullscreen);

    QMenu* options = menuBar()->addMenu(tr("&Options"));
    options->addAction(tr("Video"), this, &MainWindow::openVideoWindow);
    options->addAction(tr("Input"), this, &MainWindow::openInputWindow);
    options->addAction(tr("Audio"), this, &MainWindow::openAudioWindow);

    QMenu* help = menuBar()->addMenu(tr("&Help"));
    help->addAction(tr("Report an Issue"), this, &MainWindow::openGithubPage);
    help->addAction(tr("About"), this, &MainWindow::openAboutWindow);
}

void MainWindow::load() {
    QString rom = QFileDialog::getOpenFileName(this, "Open a rom...", QDir::currentPath(), tr("Gameboy roms (*.gb);;All Files (*)"));
    core = new GB();
    core->load(rom.toStdString());
}

void MainWindow::pause() {

}

void MainWindow::resume() {

}

void MainWindow::fullscreen() {

}

void MainWindow::openVideoWindow() {

}

void MainWindow::openInputWindow() {

}

void MainWindow::openAudioWindow() {

}

void MainWindow::openGithubPage() {
    QDesktopServices::openUrl(QUrl())
}

void MainWindow::openAboutWindow() {

}
