#include <QtGui/QApplication>
#include <QtGui/QLabel>
#include <QtGui/QGridLayout>
#include <QtGui/QGraphicsScene>
#include <QtGui/QGraphicsPixmapItem>
#include <QtGui/QGraphicsView>

#include "Display.hh"
#include "ColorMap.hh"
#include <climits>
#include <cfloat>

typedef unsigned short Pixel;

static void display(char *title, Matrix<Pixel>& m) {
  printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!! %s\n", title);
  int argc = 1;
  char* argv[] = { "Display" };
  QApplication app(argc, argv);
  const int npx = m.npx();
  const int npy = m.npy();
  QImage image(npx, npy, QImage::Format_RGB32);
  for (int y = 0; y < npy; y++) {
    for (int x = 0; x < npx; x++) {
      ushort val = m[x][y] & 0xff;
      unsigned int color = colormap_jet[val];
      QRgb pixel = qRgb((color >> 16) & 0xff, (color & 0xff00) >> 8, color & 0xff);
      image.setPixel(x, y, pixel);
    }
  }
  QPixmap pixmap = QPixmap::fromImage(image);
  QGraphicsScene scene;
  scene.addText(title);
  QGraphicsPixmapItem item(pixmap);
  scene.addItem(&item);
  QGraphicsView view(&scene);
  view.setWindowTitle(title);
  view.show();
  app.exec();
}

void display(char *title, RMatrix& m) {
  const int npx = m.npx();
  const int npy = m.npy();
  double min = DBL_MAX;
  double max = DBL_MIN;
  for (int x = 0; x < npx; x++) {
    for (int y = 0; y < npy; y++) {
      const double val = m[x][y];
      if (min > val) min = val;
      if (max < val) max = val;
    }
  }
  double range = max - min;
  if (range == 0) {
    range = 1;
  }
  Matrix<Pixel> pm(npx, npy);
  for (int x = 0; x < npx; x++) {
    for (int y = 0; y < npy; y++) {
      pm[x][y] = (Pixel) (255 * (m[x][y] - min) / range);
    }
  }
  display(title, pm);
}

void display(char *title, CMatrix& m) {
  const int npx = m.npx();
  const int npy = m.npy();
  RMatrix r(npx, npy);
  for (int x = 0; x < npx; x++) {
    for (int y = 0; y < npy; y++) {
      Complex val = m[x][y];
      Real r0 = val.real();
      Real i0 = val.imag();
      Real abs_squared = r0*r0 + i0*i0;
      r[x][y] = log(abs_squared);
    }
  }
  display(title, r);
}
