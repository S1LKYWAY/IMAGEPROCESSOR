#include <QApplication>
#include <QMainWindow>
#include <QMenuBar>
#include <QFileDialog>
#include <QLabel>
#include <QImageReader>
#include <QMessageBox>
#include <QGridLayout>
#include <QPushButton>
#include <QColor>
#include <QDir>
#include <QtMath> // Added for qPow function

class MainWindow : public QMainWindow
{
public:
    MainWindow()
    {
        auto fileMenu = menuBar()->addMenu("File");
        auto openAction = fileMenu->addAction("Open File");
        connect(openAction, &QAction::triggered, this, &MainWindow::openFile);

        // Create a central widget and set a layout
        centralWidget = new QWidget(this);
        setCentralWidget(centralWidget);

        layout = new QGridLayout(centralWidget);

        originalImageLabel = new QLabel(this);
        processedImageLabel = new QLabel(this);

        layout->addWidget(originalImageLabel, 0, 0);
        layout->addWidget(processedImageLabel, 0, 1);

        // Add the "Next" button and initially hide it
        nextButton = new QPushButton("Next", this);
        layout->addWidget(nextButton, 1, 0, 1, 2);
        nextButton->hide();

        // Connect the button click signal to the nextImage function
        connect(nextButton, &QPushButton::clicked, this, &MainWindow::nextImage);
    }

private slots:
    void openFile()
    {
        QString fileName = QFileDialog::getOpenFileName(this, "Open File", QString(), "Images (*.jpeg *.jpg *.png)");

        if (!fileName.isEmpty())
        {
            QImageReader reader(fileName);
            reader.setAutoTransform(true);
            const QImage newImage = reader.read();
            if (newImage.isNull())
            {
                QMessageBox::information(this, QGuiApplication::applicationDisplayName(),
                                         tr("Cannot load %1: %2")
                                             .arg(QDir::toNativeSeparators(fileName), reader.errorString()));
            }
            else
            {
                originalColoredImage = newImage;
                updateDisplayedImages();

                // Show the "Next" button after loading and displaying the images
                nextButton->show();
            }
        }
    }

    void nextImage()
    {
        if (!originalColoredImage.isNull())
        {
            if (currentState == State::ColoredAndGrayscale)
            {
                // Move to the next state
                currentState = State::ColoredAndDarkened;

                // Display the original colored image on the left
                originalImageLabel->setPixmap(QPixmap::fromImage(originalColoredImage));
                originalImageLabel->adjustSize();

                // Darken the colored image
                QImage darkenedImage = adjustBrightness(originalColoredImage, 0.5);

                // Display the darkened image on the right
                processedImageLabel->setPixmap(QPixmap::fromImage(darkenedImage));
                processedImageLabel->adjustSize();
            }
            else if (currentState == State::ColoredAndDarkened)
            {
                // Move to the next state
                currentState = State::GrayscaleAndDithered;

                // Convert the original image to grayscale
                QImage grayscaleImage = originalColoredImage.convertToFormat(QImage::Format_Grayscale8);

                // Display the grayscale image on the left
                originalImageLabel->setPixmap(QPixmap::fromImage(grayscaleImage));
                originalImageLabel->adjustSize();

                // Dither the grayscale image
                QImage ditheredImage = ditherImage(grayscaleImage);

                // Display the dithered image on the right
                processedImageLabel->setPixmap(QPixmap::fromImage(ditheredImage));
                processedImageLabel->adjustSize();

                // Hide the "Next" button after the last state
                nextButton->hide();
            }
        }
    }

    void updateDisplayedImages()
    {
        // Display the colored image on the left
        originalImageLabel->setPixmap(QPixmap::fromImage(originalColoredImage));
        originalImageLabel->adjustSize();

        // Convert the original image to grayscale
        QImage grayscaleImage = originalColoredImage.convertToFormat(QImage::Format_Grayscale8);

        // Display the grayscale image on the right
        processedImageLabel->setPixmap(QPixmap::fromImage(grayscaleImage));
        processedImageLabel->adjustSize();
    }

    QImage adjustBrightness(const QImage &image, qreal factor)
    {
        QImage adjustedImage = image;

        // Convert the image to a suitable format for pixel manipulation
        if (adjustedImage.format() == QImage::Format_Mono || adjustedImage.format() == QImage::Format_Indexed8)
        {
            adjustedImage = adjustedImage.convertToFormat(QImage::Format_RGB32);
        }

        for (int y = 0; y < adjustedImage.height(); ++y)
        {
            for (int x = 0; x < adjustedImage.width(); ++x)
            {
                QRgb color = adjustedImage.pixel(x, y);

                int adjustedRed = qBound(0, static_cast<int>(qRed(color) * factor), 255);
                int adjustedGreen = qBound(0, static_cast<int>(qGreen(color) * factor), 255);
                int adjustedBlue = qBound(0, static_cast<int>(qBlue(color) * factor), 255);
                int adjustedAlpha = qBound(0, static_cast<int>(qAlpha(color)), 255);

                QColor adjustedColor = QColor::fromRgb(adjustedRed, adjustedGreen, adjustedBlue, adjustedAlpha);
                adjustedImage.setPixelColor(x, y, adjustedColor);
            }
        }

        return adjustedImage;
    }

    QImage ditherImage(const QImage &image)
    {
        QImage ditheredImage = image;

        // Dither using a simple 2x2 matrix
        int matrix[2][2] = {{1, 2},
                            {3, 0}};

        for (int y = 0; y < ditheredImage.height(); ++y)
        {
            for (int x = 0; x < ditheredImage.width(); ++x)
            {
                int oldPixel = qGray(ditheredImage.pixel(x, y));
                int newPixel = (oldPixel > matrix[x % 2][y % 2] * 255 / 3) ? 255 : 0;
                ditheredImage.setPixel(x, y, qRgb(newPixel, newPixel, newPixel));
            }
        }

        return ditheredImage;
    }

private:
    QWidget *centralWidget;
    QGridLayout *layout;
    QLabel *originalImageLabel;
    QLabel *processedImageLabel;
    QPushButton *nextButton;
    QImage originalColoredImage;

    enum class State
    {
        ColoredAndGrayscale,
        ColoredAndDarkened,
        GrayscaleAndDithered
    };

    State currentState = State::ColoredAndGrayscale;
};

int main(int argc, char **argv)
{
    QApplication app(argc, argv);

    MainWindow window;
    window.show();

    return app.exec();
}

