#include "rover_view_widget.h"
#include <QPainter>

RoverViewWidget::RoverViewWidget(QWidget *parent)
    : QWidget(parent)
    , wheelAngleALF(0.0f)
    , wheelAngleALR(0.0f)
    , wheelAngleARF(0.0f)
    , wheelAngleARR(0.0f)
{
    setMinimumSize(400, 400);
    setBackgroundRole(QPalette::Base);
    setAutoFillBackground(true);
}

void RoverViewWidget::updateWheelAngles(float alf, float alr, float arf, float arr)
{
    wheelAngleALF = alf;
    wheelAngleALR = alr;
    wheelAngleARF = arf;
    wheelAngleARR = arr;
    update(); // Перерисовка виджета
}

void RoverViewWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // Центрируем марсоход в виджете
    float centerX = width() / 2.0f;
    float centerY = height() / 2.0f;

    // Смещаем начало координат в центр
    painter.translate(centerX, centerY);

    // Рисуем корпус марсохода
    drawRoverBody(painter);

    // Рисуем колеса
    float wheelOffsetX = ROVER_WIDTH / 2.0f;
    float wheelOffsetY = ROVER_LENGTH / 2.0f;

    // Переднее левое колесо
    drawWheel(painter, -wheelOffsetX, -wheelOffsetY, wheelAngleALF);
    // Заднее левое колесо
    drawWheel(painter, -wheelOffsetX, wheelOffsetY, wheelAngleALR);
    // Переднее правое колесо
    drawWheel(painter, wheelOffsetX, -wheelOffsetY, wheelAngleARF);
    // Заднее правое колесо
    drawWheel(painter, wheelOffsetX, wheelOffsetY, wheelAngleARR);
}

void RoverViewWidget::drawRoverBody(QPainter &painter)
{
    // Рисуем корпус марсохода
    QRectF bodyRect(-ROVER_WIDTH/2, -ROVER_LENGTH/2, ROVER_WIDTH, ROVER_LENGTH);
    
    // Основной корпус
    painter.setPen(QPen(Qt::black, 2));
    painter.setBrush(QBrush(Qt::lightGray));
    painter.drawRect(bodyRect);

    // Рисуем направление движения (стрелка)
    QPolygonF arrow;
    arrow << QPointF(0, -ROVER_LENGTH/2 - 20)
          << QPointF(-10, -ROVER_LENGTH/2)
          << QPointF(10, -ROVER_LENGTH/2);
    
    painter.setBrush(QBrush(Qt::red));
    painter.drawPolygon(arrow);
}

void RoverViewWidget::drawWheel(QPainter &painter, float x, float y, float angle)
{
    painter.save();
    
    // Перемещаем в позицию колеса
    painter.translate(x, y);
    // Поворачиваем на заданный угол
    painter.rotate(angle);

    // Рисуем колесо
    QRectF wheelRect(-WHEEL_RADIUS, -WHEEL_WIDTH/2, 
                     WHEEL_RADIUS * 2, WHEEL_WIDTH);
    
    painter.setPen(QPen(Qt::black, 2));
    painter.setBrush(QBrush(Qt::darkGray));
    painter.drawRect(wheelRect);

    // Рисуем спицы колеса
    painter.setPen(QPen(Qt::black, 1));
    for(int i = 0; i < 4; i++) {
        painter.drawLine(0, 0, WHEEL_RADIUS * cos(i * M_PI/2), 
                        WHEEL_RADIUS * sin(i * M_PI/2));
    }

    painter.restore();
} 