#ifndef ROUNDEDDIALOG_H
#define ROUNDEDDIALOG_H

#include <QDialog>
class RoundedDialog : public QDialog {
    Q_OBJECT
public:
    explicit RoundedDialog(QWidget* parent = nullptr, int radius = 8, int borderWidth = 2);
    void setStyleParams(int radius, int borderWidth, const QColor& borderColor); // 设置圆角半径
protected:
    void paintEvent(QPaintEvent* event) override; // 重写绘制事件

private:
    int m_radius;             // 圆角半径
    int m_borderWidth;        //边框厚度
    QColor m_borderColor = QColor(204, 204, 204);  //边框颜色
    double m_pixelRatio;
};

#endif // ROUNDEDDIALOG_H
