#ifndef ASPECTRATIOPIXMAPLABEL_H
#define ASPECTRATIOPIXMAPLABEL_H

#include <QLabel>
#include <QPixmap>
#include <QResizeEvent>

class AspectRatioPixmapLabel : public QLabel
{
    Q_OBJECT

public:
    explicit AspectRatioPixmapLabel(QWidget *parent = 0);
    virtual int heightForWidth(int width) const;
    virtual QSize sizeHint() const;

public slots:
    void setPixmap (const QPixmap &pixmap);
    void resizeEvent(QResizeEvent *event);

private:
    QPixmap pix;
};

#endif // ASPECTRATIOPIXMAPLABEL_H
