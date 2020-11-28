#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QLabel>
#include <QFile>
#include <QDir>
#include <QPushButton>
#include <QImage>
#include <QString>
#include <QMessageBox>
#include <QTime>
#include <QResource>
#include <string.h>

#include "time.h"
#include "assert.h"
#include "rules.h"
#include "credits.h"
#include "connect.h"
#include "settings.h"
#include "online.h"
#include "cgame.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    srand(static_cast<unsigned int>(time(nullptr)));

    ui->textEdit->setTextColor(Qt::yellow);
    ui->textEdit->setTextBackgroundColor(Qt::black);

    stop_delay = false;

    setWindowTitle(version);

    init_pointers();

    load_sounds();

    config = new CConfig;

    hearts = new CHearts;

    stats = new CStatistics;

    deck = new CDeck(config->get_deck_style());

    table_list = new CTable();

    client = new CClient();

#ifdef DEBUG
    debug = new CDebug(deck);
#endif

    online_hide_progress_bar();

    timer = new QTimer();

    init_vars();
    online_show_buttons(false);
    set_settings();
    set_options();
    set_language(config->get_language());

    light_connected(false);
    set_online_buttons_styles();

    connect(timer, SIGNAL(timeout()), this, SLOT(update_bar()));

    connect(table_list, SIGNAL(clicked(int, char)), this, SLOT(join_game(int, char)));

    connect(client, SIGNAL(sig_message(QString)), this, SLOT(message(QString)));
    connect(client, SIGNAL(sig_error(QString)), this, SLOT(error(QString)));
    connect(client, SIGNAL(sig_action(unsigned int, QString)), this, SLOT(online_action(unsigned int, QString)));

    connect(hearts, SIGNAL(sig_refresh_deck(bool)), this, SLOT(refresh_deck(bool)));
    connect(hearts, SIGNAL(sig_play_card(int,int)), this, SLOT(play_card(int,int)));
    connect(hearts, SIGNAL(sig_clear_table()), this, SLOT(clear_table()));
    connect(hearts, SIGNAL(sig_score(int,int)), this, SLOT(refresh_score(int,int)));
    connect(hearts, SIGNAL(sig_hand_score(int,int)), this, SLOT(refresh_hand_score(int,int)));
    connect(hearts, SIGNAL(sig_end_hand(int,int,int,int)), this, SLOT(end_of_hand(int,int,int,int)));
    connect(hearts, SIGNAL(sig_your_turn(int)), this, SLOT(show_your_turn(int)));
    connect(hearts, SIGNAL(sig_breaking_heart()), this, SLOT(breaking_hearts()));
    connect(hearts, SIGNAL(sig_game_over(int,int,int,int)), this, SLOT(game_over(int,int,int,int)));
    connect(hearts, SIGNAL(sig_tram(int)), this, SLOT(tram(int)));
    connect(hearts, SIGNAL(sig_shoot_moon(int)), this, SLOT(shoot_moon(int)));
    connect(hearts, SIGNAL(sig_perfect_100(int)), this, SLOT(perfect_100(int)));
    connect(hearts, SIGNAL(sig_bonus(int,int,int)), this, SLOT(receive_bonus(int,int,int)));
    connect(hearts, SIGNAL(sig_pass_to(int)), this, SLOT(pass_to(int)));
    connect(hearts, SIGNAL(sig_got_queen_spade(int)), this, SLOT(got_queen_spade(int)));
    connect(hearts, SIGNAL(sig_pass_cards(int,int,int,int,int,int,int,int,int)), this, SLOT(animate_pass_cards(int,int,int,int,int,int,int,int,int)));
    connect(hearts, SIGNAL(sig_new_game()), this, SLOT(handle_new_game()));

    connect(qApp, SIGNAL(aboutToQuit()), this, SLOT(aboutToQuit()));
    connect(ui->actionQuit, SIGNAL(triggered(bool)), this, SLOT(close()));

    message(tr("Welcome to ") + QString(version));

    if (stats->is_file_corrupted())
      error(tr("The statistics file is corrupted!"));

    start_game(true);
}

MainWindow::~MainWindow()
{
  delete client;
  delete timer;
  delete hearts;
  delete config;
  delete stats;
  delete deck;
  delete table_list;

#ifdef DEBUG
  delete debug;
#endif

  delete ui;
  delete img_connected;
  delete img_disconnected;

  delete img_pass[pLEFT];
  delete img_pass[pRIGHT];
  delete img_pass[pACCROSS];
  delete img_pass[pNOPASS];

  destroy_sounds();
}

void MainWindow::init_vars()
{
  // online variables
  online_table_id = 0;
  online_passto = pLEFT;
  online_connected = false;
  online_can_sit = false;
  online_game_started = false;
  online_button_mode = mode_button_command;

  card_played[PLAYER_NORTH] = empty;
  card_played[PLAYER_SOUTH] = empty;
  card_played[PLAYER_EAST] = empty;
  card_played[PLAYER_WEST] = empty;

  card_height = ui->label_card_s1->height();

  original_pos_cards[PLAYER_NORTH] = ui->label_card_n1->y();
  original_pos_cards[PLAYER_SOUTH] = ui->label_card_s1->y();
  original_pos_cards[PLAYER_EAST] = ui->label_card_e1->x();
  original_pos_cards[PLAYER_WEST] = ui->label_card_w1->x();
}

void MainWindow::load_sounds()
{
#ifdef __al_included_allegro5_allegro_audio_h
    al_install_audio();
    al_init_acodec_addon();
    al_reserve_samples(14);

    QResource resource_snd1(":/sounds/34201__themfish__glass-house1.wav");
    QResource resource_snd2(":/sounds/240777__f4ngy__dealing-card.wav");
    QResource resource_snd3(":/sounds/253886__themusicalnomad__negative-beeps.wav");
    QResource resource_snd4(":/sounds/133283__fins__game-over.wav");
    QResource resource_snd5(":/sounds/171671__fins__success-1.wav");
    QResource resource_snd6(":/sounds/Soundwhich - Correct (LittleRainySeasons) [public].wav");
    QResource resource_snd7(":/sounds/344013__reitanna__high-pitched-ah2.wav");
    QResource resource_snd8(":/sounds/96127__bmaczero__contact1.wav");
    QResource resource_snd9(":/sounds/434472__dersuperanton__taking-card.wav");
    QResource resource_snd10(":/sounds/423767__someonecool14__card-shuffling.wav");
    QResource resource_snd11(":/sounds/400163__vitovsky1__fanfare-short.wav");
    QResource resource_snd12(":/sounds/322897__rhodesmas__connected-01.wav");
    QResource resource_snd13(":/sounds/322895__rhodesmas__disconnected-01.wav");
    QResource resource_snd14(":/sounds/493696__stib__bingbong.wav");

    ALLEGRO_FILE *fp = al_open_memfile(reinterpret_cast<void *>(const_cast<unsigned char *>(resource_snd1.data())), resource_snd1.size(), "rb");
    snd_breaking_heart = al_load_sample_f(fp, ".wav");
    al_fclose(fp);

    fp = al_open_memfile(reinterpret_cast<void *>(const_cast<unsigned char *>(resource_snd2.data())), resource_snd2.size(), "rb");
    snd_dealing_card = al_load_sample_f(fp, ".wav");
    al_fclose(fp);

    fp = al_open_memfile(reinterpret_cast<void *>(const_cast<unsigned char *>(resource_snd3.data())), resource_snd3.size(), "rb");
    snd_error = al_load_sample_f(fp, ".wav");
    al_fclose(fp);

    fp = al_open_memfile(reinterpret_cast<void *>(const_cast<unsigned char *>(resource_snd4.data())), resource_snd4.size(), "rb");
    snd_game_over = al_load_sample_f(fp, ".wav");
    al_fclose(fp);

    fp = al_open_memfile(reinterpret_cast<void *>(const_cast<unsigned char *>(resource_snd5.data())), resource_snd5.size(), "rb");
    snd_shoot_moon = al_load_sample_f(fp, ".wav");
    al_fclose(fp);

    fp = al_open_memfile(reinterpret_cast<void *>(const_cast<unsigned char *>(resource_snd6.data())), resource_snd6.size(), "rb");
    snd_your_turn = al_load_sample_f(fp, ".wav");
    al_fclose(fp);

    fp = al_open_memfile(reinterpret_cast<void *>(const_cast<unsigned char *>(resource_snd7.data())), resource_snd7.size(), "rb");
    snd_queen_spade = al_load_sample_f(fp, ".wav");
    al_fclose(fp);

    fp = al_open_memfile(reinterpret_cast<void *>(const_cast<unsigned char *>(resource_snd8.data())), resource_snd8.size(), "rb");
    snd_contact = al_load_sample_f(fp, ".wav");
    al_fclose(fp);

    fp = al_open_memfile(reinterpret_cast<void *>(const_cast<unsigned char *>(resource_snd9.data())), resource_snd9.size(), "rb");
    snd_passing_cards = al_load_sample_f(fp, ".wav");
    al_fclose(fp);

    fp = al_open_memfile(reinterpret_cast<void *>(const_cast<unsigned char *>(resource_snd10.data())), resource_snd10.size(), "rb");
    snd_shuffling_cards = al_load_sample_f(fp, ".wav");
    al_fclose(fp);

    fp = al_open_memfile(reinterpret_cast<void *>(const_cast<unsigned char *>(resource_snd11.data())), resource_snd11.size(), "rb");
    snd_perfect_100 = al_load_sample_f(fp, ".wav");
    al_fclose(fp);

    fp = al_open_memfile(reinterpret_cast<void *>(const_cast<unsigned char *>(resource_snd12.data())), resource_snd12.size(), "rb");
    snd_connected = al_load_sample_f(fp, ".wav");
    al_fclose(fp);

    fp = al_open_memfile(reinterpret_cast<void *>(const_cast<unsigned char *>(resource_snd13.data())), resource_snd13.size(), "rb");
    snd_disconnected = al_load_sample_f(fp, ".wav");
    al_fclose(fp);

    fp = al_open_memfile(reinterpret_cast<void *>(const_cast<unsigned char *>(resource_snd14.data())), resource_snd14.size(), "rb");
    snd_announcement = al_load_sample_f(fp, ".wav");
    al_fclose(fp);

    assert(snd_breaking_heart);
    assert(snd_dealing_card);
    assert(snd_error);
    assert(snd_game_over);
    assert(snd_shoot_moon);
    assert(snd_your_turn);
    assert(snd_queen_spade);
    assert(snd_contact);
    assert(snd_passing_cards);
    assert(snd_shuffling_cards);
    assert(snd_perfect_100);
    assert(snd_connected);
    assert(snd_disconnected);
    assert(snd_announcement);
#else
    ui->actionSounds->setDisabled(true);
#endif
}

void MainWindow::destroy_sounds()
{
#ifdef __al_included_allegro5_allegro_audio_h
    al_destroy_sample(snd_shoot_moon);
    al_destroy_sample(snd_your_turn);
    al_destroy_sample(snd_game_over);
    al_destroy_sample(snd_error);
    al_destroy_sample(snd_dealing_card);
    al_destroy_sample(snd_breaking_heart);
    al_destroy_sample(snd_queen_spade);
    al_destroy_sample(snd_passing_cards);
    al_destroy_sample(snd_shuffling_cards);
    al_destroy_sample(snd_connected);
    al_destroy_sample(snd_disconnected);
    al_destroy_sample(snd_announcement);
#endif
}

void MainWindow::init_pointers()
{
    img_connected = new QImage(":/icons/connected.png", "PNG");
    img_disconnected = new QImage(":/icons/disconnected.png", "PNG");
    img_pass[pLEFT] = new QImage(":/icons/left-icon.png", "PNG");
    img_pass[pRIGHT] = new QImage(":/icons/right-icon.png", "PNG");
    img_pass[pACCROSS] = new QImage(":/icons/up-icon.png", "PNG");
    img_pass[pNOPASS] = new QImage(":/icons/no_pass-icon.png", "PNG");

    label_cards[PLAYER_SOUTH][0] = ui->label_card_s1;
    label_cards[PLAYER_SOUTH][1] = ui->label_card_s2;
    label_cards[PLAYER_SOUTH][2] = ui->label_card_s3;
    label_cards[PLAYER_SOUTH][3] = ui->label_card_s4;
    label_cards[PLAYER_SOUTH][4] = ui->label_card_s5;
    label_cards[PLAYER_SOUTH][5] = ui->label_card_s6;
    label_cards[PLAYER_SOUTH][6] = ui->label_card_s7;
    label_cards[PLAYER_SOUTH][7] = ui->label_card_s8;
    label_cards[PLAYER_SOUTH][8] = ui->label_card_s9;
    label_cards[PLAYER_SOUTH][9] = ui->label_card_s10;
    label_cards[PLAYER_SOUTH][10] = ui->label_card_s11;
    label_cards[PLAYER_SOUTH][11] = ui->label_card_s12;
    label_cards[PLAYER_SOUTH][12] = ui->label_card_s13;

    label_cards[PLAYER_EAST][0] = ui->label_card_e1;
    label_cards[PLAYER_EAST][1] = ui->label_card_e2;
    label_cards[PLAYER_EAST][2] = ui->label_card_e3;
    label_cards[PLAYER_EAST][3] = ui->label_card_e4;
    label_cards[PLAYER_EAST][4] = ui->label_card_e5;
    label_cards[PLAYER_EAST][5] = ui->label_card_e6;
    label_cards[PLAYER_EAST][6] = ui->label_card_e7;
    label_cards[PLAYER_EAST][7] = ui->label_card_e8;
    label_cards[PLAYER_EAST][8] = ui->label_card_e9;
    label_cards[PLAYER_EAST][9] = ui->label_card_e10;
    label_cards[PLAYER_EAST][10] = ui->label_card_e11;
    label_cards[PLAYER_EAST][11] = ui->label_card_e12;
    label_cards[PLAYER_EAST][12] = ui->label_card_e13;

    label_cards[PLAYER_NORTH][0] = ui->label_card_n1;
    label_cards[PLAYER_NORTH][1] = ui->label_card_n2;
    label_cards[PLAYER_NORTH][2] = ui->label_card_n3;
    label_cards[PLAYER_NORTH][3] = ui->label_card_n4;
    label_cards[PLAYER_NORTH][4] = ui->label_card_n5;
    label_cards[PLAYER_NORTH][5] = ui->label_card_n6;
    label_cards[PLAYER_NORTH][6] = ui->label_card_n7;
    label_cards[PLAYER_NORTH][7] = ui->label_card_n8;
    label_cards[PLAYER_NORTH][8] = ui->label_card_n9;
    label_cards[PLAYER_NORTH][9] = ui->label_card_n10;
    label_cards[PLAYER_NORTH][10] = ui->label_card_n11;
    label_cards[PLAYER_NORTH][11] = ui->label_card_n12;
    label_cards[PLAYER_NORTH][12] = ui->label_card_n13;

    label_cards[PLAYER_WEST][0] = ui->label_card_w1;
    label_cards[PLAYER_WEST][1] = ui->label_card_w2;
    label_cards[PLAYER_WEST][2] = ui->label_card_w3;
    label_cards[PLAYER_WEST][3] = ui->label_card_w4;
    label_cards[PLAYER_WEST][4] = ui->label_card_w5;
    label_cards[PLAYER_WEST][5] = ui->label_card_w6;
    label_cards[PLAYER_WEST][6] = ui->label_card_w7;
    label_cards[PLAYER_WEST][7] = ui->label_card_w8;
    label_cards[PLAYER_WEST][8] = ui->label_card_w9;
    label_cards[PLAYER_WEST][9] = ui->label_card_w10;
    label_cards[PLAYER_WEST][10] = ui->label_card_w11;
    label_cards[PLAYER_WEST][11] = ui->label_card_w12;
    label_cards[PLAYER_WEST][12] = ui->label_card_w13;

    label_scores[PLAYER_NORTH] = ui->label_score_n;
    label_scores[PLAYER_SOUTH] = ui->label_score_s;
    label_scores[PLAYER_EAST] = ui->label_score_e;
    label_scores[PLAYER_WEST] = ui->label_score_w;

    label_player_name[PLAYER_NORTH] = ui->label_name_n;
    label_player_name[PLAYER_SOUTH] = ui->label_name_s;
    label_player_name[PLAYER_WEST] = ui->label_name_w;
    label_player_name[PLAYER_EAST] = ui->label_name_e;

    label_card_played[PLAYER_NORTH] = ui->label_played_n;
    label_card_played[PLAYER_SOUTH] = ui->label_played_s;
    label_card_played[PLAYER_WEST] = ui->label_played_w;
    label_card_played[PLAYER_EAST] = ui->label_played_e;

    progress_bar_time[PLAYER_NORTH] = ui->progressBar_bank_n; // time bank north
    progress_bar_time[PLAYER_SOUTH] = ui->progressBar_bank_s; // time bank south
    progress_bar_time[PLAYER_WEST] = ui->progressBar_bank_w;  // time bank west
    progress_bar_time[PLAYER_EAST] = ui->progressBar_bank_e;  // time bank east
    progress_bar_time[4] = ui->progressBar;                   // regular time left
}

void MainWindow::aboutToQuit()
{
  client->disconnect();
  timer->disconnect();

  save_files();
}

void MainWindow::save_files()
{    
 stats->save_stats_file();

 if (!ui->actionSave_Game_Quit->isChecked())
   return;

 if (hearts->is_game_over())
   return;

 if (hearts->is_fresh_game())
   return;

 hearts->save_game(plr_names_idx[0], plr_names_idx[1], plr_names_idx[2], plr_names_idx[3]);
}

void MainWindow::load_saved_game()
{
  message(tr("[Info]: Previous saved game has been loaded!"));

#ifdef DEBUG
// set played cards in the deck class
    for (int i=0; i<DECK_SIZE; i++)
      if (hearts->is_card_played(i)) {
        deck->set_card_played(i);
      }
#endif // DEBUG

  for (int i=0; i<4; i++) {
    int name_id = hearts->get_plr_name_id(i);

    plr_names_idx[i] = name_id;
    label_player_name[i]->setText(names[name_id]);
  }

  if (hearts->is_mode_playing()) {
    ui->label_18->setDisabled(true);

    int cpt = PLAYER_SOUTH;
    for (int i=0; i<3; i++) {
      int card = hearts->get_plr_hand_card(2-i);

      if (card == empty) continue;

      if (--cpt < 0)
        cpt = 3;

      label_card_played[cpt]->setPixmap(QPixmap::fromImage(deck->get_img_card(card)->scaledToHeight(card_height)));
      card_played[cpt] = card;
    }

// save the cards in the proper order in the debug cards history.
#ifdef DEBUG
    int turn = hearts->get_turn();
    while ((card_played[turn] == empty) || (card_played[turn] == your_turn)) {
      if (++turn > 3) turn = 0;
      if (turn == hearts->get_turn())
        return;
    }

    for (int i=0; i<3; i++) {
       debug->save_card(card_played[turn], names[plr_names_idx[turn]]);
       if (++turn > 3) turn = 0;
       if ((card_played[turn] == empty) || (card_played[turn] == your_turn)) break;
    }
#endif
  }
}

// DESC: This is a slot receiving a new_game signal from hearts.
//       it is used to send the sound shuffling cards.
//
// This is called if: - menu "New game" is triggered.
//                    - hearts->load_saved_game() failed to find a save game after
//                      leaving an online connection.
// This is not called when booting the game.
//
// NOTE: 1) The sound is "off-sync". If it is send from start_game() and new_game() directly.
//       2) When booting the game with no saved games. The sound was sent before the GUI
//          appeared. So, because of that it is disabled.
void MainWindow::handle_new_game()
{
#ifdef __al_included_allegro5_allegro_audio_h
  if (ui->actionSounds->isChecked()) {
    stop_delay = false;
    ui->actionNew->setDisabled(true);
    al_play_sample(snd_shuffling_cards, 1.0, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, nullptr);
    delay(config->get_speed(SPEED_SHUFFLE));
    ui->actionNew->setDisabled(false);
  }
#endif
}

void MainWindow::start_game(bool booting)
{
  int errnum = hearts->load_saved_game();

  if (errnum == NOERROR)
    load_saved_game();
  else {
    if (errnum == FCORRUPTED) {
      error(tr("The saved game file is corrupted! Deleted!"));

      QFile file(QDir::homePath() + SAVEDGAME_FILENAME);
      file.remove();
    }
    stats->increase_stats(0, STATS_GAME_STARTED);

    hearts->new_game(booting);
    set_plr_names();
  }
}

void MainWindow::light_connected(bool connected)
{
  if (connected)
    ui->label_24->setPixmap(QPixmap::fromImage(img_connected->scaledToHeight(15)));
  else
    ui->label_24->setPixmap(QPixmap::fromImage(img_disconnected->scaledToHeight(15)));
}

void MainWindow::set_settings()
{
#ifdef DEBUG
  ui->actionCheat->setChecked(config->is_cheat_mode());
#endif

  ui->actionAuto_Centering->setChecked(config->is_auto_centering());
  ui->actionInfo_Channel->setChecked(config->is_info_channel());
  ui->actionSounds->setChecked(config->is_sounds());
  ui->actionTram->setChecked(config->is_detect_tram());
  ui->actionEasy_card_selection->setChecked(config->is_easy_card_selection());
  ui->actionAuto_Start->setChecked(config->is_auto_start());

  ui->actionPerfect_100->setChecked(config->is_perfect_100());
  ui->actionOmnibus->setChecked(config->is_omnibus());
  ui->actionQueen_Spade_Break_Heart->setChecked(config->is_queen_spade_break_heart());
  ui->actionNo_Trick_Bonus->setChecked(config->is_no_trick_bonus());
  ui->actionNew_Moon->setChecked(config->is_new_moon());
  ui->actionNo_Draw->setChecked(config->is_no_draw());
  ui->actionSave_Game_Quit->setChecked(config->is_save_game());
  ui->actionAnimate_Play->setChecked(config->is_animated_play());

  set_info_channel_enabled(config->is_info_channel());

  switch (config->get_deck_style()) {
    case STANDARD_DECK: ui->actionStandard->setChecked(true); break;
    case ENGLISH_DECK:  ui->actionEnglish_2->setChecked(true); break;
    case RUSSIAN_DECK:  ui->actionRussian_2->setChecked(true); break;
  }

  switch (config->get_speed()) {
    case SPEED_SLOW:    ui->actionSlow->setChecked(true); break;
    case SPEED_NORMAL:  ui->actionNormal->setChecked(true); break;
    case SPEED_FAST:    ui->actionFast->setChecked(true); break;
  }
}

void MainWindow::set_options()
{
  hearts->set_tram_enabled(config->is_detect_tram());
  hearts->set_perfect_100(config->is_perfect_100());
  hearts->set_omnibus(config->is_omnibus());
  hearts->set_queen_spade_break_heart(config->is_queen_spade_break_heart());
  hearts->set_no_trick_bonus(config->is_no_trick_bonus());
  hearts->set_new_moon(config->is_new_moon());
  hearts->set_no_draw(config->is_no_draw());
  hearts->set_auto_start(config->is_auto_start());
}

void MainWindow::set_plr_names()
{
  bool name_taken[MAX_PLR_NAMES];

  for (int i=0; i<MAX_PLR_NAMES; i++)
     name_taken[i] = false;

  for (int i=0; i<4; i++) {
     if (i == PLAYER_SOUTH) continue;

    int x = rand() % MAX_PLR_NAMES;
    while (!x || name_taken[x])
        x = rand() % MAX_PLR_NAMES;

    name_taken[x] = true;;
    label_player_name[i]->setText(names[x]);
    plr_names_idx[i] = x;

    hearts->AI_set_cpu_flags(i, AI_CPU_flags[x]);
  }

  label_player_name[PLAYER_SOUTH]->setText(tr("You"));
  plr_names_idx[PLAYER_SOUTH] = 0;
}

void MainWindow::got_queen_spade(int plr)
{
  stats->increase_stats(plr_names_idx[plr], STATS_QUEEN_SPADE);
}

void MainWindow::receive_bonus(int plr, int bonus, int value)
{
  QString mesg, msg_bonus;

  switch (bonus) {
     case OMNIBUS_BONUS  : msg_bonus = tr("omnibus ");
                           stats->increase_stats(plr_names_idx[plr], STATS_OMNIBUS);
                           break;
     case NO_TRICK_BONUS : msg_bonus = tr("no trick bonus ");
                           stats->increase_stats(plr_names_idx[plr], STATS_NO_TRICKS);
                           break;
     default: msg_bonus = tr("<unknown> ");
  }

  if (plr == PLAYER_SOUTH)
    mesg = tr("[Info]: You receive the bonus: ") + msg_bonus + QString::number(value) + tr(" point(s)");
  else
    mesg = tr("[Info]: Player '") + label_player_name[plr]->text() + tr("' receive the bonus: ") + msg_bonus +
            QString::number(value) + tr(" point(s)");

  message(mesg);
}

void MainWindow::online_receive_bonus(int plr, int bonus, int value)
{
  QString mesg, msg_bonus;

  switch (bonus) {
     case OMNIBUS_BONUS  : msg_bonus = tr("omnibus ");
                           break;
     case NO_TRICK_BONUS : msg_bonus = tr("no trick bonus ");
                           break;
     default: msg_bonus = tr("<unknown> ");
  }

  if (plr == convert_chair(online_my_turn))
    mesg = tr("[Info]: You receive the bonus: ") + msg_bonus + QString::number(value) + tr(" point(s)");
  else
    mesg = tr("[Info]: Player '") + label_player_name[plr]->text() + tr("' receive the bonus: ") + msg_bonus +
            QString::number(value) + tr(" point(s)");

  message(mesg);
}

void MainWindow::perfect_100(int plr)
{
 stats->increase_stats(plr_names_idx[plr], STATS_PERFECT_100);

 QString mesg;

 if (plr == PLAYER_SOUTH)
   mesg = tr("[Info]: You got the perfect 100!\n[Info]: Your score has been set to 50.");
 else
   mesg = tr("[Info]: Player '") + label_player_name[plr]->text() + tr("' got the perfect 100!\n[Info]: Player '") +
                                   label_player_name[plr]->text() + tr("' score has been set to 50.");

 message(mesg);

#ifdef __al_included_allegro5_allegro_audio_h
  if (ui->actionSounds->isChecked())
    al_play_sample(snd_perfect_100, 1.0, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, nullptr);
#endif
}

void MainWindow::online_perfect_100(int plr)
{
  QString mesg;

  if (plr == convert_chair(online_my_turn))
    mesg = tr("[Info]: You got the perfect 100!\n[Info]: Your score has been set to 50.");
  else
    mesg = tr("[Info]: Player '") + label_player_name[plr]->text() + tr("' got the perfect 100!\n[Info]: Player '") +
                                    label_player_name[plr]->text() + tr("' score has been set to 50.");

  message(mesg);

#ifdef __al_included_allegro5_allegro_audio_h
  if (ui->actionSounds->isChecked())
    al_play_sample(snd_perfect_100, 1.0, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, nullptr);
#endif
}

void MainWindow::breaking_hearts()
{
  message(tr("[Info]: Hearts has been broken!"));

#ifdef __al_included_allegro5_allegro_audio_h
  if (ui->actionSounds->isChecked())
    al_play_sample(snd_breaking_heart, 1.0, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, nullptr);
#endif
}

void MainWindow::game_over(int score1, int score2, int score3, int score4)
{
  QString mesg, mesg2;

  stats->increase_stats(0, STATS_GAME_FINISHED);

  clear_deck();

  if (!hearts->is_it_draw())
    mesg2 = tr("Won !");
  else
    mesg2 = tr("Drew !");

 int lowest = 0;
 int unsorted[4];

 unsorted[0] = score1;
 unsorted[1] = score2;
 unsorted[2] = score3;
 unsorted[3] = score4;

 for (int i=0; i<4; i++) {
   int cpt = 0;
   for (int i2=0; i2<4; i2++)
     if (unsorted[i] > unsorted[i2])
       cpt++;

   switch(cpt) {
     case 0 : stats->increase_stats(plr_names_idx[i], STATS_FIRST_PLACE);
              lowest = unsorted[i];
              break;
     case 1 : stats->increase_stats(plr_names_idx[i], STATS_SECOND_PLACE);
              break;
     case 2 : stats->increase_stats(plr_names_idx[i], STATS_THIRD_PLACE);
              break;
     case 3 : stats->increase_stats(plr_names_idx[i], STATS_FOURTH_PLACE);
              break;
   }
 }

 stats->set_score(plr_names_idx[0], score1);
 stats->set_score(plr_names_idx[1], score2);
 stats->set_score(plr_names_idx[2], score3);
 stats->set_score(plr_names_idx[3], score4);

 mesg = tr("[Info]: GAME OVER!\n[Info]: Player '")  + label_player_name[PLAYER_SOUTH]->text() + "': " +
         QString::number(score1) + tr(" point(s) ") + (score1 == lowest ? mesg2 : "") +
        tr("\n[Info]: Player '")                    + label_player_name[PLAYER_WEST]->text() + "': " +
         QString::number(score2) + tr(" point(s) ") + (score2 == lowest ? mesg2 : "") +
        tr("\n[Info]: Player '")                    + label_player_name[PLAYER_NORTH]->text() + "': " +
         QString::number(score3) + tr(" point(s) ") + (score3 == lowest ? mesg2 : "") +
        tr("\n[Info]: Player '")                    + label_player_name[PLAYER_EAST]->text() + "': " +
         QString::number(score4) + tr(" point(s) ") + (score4 == lowest ? mesg2 : "");

 message(mesg);

#ifdef __al_included_allegro5_allegro_audio_h
  if (ui->actionSounds->isChecked())
    al_play_sample(snd_game_over, 1.0, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, nullptr);
#endif

  if (!ui->actionInfo_Channel->isChecked())
    QMessageBox::information(this, tr("Information"), mesg);

  ui->actionNew->setDisabled(false);
}

void MainWindow::online_game_over(int north, int south, int west, int east)
{
  int lowest = south;
  bool drew = false;

  if ((north <= south) && (north <= west) && (north <= east)) {
    lowest = north;
  if ((north == south) || (north == west) || (north == east))
    drew = true;
  } else
      if ((south <= north) && (south <= west) && (south <= east)) {
        lowest = south;
        if ((south == north) || (south == west) || (south == east))
          drew = true;
      } else
          if ((west <= north) && (west <= south) && (west <= east)) {
            lowest = west;
            if ((west == north) || (west == south) || (west == east))
              drew = true;
          } else
              if ((east <= north) && (east <= south) && (east <= west)) {
                lowest = east;
                if ((east == north) || (east == south) || (east == west))
                  drew = true;
              }

  QString mesg, mesg2;

  if (drew)
    mesg2 = tr("Drew !");
  else
    mesg2 = tr("Won !");

  mesg = tr("[Info]: GAME OVER!\n[Info]: Player '") + label_player_name[PLAYER_SOUTH]->text() + "': " +
          QString::number(south) + tr(" point(s) ") + (south == lowest ? mesg2 : "") +
         tr("\n[Info]: Player '")                   + label_player_name[PLAYER_WEST]->text() + "': " +
          QString::number(west) + tr(" point(s) ")  + (west == lowest ? mesg2 : "") +
         tr("\n[Info]: Player '")                   + label_player_name[PLAYER_NORTH]->text() + "': " +
          QString::number(north) + tr(" point(s) ") + (north == lowest ? mesg2 : "") +
         tr("\n[Info]: Player '")                   + label_player_name[PLAYER_EAST]->text() + "': " +
          QString::number(east) + tr(" point(s) ")  + (east == lowest ? mesg2 : "");

  message(mesg);

 #ifdef __al_included_allegro5_allegro_audio_h
   if (ui->actionSounds->isChecked())
     al_play_sample(snd_game_over, 1.0, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, nullptr);
 #endif

   if (!ui->actionInfo_Channel->isChecked())
     QMessageBox::information(this, tr("Information"), mesg);
}

void MainWindow::tram(int plr)
{
  QString mesg;

  if (plr == PLAYER_SOUTH)
    mesg = tr("[Info]: You takes the rest!");
  else
    mesg = tr("[Info]: Player '") + label_player_name[plr]->text() + tr("' takes the rest!");

  message(mesg);
  clear_deck();
}

void MainWindow::shoot_moon(int plr)
{
  stats->increase_stats(plr_names_idx[plr], STATS_SHOOT_MOON);

#ifdef __al_included_allegro5_allegro_audio_h
  if (ui->actionSounds->isChecked())
    al_play_sample(snd_shoot_moon, 1.0, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, nullptr);
#endif

  QString mesg;

  if (plr == PLAYER_SOUTH) {
    mesg = tr("[Info]: You shoot the moon!");

    if (ui->actionNew_Moon->isChecked() && (hearts->get_my_score() >= 26)) {
      message(mesg);

      QMessageBox msgBox(this);
      msgBox.setWindowTitle(tr("You shoot the moon!"));
      msgBox.setText(tr("What is your choice ?"));
      QPushButton *button_add = msgBox.addButton(tr("Add"), QMessageBox::YesRole);
      msgBox.addButton(tr("Subtract"), QMessageBox::NoRole);
      msgBox.setDefaultButton(button_add);
      msgBox.exec();
      if (msgBox.clickedButton() == button_add) {
         message(tr("[Info]: You added 26 pts to everyone's scores!"));
         hearts->set_moon_add_to_score(true);
      } else {
          message(tr("[Info]: You substracted 26 pts to your score!"));
          hearts->set_moon_add_to_score(false);
         }
      return;
    }
  } else {
      mesg = tr("[Info]: Player '") + label_player_name[plr]->text() + tr("' shoot the moon!");
    }

  message(mesg);

  if (!ui->actionInfo_Channel->isChecked())
    QMessageBox::information(this, tr("information"), mesg);
}

// this is the online version of shoot_moon()
void MainWindow::shoot_moon(int plr, int delay)
{
#ifdef __al_included_allegro5_allegro_audio_h
  if (ui->actionSounds->isChecked())
    al_play_sample(snd_shoot_moon, 1.0, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, nullptr);
#endif

  QString mesg;

  if (plr == convert_chair(online_my_turn)) {
    mesg = tr("[Info]: You shoot the moon!");

    if (delay) {
      activate_timer(delay, 4, delay);

      message(mesg);

      QMessageBox msgBox(this);
      msgBox.setWindowTitle(tr("You shoot the moon!"));
      msgBox.setText(tr("What is your choice ?"));
      QPushButton *button_add = msgBox.addButton(tr("Add"), QMessageBox::YesRole);
      button_add->animateClick(delay * 10); // delay is in cs, animateClick require in ms
      msgBox.addButton(tr("Subtract"), QMessageBox::NoRole);
      msgBox.setDefaultButton(button_add);
      msgBox.exec();
      if (msgBox.clickedButton() == button_add) {
         message(tr("[Info]: You added 26 pts to everyone's scores!"));

         client->send("moon +");
      } else {
         message(tr("[Info]: You substracted 26 pts to your score!"));

         client->send("moon -");
        }

      remove_timer();
      return;
    }
  }
  else {
    mesg = tr("[Info]: Player '") + label_player_name[plr]->text() + tr("' shoot the moon!");
  }

  message(mesg);

  if (!ui->actionInfo_Channel->isChecked())
    QMessageBox::information(this, tr("information"), mesg);
}

void MainWindow::pass_to(int pass_to)
{
  if (!online_connected) {
    ui->actionNew->setDisabled(false);
    ui->actionConnect->setDisabled(false);
  }
  ui->label_18->setPixmap(QPixmap::fromImage(img_pass[pass_to]->scaledToHeight(80)));
  ui->label_18->setDisabled(pass_to == pNOPASS);

#ifdef DEBUG
  deck->reset_cards_played();
#endif // DEBUG
}

void MainWindow::refresh_score(int plr, int score)
{
  scores[plr] = score;
  label_scores[plr]->setText(QString::number(scores[plr]) + " / " + QString::number(hand_scores[plr]));
}

void MainWindow::refresh_hand_score(int plr, int score)
{
  hand_scores[plr] = score;
  label_scores[plr]->setText(QString::number(scores[plr]) + " / " + QString::number(hand_scores[plr]));
}

void MainWindow::end_of_hand(int score1, int score2, int score3, int score4)
{
  stats->increase_stats(0, STATS_HANDS_PLAYED);

#ifdef DEBUG
  debug->save_card(empty, nullptr);
#endif

  refresh_hand_score(PLAYER_SOUTH, score1);
  refresh_hand_score(PLAYER_WEST, score2);
  refresh_hand_score(PLAYER_NORTH, score3);
  refresh_hand_score(PLAYER_EAST, score4);

  QString mesg;

  mesg = tr("[Info]: New scores: '") + label_player_name[PLAYER_SOUTH]->text() + ": " +
          QString::number(scores[0]) + " (" + QString::number(hand_scores[0]) + ")', '" +
                                   label_player_name[PLAYER_WEST]->text() + ": " +
          QString::number(scores[1]) + " (" + QString::number(hand_scores[1]) + ")', '" +
                                   label_player_name[PLAYER_NORTH]->text() + ": " +
          QString::number(scores[2]) + " (" + QString::number(hand_scores[2]) + ")', '" +
                                   label_player_name[PLAYER_EAST]->text() + ": " +
          QString::number(scores[3]) + " (" + QString::number(hand_scores[3]) + ")'";

  message(mesg);

#ifdef __al_included_allegro5_allegro_audio_h
  if (ui->actionSounds->isChecked() && !hearts->is_game_over()) {
    al_play_sample(snd_shuffling_cards, 1.0, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, nullptr);
    delay(config->get_speed(SPEED_SHUFFLE));
  }
#endif
}

void MainWindow::online_end_hand(int north, int south, int west, int east) {

#ifdef DEBUG
  debug->save_card(empty, nullptr);
#endif

  refresh_score(PLAYER_SOUTH, south);
  refresh_score(PLAYER_WEST, west);
  refresh_score(PLAYER_NORTH, north);
  refresh_score(PLAYER_EAST, east);

  QString mesg;

  mesg = tr("[Info]: New scores: '") + label_player_name[PLAYER_SOUTH]->text() + ": " +
          QString::number(scores[0]) + " (" + QString::number(hand_scores[0]) + ")', '" +
                                   label_player_name[PLAYER_WEST]->text() + ": " +
          QString::number(scores[1]) + " (" + QString::number(hand_scores[1]) + ")', '" +
                                   label_player_name[PLAYER_NORTH]->text() + ": " +
          QString::number(scores[2]) + " (" + QString::number(hand_scores[2]) + ")', '" +
                                   label_player_name[PLAYER_EAST]->text() + ": " +
          QString::number(scores[3]) + " (" + QString::number(hand_scores[3]) + ")'";

  message(mesg);

  for (int i=0; i<4; i++)
    refresh_hand_score(i, 0);
}

void MainWindow::animate_pass_cards(int w1, int w2, int w3, int n1, int n2, int n3, int e1, int e2, int e3)
{
  if (!ui->actionAnimate_Play->isChecked()) return;

  label_cards[PLAYER_WEST][w1]->move(label_cards[PLAYER_WEST][w1]->x() + 15, label_cards[PLAYER_WEST][w1]->y());
  label_cards[PLAYER_WEST][w2]->move(label_cards[PLAYER_WEST][w2]->x() + 15, label_cards[PLAYER_WEST][w2]->y());
  label_cards[PLAYER_WEST][w3]->move(label_cards[PLAYER_WEST][w3]->x() + 15, label_cards[PLAYER_WEST][w3]->y());

  label_cards[PLAYER_NORTH][n1]->move(label_cards[PLAYER_NORTH][n1]->x(), label_cards[PLAYER_NORTH][n1]->y() + 15);
  label_cards[PLAYER_NORTH][n2]->move(label_cards[PLAYER_NORTH][n2]->x(), label_cards[PLAYER_NORTH][n2]->y() + 15);
  label_cards[PLAYER_NORTH][n3]->move(label_cards[PLAYER_NORTH][n3]->x(), label_cards[PLAYER_NORTH][n3]->y() + 15);

  label_cards[PLAYER_EAST][e1]->move(label_cards[PLAYER_EAST][e1]->x() - 15, label_cards[PLAYER_EAST][e1]->y());
  label_cards[PLAYER_EAST][e2]->move(label_cards[PLAYER_EAST][e2]->x() - 15, label_cards[PLAYER_EAST][e2]->y());
  label_cards[PLAYER_EAST][e3]->move(label_cards[PLAYER_EAST][e3]->x() - 15, label_cards[PLAYER_EAST][e3]->y());
  delay(100);
}

void MainWindow::show_deck(bool animate, bool replace)
{
  for (int i=0; i<13; i++) {
    if (replace)
      label_cards[PLAYER_SOUTH][i]->move(label_cards[PLAYER_SOUTH][i]->x(), original_pos_cards[PLAYER_SOUTH]);
    label_cards[PLAYER_SOUTH][i]->hide();

    if (replace)
      label_cards[PLAYER_EAST][i]->move(original_pos_cards[PLAYER_EAST], label_cards[PLAYER_EAST][i]->y());
    label_cards[PLAYER_EAST][i]->hide();

    if (replace)
      label_cards[PLAYER_NORTH][i]->move(label_cards[PLAYER_NORTH][i]->x(), original_pos_cards[PLAYER_NORTH]);
    label_cards[PLAYER_NORTH][i]->hide();

    if (replace)
      label_cards[PLAYER_WEST][i]->move(original_pos_cards[PLAYER_WEST], label_cards[PLAYER_WEST][i]->y());
    label_cards[PLAYER_WEST][i]->hide();
  }

  QMatrix rm;
  rm.rotate(90);

  int adj = 0, total_empty, card, show_card;

  for (int i=0; i<DECK_SIZE; i++) {
    total_empty = 0;

    show_card = hearts->get_card(PLAYER_SOUTH, i/4);

    if (show_card != empty) {
      if (ui->actionAuto_Centering->isChecked())
        adj = adjust_pos[hearts->get_plr_num_cards(PLAYER_SOUTH)];
      label_cards[PLAYER_SOUTH][i/4+adj]->setPixmap(QPixmap::fromImage(deck->get_img_card(show_card)->scaledToHeight(card_height)));
      label_cards[PLAYER_SOUTH][i/4+adj]->show();
    } else
        total_empty++;

    show_card = back_card;

    card = hearts->get_card(PLAYER_EAST, i/4);
#ifdef DEBUG
    if (ui->actionCheat->isChecked())
      show_card = card;
#endif

    if (card != empty) {
      if (ui->actionAuto_Centering->isChecked())
        adj = adjust_pos[hearts->get_plr_num_cards(PLAYER_EAST)];
      label_cards[PLAYER_EAST][i/4+adj]->setPixmap(QPixmap::fromImage(deck->get_img_card(show_card)->transformed(rm).scaledToHeight(60)));
      label_cards[PLAYER_EAST][i/4+adj]->show();
    } else
        total_empty++;

    card = hearts->get_card(PLAYER_NORTH, i/4);
#ifdef DEBUG
    if (ui->actionCheat->isChecked())
      show_card = card;
    else
      show_card = back_card;
#endif

    if (card != empty) {
      if (ui->actionAuto_Centering->isChecked())
        adj = adjust_pos[hearts->get_plr_num_cards(PLAYER_NORTH)];
      label_cards[PLAYER_NORTH][i/4+adj]->setPixmap(QPixmap::fromImage(deck->get_img_card(show_card)->scaledToWidth(60)));
      label_cards[PLAYER_NORTH][i/4+adj]->show();
    } else
        total_empty++;

    card  = hearts->get_card(PLAYER_WEST, i/4);
#ifdef DEBUG
    if (ui->actionCheat->isChecked())
      show_card = card;
    else
      show_card = back_card;
#endif

    if (card != empty) {
      if (ui->actionAuto_Centering->isChecked())
        adj = adjust_pos[hearts->get_plr_num_cards(PLAYER_WEST)];
      label_cards[PLAYER_WEST][i/4+adj]->setPixmap(QPixmap::fromImage(deck->get_img_card(show_card)->transformed(rm).scaledToHeight(60)));
      label_cards[PLAYER_WEST][i/4+adj]->show();
    } else
        total_empty++;

    if (total_empty == 4) return;

    if (animate && ui->actionAnimate_Play->isChecked())
      delay(config->get_speed(SPEED_ANIMATE_PASS_CARDS));
  }
}

void MainWindow::online_show_deck()
{
  std::sort(online_myCards, online_myCards+13);

  for (int i=0; i<13; i++) {
    label_cards[PLAYER_SOUTH][i]->hide();
    label_cards[PLAYER_WEST][i]->hide();
    label_cards[PLAYER_NORTH][i]->hide();
    label_cards[PLAYER_EAST][i]->hide();
  }

  QMatrix rm;
  rm.rotate(90);

  int adj = 0;

  for (int i=0; i<online_num_cards[PLAYER_WEST]; i++) {
     if (ui->actionAuto_Centering->isChecked())
       adj = adjust_pos[online_num_cards[PLAYER_WEST]];
     label_cards[PLAYER_WEST][i + adj]->setPixmap(QPixmap::fromImage(deck->get_img_card(back_card)->transformed(rm).scaledToHeight(60)));
     label_cards[PLAYER_WEST][i + adj]->show();
  }

  for (int i=0; i<online_num_cards[PLAYER_NORTH]; i++) {
     if (ui->actionAuto_Centering->isChecked())
       adj = adjust_pos[online_num_cards[PLAYER_NORTH]];
     label_cards[PLAYER_NORTH][i + adj]->setPixmap(QPixmap::fromImage(deck->get_img_card(back_card)->scaledToWidth(60)));
     label_cards[PLAYER_NORTH][i + adj]->show();
  }

  for (int i=0; i<online_num_cards[PLAYER_EAST]; i++) {
     if (ui->actionAuto_Centering->isChecked())
       adj = adjust_pos[online_num_cards[PLAYER_EAST]];
     label_cards[PLAYER_EAST][i + adj]->setPixmap(QPixmap::fromImage(deck->get_img_card(back_card)->transformed(rm).scaledToHeight(60)));
     label_cards[PLAYER_EAST][i + adj]->show();
  }

  for (int i=0; i<online_num_cards[PLAYER_SOUTH]; i++) {
    int card = online_myCards[i];

    if (ui->actionAuto_Centering->isChecked())
      adj = adjust_pos[online_num_cards[PLAYER_SOUTH]];

    label_cards[PLAYER_SOUTH][i + adj]->setPixmap(QPixmap::fromImage(deck->get_img_card(card)->scaledToHeight(card_height)));
    label_cards[PLAYER_SOUTH][i + adj]->show();

    if (online_selected[i])
      label_cards[PLAYER_SOUTH][i + adj]->move(label_cards[PLAYER_SOUTH][i + adj]->x(), original_pos_cards[PLAYER_SOUTH] - 20);
    else
      label_cards[PLAYER_SOUTH][i + adj]->move(label_cards[PLAYER_SOUTH][i + adj]->x(), original_pos_cards[PLAYER_SOUTH]);
  }
}

void MainWindow::set_info_channel_enabled(bool enable)
{
  if (enable) {
    if (height() + ui->textEdit->height() <= 930)
      setFixedHeight(height() + ui->textEdit->height());
    ui->textEdit->show();
    if (online_connected)
      online_show_lineedit(true);
  } else {
      ui->textEdit->hide();
      setFixedHeight(height() - ui->textEdit->height());
      if (ui->lineEdit->isVisible())
        online_show_lineedit(false);
    }
}

#ifdef DEBUG
void MainWindow::disable_cheat()
{
  ui->actionCheat->setChecked(false);
  ui->actionCheat->setDisabled(true);
}
#endif

void MainWindow::refresh_deck(bool animate)
{
  show_deck(animate, true);
}

void MainWindow::clear_deck()
{
  for (int i=0; i<13; i++)
    label_cards[PLAYER_SOUTH][i]->hide();
}

void MainWindow::clear_table()
{
  delay(config->get_speed(SPEED_CLEAR_TABLE));
  for (int i=0; i<4; i++) {
    label_card_played[i]->setPixmap(QPixmap::fromImage(deck->get_img_card(empty)->scaledToHeight(card_height)));
    card_played[i] = empty;
  }
}

void MainWindow::online_select_card(int num)
{
  int card_id;

  if (ui->actionAuto_Centering->isChecked())
    card_id = num - adjust_pos[online_num_cards[PLAYER_SOUTH]];
  else
    card_id = num;

  assert((card_id >= 0) && (card_id <= 12));

 if (online_playing) {
   QString p = "play " + QString::number(online_myCards[card_id]);

   client->send(p);

   return;
 }

 if (online_selected[card_id]) {
   online_selected[card_id] = false;
   online_num_selected--;
   label_cards[PLAYER_SOUTH][card_id]->move(label_cards[PLAYER_SOUTH][card_id]->x(), original_pos_cards[PLAYER_SOUTH]);
 } else
     if (!online_selected[card_id] && (online_num_selected < 3)) {
       online_selected[card_id] = true;
       online_num_selected++;
       label_cards[PLAYER_SOUTH][card_id]->move(label_cards[PLAYER_SOUTH][card_id]->x(), original_pos_cards[PLAYER_SOUTH] - 20);

#ifdef __al_included_allegro5_allegro_audio_h
       if (ui->actionSounds->isChecked())
         al_play_sample(snd_contact, 1.0, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, nullptr);
#endif
     }
}

void MainWindow::select_card(int num)
{
  if (wait_delay) return;

  if (online_connected) {
    online_select_card(num);
    return;
  }

  int card_id;

  if (ui->actionAuto_Centering->isChecked())
    card_id = num - adjust_pos[hearts->count_my_cards()];
  else
    card_id = num;

  assert((card_id >= 0) && (card_id <= 12));

  if (hearts->is_mode_playing()) {
    if (!hearts->is_it_my_turn()) return;     // wait_delay seem to do the job, but maybe it's just by luck...

    ui->actionNew->setDisabled(true);
    ui->actionConnect->setDisabled(true);

    if (ui->actionEasy_card_selection->isChecked())
      set_cards_disabled(false);

    int errnum;

    if ((errnum = hearts->play_card(card_id))) {
      switch (errnum) {
         case ERRHEART: error(tr("You can't break hearts yet!"));
                        break;
         case ERRSUIT:  error(tr("You must follow the suit! The lead is in ") +
                              QString(suit_names[hearts->get_current_suit()]) + "!");
                        break;
         case ERRQUEEN: error(tr("You can't play the queen of spade on the first hand!"));
                        break;
         case ERR2CLUBS: error(tr("You must play the two of clubs!"));
                         break;
      } 
    }

    ui->actionNew->setDisabled(false);
    ui->actionConnect->setDisabled(false);
    return;
 }

 if (hearts->is_card_selected(card_id) && hearts->unselect_card(card_id))
   label_cards[PLAYER_SOUTH][card_id]->move(label_cards[PLAYER_SOUTH][card_id]->x(), original_pos_cards[PLAYER_SOUTH]);
 else {
    if (!hearts->is_card_selected(card_id) && hearts->select_card(card_id)) {
      label_cards[PLAYER_SOUTH][card_id]->move(label_cards[PLAYER_SOUTH][card_id]->x(), original_pos_cards[PLAYER_SOUTH] - 20);

#ifdef __al_included_allegro5_allegro_audio_h
    if (ui->actionSounds->isChecked())
      al_play_sample(snd_contact, 1.0, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, nullptr);
#endif
    }
 }
}

void MainWindow::play_card(int card, int plr)
{
  if (card == two_clubs)
    delay(config->get_speed(SPEED_PLAY_TWO_CLUBS));
  else
    delay(config->get_speed(SPEED_PLAY_CARD));

  // Animate play card
  if (!online_connected && (plr != PLAYER_SOUTH) && (ui->actionAnimate_Play->isChecked())) {
    int pos = hearts->get_card_position(plr, card);
    if (pos != NOT_FOUND) {
      QMatrix rm;
      rm.rotate(90);

      int adj = 0;

      switch (plr) {
         case PLAYER_WEST: if (ui->actionAuto_Centering->isChecked())
                             adj = adjust_pos[hearts->get_plr_num_cards(PLAYER_WEST)];
                           label_cards[PLAYER_WEST][pos+adj]->move(label_cards[PLAYER_WEST][pos+adj]->x() + 15, label_cards[PLAYER_WEST][pos+adj]->y());
                           label_cards[PLAYER_WEST][pos+adj]->setPixmap(QPixmap::fromImage(deck->get_img_card(card)->transformed(rm).scaledToHeight(60)));
                           break;
         case PLAYER_NORTH: if (ui->actionAuto_Centering->isChecked())
                              adj = adjust_pos[hearts->get_plr_num_cards(PLAYER_NORTH)];
                           label_cards[PLAYER_NORTH][pos+adj]->move(label_cards[PLAYER_NORTH][pos+adj]->x(), label_cards[PLAYER_NORTH][pos+adj]->y() + 15);
                           label_cards[PLAYER_NORTH][pos+adj]->setPixmap(QPixmap::fromImage(deck->get_img_card(card)->scaledToWidth(60)));
                           break;
         case PLAYER_EAST:  if (ui->actionAuto_Centering->isChecked())
                              adj = adjust_pos[hearts->get_plr_num_cards(PLAYER_EAST)];
                           label_cards[PLAYER_EAST][pos+adj]->move(label_cards[PLAYER_EAST][pos+adj]->x() - 15, label_cards[PLAYER_EAST][pos+adj]->y());
                           label_cards[PLAYER_EAST][pos+adj]->setPixmap(QPixmap::fromImage(deck->get_img_card(card)->transformed(rm).scaledToHeight(60)));
      }
      delay(config->get_speed(SPEED_ANIMATE_PLAY_CARD));
    }
  }

#ifdef DEBUG
 deck->set_card_played(card);

// This debug->save_card() need to be inside play_card function(), to save cards coming from hearts
// signals. While save_card() for online games can't be done here, because the names are saved in
// real position into online_names[].
// while here plr = relative position. So, card played from online games are save at ACTION_PLAY.
// using real position.
  if (!online_connected)
   debug->save_card(card, names[plr_names_idx[plr]]);
#endif

 card_played[plr] = card;

 label_card_played[plr]->setPixmap(QPixmap::fromImage(deck->get_img_card(card)->scaledToHeight(card_height)));

#ifdef __al_included_allegro5_allegro_audio_h
 if (ui->actionSounds->isChecked()) {
    al_play_sample(snd_dealing_card, 1.0, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, nullptr);

    if (card == queen_spade)
      al_play_sample(snd_queen_spade, 1.0, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, nullptr);
 }
#endif
}

void MainWindow::check_easy_cards()
{
  if (ui->actionEasy_card_selection->isChecked()) {
    int adj = 0;

    if (ui->actionAuto_Centering->isChecked())
     adj = adjust_pos[hearts->get_plr_num_cards(PLAYER_SOUTH)];

    for (int i=0; i<13; i++) {
      int card = hearts->get_card(PLAYER_SOUTH, i);

      if (card == empty) break;

      bool disable;
      if (hearts->is_card_selectable(card))
        disable = false;
      else
        disable = true;

      label_cards[PLAYER_SOUTH][i+adj]->setDisabled(disable);
    }
  }
}

void MainWindow::show_your_turn(int idx)
{
  delay(config->get_speed(SPEED_YOUR_TURN));

  label_card_played[idx]->setPixmap(QPixmap::fromImage(deck->get_img_card(your_turn)->scaledToHeight(card_height)));
  card_played[idx] = your_turn;

#ifdef __al_included_allegro5_allegro_audio_h
  if (ui->actionSounds->isChecked())
    al_play_sample(snd_your_turn, 1.0, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, nullptr);
#endif

  if (!online_connected) {
    check_easy_cards();
    ui->actionNew->setDisabled(false);
    ui->actionConnect->setDisabled(false);
  }
}

void MainWindow::online_pass_cards()
{
  if (online_passto == pNOPASS) return;

  if (online_num_selected != 3) {
    error(tr("You needs to select 3 cards to pass!"));

    return;
  }

  ui->label_18->setDisabled(true);

  int n = 0;
  QString s = "pass ";

  for (int i=0; i<13; i++)
    if (online_selected[i]) {
      assert(n < 3);
      s.append(QString::number(i) + " ");
      online_cards_received_pos[n] = i;
      n++;
    }

  client->send(s);
  remove_timer();
  wait_delay = true;  // this will disable cards select, while waiting to receive more from the server
}

// This function is called when the pass icon is clicked()
void MainWindow::on_label_18_clicked()
{  
  if (online_connected) {
    online_pass_cards();
    return;
  }

  if (hearts->is_no_pass())
    return;

  if (!hearts->is_ready_to_pass()) {
    error(tr("You needs to select 3 cards to pass!"));

    return;
  }

  ui->label_18->setDisabled(true);

  ui->actionNew->setDisabled(true);
  ui->actionConnect->setDisabled(true);

  hearts->AI_pass_cpus_cards();
  hearts->pass_cards();

  int ptr = 0;
  for (int i=0; i<13; i++)
    if (hearts->is_card_selected(i))
      cards_received[ptr++] = hearts->get_card(i);

  hearts->reset_cards_passed();

  reverse_cards_rgb();
  show_deck(false, false);

#ifdef __al_included_allegro5_allegro_audio_h
  if (ui->actionSounds->isChecked())
    al_play_sample(snd_passing_cards, 1.0, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, nullptr);
#endif

  delay(config->get_speed(SPEED_PASS_CARDS));

  hearts->sort_plr_cards();

  reverse_cards_rgb();

  show_deck(false, true);

  hearts->play_2clubs();
}

void MainWindow::set_cards_disabled(bool d)
{
 for (int i=0; i<13; i++)
   label_cards[PLAYER_SOUTH][i]->setDisabled(d);
}

void MainWindow::reverse_cards_rgb()
{
  deck->reverse_card_rgb(cards_received[0]);
  deck->reverse_card_rgb(cards_received[1]);
  deck->reverse_card_rgb(cards_received[2]);
}

void MainWindow::refresh_cards_played()
{
  for (int i=0; i<4; i++) {
    int card = card_played[i];

    label_card_played[i]->setPixmap(QPixmap::fromImage(deck->get_img_card(card)->scaledToHeight(card_height)));
  }
}

void MainWindow::message(QString mesg)
{
  ui->textEdit->append(mesg);
}

void MainWindow::error(QString mesg)
{
  ui->textEdit->append(tr("[Error]: ") + mesg);

#ifdef __al_included_allegro5_allegro_audio_h
   if (ui->actionSounds->isChecked())
     al_play_sample(snd_error, 1.0, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, nullptr);
#endif
}

void MainWindow::delay(int n)
{
  if (online_connected)
    return;

  wait_delay = true;
  QTime dieTime= QTime::currentTime().addMSecs(n);
  while ((QTime::currentTime() < dieTime) && !stop_delay)
    QCoreApplication::processEvents(QEventLoop::AllEvents, 2000);
  stop_delay = false;
  wait_delay = false;
}

void MainWindow::on_label_card_s1_clicked()
{
  select_card(0);
}

void MainWindow::on_label_card_s2_clicked()
{
  select_card(1);
}

void MainWindow::on_label_card_s3_clicked()
{
  select_card(2);
}

void MainWindow::on_label_card_s4_clicked()
{
  select_card(3);
}

void MainWindow::on_label_card_s5_clicked()
{
  select_card(4);
}

void MainWindow::on_label_card_s6_clicked()
{
  select_card(5);
}

void MainWindow::on_label_card_s7_clicked()
{
  select_card(6);
}

void MainWindow::on_label_card_s8_clicked()
{
  select_card(7);
}

void MainWindow::on_label_card_s9_clicked()
{
  select_card(8);
}

void MainWindow::on_label_card_s10_clicked()
{
  select_card(9);
}

void MainWindow::on_label_card_s11_clicked()
{
  select_card(10);
}

void MainWindow::on_label_card_s12_clicked()
{
  select_card(11);
}

void MainWindow::on_label_card_s13_clicked()
{
  select_card(12);
}

void MainWindow::on_actionNew_triggered()
{
  stats->increase_stats(0, STATS_GAME_STARTED);

#ifdef DEBUG
  debug->reset();
#endif

  set_cards_disabled(false);

  message(tr("[Info]: Starting a new game."));

  set_plr_names();

  stop_delay = true;

  hearts->new_game(false);
}

#ifdef DEBUG
void MainWindow::on_actionCheat_triggered()
{
  bool checked = ui->actionCheat->isChecked();

  config->set_config_file(CONFIG_CHEAT_MODE, checked);

  if (!online_connected)
    show_deck(false, false);
}
#endif

void MainWindow::on_actionAuto_Centering_triggered()
{
  if (online_connected)
    online_show_deck();
  else
    show_deck(false, false);

  config->set_config_file(CONFIG_AUTO_CENTERING, ui->actionAuto_Centering->isChecked());
}

void MainWindow::on_actionTram_triggered()
{
  bool checked = ui->actionTram->isChecked();

  hearts->set_tram_enabled(checked);
  config->set_config_file(CONFIG_DETECT_TRAM, checked);
}

void MainWindow::on_actionInfo_Channel_triggered()
{
  bool checked = ui->actionInfo_Channel->isChecked();

  set_info_channel_enabled(checked);
  config->set_config_file(CONFIG_INFO_CHANNEL, checked);
}

void MainWindow::on_actionSounds_triggered()
{
  config->set_config_file(CONFIG_SOUNDS, ui->actionSounds->isChecked());
}

void MainWindow::on_actionPerfect_100_triggered()
{
  bool checked = ui->actionPerfect_100->isChecked();

  hearts->set_perfect_100(checked);
  config->set_config_file(CONFIG_PERFECT_100, checked);
}

void MainWindow::on_actionOmnibus_triggered()
{
  bool checked = ui->actionOmnibus->isChecked();

  hearts->set_omnibus(checked);
  config->set_config_file(CONFIG_OMNIBUS, checked);
}

void MainWindow::on_actionQueen_Spade_Break_Heart_triggered()
{
  bool checked = ui->actionQueen_Spade_Break_Heart->isChecked();

  hearts->set_queen_spade_break_heart(checked);
  config->set_config_file(CONFIG_QUEEN_SPADE_BREAK_HEART, checked);
}

void MainWindow::on_actionNo_Trick_Bonus_triggered()
{
  bool checked = ui->actionNo_Trick_Bonus->isChecked();

  hearts->set_no_trick_bonus(checked);
  config->set_config_file(CONFIG_NO_TRICK_BONUS, checked);
}

void MainWindow::on_actionNew_Moon_triggered()
{
  bool checked = ui->actionNew_Moon->isChecked();

  hearts->set_new_moon(checked);
  config->set_config_file(CONFIG_NEW_MOON, checked);
}

void MainWindow::on_actionNo_Draw_triggered()
{
  bool checked = ui->actionNo_Draw->isChecked();

  hearts->set_no_draw(checked);
  config->set_config_file(CONFIG_NO_DRAW, checked);
}

void MainWindow::on_actionSave_Game_Quit_triggered()
{
  config->set_config_file(CONFIG_SAVE_GAME, ui->actionSave_Game_Quit->isChecked());
}

void MainWindow::on_actionRules_triggered()
{
  Rules rules_diag(this);
  rules_diag.setModal(true);
  rules_diag.exec();
}

void MainWindow::on_actionCredits_triggered()
{
  Credits credits_diag(this);
  credits_diag.setModal(true);
  credits_diag.exec();
}

void MainWindow::on_actionSettings_triggered()
{
  Settings settings_diag(this);
  settings_diag.setModal(true);
  settings_diag.exec();
}

void MainWindow::on_actionOnline_triggered()
{
 online online_diag(this);
 online_diag.setModal(true);
 online_diag.exec();
}

void MainWindow::on_actionReset_triggered()
{
  QMessageBox msgBox(this);
  msgBox.setText(tr("Do you want to reset statistics?"));
  msgBox.setInformativeText(tr("Are you sure?"));
  msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
  msgBox.setDefaultButton(QMessageBox::No);
  int ret = msgBox.exec();
  if (ret == QMessageBox::Yes) {
     stats->reset();
     message(tr("[Info]: You resetted the statistics!"));
  }
}

void MainWindow::on_actionShow_triggered()
{
  stats->show();
}

#ifdef DEBUG
void MainWindow::on_actionShow_2_triggered()
{
  debug->show();
}
#endif


void MainWindow::set_online_buttons_styles()
{
  QColor color = QColor(63, 159, 82, 255);
  QString style = QString("background-color: %1; border: none;").arg(color.name());

  ui->pushButton->setFocusPolicy(Qt::NoFocus);
  ui->pushButton_2->setFocusPolicy(Qt::NoFocus);
  ui->pushButton_3->setFocusPolicy(Qt::NoFocus);
  ui->pushButton_4->setFocusPolicy(Qt::NoFocus);
  ui->pushButton_5->setFocusPolicy(Qt::NoFocus);
  ui->pushButton_7->setFocusPolicy(Qt::NoFocus);

  ui->pushButton->setStyleSheet(style);
  ui->pushButton_2->setStyleSheet(style);
  ui->pushButton_3->setStyleSheet(style);
  ui->pushButton_4->setStyleSheet(style);
  ui->pushButton_5->setStyleSheet(style);
  ui->pushButton_7->setStyleSheet(style);
}

void MainWindow::set_language(int lang)
{
 ui->actionEnglish->setChecked(false);
 ui->actionFrench->setChecked(false);
 ui->actionRussian->setChecked(false);

 qApp->removeTranslator(&translator);

 switch (lang) {
    case LANG_ENGLISH: if (translator.load(QLocale(QLocale::English), QLatin1String("translation"), QLatin1String("_"), QLatin1String(":/languages"))) {
                         qApp->installTranslator(&translator);

                         ui->actionEnglish->setChecked(true);
                       }
                       break;
    case LANG_FRENCH:  if (translator.load(QLocale(QLocale::French), QLatin1String("translation"), QLatin1String("_"), QLatin1String(":/languages"))) {
                         qApp->installTranslator(&translator);

                         ui->actionFrench->setChecked(true);
                       }
                       break;
    case LANG_RUSSIAN: if (translator.load(QLocale(QLocale::Russian), QLatin1String("translation"), QLatin1String("_"), QLatin1String(":/languages"))) {
                         qApp->installTranslator(&translator);

                         ui->actionRussian->setChecked(true);
                       }
                       break;
 }

 ui->retranslateUi(this);
 table_list->Translate();
 stats->Translate();

#ifdef DEBUG
  debug->Translate();
  deck->Translate();
#else
  ui->menuDebug->setTitle("");
  ui->menuDebug->setEnabled(false);
#endif

}

void MainWindow::on_actionEnglish_triggered(bool checked)
{
  if (!checked) {
    ui->actionEnglish->setChecked(true);
    return;
  }

  set_language(LANG_ENGLISH);

  config->set_language(LANG_ENGLISH);
}

void MainWindow::on_actionFrench_triggered(bool checked)
{
  if (!checked) {
    ui->actionFrench->setChecked(true);
    return;
  }

  set_language(LANG_FRENCH);

  config->set_language(LANG_FRENCH);
}

void MainWindow::on_actionRussian_triggered(bool checked)
{
  if (!checked) {
    ui->actionRussian->setChecked(true);
    return;
  }

  set_language(LANG_RUSSIAN);

  config->set_language(LANG_RUSSIAN);
}

void MainWindow::on_actionEasy_card_selection_triggered()
{
 bool checked = ui->actionEasy_card_selection->isChecked();

 if (!online_connected) {
   if (checked)
     check_easy_cards();
   else
     set_cards_disabled(false);
 }

 config->set_config_file(CONFIG_EASY_CARD_SELECTION, checked);
}

// Animated Play
void MainWindow::on_actionAnimate_Play_triggered()
{
  bool checked = ui->actionAnimate_Play->isChecked();

  config->set_config_file(CONFIG_ANIMATED_PLAY, checked);
}

// Two of clubs auto start
void MainWindow::on_actionAuto_Start_triggered()
{
  bool checked = ui->actionAuto_Start->isChecked();

  config->set_config_file(CONFIG_AUTO_START, checked);

  // FIX ME: maybe send 0 if connected and easy card is set.
  if (!online_connected) {
    if (hearts->is_starting() && ui->actionEasy_card_selection->isChecked())
      set_cards_disabled(false);
    hearts->set_auto_start(checked);
  }
}

#ifdef DEBUG
void MainWindow::on_actionCards_Played_triggered()
{
  deck->show_cards_played();
}
#endif // DEBUG

void MainWindow::on_actionStandard_triggered()
{
  ui->actionStandard->setChecked(true);
  ui->actionEnglish_2->setChecked(false);
  ui->actionRussian_2->setChecked(false);

  if (config->get_deck_style() == STANDARD_DECK)
    return;

  deck->set_deck(STANDARD_DECK);

#ifdef DEBUG
  debug->refresh();
#endif

  if (online_connected)
    online_show_deck();
  else
    show_deck(false, true);

  refresh_cards_played();
  config->set_deck_style(STANDARD_DECK);
}

void MainWindow::on_actionEnglish_2_triggered()
{
  ui->actionStandard->setChecked(false);
  ui->actionEnglish_2->setChecked(true);
  ui->actionRussian_2->setChecked(false);

  if (config->get_deck_style() == ENGLISH_DECK)
    return;

  deck->set_deck(ENGLISH_DECK);

#ifdef DEBUG
  debug->refresh();
#endif

  if (online_connected)
    online_show_deck();
  else
    show_deck(false, true);

  refresh_cards_played();
  config->set_deck_style(ENGLISH_DECK);
}

void MainWindow::on_actionRussian_2_triggered()
{
  ui->actionStandard->setChecked(false);
  ui->actionEnglish_2->setChecked(false);
  ui->actionRussian_2->setChecked(true);

  if (config->get_deck_style() == RUSSIAN_DECK)
    return;

  deck->set_deck(RUSSIAN_DECK);

#ifdef DEBUG
  debug->refresh();
#endif

  if (online_connected)
    online_show_deck();
  else
    show_deck(false, true);

  refresh_cards_played();
  config->set_deck_style(RUSSIAN_DECK);
}

// Speed Slow
void MainWindow::on_actionSlow_triggered()
{
 ui->actionSlow->setChecked(true);
 ui->actionNormal->setChecked(false);
 ui->actionFast->setChecked(false);

 config->set_speed(SPEED_SLOW);
}

// Speed Normal
void MainWindow::on_actionNormal_triggered()
{
 ui->actionSlow->setChecked(false);
 ui->actionNormal->setChecked(true);
 ui->actionFast->setChecked(false);

 config->set_speed(SPEED_NORMAL);
}

// Speed Fast
void MainWindow::on_actionFast_triggered()
{
 ui->actionSlow->setChecked(false);
 ui->actionNormal->setChecked(false);
 ui->actionFast->setChecked(true);

 config->set_speed(SPEED_FAST);
}

void MainWindow::on_lineEdit_returnPressed()
{
  QString data;

  switch (online_button_mode) {
     case mode_button_command: break;
     case mode_button_help: data = "help "; break;
     case mode_button_says: data = "say "; break;
  }

  data += ui->lineEdit->text();

  client->send(data);
  message(data);

  ui->lineEdit->clear();
}

void MainWindow::on_actionConnect_triggered()
{
  bool warning = config->Warning();

  Connect connect_diag(this);
  connect_diag.setModal(true);
  connect_diag.config(config->Username(), config->Password(), warning);

  connect_diag.exec();

  client->setHandle(connect_diag.getHandle());
  client->setPassword(connect_diag.getPassword());

  config->set_online(connect_diag.getHandle(), connect_diag.getPassword());

  if (warning && connect_diag.WarningDisabled())
    config->set_config_file(CONFIG_WARNING, false);

  if (connect_diag.WarningAccepted())
    switch (connect_diag.result()) {
      case QDialog::Accepted: client->socketConnect(true, connect_diag.getHost(), connect_diag.getPort());
                              break;
      case QDialog::Rejected:
                              break;
      default: client->socketConnect(false, connect_diag.getHost(), connect_diag.getPort());
    }
}

void MainWindow::on_actionCreate_Table_triggered()
{
  unsigned int flags = 0;

  CGame table(this);
  table.setModal(true);

  if (ui->actionQueen_Spade_Break_Heart->isChecked())
    flags = QUEEN_SPADE_f;

  if (ui->actionPerfect_100->isChecked())
    flags |= PERFECT_100_f;

  if (ui->actionOmnibus->isChecked())
    flags |= OMNIBUS_f;

  if (ui->actionNo_Trick_Bonus->isChecked())
    flags |= NO_TRICK_BONUS_f;

  if (ui->actionNew_Moon->isChecked())
    flags |= NEW_MOON_f;

  if (ui->actionNo_Draw->isChecked())
    flags |= NO_DRAW_f;

  table.set_flags(flags);

  table.exec();

  if (table.result() == QDialog::Accepted) {
    QString m;

    m.sprintf("new %d", table.get_flags());
    client->send(m);
  }
}

// South
void MainWindow::on_label_played_s_clicked()
{
  if (online_can_sit)
    client->send("sit s");
}

// North
void MainWindow::on_label_played_n_clicked()
{
  if (online_can_sit)
    client->send("sit n");
}

// West
void MainWindow::on_label_played_w_clicked()
{
  if (online_can_sit)
    client->send("sit w");
}

// East
void MainWindow::on_label_played_e_clicked()
{
  if (online_can_sit)
    client->send("sit e");
}

void MainWindow::on_label_heart_s_clicked()
{
  on_label_played_s_clicked();
}

void MainWindow::on_label_score_s_clicked()
{
  on_label_played_s_clicked();
}

void MainWindow::on_label_name_s_clicked()
{
  on_label_played_s_clicked();
}

void MainWindow::on_label_heart_w_clicked()
{
  on_label_played_w_clicked();
}

void MainWindow::on_label_score_w_clicked()
{
  on_label_played_w_clicked();
}

void MainWindow::on_label_name_w_clicked()
{
  on_label_played_w_clicked();
}

void MainWindow::on_label_heart_n_clicked()
{
  on_label_played_n_clicked();
}

void MainWindow::on_label_score_n_clicked()
{
  on_label_played_n_clicked();
}

void MainWindow::on_label_name_n_clicked()
{
  on_label_played_n_clicked();
}

void MainWindow::on_label_heart_e_clicked()
{
  on_label_played_e_clicked();
}

void MainWindow::on_label_score_e_clicked()
{
  on_label_played_e_clicked();
}

void MainWindow::on_label_name_e_clicked()
{
  on_label_played_e_clicked();
}

void MainWindow::init_online_game()
{
  online_table_id = 0;

  for (int i=0; i<4; i++)
    card_played[i] = empty;

  for (int i=0; i<13; i++) {
     online_myCards[i] = empty;
     online_selected[i] = false;
     online_num_cards[PLAYER_SOUTH] = 0;
     online_num_cards[PLAYER_WEST] = 0;
     online_num_cards[PLAYER_NORTH] = 0;
     online_num_cards[PLAYER_EAST] = 0;
  }

  // Clear players's name
  label_player_name[PLAYER_SOUTH]->setText("");
  label_player_name[PLAYER_WEST]->setText("");
  label_player_name[PLAYER_NORTH]->setText("");
  label_player_name[PLAYER_EAST]->setText("");

  // Clear score and hand score
  for (int i=0; i<4; i++) {
    refresh_score(i, 0);
    refresh_hand_score(i, 0);
  }

  pass_to(pNOPASS);
  refresh_cards_played();

  for (int i=0; i<13; i++)
    label_cards[PLAYER_SOUTH][i]->setEnabled(true);

  online_show_deck();
}

int MainWindow::get_name_label(QString p)
{
  if (p == "n")
    return PLAYER_NORTH;
  if (p == "s")
    return PLAYER_SOUTH;
  if (p == "w")
    return PLAYER_WEST;
  if (p == "e")
    return PLAYER_EAST;

  return 0;
}

void MainWindow::online_set_settings(bool disable)
{
#ifdef DEBUG
  ui->actionCards_Played->setDisabled(disable);
  ui->actionCheat->setDisabled(disable);
  if (disable)
    deck->hide_cards_played();
#endif

  ui->actionConnect->setDisabled(disable);
  ui->actionNew->setDisabled(disable);

  ui->actionAnimate_Play->setDisabled(disable);
  ui->actionAuto_Start->setDisabled(disable);
  ui->actionEasy_card_selection->setDisabled(disable);
  ui->actionTram->setDisabled(disable);
  ui->actionSave_Game_Quit->setDisabled(disable);
  ui->menuGame_Speed->setDisabled(disable);

  ui->menuGame_Variations->setDisabled(disable);
}

void MainWindow::online_action(unsigned int action, QString param)
{
  QString m, name;
  QStringList pList = param.split(" ");
  QByteArray ba;

  int wait, c1, c2, c3, p1, p2, p3;
  int tmp[4];
  int north, south, west, east;
  long int s1, s2, s3, s4;

#ifdef DEBUG_DATAGRAMS
  m = "Action: " + QString::number(action) + " Param : " + param;
  message(m);
#endif

  switch (action) {
    case ACTION_CONNECTED:
           // Save the current game
           save_files();

           online_connected = true;
           wait_delay = true; // disable cards select

           // Disable New & Connect Buttons
           online_set_settings(true);

#ifdef DEBUG
           debug->reset();
#endif

           light_connected(true);
           online_show_buttons(true);

           init_online_game();

#ifdef __al_included_allegro5_allegro_audio_h
            if (ui->actionSounds->isChecked())
              al_play_sample(snd_connected, 1.0, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, nullptr);
#endif
           break;
    case ACTION_CHOOSE_CHAIR:
            if (pList.size() != 1) {
              error("ACTION_CHOOSE_CHAIR");
              break;
            }
            online_can_sit = true;
            online_table_id = pList.at(0).toInt();

            for (int i=0; i<4; i++) {
              card_played[i] = sit_here;
              label_card_played[i]->setPixmap(QPixmap::fromImage(deck->get_img_card(sit_here)->scaledToHeight(card_height)));
            }

            break;
    case ACTION_SIT_CHAIR:
            // 0 = table_id   1 = n,s,w,e   2 = bool muted, 3 = player name
           if (pList.size() != 4) {
             error("ACTION_SIT_CHAIR");
             break;
           }

           // update the game list window
           table_list->SetPlayer(pList.at(0), pList.at(3), pList.at(1));

           // not my table --> break
           if (pList.at(0).toInt() != online_table_id) break;

           ui->pushButton_5->setDisabled(pList.at(2).toInt());

           c1 = get_name_label(pList.at(1));

           ba = pList.at(3).toLocal8Bit();
           strncpy(online_names[c1], ba.data(), 20);

           // This is to catch a reconnection
           // If it's a reconnection, i have to rotate the chair.
           // otherwise, we don't rotate the chair.
           if (online_game_started)
              label_player_name[convert_chair(c1)]->setText(pList.at(3));
           else
              label_player_name[c1]->setText(pList.at(3));

           break;
    case ACTION_MY_CHAIR:
            if (pList.size() != 1) {
              error("ACTION_MY_CHAIR");
              break;
             }
            online_my_turn = pList.at(0).toInt();
            online_game_started = false;
#ifdef DEBUG
            debug->reset();
#endif
            break;
    case ACTION_STAND_CHAIR:
            if (pList.size() != 2) {
              error("ACTION_STAND_CHAIR");
              break;
            }
            // update the game list window
            table_list->SetPlayer(pList.at(0), "", pList.at(1));

            // not my table --> break
            if (pList.at(0).toInt() != online_table_id) break;

            c1 = get_name_label(pList.at(1));

            if (online_game_started)
              name = "(" + label_player_name[c1]->text() + ")";
            else
              name = "";

            label_player_name[c1]->setText(name);

            if (!online_game_started)
              label_card_played[c1]->setPixmap(QPixmap::fromImage(deck->get_img_card(sit_here)->scaledToHeight(card_height)));

            break;
    case ACTION_CREATE_TABLE:
            if (pList.size() != 2) {
               error("ACTION_CREATE_TABLE");
               break;
             }
             table_list->AddRow(pList.at(0), pList.at(1));
            break;
    case ACTION_DELETE_TABLE:
            table_list->RemoveRow(param);
            if (online_table_id == param.toInt()) {
              init_online_game();
            }
            break;
    case ACTION_DISCONNECTED:
            // If you connect to a wrong server. You won't receive the datagram for ACTION_CONNECTED.
            // online_connected will be false, and the current game unsaved.
            // If after that you connect to a heart server. The previous connection will trigger
            // ACTION_DISCONNECTED that would call start_game(), and because it was unsaved it will start a
            // new game. On the next disconnection the original unsaved game will be lost and replaced.
            // So, if online_connected is false we must break.
            if (!online_connected)
              break;

            remove_timer();

            table_list->Empty();

            light_connected(false);

            online_show_buttons(false);
            online_set_settings(false);

#ifdef __al_included_allegro5_allegro_audio_h
            if (ui->actionSounds->isChecked())
              al_play_sample(snd_disconnected, 1.0, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, nullptr);
#endif
            online_connected = false;

#ifdef DEBUG
            debug->reset();
#endif

            start_game(false);
            break;
    case ACTION_SET_CARDS:
            if (pList.size() != 15) {
              error("ACTION_SET_CARDS");
              break;
            }

            for (int i=0; i<13;i++)
               online_selected[i] = false;

            online_passto = pList.at(0).toInt();
            wait = pList.at(1).toInt();

            for (int i=2; i<pList.size(); i++)
              online_myCards[i-2] = pList.at(i).toInt();

            online_num_cards[PLAYER_SOUTH] = 13;
            online_num_cards[PLAYER_WEST] = 13;
            online_num_cards[PLAYER_NORTH] = 13;
            online_num_cards[PLAYER_EAST] = 13;

            online_game_started = true;
            online_heart_broken = false;
            online_can_sit = false;
            online_playing = false;
            online_num_selected = 0;

            pass_to(online_passto);
            online_show_deck();

            wait_delay = online_passto == pNOPASS;

            activate_timer(wait, 4, wait);
            break;
    case ACTION_FORCE_PASS:
            remove_timer();
            if (pList.size() != 3) {
              error("ACTION_FORCE_PASS(1)");
              return;
            }
            c1 = pList.at(0).toInt(); // card pos1
            c2 = pList.at(1).toInt(); // card pos2
            c3 = pList.at(2).toInt(); // card pos3
            if ((c1 < 0) || (c2 < 0) || (c3 < 0) || (c1 > 12) || (c2 > 12) || (c3 > 12)) {
              error("ACTION_FORCE_PASS(2)");
              break;
            }
            online_cards_received_pos[0] = c1;
            online_cards_received_pos[1] = c2;
            online_cards_received_pos[2] = c3;
            wait_delay = true;
            ui->label_18->setDisabled(true);

            for (int i=0; i<13; i++)
              online_selected[i] = false;

            online_show_deck(); // clear selected cards

            label_cards[PLAYER_SOUTH][c1]->move(label_cards[PLAYER_SOUTH][c1]->x(), original_pos_cards[PLAYER_SOUTH] - 20);
            label_cards[PLAYER_SOUTH][c2]->move(label_cards[PLAYER_SOUTH][c2]->x(), original_pos_cards[PLAYER_SOUTH] - 20);
            label_cards[PLAYER_SOUTH][c3]->move(label_cards[PLAYER_SOUTH][c3]->x(), original_pos_cards[PLAYER_SOUTH] - 20);
            break;
    case ACTION_RECEIVE_CARDS:
            if (pList.size() != 3) {
               error("ACTION_RECEIVE_CARDS");
               break;
            }
            pList.sort();
#ifdef __al_included_allegro5_allegro_audio_h
            if (ui->actionSounds->isChecked())
              al_play_sample(snd_passing_cards, 1.0, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, nullptr);
#endif
            p1 = online_cards_received_pos[0];
            p2 = online_cards_received_pos[1];
            p3 = online_cards_received_pos[2];
            c1 = pList.at(0).toInt();  // card 1
            c2 = pList.at(1).toInt();  // card 2
            c3 = pList.at(2).toInt();  // card 3
            cards_received[0] = c1;
            cards_received[1] = c2;
            cards_received[2] = c3;
            online_myCards[p1] = c1;
            online_myCards[p2] = c2;
            online_myCards[p3] = c3;

            label_cards[PLAYER_SOUTH][p1]->setPixmap(QPixmap::fromImage(deck->get_img_card(c1)->scaledToHeight(card_height)));
            label_cards[PLAYER_SOUTH][p2]->setPixmap(QPixmap::fromImage(deck->get_img_card(c2)->scaledToHeight(card_height)));
            label_cards[PLAYER_SOUTH][p3]->setPixmap(QPixmap::fromImage(deck->get_img_card(c3)->scaledToHeight(card_height)));

            for (int i=0; i<13; i++)
              online_selected[i] = false;

            break;
    case ACTION_YOUR_TURN:
            if (pList.size() != 1) {
              error("ACTION_YOUR_TURN");
            } else
                activate_timer(pList.at(0).toInt(), 4, pList.at(0).toInt());
            wait_delay = false;                        // enable card playing
            show_your_turn(PLAYER_SOUTH);
            online_playing = true;

            online_show_deck(); // refresh any selected cards, if you have 2 clubs in your hand.
            break;
    case ACTION_PLAY:
            wait_delay = true;                         // disable card playing
            if (pList.size() != 2) {
              error("ACTION PLAY");
              break;
            }
            c1 = convert_chair(pList.at(0).toInt());    // playerid
            if ((c1 < 0) || (c1 > 3)) {
              error("ACTION PLAY(2)");   // card
              break;
            }
            c2 = pList.at(1).toInt();
            if ((c2 < 0) || (c2 > DECK_SIZE - 1)) {
              error("ACTION PLAY(3)");
              break;
            }
            if (!online_heart_broken && (c2 / 13 == HEART)) {
              online_heart_broken = true;
              breaking_hearts();
            }

            online_num_cards[c1]--;
            play_card(c2, c1);

#ifdef DEBUG
            debug->save_card(c2, reinterpret_cast<const char *>(&online_names[pList.at(0).toInt()]));
#endif

            if (c1 == PLAYER_SOUTH) {
              for (int i=0; i<13; i++)
                if (online_myCards[i] == c2) {
                  online_myCards[i] = empty;
                 break;
                }
            }

            remove_timer();
            online_show_deck();

            break;
    case ACTION_HAND_SCORE:
            if (pList.size() != 2) {
              error("ACTION_HAND_SCORE");
              break;
            }
            c1 = convert_chair(pList.at(0).toInt()); // player
            c2 = pList.at(1).toInt(); // hand score
            refresh_hand_score(c1, c2);
            break;
    case ACTION_SCORE:
            clear_table();

            if (pList.size() != 4) {
              error("ACTION_SCORE");
              break;
            }

            tmp[convert_chair(PLAYER_NORTH)] = pList.at(0).toInt(); // score NORTH
            tmp[convert_chair(PLAYER_SOUTH)] = pList.at(1).toInt(); // score SOUTH
            tmp[convert_chair(PLAYER_WEST)]  = pList.at(2).toInt(); // score WEST
            tmp[convert_chair(PLAYER_EAST)]  = pList.at(3).toInt();  // score EAST

            north = tmp[PLAYER_NORTH];
            south = tmp[PLAYER_SOUTH];
            west = tmp[PLAYER_WEST];
            east = tmp[PLAYER_EAST];

            online_end_hand(north, south, west, east);
            break;
    case ACTION_SHOOT_MOON:
            if (pList.size() != 2) {
              error("ACTION_SHOOT_MOON");
              break;
            }
            c1 = convert_chair(pList.at(0).toInt());
            c2 = pList.at(1).toInt();

            assert((c1 >= 0) && (c1 <= 3));

            shoot_moon(c1, c2);
            break;
    case ACTION_BONUS_NO_TRICKS:
            if (pList.size() != 2) {
              error("ACTION_BONUS_NO_TRICKS");
              break;
            }
            c1 = convert_chair(pList.at(0).toInt()); // player who receive the bonus
            c2 = pList.at(1).toInt();                // value of the bonus to subs

            c3 = hand_scores[c1] - c2;
            refresh_hand_score(c1, c3);
            online_receive_bonus(c1, NO_TRICK_BONUS, c2);
            break;
    case ACTION_BONUS_OMNIBUS:
            if (pList.size() != 2) {
              error("ACTION_OMNIBUS");
              break;
            }
            c1 = convert_chair(pList.at(0).toInt()); // player who receive the bonus
            c2 = pList.at(1).toInt();                // value of the bonus

            c3 = hand_scores[c1] - c2;
            refresh_hand_score(c1, c3);
            online_receive_bonus(c1, OMNIBUS_BONUS, c2);
            break;
    case ACTION_PERFECT_100:
            if (pList.size() != 2) {
              error("ACTION_PERFECT_100");
              break;
            }
            c1 = convert_chair(pList.at(0).toInt());
            online_perfect_100(c1);
            break;
    case ACTION_GAMEOVER:
            if (pList.size() != 4) {
              error("ACTION_SCORE");
              break;
            }
            remove_timer();

            tmp[convert_chair(PLAYER_NORTH)] = pList.at(0).toInt(); // score NORTH
            tmp[convert_chair(PLAYER_SOUTH)] = pList.at(1).toInt(); // score SOUTH
            tmp[convert_chair(PLAYER_WEST)] = pList.at(2).toInt(); // score WEST
            tmp[convert_chair(PLAYER_EAST)] = pList.at(3).toInt();  // score EAST

            north = tmp[PLAYER_NORTH];
            south = tmp[PLAYER_SOUTH];
            west = tmp[PLAYER_WEST];
            east = tmp[PLAYER_EAST];

            online_game_over(north, south, west, east);
            break;
    case ACTION_ANNOUNCEMENT:
#ifdef __al_included_allegro5_allegro_audio_h
            if (ui->actionSounds->isChecked())
              al_play_sample(snd_announcement, 1.0, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, nullptr);
#endif
            if (param.isEmpty()) {
              error("ACTION_ANNOUNCEMENT");
              break;
            }
            m = tr("**** ANNOUNCEMENT from ") + param + " ****";
            message(m);
            break;
    case ACTION_PLAYER_MOON:
            clear_table();
            if (pList.size() != 2) {
              error("ACTION_PLAYER_MOON");
              return;
            }
            c1 = convert_chair(pList.at(0).toInt()); // player who shot the moon
            c2 = pList.at(1).toInt();                // bool 1=add, 0=subs
            if (c2) {
              if (c1 != PLAYER_SOUTH) refresh_hand_score(PLAYER_SOUTH, 26);
              if (c1 != PLAYER_NORTH) refresh_hand_score(PLAYER_NORTH, 26);
              if (c1 != PLAYER_WEST) refresh_hand_score(PLAYER_WEST, 26);
              if (c1 != PLAYER_EAST) refresh_hand_score(PLAYER_EAST, 26);
              refresh_hand_score(c1, 0);
            } else {
                 if (c1 != PLAYER_SOUTH) refresh_hand_score(PLAYER_SOUTH, 0);
                 if (c1 != PLAYER_NORTH) refresh_hand_score(PLAYER_NORTH, 0);
                 if (c1 != PLAYER_WEST) refresh_hand_score(PLAYER_WEST, 0);
                 if (c1 != PLAYER_EAST) refresh_hand_score(PLAYER_EAST, 0);
                 refresh_hand_score(c1, -26);
              }
            break;
    case ACTION_LEAVE_TABLE:
            remove_timer();
            init_online_game();
            break;
    case ACTION_CLEAR_TABLE:
            clear_table();
            break;
    case ACTION_TABLE_MUTED:
            ui->pushButton_5->setDisabled(true);
            message(param);
            break;
    case ACTION_SHUFFLE:
#ifdef __al_included_allegro5_allegro_audio_h
            if (ui->actionSounds->isChecked())
              al_play_sample(snd_shuffling_cards, 1.0, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, nullptr);
#endif
            break;
    case ACTION_STATS:
            if (pList.size() != 4) {
               error("ACTION_STATS");
               return;
            }
            s1 = pList.at(0).toLong();
            s2 = pList.at(1).toLong();
            s3 = pList.at(2).toLong();
            s4 = pList.at(3).toLong();
            m = tr("Your online statistics:\r\n") +
                tr("Games  : ")     + QString::number(s1+s2+s3+s4) +
                tr("\r\nFirst  : ") + QString::number(s1) +
                tr("\r\nSecond : ") + QString::number(s2) +
                tr("\r\nThird  : ") + QString::number(s3) +
                tr("\r\nFourth : ") + QString::number(s4);
            message(m);
            break;
    case ACTION_GAME_STARTED:
            if (pList.size() != 1) {
              error("ACTION_GAME_STARTED");
              return;
            }
            clear_table();
            online_rotate_players_name();

            table_list->RemoveRow(param);
            break;
    case ACTION_RECONNECTED:
      // 0   = table id
      // 1   = score S
      // 2   = score W
      // 3   = score N
      // 4   = score E
      // 5   = hand score S
      // 6   = hand score W
      // 7   = hand score N
      // 8   = hand score E
      // 9   = card played S
      // 10  = card played W
      // 11  = card played N
      // 12  = card played E
      // 13  = my chair
      // 14  = Status
      // 15  = Time left
      // 16  = game passto
      // 17  = bool heart broken
      // 18 19 20 21 22 23 24 25 26 27 28 29 30 = my cards (empty card included)
            if (pList.size() != 31) {
              error("ACTION_RECONNECTED");
              return;
            }
            online_table_id = pList.at(0).toInt();
            online_my_turn = pList.at(13).toInt();

            tmp[convert_chair(PLAYER_SOUTH)] = pList.at(1).toInt();
            tmp[convert_chair(PLAYER_WEST)]  = pList.at(2).toInt();
            tmp[convert_chair(PLAYER_NORTH)] = pList.at(3).toInt();
            tmp[convert_chair(PLAYER_EAST)]  = pList.at(4).toInt();

            south = tmp[PLAYER_SOUTH];
            west  = tmp[PLAYER_WEST];
            north = tmp[PLAYER_NORTH];
            east  = tmp[PLAYER_EAST];

            refresh_score(PLAYER_SOUTH, south);
            refresh_score(PLAYER_WEST, west);
            refresh_score(PLAYER_NORTH, north);
            refresh_score(PLAYER_EAST, east);

            tmp[convert_chair(PLAYER_SOUTH)] = pList.at(5).toInt();
            tmp[convert_chair(PLAYER_WEST)]  = pList.at(6).toInt();
            tmp[convert_chair(PLAYER_NORTH)] = pList.at(7).toInt();
            tmp[convert_chair(PLAYER_EAST)]  = pList.at(8).toInt();

            south = tmp[PLAYER_SOUTH];
            west  = tmp[PLAYER_WEST];
            north = tmp[PLAYER_NORTH];
            east  = tmp[PLAYER_EAST];

            refresh_hand_score(PLAYER_SOUTH, south);
            refresh_hand_score(PLAYER_WEST, west);
            refresh_hand_score(PLAYER_NORTH, north);
            refresh_hand_score(PLAYER_EAST, east);

            card_played[convert_chair(PLAYER_SOUTH)] = pList.at(9).toInt();
            card_played[convert_chair(PLAYER_WEST)]  = pList.at(10).toInt();
            card_played[convert_chair(PLAYER_NORTH)] = pList.at(11).toInt();
            card_played[convert_chair(PLAYER_EAST)]  = pList.at(12).toInt();

            refresh_cards_played();
;
            c1 = pList.at(14).toInt(); // status
            c2 = pList.at(15).toInt(); // timeleft

            wait_delay = true;
            online_playing = true;

            if (c1 == STATUS_YOUR_TURN) {
              wait_delay = false;
              show_your_turn(PLAYER_SOUTH);
              if (c2) activate_timer(c2, 4, c2);
            } else
                if (c1 == STATUS_PASSING) {
                  online_playing = false;
                  wait_delay = false;
                 if (c2) activate_timer(c2, 4, c2);
                }

            online_passto = pList.at(16).toInt();

            pass_to(online_passto);
            if ((c1 == STATUS_PASSED) || (c1 != STATUS_PASSING))
              ui->label_18->setDisabled(true);

            // TODO: check to disable passto button
            online_heart_broken = pList.at(17).toInt();

            // TODO: Needs other players num cards
            online_num_cards[PLAYER_SOUTH] = 0;
            online_can_sit = false;
            table_list->RemoveRow(pList.at(0));
            for (int i=0; i<13; i++) {
               c1 = pList.at(18+i).toInt();
               online_myCards[i] = c1;
               online_selected[i] = false;
               if (c1 != empty)
                 online_num_cards[PLAYER_SOUTH]++;
            }

            online_num_cards[PLAYER_WEST] = online_num_cards[PLAYER_SOUTH];
            online_num_cards[PLAYER_NORTH] = online_num_cards[PLAYER_SOUTH];
            online_num_cards[PLAYER_EAST] = online_num_cards[PLAYER_SOUTH];

            if (card_played[PLAYER_WEST] != empty) online_num_cards[PLAYER_WEST]--;
            if (card_played[PLAYER_NORTH] != empty) online_num_cards[PLAYER_NORTH]--;
            if (card_played[PLAYER_EAST] != empty) online_num_cards[PLAYER_EAST]--;

            online_game_started = true;

            online_show_deck();
            break;
    case ACTION_WRONG_VALUE:
            if (pList.size() != 2) {
              error("ACTION_WRONG_VALUE");
              return;
            }
            m = tr("Wrong value ! The range is: [") + QString::number(pList.at(0).toInt()) + ", " + QString::number(pList.at(1).toInt()) + "]";
            message(m);
            break;
    case ACTION_TIME_BANK:
            if (pList.size() != 3) {
               error("ACTION_TIME_BANK");
               return;
            }

            c1 = convert_chair(pList.at(0).toInt()); // player
            c2 = pList.at(1).toInt(); // delay in cs
            c3 = pList.at(2).toInt(); // max delay

            remove_timer();
            activate_timer(c2, c1, c3);
            break;
  }
}

void MainWindow::online_rotate_players_name()
{
  QString name_s, name_w, name_n, name_e;

  // if my sitting at south, i don't needs to rotate player's name
  if (online_my_turn != PLAYER_SOUTH) {
    name_s = label_player_name[PLAYER_SOUTH]->text();
    name_w = label_player_name[PLAYER_WEST]->text();
    name_n = label_player_name[PLAYER_NORTH]->text();
    name_e = label_player_name[PLAYER_EAST]->text();

    label_player_name[convert_chair(PLAYER_WEST)]->setText(name_w);
    label_player_name[convert_chair(PLAYER_NORTH)]->setText(name_n);
    label_player_name[convert_chair(PLAYER_EAST)]->setText(name_e);
    label_player_name[convert_chair(PLAYER_SOUTH)]->setText(name_s);
  }
}

int MainWindow::convert_chair(int chair)
{
  if (online_my_turn == PLAYER_SOUTH) return chair;

  if (online_my_turn == PLAYER_WEST) {
    if (chair == PLAYER_SOUTH) return PLAYER_EAST;
    if (chair == PLAYER_WEST) return PLAYER_SOUTH;
    if (chair == PLAYER_NORTH) return PLAYER_WEST;
 // if (chair == PLAYER_EAST)
    return PLAYER_NORTH;
  }

  if (online_my_turn == PLAYER_NORTH) {
    if (chair == PLAYER_SOUTH) return PLAYER_NORTH;
    if (chair == PLAYER_WEST) return PLAYER_EAST;
    if (chair == PLAYER_NORTH) return PLAYER_SOUTH;
//  if (chair == PLAYER_EAST)
    return PLAYER_WEST;
  }

  if (online_my_turn == PLAYER_EAST) {
    if (chair == PLAYER_SOUTH) return PLAYER_WEST;
    if (chair == PLAYER_WEST) return PLAYER_NORTH;
    if (chair == PLAYER_NORTH) return PLAYER_EAST;
 // if (chair == PLAYER_EAST)
    return PLAYER_SOUTH;
  }

  return PLAYER_SOUTH; // should never comes here, only to remove a compile warning
}

void MainWindow::join_game(int id, char chair)
{
  QString m;

  if (id == online_table_id) {
    if (chair != ' ')
      m.sprintf("sit %c", chair);
    else
      return;
  } else {
      if (chair == ' ')
        m.sprintf("join %d", id);
      else
        m.sprintf("join %d %c", id, chair);
    }
  client->send(m);
}

void MainWindow::on_actionTables_triggered()
{
  table_list->show();
}

void MainWindow::activate_timer(int cs, int bar, int max)
{
  online_bar = bar;

  progress_bar_time[bar]->setMaximum(max * 10);
  progress_bar_time[bar]->setValue(cs * 10);
  progress_bar_time[bar]->show();
  timer->setInterval(100);
  timer->start();
}

void MainWindow::remove_timer()
{
  timer->stop();
  online_hide_progress_bar();
}

void MainWindow::update_bar()
{
  int value = progress_bar_time[online_bar]->value();

  value -= 100;
  if (value <= 0) {
    value = 0;
    remove_timer();
  }
  progress_bar_time[online_bar]->setValue(value);
}

// online button exit
void MainWindow::on_pushButton_clicked()
{
  QMessageBox msgBox(this);

  msgBox.setText(tr("Do you want to leave the server?"));
  msgBox.setInformativeText(tr("Are you sure?"));
  msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
  msgBox.setDefaultButton(QMessageBox::No);
  int ret = msgBox.exec();
  if (ret == QMessageBox::Yes) {
    client->send("exit");
  }
}

// online button leave
void MainWindow::on_pushButton_2_clicked()
{
  client->send("leave");
}

// online button table
void MainWindow::on_pushButton_3_clicked()
{
  on_actionTables_triggered();
}

// online button create table
void MainWindow::on_pushButton_4_clicked()
{
  on_actionCreate_Table_triggered();
}

// online mute
void MainWindow::on_pushButton_5_clicked()
{
  client->send("mute");
}

// start
void MainWindow::on_pushButton_7_clicked()
{
  client->send("start");
}

// online button action: command, help, says
void MainWindow::on_pushButton_6_clicked()
{
  online_button_mode++;
  if (online_button_mode > mode_button_last)
    online_button_mode = mode_button_first;

  switch (online_button_mode) {
     case mode_button_command : ui->pushButton_6->setText(">"); break;
     case mode_button_help :    ui->pushButton_6->setText("?");
                                client->send("help");
                                break;
     case mode_button_says :    ui->pushButton_6->setText("@"); break;
  }
}

// Show or hide the line edit, and the button_6
void MainWindow::online_show_lineedit(bool enable)
{
 if (enable) {
   setFixedHeight(height() + ui->lineEdit->height());
   ui->pushButton_6->show();
   ui->lineEdit->show();
 } else {
     setFixedHeight(height() - ui->lineEdit->height());
     ui->pushButton_6->hide();
     ui->lineEdit->hide();
 }
}

// Show online buttons when connected.
// Hide them when disconnecting.
void MainWindow::online_show_buttons(bool enable)
{
  if (enable) {
    ui->pushButton->show();
    ui->pushButton_2->show();
    ui->pushButton_3->show();
    ui->pushButton_4->show();
    ui->pushButton_5->show();
    ui->pushButton_7->show();
    if (ui->actionInfo_Channel->isChecked())
      online_show_lineedit(true);
  } else {
      ui->pushButton->hide();
      ui->pushButton_2->hide();
      ui->pushButton_3->hide();
      ui->pushButton_4->hide();
      ui->pushButton_5->hide();
      ui->pushButton_7->hide();
      if (ui->actionInfo_Channel->isChecked())
        online_show_lineedit(false);
    }
}

void MainWindow::online_hide_progress_bar()
{
  progress_bar_time[PLAYER_SOUTH]->hide();
  progress_bar_time[PLAYER_WEST]->hide();
  progress_bar_time[PLAYER_NORTH]->hide();
  progress_bar_time[PLAYER_EAST]->hide();
  progress_bar_time[4]->hide();
}
