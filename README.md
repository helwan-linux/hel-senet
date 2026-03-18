# 🏺 Senet Pro

Ancient Egyptian Board Game for Helwan Linux

Welcome to Senet Pro, a digital recreation of one of the oldest known board games in history, dating back to Ancient Egypt (c. 3100 BC). Developed by Saeed Badreldin, this version brings a professional, fast, and native experience to your Linux desktop.

## ✨ Features

* Native Performance: Built with C and GTK3 for a lightweight and responsive experience.

* Traditional Rules: Accurate implementation of movement, protection, and the "Water Trap."

* Built-in Guide: Easy access to "How to Play" and game mechanics within the app.

* Helwan Linux Integration: Fully compatible with GNOME, Cinnamon, and Xfce environments.

## 🚀 Installation

### On Helwan Linux (or Arch-based systems)

If you have the source files, you can build and install the package using the provided PKGBUILD:

1. Open your terminal in the project folder.

2. Run the following command:

```bash
makepkg -si
```

This will compile the game, install the icon, and add it to your application menu.

### Manual Compilation

If you prefer to compile the single file manually:

```bash
gcc `pkg-config --cflags gtk+-3.0` -o senet_pro senet.c `pkg-config --libs gtk+-3.0`
./senet_pro
```

## 🎮 How to Play

* Roll the Sticks: Click "ROLL STICKS" to get a move value (1, 2, 3, 4, or 5).

* Move Your Pieces: Click on your colored pieces (Orange for P1, Blue for P2) to move them forward.

* The Goal: Be the first to move all your pieces off the board.

* Extra Turns: Rolling a 1, 4, or 5 gives you another throw!

* Watch Out: Avoid Square 27 (The Water) or you will be sent back to Square 15.

## 🛠 Developer Info

* Lead Developer: Saeed Badreldin

* Project: Helwan Linux Distribution

* Language: C (GTK3)

---

Enjoy a piece of history on your modern desktop!
