#include "verifycodewidget.h"

#include "model/data.h"

#include <QPainter>

VerifyCodeWidget::VerifyCodeWidget(QWidget *parent)
    : QWidget{parent}
    ,randomGenerator(model::getTime())
{
    verifyCode = generateVerifyCode();
}

void VerifyCodeWidget::refreshVerifyCode()
{
    // 通过 generate 就可以重新生成验证码, 直接使用成员 verifyCode 保存就达到了数据上的刷新
    verifyCode = generateVerifyCode();
    // 通过 update 就可以起到 "刷新界面" , 本身就是触发 paintEvent, 从而达到界面上的刷新
    this->update(); //update比repaint更优
}

// 下列代码, 生成的是 "随机的大写字母", 类似的思路, 也可以生成小写字母和阿拉伯数字
// 如果代码中, 同时混着存在大小写字母和阿拉伯数字的话, 可能存在一个问题:
// 有些字符, 会混淆. 比如 O o 0; 再比如 I 1 l;
QString VerifyCodeWidget::generateVerifyCode()
{
    QString code;
    for (int i = 0; i < 4; ++i) {
        // 每次循环生成一个字符.
        int init = 'A';
        init += randomGenerator.generate() % 26;
        code += static_cast<QChar>(init);
    }
    return code;
}

bool VerifyCodeWidget::checkVerifyCode(const QString &verifyCode)
{
    // 此处比较验证码的时候, 需要忽略大小写.
    return this->verifyCode.compare(verifyCode, Qt::CaseInsensitive) == 0;
}

void VerifyCodeWidget::paintEvent(QPaintEvent *event)
{
    //固定套路的代码，手写比较麻烦的代码，直接复制粘贴
    (void) event;
    const int width = 80;   //固定宽度
    const int height = 40;  //固定高度
    const int codeLength = verifyCode.size();   //验证码长度
    if(codeLength <= 0) return;

    QPainter painter(this);
    // painter.setRenderHint(QPainter::Antialiasing);   //抗锯齿
    painter.fillRect(0, 0, width, height, Qt::white);   //绘制背景
    QPen pen;
    int fontSize = qMin(24, height-10);
    QFont font("楷体", fontSize, QFont::Bold, true);
    painter.setFont(font);

    // 计算字符之间的间距，根据验证码长度动态调整
    // 左右各留5px边距，字符间均匀分布
    int totalSpacing = width - 10;   //减去左右边距
    int charWidth = totalSpacing / codeLength;  //每个字符的宽度空间

    // 绘制验证码
    for(int i = 0; i < codeLength; i++)
    {
        // 避免过浅的颜色
        pen = QPen(QColor(randomGenerator.generate() % 200 + 55,
                          randomGenerator.generate() % 200 + 55,
                          randomGenerator.generate() % 200 + 55));
        painter.setPen(pen);

        int x = 5 + i*charWidth;
        int y = height/2 + fontSize/3
                + (randomGenerator()%8); //yOffset: (0,8)之间的y偏移位置
        painter.drawText(x, y, QString(verifyCode[i]));
    }

    // 画点: 添加随机噪点
    for(int i = 0; i < 100; i++)
    {
        pen = QPen(QColor(randomGenerator.generate() % 256,
                          randomGenerator.generate() % 256,
                          randomGenerator.generate() % 256));
        painter.setPen(pen);
        painter.drawPoint(randomGenerator.generate() % width,
                          randomGenerator.generate() % height);
    }

    // 画线: 添加随机干扰线
    for(int i = 0; i < 5; i++)
    {
        pen = QPen(QColor(randomGenerator.generate() % 256,
                          randomGenerator.generate() % 256,
                          randomGenerator.generate() % 256));
        painter.setPen(pen);
        painter.drawLine(randomGenerator.generate() % width,
                         randomGenerator.generate() % height,
                         randomGenerator.generate() % width,
                         randomGenerator.generate() % height);
    }
}

void VerifyCodeWidget::mousePressEvent(QMouseEvent *event)
{
    (void) event;
    this->refreshVerifyCode();
}
