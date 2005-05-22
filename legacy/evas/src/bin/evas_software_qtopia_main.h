#ifndef EVAS_SOFTWARE_QTOPIA_MAIN_H
#define EVAS_SOFTWARE_QTOPIA_MAIN_H

#ifndef QT_QWS_EBX
#define QT_QWS_EBX
#endif
#ifndef QT_QWS_CUSTOM
#define QT_QWS_CUSTOM
#endif
#ifndef QWS
#define QWS
#endif

#include <unistd.h>
#include <qwidget.h>
#include <qmainwindow.h>
#include <qpe/qlibrary.h>
#include <qtimer.h>
#include <qwidget.h>
#include <qwidgetstack.h>
#include <qpainter.h>
#include <qdirectpainter_qws.h>

class EvasTargetWidget : public QWidget
{
   Q_OBJECT

 public:
   EvasTargetWidget(QWidget *parent=NULL, const char *name=NULL);

   QSizePolicy sizePolicy() const;

   void enableFullscreen(void);
   void disableFullscreen(void);
   int fullscreen;

 protected:
   void mousePressEvent(QMouseEvent *);
   void mouseReleaseEvent(QMouseEvent *);
   void mouseMoveEvent(QMouseEvent *);
   void paintEvent(QPaintEvent *);
   void resizeEvent(QResizeEvent *);

 private slots:
   void idleHandler(void);
 private:
   QTimer *idleTimer;
};

class EvasTestApp : public QMainWindow
{
   Q_OBJECT
 public:
   EvasTestApp(QWidget *parent=0, const char *name=0, int wFlags=0);
   ~EvasTestApp();

   QSizePolicy sizePolicy() const;

 protected:
   void resizeEvent(QResizeEvent *);

   EvasTargetWidget *evas_target_widget;
};

#endif
