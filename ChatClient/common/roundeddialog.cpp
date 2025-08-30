#include "roundeddialog.h"
#include <QPainter>
#include <QPainterPath>
#include <QGuiApplication>
#include <QScreen>

RoundedDialog::RoundedDialog(QWidget* parent, int radius, int borderWidth)
    : QDialog(parent), m_radius(radius), m_borderWidth(borderWidth)
{
    setWindowFlag(Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setStyleSheet("background: transparent;");

    // 计算屏幕像素比例（用于HiDPI支持）
    m_pixelRatio = QGuiApplication::primaryScreen()->devicePixelRatio();
}

void RoundedDialog::setStyleParams(int radius, int borderWidth, const QColor& borderColor) {
    m_radius = radius;
    m_borderWidth = borderWidth;
    m_borderColor = borderColor;
    update();
}

void RoundedDialog::paintEvent(QPaintEvent* event) {
    QPainter painter(this);

    // 使用最高质量的抗锯齿模式
    painter.setRenderHints(QPainter::Antialiasing |
                               QPainter::SmoothPixmapTransform |
                               QPainter::TextAntialiasing, true);

    // 1. 创建高精度绘制区域（使用浮点坐标）
    const double pixelAdjust = 0.5 / m_pixelRatio;
    const QRectF adjustedRect = QRectF(rect()).adjusted(
        pixelAdjust,
        pixelAdjust,
        -pixelAdjust,
        -pixelAdjust
        );

    // 2. 绘制高质量圆角背景
    QPainterPath backgroundPath;
    backgroundPath.addRoundedRect(adjustedRect, m_radius, m_radius);

    // 纯色填充
    painter.fillPath(backgroundPath, palette().window());

    // 3. 绘制边框（使用分段渲染获得完美圆角）
    if (m_borderWidth > 0) {
        QPen borderPen(m_borderColor);
        borderPen.setWidthF(m_borderWidth);
        borderPen.setJoinStyle(Qt::RoundJoin);
        painter.setPen(borderPen);
        painter.setBrush(Qt::NoBrush);

        // 针对每个角分别精确渲染
        const double r = m_radius;
        const double hw = adjustedRect.width();
        const double hh = adjustedRect.height();
        const double x = adjustedRect.x();
        const double y = adjustedRect.y();

        QPainterPath borderPath;

        // 左上角圆弧
        borderPath.moveTo(x, y + r);
        borderPath.arcTo(x, y, 2 * r, 2 * r, 180, -90);

        // 右上角圆弧
        borderPath.lineTo(x + hw - r, y);
        borderPath.arcTo(x + hw - 2 * r, y, 2 * r, 2 * r, 90, -90);

        // 右下角圆弧
        borderPath.lineTo(x + hw, y + hh - r);
        borderPath.arcTo(x + hw - 2 * r, y + hh - 2 * r, 2 * r, 2 * r, 0, -90);

        // 左下角圆弧
        borderPath.lineTo(x + r, y + hh);
        borderPath.arcTo(x, y + hh - 2 * r, 2 * r, 2 * r, 270, -90);

        borderPath.closeSubpath();

        painter.drawPath(borderPath);
    }

    // 4. 创建精确的剪辑区域
    QPainterPath clipPath;
    clipPath.addRoundedRect(adjustedRect.adjusted(
                                m_borderWidth,
                                m_borderWidth,
                                -m_borderWidth,
                                -m_borderWidth
                                ), m_radius - m_borderWidth, m_radius - m_borderWidth);
    painter.setClipPath(clipPath);

    // 5. 绘制子控件内容
    QWidget::paintEvent(event);
}
