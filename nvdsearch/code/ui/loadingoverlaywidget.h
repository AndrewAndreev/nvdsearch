#ifndef LOADINGOVERLAYWIDGET_H
#define LOADINGOVERLAYWIDGET_H

#include <QDateTime>
#include <QPaintEvent>
#include <QTimer>
#include <QWidget>

class SpinnerIndicator : public QWidget
{
  Q_OBJECT
public:
  explicit SpinnerIndicator( QWidget *parent = nullptr,
                             qreal animation_speed = 1.4 );
  ~SpinnerIndicator();

protected:
  void paintEvent( QPaintEvent * ) override;

private:
  QTimer _update_timer;
  qreal _animation_speed;  // seconds on one animation cycle.
  int _angle;              // Starting point of a circle.
  int _offset;             // Back point of a circle.
};

class LoadingOverlayWidget : public QWidget
{
  Q_OBJECT
public:
  explicit LoadingOverlayWidget( QWidget *parent = nullptr );
  ~LoadingOverlayWidget();

protected:
  void paintEvent( QPaintEvent * ) override;
  void resizeEvent( QResizeEvent * ) override;

private:
  SpinnerIndicator *_spinner;
};

#endif  // !LOADINGOVERLAYWIDGET_H
