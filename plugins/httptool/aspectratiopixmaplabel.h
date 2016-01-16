#ifndef ASPECTRATIOPIXMAPLABEL_H
#define ASPECTRATIOPIXMAPLABEL_H

#include <QLabel>
#include <QPixmap>
#include <QResizeEvent>
#include <QPen>

class AspectRatioPixmapLabel : public QLabel
{
    Q_OBJECT

public:
    explicit AspectRatioPixmapLabel(QWidget *parent = 0);
    int heightForWidth(int width) const;
    QSize sizeHint() const;
    void setBorder(QPen pen);

public slots:
    void setPixmap(const QPixmap &pixmap, QPen borderPen = QPen(Qt::black, 0));
    void resizeEvent(QResizeEvent *event);

private:
    QPixmap m_pix;
    QPen m_borderPen;
};

#endif // ASPECTRATIOPIXMAPLABEL_H
