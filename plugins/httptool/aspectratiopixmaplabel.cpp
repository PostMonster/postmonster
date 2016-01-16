#include "aspectratiopixmaplabel.h"

#include <QPainter>

AspectRatioPixmapLabel::AspectRatioPixmapLabel(QWidget *parent)
    : QLabel(parent), m_borderPen(QPen(Qt::black, 0))
{
    this->setMinimumSize(1, 1);
}

void AspectRatioPixmapLabel::setPixmap(const QPixmap &pixmap, QPen borderPen)
{
    m_pix = pixmap;
    m_borderPen = borderPen;

    resizeEvent(nullptr);
}

int AspectRatioPixmapLabel::heightForWidth(int width) const
{
    return ((qreal) m_pix.height() * width) / m_pix.width();
}

QSize AspectRatioPixmapLabel::sizeHint() const
{
    int w = width();
    return QSize(w, heightForWidth(w));
}

void AspectRatioPixmapLabel::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event);

    if (m_pix.isNull())
        return;

    QPixmap scaled = m_pix;
    QSize borderShift(m_borderPen.width() * 2, m_borderPen.width() * 2);

    if (m_pix.width() + borderShift.width() > contentsRect().width()
            || m_pix.height() + borderShift.height() > contentsRect().height()) {
        scaled = m_pix.scaled(contentsRect().size() - borderShift, Qt::KeepAspectRatio);
    }

    QPixmap withBorder(scaled.size() + borderShift);
    withBorder.fill(Qt::transparent);

    QPainter painter;
    painter.begin(&withBorder);
    painter.setPen(m_borderPen);
    painter.drawRect(0, 0, withBorder.width() - m_borderPen.width(),
                           withBorder.height() - m_borderPen.width());
    painter.drawPixmap(m_borderPen.width(), m_borderPen.width(), scaled);
    painter.end();

    QLabel::setPixmap(withBorder);
}

void AspectRatioPixmapLabel::setBorder(QPen pen)
{
    m_borderPen = pen;
    resizeEvent(nullptr);
}
