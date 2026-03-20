/*
 * Senet: Ancient Egyptian Board Game - Professional Edition
 * Developed by: Saeed Badreldin
 * Build Command: gcc `pkg-config --cflags gtk+-3.0` -o senet_pro senet_pro.c `pkg-config --libs gtk+-3.0`
 */

#include <gtk/gtk.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <gdk/gdkkeysyms.h>

#define BOARD_SIZE 30
#define TURN_TIME 30
#define SCORE_FILE "scores.txt"
#define WINNING_SCORE 50 // 5 pieces * 10 points

// Special Squares Indices
#define SQ_REBIRTH 14  // Square 15
#define SQ_BEAUTY  25  // Square 26 (Must stop here)
#define SQ_WATER   26  // Square 27 (The Trap)

typedef struct {
    GtkWidget *buttons[BOARD_SIZE];
    GtkWidget *info_label;
    GtkWidget *timer_label;
    GtkWidget *score_label;
    GtkWidget *window; // Reference for dialogs
    int board[BOARD_SIZE]; 
    int current_player;
    int last_roll;
    int has_rolled;
    int time_left;
    int score_p1;
    int score_p2;
    int is_demo_mode;
    int game_over;
} SenetGame;

// --- Logic Functions ---

// إعادة ضبط اللعبة بالكامل
void reset_game(SenetGame *game) {
    game->current_player = 1;
    game->last_roll = 0;
    game->has_rolled = 0;
    game->time_left = TURN_TIME;
    game->score_p1 = 0;
    game->score_p2 = 0;
    game->game_over = 0;
    for (int i = 0; i < BOARD_SIZE; i++) game->board[i] = 0;
    for (int i = 0; i < 10; i++) game->board[i] = (i % 2 == 0) ? 1 : 2;
}

// فحص الفائز وإظهار الرسالة
void check_winner(SenetGame *game) {
    int winner = 0;
    if (game->score_p1 >= WINNING_SCORE) winner = 1;
    else if (game->score_p2 >= WINNING_SCORE) winner = 2;

    if (winner > 0 && !game->game_over) {
        game->game_over = 1;
        game->is_demo_mode = 0;
        char msg[128];
        sprintf(msg, "🎉 Congratulations! Player %d has won the game!", winner);
        GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(game->window),
            GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_INFO, GTK_BUTTONS_OK, "%s", msg);
        gtk_window_set_title(GTK_WINDOW(dialog), "Game Over");
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
        reset_game(game); // العودة للبداية بعد الفوز
    }
}

int is_protected(SenetGame *game, int idx) {
    if (idx < 0 || idx >= BOARD_SIZE) return 0;
    int p = game->board[idx];
    if (p == 0) return 0;
    if (idx >= SQ_BEAUTY) return 1;
    if (idx > 0 && game->board[idx - 1] == p) return 1;
    if (idx < BOARD_SIZE - 1 && game->board[idx + 1] == p) return 1;
    return 0;
}

int is_path_blocked(SenetGame *game, int start, int end) {
    for (int i = start + 1; i <= end && i < BOARD_SIZE; i++) {
        int p = game->board[i];
        if (p != 0 && p != game->current_player) {
            if (i < BOARD_SIZE - 1 && game->board[i+1] == p) return 1;
        }
    }
    return 0;
}

void save_score(int p1, int p2) {
    FILE *f = fopen(SCORE_FILE, "a");
    if (f) {
        time_t t = time(NULL);
        struct tm tm = *localtime(&t);
        fprintf(f, "[%04d-%02d-%02d] P1: %d | P2: %d\n", 
                tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, p1, p2);
        fclose(f);
    }
}

// --- UI Rendering ---

void refresh_ui(SenetGame *game) {
    for (int i = 0; i < BOARD_SIZE; i++) {
        GtkStyleContext *context = gtk_widget_get_style_context(game->buttons[i]);
        gtk_style_context_remove_class(context, "p1");
        gtk_style_context_remove_class(context, "p2");
        gtk_style_context_remove_class(context, "special");

        if (game->board[i] == 1) gtk_style_context_add_class(context, "p1");
        else if (game->board[i] == 2) gtk_style_context_add_class(context, "p2");
        if (i >= SQ_BEAUTY) gtk_style_context_add_class(context, "special");
    }

    char status[256], score_txt[64];
    if (game->is_demo_mode) {
        sprintf(status, "🎬 DEMO (ESC to Reset) | Player %d | Roll: %d", 
                game->current_player, game->last_roll);
    } else {
        sprintf(status, "Turn: Player %d | Roll: %d | %s", 
                game->current_player, game->last_roll, 
                game->has_rolled ? "Select Piece" : "Roll Sticks");
    }
    sprintf(score_txt, "🏆 P1: %d | P2: %d", game->score_p1, game->score_p2);
    
    gtk_label_set_text(GTK_LABEL(game->info_label), status);
    gtk_label_set_text(GTK_LABEL(game->score_label), score_txt);
}

// --- Actions ---

void on_roll_clicked(GtkWidget *widget, gpointer data) {
    SenetGame *game = (SenetGame *)data;
    if (game->has_rolled || game->game_over) return;
    int r = 0;
    for(int i=0; i<4; i++) r += rand() % 2;
    game->last_roll = (r == 0) ? 5 : r;
    game->has_rolled = 1;
    refresh_ui(game);
}

void execute_move(SenetGame *game, int idx) {
    if (game->game_over || !game->has_rolled || game->board[idx] != game->current_player) return;

    int target = idx + game->last_roll;

    // منطق الخروج من اللوحة
    if (target >= BOARD_SIZE) {
        if (idx >= SQ_BEAUTY) { // الخروج مسموح فقط من المربعات الأخيرة
            game->board[idx] = 0;
            if (game->current_player == 1) game->score_p1 += 10; else game->score_p2 += 10;
            save_score(game->score_p1, game->score_p2);
            check_winner(game);
        } else return; 
    } 
    else if (target == SQ_WATER) {
        game->board[idx] = 0;
        if (game->board[SQ_REBIRTH] == 0) game->board[SQ_REBIRTH] = game->current_player;
        else {
            for(int i = SQ_REBIRTH; i >= 0; i--) 
                if(game->board[i] == 0) { game->board[i] = game->current_player; break; }
        }
    } 
    else {
        if (is_path_blocked(game, idx, target)) return;
        if (game->board[target] != 0 && game->board[target] != game->current_player) {
            if (is_protected(game, target)) return;
            int opponent = game->board[target];
            game->board[target] = game->current_player;
            game->board[idx] = opponent;
        } else {
            game->board[target] = game->current_player;
            game->board[idx] = 0;
        }
    }

    if (!game->game_over && game->last_roll != 1 && game->last_roll != 4 && game->last_roll != 5) {
        game->current_player = (game->current_player == 1) ? 2 : 1;
    }

    game->has_rolled = 0;
    game->time_left = TURN_TIME;
    refresh_ui(game);
}

void on_square_clicked(GtkWidget *widget, gpointer data) {
    SenetGame *game = (SenetGame *)data;
    int idx = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(widget), "index"));
    execute_move(game, idx);
}

void auto_play_step(SenetGame *game) {
    if (!game->is_demo_mode || game->game_over) return;

    if (!game->has_rolled) {
        on_roll_clicked(NULL, game);
    } else {
        for (int i = BOARD_SIZE - 1; i >= 0; i--) {
            if (game->board[i] == game->current_player) {
                int target = i + game->last_roll;
                if (target >= BOARD_SIZE && i >= SQ_BEAUTY) {
                    execute_move(game, i);
                    return;
                }
                if (target < BOARD_SIZE && !is_path_blocked(game, i, target)) {
                     if (game->board[target] != 0 && game->board[target] != game->current_player) {
                         if (is_protected(game, target)) continue;
                     }
                     execute_move(game, i);
                     return;
                }
            }
        }
        game->current_player = (game->current_player == 1) ? 2 : 1;
        game->has_rolled = 0;
        refresh_ui(game);
    }
}

gboolean update_timer(gpointer data) {
    SenetGame *game = (SenetGame *)data;
    if (game->game_over) return TRUE;
    if (game->is_demo_mode) {
        auto_play_step(game);
        return TRUE;
    }
    
    if (game->time_left > 0) {
        game->time_left--;
        char buf[32]; sprintf(buf, "⏳ %ds", game->time_left);
        gtk_label_set_text(GTK_LABEL(game->timer_label), buf);
    } else {
        game->current_player = (game->current_player == 1) ? 2 : 1;
        game->time_left = TURN_TIME;
        game->has_rolled = 0;
        refresh_ui(game);
    }
    return TRUE;
}

gboolean on_key_press(GtkWidget *widget, GdkEventKey *event, gpointer data) {
    SenetGame *game = (SenetGame *)data;
    if (event->keyval == GDK_KEY_Escape) {
        game->is_demo_mode = 0;
        reset_game(game);
        refresh_ui(game);
    }
    return FALSE;
}

void start_demo(GtkWidget *widget, gpointer data) {
    SenetGame *game = (SenetGame *)data;
    reset_game(game);
    game->is_demo_mode = 1;
    refresh_ui(game);
}

void show_about_senet(GtkWidget *widget, gpointer data) {
    GtkWidget *dialog = gtk_about_dialog_new();
    GdkPixbuf *logo = gdk_pixbuf_new_from_file_at_size("icon.svg", 64, 64, NULL);
    
    gtk_about_dialog_set_program_name(GTK_ABOUT_DIALOG(dialog), "Senet Pro");
    gtk_about_dialog_set_version(GTK_ABOUT_DIALOG(dialog), "1.0 Professional");
    gtk_about_dialog_set_copyright(GTK_ABOUT_DIALOG(dialog), "© 2026 Saeed Badreldin");
    gtk_about_dialog_set_logo(GTK_ABOUT_DIALOG(dialog), logo);
    gtk_about_dialog_set_comments(GTK_ABOUT_DIALOG(dialog), "Ancient Egyptian Board Game for Helwan Linux.");
    gtk_about_dialog_set_website(GTK_ABOUT_DIALOG(dialog), "https://helwan-linux.github.io/helwanlinux/");
    
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
}


void show_help_senet(GtkWidget *widget, gpointer data) {
    GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(data),
        GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_INFO, GTK_BUTTONS_OK,
        "Senet Quick Rules:\n\n"
        "• Objective: Move all your pieces from the start to the end to exit the board.\n"
        "• Movement: Rolling 1, 4, or 5 grants you an extra throw.\n"
        "• Protection: Two pieces of the same color side-by-side cannot be attacked.\n"
        "• The Trap: Square 27 (The Water) sinks your piece and returns it to Square 15.");
    gtk_window_set_title(GTK_WINDOW(dialog), "Game Manual");
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
}

int main(int argc, char *argv[]) {
    srand(time(NULL));
    gtk_init(&argc, &argv);

    GtkCssProvider *provider = gtk_css_provider_new();
    gtk_css_provider_load_from_data(provider,
		"window { background-color: #2c3e50; }"
		/* أزرار اللوحة الرئيسية */
		"button { min-width: 60px; min-height: 60px; border-radius: 4px; font-weight: bold; }"
		"button label { color: black; }" 
		/* المربعات الحوارية (About & Help) */
		"messagedialog label, aboutdialog label { color: black; }"
		"messagedialog { background-color: #ecf0f1; }"
		"aboutdialog { background-color: #ecf0f1; }"
		/* التنسيقات الخاصة بقطع اللاعبين */
		".p1 { background-color: #e67e22; border: 3px solid #d35400; }"
		".p2 { background-color: #3498db; border: 3px solid #2980b9; }"
		".special { border: 2px dashed #f1c40f; }"
		/* نصوص اللوحة الأساسية (العداد والسكور) تظل بيضاء للخلفية الغامقة */
		"window > box > label { color: white; font-size: 16px; }", -1, NULL);
        
    gtk_style_context_add_provider_for_screen(gdk_screen_get_default(), GTK_STYLE_PROVIDER(provider), GTK_STYLE_PROVIDER_PRIORITY_USER);

    SenetGame *game = g_new0(SenetGame, 1);
    reset_game(game);

    game->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    GdkPixbuf *app_icon = gdk_pixbuf_new_from_file_at_size("icon.svg", 48, 48, NULL);
    if (app_icon) {
        gtk_window_set_icon(GTK_WINDOW(game->window), app_icon);
        g_object_unref(app_icon);
    }
    gtk_window_set_title(GTK_WINDOW(game->window), "Senet Pro - Saeed Edition");
    g_signal_connect(game->window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    g_signal_connect(game->window, "key-press-event", G_CALLBACK(on_key_press), game);

    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    game->score_label = gtk_label_new("");
    game->timer_label = gtk_label_new("");
    game->info_label = gtk_label_new("");
    GtkWidget *grid = gtk_grid_new();
    GtkWidget *roll_btn = gtk_button_new_with_label("ROLL STICKS");

    for (int i = 0; i < BOARD_SIZE; i++) {
        char id[4]; sprintf(id, "%d", i + 1);
        game->buttons[i] = gtk_button_new_with_label(id);
        g_object_set_data(G_OBJECT(game->buttons[i]), "index", GINT_TO_POINTER(i));
        g_signal_connect(game->buttons[i], "clicked", G_CALLBACK(on_square_clicked), game);
        int row = i / 10, col = (row == 1) ? (9 - (i % 10)) : (i % 10);
        gtk_grid_attach(GTK_GRID(grid), game->buttons[i], col, row, 1, 1);
    }

    g_signal_connect(roll_btn, "clicked", G_CALLBACK(on_roll_clicked), game);
    
    gtk_box_pack_start(GTK_BOX(vbox), game->score_label, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), game->timer_label, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), game->info_label, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), grid, TRUE, TRUE, 10);
    gtk_box_pack_start(GTK_BOX(vbox), roll_btn, FALSE, FALSE, 10);

    GtkWidget *h_btn_box = gtk_button_box_new(GTK_ORIENTATION_HORIZONTAL);
    GtkWidget *help_btn = gtk_button_new_with_label("Help");
    GtkWidget *about_btn = gtk_button_new_with_label("About");
    GtkWidget *demo_btn = gtk_button_new_with_label("Watch Demo");

    g_signal_connect(help_btn, "clicked", G_CALLBACK(show_help_senet), game->window);
    g_signal_connect(about_btn, "clicked", G_CALLBACK(show_about_senet), game);
    g_signal_connect(demo_btn, "clicked", G_CALLBACK(start_demo), game);

    gtk_container_add(GTK_CONTAINER(h_btn_box), help_btn);
    gtk_container_add(GTK_CONTAINER(h_btn_box), about_btn);
    gtk_container_add(GTK_CONTAINER(h_btn_box), demo_btn);
    gtk_box_pack_start(GTK_BOX(vbox), h_btn_box, FALSE, FALSE, 5);

    gtk_container_add(GTK_CONTAINER(game->window), vbox);
    refresh_ui(game);
    g_timeout_add_seconds(1, update_timer, game);

    gtk_widget_show_all(game->window);
    gtk_main();
    return 0;
}
