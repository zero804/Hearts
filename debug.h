#ifndef DEBUG_H
#define DEBUG_H

#include "define.h"

#ifdef DEBUG

#include "cdeck.h"
#include <QWidget>
#include <QLabel>
#include <QScrollBar>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QImage>

const int MAX_HISTORY_SIZE = 1000;

class CDebug : public QWidget
{
    Q_OBJECT

private: // variables;
    QVBoxLayout *layout;
    QHBoxLayout *box1;
    QHBoxLayout *box2;
    QScrollBar *bar;

    CDeck *deck;
    QLabel *labels[14];

    const char *plr_names[MAX_HISTORY_SIZE];

    bool winners[MAX_HISTORY_SIZE];

    int cards[MAX_HISTORY_SIZE];
    int ptr_screen;
    int history_size;
    int cards_saved;
    int suit;
    int best;
    int best_pos;

private: // functions
    void show_history(int slide);

public:
    CDebug(CDeck *d);
    ~CDebug();

    void save_card(int card, const char *name, QImage *img);
    void reset();
    void refresh();
    void Translate();

public slots:
    void handle_bar(int value);
};
#endif // DEBUG

#endif // DEBUG_H
