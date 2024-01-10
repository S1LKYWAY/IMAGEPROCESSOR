// mainwindow.cpp
#include "mainwindow.h"
#include <QMenuBar>
#include <QFileDialog>
#include <QPixmap>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    imageLabel = new QLabel(this);
    imageLabel->setScaledContents(true);

    setCentralWidget(imageLabel);

    QMenu *fileMenu = menuBar()->addMenu(tr("File"));
    QAction *openAction = fileMenu->addAction(tr("Open File"));
    connect(openAction, &QAction::triggered, this, &MainWindow::openFile);
}

void MainWindow::openFile()
{
    QString filePath = QFileDialog::getOpenFileName(this, tr("Open TIFF File"), QString(), tr("TIFF Files (*.tif *.tiff)"));

    if (!filePath.isEmpty())
    {
        displayImage(filePath);
    }
}

void MainWindow::displayImage(const QString &filePath)
{
    QPixmap pixmap(filePath);

    if (!pixmap.isNull())
    {
        imageLabel->setPixmap(pixmap);
        adjustSize();
    }
    else
    {
        // Handle error loading image
    }
}
