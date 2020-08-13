#include <stdarg.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>

#include "inkview.h"
#include "frontend/chooser.h"

void chooserTap(int x, int y) {
    init_tap_x = x;
    init_tap_y = y;
    int i;

    if (coord_in_button(x, y, &btn_home)) button_to_tapped(&btn_home);
    if (coord_in_button(x, y, &btn_draw)) button_to_tapped(&btn_draw);
    if (coord_in_button(x, y, &btn_prev)) button_to_tapped(&btn_prev);
    if (coord_in_button(x, y, &btn_next)) button_to_tapped(&btn_next);
    for (i=0;i<num_games;i++) {
        if (coord_in_button(x, y, &btn_chooser[i])) {
            button_to_tapped(&btn_chooser[i]);
            break;
        }
    }
}

void chooserLongTap(int x, int y) {
}

void chooserDrag(int x, int y) {
}

void chooserRelease(int x, int y) {
    int i;
    if (coord_in_button(init_tap_x, init_tap_y, &btn_home)) button_to_normal(&btn_home, true);
    if (coord_in_button(init_tap_x, init_tap_y, &btn_draw)) button_to_normal(&btn_draw, true);
    if (coord_in_button(init_tap_x, init_tap_y, &btn_prev)) button_to_normal(&btn_prev, true);
    if (coord_in_button(init_tap_x, init_tap_y, &btn_next)) button_to_normal(&btn_next, true);
    for (i=0;i<num_games;i++) {
        if (coord_in_button(init_tap_x, init_tap_y, &btn_chooser[i])) {
            button_to_normal(&btn_chooser[i], true);
            switchToGame(btn_chooser[i].thegame);
            return;
        }
    }

    if (release_button(x, y, &btn_home)) {
        exitApp();
    }
    else if (release_button(x, y, &btn_draw)) {
        chooserShowPage();
    }
    else if (release_button(x, y, &btn_prev) &&
        (current_chooserpage > 0)) {
            current_chooserpage -= 1;
            chooserShowPage();
    }
    else if (release_button(x, y, &btn_next) &&
        (current_chooserpage <= chooser_lastpage)) {
            current_chooserpage += 1;
            chooserShowPage();
    }
}

void chooserPrev() {
    if (current_chooserpage > 0) {
        current_chooserpage -= 1;
        chooserShowPage();
    }
}

void chooserNext() {
    if (current_chooserpage <= chooser_lastpage) {
        current_chooserpage += 1;
        chooserShowPage();
    }
}

static void chooserDrawChooserButtons(int page) {
    ifont *font;
    int i;
    FillArea(0, chooserlayout.maincanvas.starty, ScreenWidth(), chooserlayout.maincanvas.height, 0x00FFFFFF);
    font = OpenFont("LiberationSans-Bold", 32, 0);
    for(i=0;i<num_games;i++) {
        if (btn_chooser[i].page == page) {
            btn_chooser[i].active = true;
            button_to_normal(&btn_chooser[i], false);
            DrawTextRect(btn_chooser[i].posx-(chooser_padding/2),
                         btn_chooser[i].posy+btn_chooser[i].size+5,
                         btn_chooser[i].size+chooser_padding, 32,
                         btn_chooser[i].thegame->name, ALIGN_CENTER);
        }
        else {
            btn_chooser[i].active = false;
        }
    }
    CloseFont(font);
}

static void chooserDrawControlButtons(int page) {
    FillArea(0, chooserlayout.buttonpanel.starty, ScreenWidth(), chooserlayout.buttonpanel.height, 0x00FFFFFF);
    FillArea(0, chooserlayout.buttonpanel.starty, ScreenWidth(), 1, 0x00000000);

    if (page == 0) {
        btn_prev.active = false;
        button_to_cleared(&btn_prev, false);
    }
    else {
        btn_prev.active = true;
        button_to_normal(&btn_prev, false);
    }
    if (page == chooser_lastpage) {
        btn_next.active = false;
        button_to_cleared(&btn_next, false);
    }
    else {
        btn_next.active = true;
        button_to_normal(&btn_next, false);
    }
}

static void chooserDrawMenu() {
    ifont *font;
    FillArea(0, chooserlayout.menu.starty, ScreenWidth(), chooserlayout.menu.height, 0x00FFFFFF);
    FillArea(0, chooserlayout.menu.starty + chooserlayout.menu.height-2, ScreenWidth(), 1, 0x00000000);

    button_to_normal(&btn_home, false);
    button_to_normal(&btn_draw, false);

    font = OpenFont("LiberationSans-Bold", 32, 0);
    DrawTextRect(0, (chooserlayout.menubtn_size/2)-(32/2), ScreenWidth(), 32, "PUZZLES", ALIGN_CENTER);
    CloseFont(font);
}

static void chooserSetupChooserButtons() {
    int i;
    int c,r,p,pi;
    int col = chooser_cols;
    int row = chooser_rows;

    for(i=0;i<num_games;i++) {
        p = i / (col*row);
        pi = i % (col*row);
        c = pi % col;
        r = pi / col;
        btn_chooser[i].posx = (c+1)*chooser_padding + c*chooserlayout.chooser_size;
        btn_chooser[i].posy = 50 + chooserlayout.maincanvas.starty + r*(20+32+chooserlayout.chooser_size);
        btn_chooser[i].page = p;
        btn_chooser[i].size = chooserlayout.chooser_size;
    }
}

static void chooserSetupControlButtons() {
    btn_prev.posx = control_padding;
    btn_prev.posy = chooserlayout.buttonpanel.starty + 2;
    btn_prev.size = chooserlayout.control_size;
    btn_next.posx = 2*control_padding + chooserlayout.control_size;
    btn_next.posy = chooserlayout.buttonpanel.starty + 2;
    btn_next.size = chooserlayout.control_size;
}

static void chooserSetupMenuButtons() {
    btn_home.active = true;
    btn_home.posx = 10;
    btn_home.posy = chooserlayout.menu.starty;
    btn_home.size = chooserlayout.menubtn_size; 

    btn_draw.active = true;
    btn_draw.posx = ScreenWidth() - chooserlayout.menubtn_size - 10;
    btn_draw.posy = chooserlayout.menu.starty;
    btn_draw.size = chooserlayout.menubtn_size; 
}

void chooserShowPage() {
    ClearScreen();
    DrawPanel(NULL, "", "", 0);
    chooserDrawMenu();
    chooserDrawChooserButtons(current_chooserpage);
    chooserDrawControlButtons(current_chooserpage);
    FullUpdate();
}

void chooserInit() {
    num_games = 0;
    while (true) {
        if (btn_chooser[num_games].type == BTN_NULL) break;
        num_games++;
    }
    current_chooserpage = 0;
    chooser_lastpage = (num_games-1) / (chooser_cols * chooser_rows);

    chooserlayout = getLayout(LAYOUT_BUTTONBAR);
    control_padding = (ScreenWidth()-(control_num*chooserlayout.control_size))/(control_num+1);
    chooser_padding = (ScreenWidth()-(chooser_cols*chooserlayout.chooser_size))/(chooser_cols+1);

    chooserSetupMenuButtons();
    chooserSetupChooserButtons();
    chooserSetupControlButtons();
}


