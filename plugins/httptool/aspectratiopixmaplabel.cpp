#include "aspectratiopixmaplabel.h"
//#include <QDebug>

AspectRatioPixmapLabel::AspectRatioPixmapLabel(QWidget *parent)
    : QLabel(parent)
{
    this->setMinimumSize(1, 1);
}

void AspectRatioPixmapLabel::setPixmap(const QPixmap &pixmap)
{
    pix = pixmap;
    QLabel::setPixmap(pix);

    resizeEvent(nullptr);
}

int AspectRatioPixmapLabel::heightForWidth(int width) const
{
    return ((qreal) pix.height() * width) / pix.width();
}

QSize AspectRatioPixmapLabel::sizeHint() const
{
    int w = this->width();
    return QSize(w, heightForWidth(w));
}

void AspectRatioPixmapLabel::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event);

    if (!pix.isNull() && pix.width() > width() || pix.height() > height())
        QLabel::setPixmap(pix.scaled(this->size(), Qt::KeepAspectRatio));
}
