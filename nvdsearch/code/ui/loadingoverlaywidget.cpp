#include <QPainter>

#include "loadingoverlaywidget.h"

//==============================================================================
// SpinnerIndicator
//==============================================================================
SpinnerIndicator::SpinnerIndicator( QWidget *parent, qreal animation_speed )
    : QWidget( parent ), _animation_speed( animation_speed )
{
  _angle = 0;
  _offset = 0;

  connect( &_update_timer, &QTimer::timeout, this,
           [this]() { this->update(); } );

  // Frames Per Second.
  auto fps = 60;
  _update_timer.start( int( 1000 / fps ) );
}

SpinnerIndicator::~SpinnerIndicator()
{
}

void SpinnerIndicator::paintEvent( QPaintEvent * )
{
  QPainter painter( this );
  auto current_time = QDateTime::currentDateTime();
  auto circle_color = QColor( "#3f88f8" );  // Blue color.

  painter.setRenderHint( QPainter::Antialiasing );
  painter.setRenderHint( QPainter::HighQualityAntialiasing );

  int pen_width = 4;
  QRectF rect( pen_width / 2, pen_width / 2, this->width() - pen_width - 1,
               this->height() - pen_width - 1 );
  // Background Arc.
  QPen pen( circle_color, pen_width, Qt::SolidLine, Qt::FlatCap,
            Qt::MiterJoin );
  painter.setPen( pen );
  // Recalculate angle and offset.
  auto t = current_time.toMSecsSinceEpoch() % qint64( _animation_speed * 1000 );
  auto nt = t / qreal( _animation_speed * 1000 - 1 );  // Normalized time [0-1].
  auto ease_in_out_cubic = []( qreal t ) {
    return t < .5 ? 4 * t * t * t
                  : ( t - 1 ) * ( 2 * t - 2 ) * ( 2 * t - 2 ) + 1;
  };
  auto ease_out_quad = []( qreal t ) { return t * ( 2 - t ); };
  // t [0, 1] - mapping value
  // ease_time_map maps it such that [a, b] => [0, 1]
  auto ease_time_map = []( qreal t, qreal a, qreal b ) {
    return ( t - a ) / ( b - a );
  };
  // t [0, 1] => [a, b] * 16
  auto deg_scale_map = []( qreal t, int a, int b ) {
    return int( t * ( b - a ) + a ) * 16;
  };
  auto linear = []( qreal t ) { return t; };
  auto min_offset = 5;    // deg.
  auto max_offset = 187;  // deg.
  auto transition_point = 270.0 / 360.0;
  if ( nt < transition_point ) {  // until 270 deg.
    _offset = deg_scale_map( linear( ease_time_map( nt, 0, transition_point ) ),
                             min_offset, max_offset );
  } else {  // after 270 deg.
    _offset = deg_scale_map(
        1.0 - ease_in_out_cubic( ease_time_map( nt, transition_point, 1.0 ) ),
        min_offset, max_offset );
  }
  _angle = deg_scale_map( ease_out_quad( nt ), 0, 360 );
  // Negative angle for clockwise rotation.
  painter.drawArc( rect, -_angle, _offset );
}

//==============================================================================
// LoadingOverlayWidget
//==============================================================================
LoadingOverlayWidget::LoadingOverlayWidget( QWidget *parent )
    : QWidget( parent )
{
  _spinner = new SpinnerIndicator( this );
  _spinner->show();
}

LoadingOverlayWidget::~LoadingOverlayWidget()
{
}

void LoadingOverlayWidget::paintEvent( QPaintEvent * )
{
  QPainter painter( this );
  QRect rect( QPoint( 0, 0 ), size() );
  // Fill widget with transparent gray.
  painter.fillRect( rect, QBrush( QColor( 128, 128, 128, 128 ) ) );
}

void LoadingOverlayWidget::resizeEvent( QResizeEvent * )
{
  auto s = size();
  auto spinner_size = s.height() / 4;  // 25% of total height.
  _spinner->setGeometry( width() / 2 - spinner_size / 2,
                         height() / 2 - spinner_size / 2, spinner_size,
                         spinner_size );
}
