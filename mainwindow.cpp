#include "mainwindow.h"

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    setObjectName("GameboyQt");
    setWindowTitle("Lameboi");
    setWindowIcon(QIcon(":/LBLogo.svg"));

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

    setupMenus();
    show();
}

void MainWindow::setupMenus() {
    QMenu* file = menuBar()->addMenu(tr("&File"));
    file->addAction(tr("Load rom"), this, &MainWindow::load);
    file->addSeparator();
    file->addAction(tr("&Quit"), this, &QWidget::close);
    context.addMenu(file);

    emulation = menuBar()->addMenu(tr("&Emulation"));
    emulation->addAction(tr("Pause"), this, &MainWindow::togglePause);
    emulation->actions().at(0)->setEnabled(false);
    emulation->addAction(tr("Fullscreen"), this, &MainWindow::fullscreen);
    emulation->actions().at(1)->setCheckable(true);
    emulation->actions().at(1)->setEnabled(false); //separator is an action
    emulation->addSeparator();
    //submenu
    debug = emulation->addMenu(tr("&Debugging"));
    debug->setEnabled(false);
    debug->addAction(tr("Debugger"), this, &MainWindow::openDebuggerWindow);
    debug->addAction(tr("VRAM Viewer"), this, &MainWindow::openVRAMWindow);

    emulation->addSeparator();
    emulation->addAction(tr("Options"), this, &MainWindow::openOptionsWindow);
    context.addMenu(emulation);

    QMenu* help = menuBar()->addMenu(tr("&Help"));
    help->addAction(tr("Report an Issue"), this, &MainWindow::openGithubPage);
    help->addAction(tr("About"), this, &MainWindow::openAboutWindow);
}

void MainWindow::load() {
    QString rom = QFileDialog::getOpenFileName(this, "Open a rom...", QDir::currentPath(), tr("Gameboy roms (*.gb);;All Files (*)"));

    if(!rom.isEmpty()) {
        glWidget = new EmulatorScreen(this);
        setCentralWidget(glWidget);

        core = new GB();
        core->load(rom.toStdString(), glWidget);
    }
}

void MainWindow::togglePause() {
    QAction* action = emulation->actions().at(0);
    action->setText((action->text() == "Pause") ? tr("Resume") : tr("Pause"));
}

void MainWindow::fullscreen() {
    fullscreenV = !fullscreenV;
    emulation->actions().at(1)->setChecked(fullscreenV);

    //if in game, go ahead and do dis
    if(fullscreenV) {
        menuBar()->hide();
        showFullScreen();
    } else {
        menuBar()->show();
        showNormal();
    }
}

void MainWindow::openOptionsWindow() {
    
}

void MainWindow::openDebuggerWindow() {

}

void MainWindow::openVRAMWindow() {

}

void MainWindow::openGithubPage() {
    QDesktopServices::openUrl(QUrl("https://github.com/ThePixelGamer/Lameboi/issues"));
}

void MainWindow::openAboutWindow() {
    QDialog* popup = new QDialog(this, (Qt::WindowCloseButtonHint | Qt::WindowTitleHint));
    popup->setWindowTitle("About");
    popup->setWindowIcon(QIcon(":/LBLogo.svg"));
    popup->setFixedSize(350, 300); //w,h

    QVBoxLayout* lAbout = new QVBoxLayout(popup);
    QLabel* wPicture = new QLabel(popup);
    QLabel* tAbout = new QLabel(popup);

    wPicture->setPixmap(QPixmap(":/LBLogoText.svg").scaled(320, 200, Qt::KeepAspectRatio));

    tAbout->setText("Gameboy Emulator written entirely using the qt api<br><div align=\"center\"><a href=\"https://github.com/ThePixelGamer/Lameboi\">Repository</a></div>");
    tAbout->setTextInteractionFlags(Qt::TextSelectableByMouse);
    tAbout->setTextInteractionFlags(Qt::TextBrowserInteraction);
    tAbout->setOpenExternalLinks(true);

    lAbout->addWidget(wPicture);
    lAbout->addWidget(tAbout);

    lAbout->setAlignment(wPicture, Qt::AlignHCenter);
    lAbout->setAlignment(tAbout, Qt::AlignHCenter);

    popup->setLayout(lAbout);
    popup->show();
}

void MainWindow::contextMenuEvent(QContextMenuEvent* event) {
    context.exec(event->globalPos());
}

void MainWindow::resizeEvent(QResizeEvent* event)
{
   QMainWindow::resizeEvent(event);
}
