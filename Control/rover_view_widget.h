#ifndef ROVER_VIEW_WIDGET_H
#define ROVER_VIEW_WIDGET_H

#include <QWidget>
#include <QPainter>
#include <QTimer>
#include <cmath>

class RoverViewWidget : public QWidget
{
    Q_OBJECT

public:
    explicit RoverViewWidget(QWidget *parent = nullptr);
    void updateWheelAngles(float alf, float alr, float arf, float arr);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    // Углы поворота колес
    float wheelAngleALF; // Переднее левое
    float wheelAngleALR; // Заднее левое
    float wheelAngleARF; // Переднее правое
    float wheelAngleARR; // Заднее правое

    // Размеры марсохода
    const float ROVER_WIDTH = 200.0f;  // Ширина в пикселях
    const float ROVER_LENGTH = 300.0f; // Длина в пикселях
    const float WHEEL_RADIUS = 30.0f;  // Радиус колеса
    const float WHEEL_WIDTH = 15.0f;   // Ширина колеса

    void drawRoverBody(QPainter &painter);
    void drawWheel(QPainter &painter, float x, float y, float angle);
};

#endif // ROVER_VIEW_WIDGET_H 