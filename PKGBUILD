# Maintainer: Saeed Badreldin <saeed@helwan-linux.org>
pkgname=hel-senet
pkgver=1.0
pkgrel=3
pkgdesc="Ancient Egyptian Board Game (Senet) for Helwan Linux"
arch=('x86_64')
url="https://github.com/helwan-linux/hel-senet"
license=('GPL')
depends=('gtk3' 'librsvg' 'libcanberra' 'sound-theme-freedesktop')
source=("git+https://github.com/helwan-linux/hel-senet.git")
md5sums=('SKIP')

build() {
    cd "$srcdir/$pkgname/senet"
    # تجميع الكود مع ربط مكتبات GTK
    gcc $(pkg-config --cflags gtk+-3.0) -o senet_pro senet.c $(pkg-config --libs gtk+-3.0)
}

package() {
    cd "$srcdir/$pkgname/senet"
    install -Dm755 senet_pro "$pkgdir/usr/bin/senet_pro"
    
    # تثبيت الأيقونة باسم senet_pro ليتطابق مع ملف الديسكتوب
    install -Dm644 icon.svg "$pkgdir/usr/share/pixmaps/senet_pro.svg"
    install -Dm644 icon.svg "$pkgdir/usr/share/icons/hicolor/scalable/apps/senet_pro.svg"
    
    install -Dm644 senet.desktop "$pkgdir/usr/share/applications/hel-senet.desktop"
}
