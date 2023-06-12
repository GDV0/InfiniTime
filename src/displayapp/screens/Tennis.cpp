#include "displayapp/screens/Tennis.h"
#include "displayapp/DisplayApp.h"
#include "displayapp/LittleVgl.h"
#include "displayapp/Apps.h"
#include "displayapp/screens/Symbols.h"

using namespace Pinetime::Applications::Screens;

// *** Variable definitions ***
static const int maxFormat = 9;
static const int maxService = 3;

char* FormatMenu[maxFormat] = { (char *)"1", 
                                (char *)"2", 
                                (char *)"3", 
                                (char *)"4", 
                                (char *)"5", 
                                (char *)"6", 
                                (char *)"7", 
                                (char *)"8", 
                                (char *)"Q" };

char* FormatHelp[maxFormat] = { (char *)"Format  1\n\n3 sets 6 jeux\navec JD 6-6\n", 
                                (char *)"Format  2\n\n2 sets 6 jeux\navec JD 6-6\nSet 3: SJD 10pts", 
                                (char *)"Format  3\n\n2 sets 4 jeux NoAdd\navec JD 4-4\nSet 3: SJD 10pts", 
                                (char *)"Format  4\n\n2 sets 6 jeux NoAdd\navec JD 6-6\nSet 3: SJD 10pts", 
                                (char *)"Format  5\n\n2 sets 3 jeux NoAdd\navec JD 2-2\nSet 3: SJD 10pts", 
                                (char *)"Format  6\n\n2 sets 4 jeux NoAdd\navec JD 3-3\nSet 3: SJD 10pts",  
                                (char *)"Format  7\n\n2 sets 5 jeux NoAdd\navec JD 4-4\nSet 3: SJD 10pts", 
                                (char *)"Format  8\n\n3 sets Super JD\n\n", 
                                (char *)"Quitte\n\n\n\n" };  // Tennis format help

char* ServiceMenu[maxService] = { (char *)"S", 
                                  (char *)"R", 
                                  (char *)"Q" };       // Service item names

char* ServiceHelp[maxService] = { (char *)"Serveur", 
                                  (char *)"Receveur", 
                                  (char *)"Quitte" };  // Service item help texts

char* ScoreGame[51] = { (char *)" O", (char *)"15", (char *)"3O", (char *)"4O", (char *)"Ad", (char *)"-", 
                        (char *)"  ", (char *)"  ", (char *)"  ", (char *)"  ", 
                        (char *)" O", (char *)" 1", (char *)" 2", (char *)" 3", (char *)" 4", (char *)" 5", 
                        (char *)" 6", (char *)" 7", (char *)" 8", (char *)" 9", (char *)" 1O", (char *)" 11", 
                        (char *)" 12", (char *)" 13", (char *)" 14", (char *)" 15", (char *)" 16", (char *)" 17", 
                        (char *)" 18", (char *)" 19", (char *)" 2O", (char *)" 21", (char *)" 22", (char *)" 23", 
                        (char *)" 24", (char *)" 25", (char *)" 26", (char *)" 27", (char *)" 28", (char *)" 29", 
                        (char *)" 3O", (char *)" 31", (char *)" 32", (char *)" 33", (char *)" 34", (char *)" 35", 
                        (char *)" 36", (char *)" 37", (char *)" 38", (char *)" 39", (char *)" 4O" };

uint16_t headHist;
uint16_t tailHist;
uint16_t memHist;

#define HIST_SIZE 20   // Warning: direct impact on RAM overflow during compilation

typedef struct score_t {
  //uint32_t time;
  uint8_t server;
  uint8_t set;
  uint8_t sets[2][3];
  uint8_t tbScore[2][3];
  uint8_t gainSets[2];
  uint8_t jeu[2];
  uint8_t score[2];
  bool tiebreak;
  bool noAdd;
  bool chgtEnds;
} SCORESTRUCT;

SCORESTRUCT Result[HIST_SIZE];

uint8_t format;
bool exitScore;
uint8_t appStep;
int8_t menuItem;
uint8_t matchFormat;
uint8_t matchSetType[3];
uint8_t topServer;
uint8_t maxSets;
uint8_t maxGame;
uint8_t diffGame;
uint8_t memServer;
uint32_t memTimeout;
bool memBle;

// *** LVGL objects ***
lv_obj_t *view;

// Menu screen
lv_style_t help_st;

lv_obj_t *help_lab;
lv_obj_t *item1_lab;
lv_obj_t *item2_lab;
lv_obj_t *item3_lab;
lv_obj_t *item4_lab;
lv_obj_t *item5_lab;

// Display screen
lv_style_t line_st;
lv_style_t score_st;
lv_style_t noAdd_st;
lv_style_t set_st;
lv_style_t chgt_st;

lv_obj_t *sepLine;
lv_obj_t *scoreT;
lv_obj_t *scoreB;
lv_obj_t *setT;
lv_obj_t *setB;
lv_obj_t *imgBall;
lv_obj_t *imgChgt;

// Final score screen
lv_style_t scoreT_st;
lv_style_t scoreB_st;
lv_style_t tiebreak_st;

LV_FONT_DECLARE(lv_font_montserrat_14);
LV_FONT_DECLARE(liquidCrystal_nor_24);
LV_FONT_DECLARE(liquidCrystal_nor_32);
LV_FONT_DECLARE(liquidCrystal_nor_64);
LV_FONT_DECLARE(twentySeven_80);
LV_FONT_DECLARE(lv_font_navi_80);

// *** Function prototypes ***
void Menu( lv_obj_t *Scr, int itemMax, int itemId, char** itemName, char** itemHelp, int state );
void DisplayScore( lv_obj_t *Scr, int state );
void DisplayFinal( lv_obj_t *Scr, int state );
void InitHistory( void );
void CreateHistoryStep( void );
void ManageGame( int G, int P );
void ManageGameAdd( int G, int P, int max, int diff );
void ManageGameNoAdd( int G, int P, int max, int diff) ;
void ManageSuperTBreak( int G, int P, int max, int diff) ;
void ManageGameTMC( int G, int P, int max, int diff );
void ManageSet( int G, int P, int max, int diff );
void ManageSetTMC( int G, int P, int max, int diff );
void ManageMatch( void );

/**************************************************************************//**
* Tennis class constructor
******************************************************************************/
Tennis::Tennis(Pinetime::Components::LittleVgl& lvgl, Controllers::Settings& settingsController) : lvgl {lvgl}, settingsController {settingsController} {
  menuItem = 0;
  appStep = 0;
  headHist = 0;
  memHist = 0;
  tailHist = 0;
  matchFormat = maxFormat - 1;
  maxSets = 3;
  exitScore = false;
  
  memTimeout = settingsController.GetScreenTimeOut();
  memBle = settingsController.GetBleRadioEnabled();
  view = lv_scr_act();

  Menu(view, maxFormat, menuItem, FormatMenu, FormatHelp, 0 );

  Refresh();
}  

/**************************************************************************//**
* Tennis class dectructor
******************************************************************************/
Tennis::~Tennis() {
  lv_obj_clean(view);
  settingsController.SetScreenTimeOut( memTimeout );
  settingsController.SetBleRadioEnabled( memBle );
}

/**************************************************************************//**
* Update Tennis application screen
******************************************************************************/
void Tennis::Refresh() {
  switch (appStep) {
    case 0:
        Menu( view, maxFormat, menuItem, FormatMenu, FormatHelp, 1 );
        break;

    case 1:
        Menu( view, maxService, menuItem, ServiceMenu, ServiceHelp, 1 );
        break;

    case 2:
        DisplayScore( view, 1 );
    default:
        break;
  }
}

/**************************************************************************//**
* Manage touchscreen events (Swipe, Tap, ...)
******************************************************************************/
bool Tennis::OnTouchEvent(Pinetime::Applications::TouchEvents event) {
  switch (appStep) {
    case 0: // Match format menu
        switch (event) {
          case TouchEvents::SwipeLeft:
              // Go to next Format menu item and loop
              menuItem = ( menuItem + 1 ) % maxFormat;
              Refresh();
              break;

          case TouchEvents::SwipeRight:
              // Go to previous Format menu item and loop
              menuItem = ( menuItem + maxFormat - 1 ) % maxFormat;
              Refresh();
              break;

          case TouchEvents::SwipeDown:
          case TouchEvents::DoubleTap:
              if ( menuItem == maxFormat - 1) {
                menuItem = 0;
                appStep = 0;
                matchFormat = maxFormat - 1;
                Menu(view, maxFormat, menuItem, FormatMenu, FormatHelp, 0 );
                Refresh();
              }
              else {
                matchFormat = menuItem + 1;
                menuItem = 0;

                Menu(view, maxService, menuItem, ServiceMenu, ServiceHelp, 0 );
                appStep = 1;
                Refresh();                
              }
              break;

          default:
              break;
        }
        break;

    case 1: // Server menu
        switch (event) {
          case TouchEvents::SwipeLeft:
              // Go to next Service menu item and loop
              menuItem ++;
              if (menuItem > maxService-1) menuItem = 0;
              Refresh();
              break;

          case TouchEvents::SwipeRight:
              // Go to previous Service menu item and loop
              menuItem --;
              if (menuItem < 0) menuItem = maxService-1;
              Refresh();
              break;

          case TouchEvents::SwipeDown:
          case TouchEvents::DoubleTap:
              if ( menuItem == maxService - 1) {
                menuItem = 0;
                appStep = 0;
                matchFormat = maxFormat - 1;
                Menu(view, maxFormat, menuItem, FormatMenu, FormatHelp, 0 );
                Refresh();
              }
              else {
                topServer = menuItem;
                appStep = 2;
                settingsController.SetScreenTimeOut( 4000 );
                settingsController.SetBleRadioEnabled( false );
                ManageMatch();
                DisplayScore( view, 0);
                Refresh();
              }
              break;

          default:
              break;
        }
        break;

    case 2: // Match score
        switch (event) {
          case TouchEvents::SwipeLeft:
              if ( headHist != tailHist ) 
                headHist = ( headHist + HIST_SIZE - 1 ) % HIST_SIZE;
              Refresh();
              break;

          case TouchEvents::SwipeRight:
              if (headHist != memHist) 
                headHist = ( headHist + 1 ) % HIST_SIZE;
              Refresh();
              break;

          case TouchEvents::SwipeUp: // Top player wins the point
              headHist = ( headHist + 1 ) % HIST_SIZE;
              if ( headHist == tailHist )
                tailHist = ( tailHist + 1 ) % HIST_SIZE;
              memHist = headHist;
              CreateHistoryStep();
              ManageGame(0, 1);
              Refresh();
              break;

          case TouchEvents::SwipeDown: // Bottom player wins the point
              headHist = ( headHist + 1 ) % HIST_SIZE;
              if ( headHist == tailHist )
                tailHist = ( tailHist + 1 ) % HIST_SIZE;
              memHist = headHist;
              CreateHistoryStep();
              ManageGame(1, 0);
              Refresh();
              break;

          case TouchEvents::Tap:
              break;

          default:
              break;
        }
        if (exitScore) {
          DisplayFinal(view, 0);
          settingsController.SetScreenTimeOut( memTimeout );
          appStep = 3;
        }
        break;

    case 3: // Final score
            switch (event) {
          case TouchEvents::SwipeLeft:
          case TouchEvents::SwipeRight:
          case TouchEvents::SwipeUp:
          case TouchEvents::SwipeDown:
              break;
          case TouchEvents::Tap:
          case TouchEvents::DoubleTap:
                menuItem = 0;
                appStep = 0;
                matchFormat = maxFormat - 1;
                settingsController.SetBleRadioEnabled( memBle );
                Menu(view, maxFormat, menuItem, FormatMenu, FormatHelp, 0 );
                Refresh();
                break;
          default:
              break;
        }

        break;
    default:
        break;
  }
  return true;
}

/**************************************************************************//**
* Manage touchscreen position (not used)
******************************************************************************/
bool Tennis::OnTouchEvent(uint16_t /*x*/, uint16_t /*y*/) {
  return true;	
}

/**************************************************************************//**
* Display a menu and update it
******************************************************************************/
void Menu( lv_obj_t *Scr, int itemMax, int itemId, char** itemName, char** itemHelp, int state ) {

  if ( state == 0 ) {
    // Remove previous objects from the screen
    lv_obj_clean( Scr );

    // Create menu item styles
    static lv_style_t bubble1;
    lv_style_init( &bubble1 );
    lv_style_set_text_color( &bubble1, LV_STATE_DEFAULT, LV_COLOR_YELLOW) ;
    lv_style_set_text_font(&bubble1, LV_STATE_DEFAULT, &jetbrains_mono_42);
  
    static lv_style_t bubble2;
    lv_style_init( &bubble2 );
    lv_style_set_text_color( &bubble2, LV_STATE_DEFAULT, LV_COLOR_GRAY );
    
    static lv_style_t bubble3;
    lv_style_init( &bubble3);
    lv_style_set_text_color( &bubble3, LV_STATE_DEFAULT, LV_COLOR_GRAY );
    lv_style_set_text_opa( &bubble3, LV_STATE_DEFAULT, LV_OPA_50 );
    
    // Create menu item labels
     item1_lab = lv_label_create( Scr, NULL );
    lv_obj_align( item1_lab, NULL, LV_ALIGN_CENTER, -86, -50 );
    lv_label_set_align( item1_lab, LV_LABEL_ALIGN_CENTER);
    lv_obj_add_style( item1_lab, LV_OBJ_PART_MAIN, &bubble3 );

    item2_lab = lv_label_create( Scr, NULL );
    lv_obj_align( item2_lab, NULL, LV_ALIGN_CENTER, -50, -86 );
    lv_label_set_align( item2_lab, LV_LABEL_ALIGN_CENTER);
    lv_obj_add_style( item2_lab, LV_OBJ_PART_MAIN, &bubble2 );
  
    item3_lab = lv_label_create( Scr, NULL );
    lv_label_set_align( item3_lab, LV_LABEL_ALIGN_CENTER);
    lv_obj_align( item3_lab, NULL, LV_ALIGN_CENTER, 10, -100 );
    lv_obj_add_style( item3_lab, LV_OBJ_PART_MAIN, &bubble1 );
    
    item4_lab = lv_label_create( Scr, NULL );
    lv_obj_align( item4_lab, NULL, LV_ALIGN_CENTER, 80, -86 );
    lv_label_set_align( item4_lab, LV_LABEL_ALIGN_CENTER);
    lv_obj_add_style (item4_lab, LV_OBJ_PART_MAIN, &bubble2 );

    item5_lab = lv_label_create( Scr, NULL );
    lv_obj_align( item5_lab, NULL, LV_ALIGN_CENTER, 120, -50 );
    lv_label_set_align( item5_lab, LV_LABEL_ALIGN_CENTER);
    lv_obj_add_style( item5_lab, LV_OBJ_PART_MAIN, &bubble3 );
    
    // Create menu help label
    lv_style_init( &help_st );
    lv_style_set_bg_color( &help_st, LV_LABEL_PART_MAIN, LV_COLOR_RED );
    lv_style_set_text_color( &help_st, LV_STATE_DEFAULT, LV_COLOR_WHITE );
      
    help_lab = lv_label_create( Scr, NULL );
    lv_obj_set_width( help_lab, 240 );
    lv_obj_add_style( help_lab, LV_OBJ_PART_MAIN, &help_st);
    lv_obj_align( help_lab, Scr, LV_ALIGN_CENTER, 0, 30 );
    lv_obj_set_auto_realign( help_lab, true );
    lv_label_set_align( help_lab, LV_LABEL_ALIGN_CENTER);
  
  } else {
    // Update menu item labels
    lv_label_set_text_fmt( item1_lab, "%s", itemName[(itemMax + itemId - 2) % itemMax] );
    lv_label_set_text_fmt( item2_lab, "%s", itemName[(itemMax + itemId - 1) % itemMax] );
    lv_label_set_text_fmt( item3_lab, "%s", itemName[itemId] );
    lv_label_set_text_fmt( item4_lab, "%s", itemName[(itemMax + itemId + 1) % itemMax] );
    lv_label_set_text_fmt( item5_lab, "%s", itemName[(itemMax + itemId + 2) % itemMax] );

    // Update menu help label
    lv_label_set_text_fmt( help_lab, "%s", itemHelp[itemId] );
  }
}

/**************************************************************************//**
* Display on going match score
******************************************************************************/
void DisplayScore( lv_obj_t *Scr, int state ) {
  if ( state == 0 ) {
    lv_obj_clean( Scr );

    // Draw a separation line
    lv_style_init( &line_st );
    lv_style_set_line_color( &line_st, LV_STATE_DEFAULT, LV_COLOR_WHITE );
    lv_style_set_line_width( &line_st, LV_STATE_DEFAULT, 1 );
    lv_style_set_line_rounded( &line_st, LV_STATE_DEFAULT, 1 );
    static lv_point_t line_points[] = { {0, 0}, {240, 0} };
    sepLine = lv_line_create( Scr, NULL );
    lv_line_set_points( sepLine, line_points, 2 );
    lv_obj_add_style( sepLine, LV_OBJ_PART_MAIN, &line_st );
    lv_obj_align( sepLine, NULL, LV_ALIGN_CENTER, 0, 0 );

    imgBall = lv_obj_create(lv_scr_act(), nullptr);
    lv_obj_set_style_local_bg_color(imgBall, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_YELLOW);
    lv_obj_set_style_local_radius(imgBall, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_RADIUS_CIRCLE);
    lv_obj_set_size(imgBall, 15, 15);
    lv_obj_set_hidden( imgBall, true );
  
    // Create Change ends indicator
    lv_style_init( &chgt_st ); 
    lv_style_set_text_font( &chgt_st, LV_STATE_DEFAULT, &lv_font_sys_48);
    lv_style_set_text_color( &chgt_st, LV_STATE_DEFAULT, LV_COLOR_YELLOW);

    imgChgt = lv_label_create( Scr, NULL );
    lv_obj_set_hidden( imgChgt, false );
    lv_obj_add_style( imgChgt, LV_OBJ_PART_MAIN, &chgt_st );
    lv_label_set_text( imgChgt, Symbols::chgtends);
    lv_obj_align( imgChgt, Scr, LV_ALIGN_IN_RIGHT_MID, -20, 0 );

    lv_style_init( &score_st );
    lv_style_set_text_font( &score_st, LV_STATE_DEFAULT, &twentySeven_80 );
    lv_style_set_text_color( &score_st, LV_STATE_DEFAULT, LV_COLOR_WHITE );

    lv_style_init( &noAdd_st );
    lv_style_set_text_font( &noAdd_st, LV_STATE_DEFAULT, &twentySeven_80 );
    lv_style_set_text_color( &noAdd_st, LV_STATE_DEFAULT, LV_COLOR_ORANGE );

    // Game score Top
    scoreT = lv_label_create( Scr, nullptr );
    lv_obj_add_style( scoreT, LV_OBJ_PART_MAIN, &score_st );
    lv_obj_align( scoreT, Scr, LV_ALIGN_CENTER, 40, -70 );

    // Game score Bottom
    scoreB = lv_label_create( Scr, nullptr );
    lv_obj_add_style( scoreB, LV_OBJ_PART_MAIN, &score_st );
    lv_obj_align( scoreB, Scr, LV_ALIGN_CENTER, 40, 70 );

    // Display set scores
    lv_style_init( &set_st );
    lv_style_set_text_color( &set_st, LV_STATE_DEFAULT, LV_COLOR_WHITE );
    lv_style_set_text_font( &set_st, LV_STATE_DEFAULT, &liquidCrystal_nor_32 );
  
    // Set scores Top
    setT = lv_label_create( Scr, nullptr );
    lv_obj_add_style( setT, LV_OBJ_PART_MAIN, &set_st );
    lv_obj_align( setT, Scr, LV_ALIGN_IN_LEFT_MID, 10, -20 );

    // Set scores Bottom
    setB = lv_label_create( Scr, nullptr );
    lv_obj_add_style( setB, LV_OBJ_PART_MAIN, &set_st );
    lv_obj_align( setB, Scr, LV_ALIGN_IN_LEFT_MID, 10, 20 );
  
  } else {
    // Update Server
    // Ball color depending on history navigation
    if (headHist == memHist)
      lv_obj_set_style_local_bg_color(imgBall, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_YELLOW);
    else    
      lv_obj_set_style_local_bg_color(imgBall, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_RED);

    // Update server position (Top or Bottom)
    if (Result[headHist].server == 0)
      lv_obj_align( imgBall, Scr, LV_ALIGN_IN_LEFT_MID, 40, -80 );
    else
      lv_obj_align( imgBall, Scr, LV_ALIGN_IN_LEFT_MID, 40, 80 );

    // Make Server indicator visible
    lv_obj_set_hidden(imgBall, false);

    // Make change ends indicator visible
    if (Result[headHist].chgtEnds)
      lv_obj_set_hidden(imgChgt, false);
    else
      lv_obj_set_hidden(imgChgt, true);

    // Apply a different color to game score in case of tiebreak
    if ( Result[headHist].tiebreak )
      lv_style_set_text_color( &score_st, LV_STATE_DEFAULT, LV_COLOR_GREEN );  
    else
      lv_style_set_text_color( &score_st, LV_STATE_DEFAULT, LV_COLOR_WHITE );

    // Update current game scores
    // Update Top game score
    if ( Result[headHist].noAdd && !(Result[headHist].server == 0) )
      lv_obj_add_style( scoreT, LV_OBJ_PART_MAIN, &noAdd_st );
    else
      lv_obj_add_style( scoreT, LV_OBJ_PART_MAIN, &score_st );
    lv_label_set_text( scoreT, ScoreGame[Result[headHist].score[0]] );

    // Update Bottom game score
    if (Result[headHist].noAdd && !(Result[headHist].server == 1) )
      lv_obj_add_style(scoreB, LV_OBJ_PART_MAIN, &noAdd_st);
    else
      lv_obj_add_style(scoreB, LV_OBJ_PART_MAIN, &score_st);
    lv_label_set_text( scoreB, ScoreGame[Result[headHist].score[1]] );

    // Update set scores
    // Upsate Top set score
    if ( Result[headHist].set == 0 )
      lv_label_set_text_fmt( setT, "%s", ScoreGame[Result[headHist].sets[0][0] + 10] );
    if ( Result[headHist].set == 1 )
      lv_label_set_text_fmt( setT, "%s  %s", ScoreGame[Result[headHist].sets[0][0] + 10], 
                                              ScoreGame[Result[headHist].sets[0][1] + 10] );
    if ( Result[headHist].set == 2 )
      lv_label_set_text_fmt( setT, "%s  %s  %s", ScoreGame[Result[headHist].sets[0][0] + 10],
                                                  ScoreGame[Result[headHist].sets[0][1] + 10],
                                                  ScoreGame[Result[headHist].sets[0][2] + 10] );

    // update Bottom set score
    if ( Result[headHist].set == 0 )
      lv_label_set_text_fmt( setB, "%s", ScoreGame[Result[headHist].sets[1][0] + 10] );
    if ( Result[headHist].set == 1 )
      lv_label_set_text_fmt( setB, "%s  %s", ScoreGame[Result[headHist].sets[1][0] + 10],
                                              ScoreGame[Result[headHist].sets[1][1] + 10] );
    if ( Result[headHist].set == 2 )
      lv_label_set_text_fmt( setB, "%s  %s  %s", ScoreGame[Result[headHist].sets[1][0] + 10],
                                                  ScoreGame[Result[headHist].sets[1][1] + 10],
                                                  ScoreGame[Result[headHist].sets[1][2] + 10] );
  }
}

/**************************************************************************//**
* Display final match score
******************************************************************************/
void DisplayFinal( lv_obj_t *Scr, int state ) {
  if ( state == 0 ) {
    lv_obj_clean( Scr );

    // Draw a separation line
    lv_style_init(&line_st);
    lv_style_set_line_color(&line_st, LV_STATE_DEFAULT, LV_COLOR_SILVER);
    lv_style_set_line_width(&line_st, LV_STATE_DEFAULT, 1);
    lv_style_set_line_rounded(&line_st, LV_STATE_DEFAULT, 1);
    static lv_point_t line_points[] = { {0, 0}, {240, 0} };

    sepLine = lv_line_create( Scr, NULL );
    lv_line_set_points( sepLine, line_points, 2 );
    lv_obj_add_style( sepLine, LV_OBJ_PART_MAIN, &line_st );
    lv_obj_align( sepLine, NULL, LV_ALIGN_CENTER, 0, 0 );

    // Display final game scores
    lv_style_init( &scoreT_st );
    lv_style_set_text_font( &scoreT_st, LV_STATE_DEFAULT, &twentySeven_80 );
    lv_style_set_text_color( &scoreT_st, LV_STATE_DEFAULT, LV_COLOR_SILVER );

    lv_style_init( &scoreB_st );
    lv_style_set_text_font( &scoreB_st, LV_STATE_DEFAULT, &twentySeven_80 );
    lv_style_set_text_color( &scoreB_st, LV_STATE_DEFAULT, LV_COLOR_SILVER );

    if (Result[headHist].gainSets[0] > Result[headHist].gainSets[1])
      lv_style_set_text_color(&scoreT_st, LV_STATE_DEFAULT, LV_COLOR_LIME);
    else
      lv_style_set_text_color(&scoreB_st, LV_STATE_DEFAULT, LV_COLOR_LIME);

    // Top final game scores
    lv_obj_t *Score1_1 = lv_label_create(Scr, nullptr);
    lv_obj_add_style(Score1_1, LV_OBJ_PART_MAIN, &scoreT_st);
    if ( Result[headHist].sets[0][0] != 0 )
      lv_label_set_text_fmt(Score1_1, "%s", ScoreGame[Result[headHist].sets[0][0] + 10]);
    else
      lv_label_set_text_fmt(Score1_1, "%s", "O");
    lv_obj_align(Score1_1, Scr, LV_ALIGN_CENTER, -80, -80);
    lv_obj_t *Score1_2 = lv_label_create(Scr, nullptr);
    lv_obj_add_style(Score1_2, LV_OBJ_PART_MAIN, &scoreT_st);
    if ( Result[headHist].sets[0][1] != 0 )
      lv_label_set_text_fmt(Score1_2, "%s", ScoreGame[Result[headHist].sets[0][1] + 10]);
    else
      lv_label_set_text_fmt(Score1_2, "%s", "O");
    lv_obj_align(Score1_2, Scr, LV_ALIGN_CENTER, -20, -80);
 
    if (Result[headHist].set > 1) {
      lv_obj_t *Score1_3 = lv_label_create(Scr, nullptr);
      lv_obj_add_style(Score1_3, LV_OBJ_PART_MAIN, &scoreT_st);
      if ( Result[headHist].sets[0][2] != 0 )
        lv_label_set_text_fmt(Score1_3, "%s", ScoreGame[Result[headHist].sets[0][2] + 10]);
      else
        lv_label_set_text_fmt(Score1_3, "%s", "O");
      lv_obj_align(Score1_3, Scr, LV_ALIGN_CENTER, 40, -80);
    }  

    // Bottom final game scores
    lv_obj_t *Score2_1 = lv_label_create(Scr, nullptr);
    lv_obj_add_style(Score2_1, LV_OBJ_PART_MAIN, &scoreB_st);
    if ( Result[headHist].sets[1][0] != 0 )
      lv_label_set_text_fmt(Score2_1, "%s", ScoreGame[Result[headHist].sets[1][0]+ 10]);
    else 
      lv_label_set_text_fmt(Score2_1, "%s", "O");
    lv_obj_align(Score2_1, Scr, LV_ALIGN_CENTER, -80, 80);
    lv_obj_t *Score2_2 = lv_label_create(Scr, nullptr);
    lv_obj_add_style(Score2_2, LV_OBJ_PART_MAIN, &scoreB_st);
    if ( Result[headHist].sets[1][1] != 0 )
      lv_label_set_text_fmt(Score2_2, "%s", ScoreGame[Result[headHist].sets[1][1]+ 10]);
    else
      lv_label_set_text_fmt(Score2_2, "%s", "O");
    lv_obj_align(Score2_2, Scr, LV_ALIGN_CENTER, -20, 80);
    
    if (Result[headHist].set > 1) {
      lv_obj_t *Score2_3 = lv_label_create(Scr, nullptr);
      lv_obj_add_style(Score2_3, LV_OBJ_PART_MAIN, &scoreB_st);
      if ( Result[headHist].sets[1][2] != 0 )
        lv_label_set_text_fmt(Score2_3, "%s", ScoreGame[Result[headHist].sets[1][2]+ 10]);
      else
        lv_label_set_text_fmt(Score2_3, "%s", "O");
      lv_obj_align(Score2_3, Scr, LV_ALIGN_CENTER, 40, 80);
    }  
 
    // Display tiebreaks results
    lv_style_init(&tiebreak_st);
    lv_style_set_text_color(&tiebreak_st, LV_STATE_DEFAULT, LV_COLOR_YELLOW);
    lv_style_set_text_font(&tiebreak_st, LV_STATE_DEFAULT, &liquidCrystal_nor_32);
   
     // Set 1
    if (Result[headHist].tbScore[0][0] != Result[headHist].tbScore[1][0]) {
    // Score Tiebreak Haut
      lv_obj_t *Tb0_1 = lv_label_create(view, nullptr);
      lv_obj_add_style(Tb0_1, LV_OBJ_PART_MAIN, &tiebreak_st);
      lv_label_set_text_fmt(Tb0_1, "%s", ScoreGame[Result[headHist].tbScore[0][0]]);  
      lv_obj_align(Tb0_1, Scr, LV_ALIGN_CENTER, -80, -20);
      // Score Tiebreak Bas
      lv_obj_t *Tb1_1 = lv_label_create(view, nullptr);
      lv_obj_add_style(Tb1_1, LV_OBJ_PART_MAIN, &tiebreak_st);
      lv_label_set_text_fmt(Tb1_1, "%s", ScoreGame[Result[headHist].tbScore[1][0]]);  
      lv_obj_align(Tb1_1, Scr, LV_ALIGN_CENTER, -80, 20);
    }

    // Set 2
    if (Result[headHist].tbScore[0][1] != Result[headHist].tbScore[1][1]) {
    // Score Tiebreak Haut
      lv_obj_t *Tb0_2 = lv_label_create(view, nullptr);
      lv_obj_add_style(Tb0_2, LV_OBJ_PART_MAIN, &tiebreak_st);
      lv_label_set_text_fmt(Tb0_2, "%s", ScoreGame[Result[headHist].tbScore[0][1]]);  
      lv_obj_align(Tb0_2, Scr, LV_ALIGN_CENTER, -20, -20);
      // Score Tiebreak Bas
      lv_obj_t *Tb1_2 = lv_label_create(view, nullptr);
      lv_obj_add_style(Tb1_2, LV_OBJ_PART_MAIN, &tiebreak_st);
      lv_label_set_text_fmt(Tb1_2, "%s", ScoreGame[Result[headHist].tbScore[1][1]]);  
      lv_obj_align(Tb1_2, Scr, LV_ALIGN_CENTER, -20, 20);
    }

    // Set 3
    if (Result[headHist].tbScore[0][2] != Result[headHist].tbScore[1][2]) {
    // Score Tiebreak Haut
      lv_obj_t *Tb0_3 = lv_label_create(view, nullptr);
      lv_obj_add_style(Tb0_3, LV_OBJ_PART_MAIN, &tiebreak_st);
      lv_label_set_text_fmt(Tb0_3, "%s", ScoreGame[Result[headHist].tbScore[0][2]]);  
      lv_obj_align(Tb0_3, Scr, LV_ALIGN_CENTER, 40, -20);
      // Score Tiebreak Bas
      lv_obj_t *Tb1_3 = lv_label_create(view, nullptr);
      lv_obj_add_style(Tb1_3, LV_OBJ_PART_MAIN, &tiebreak_st);
      lv_label_set_text_fmt(Tb1_3, "%s", ScoreGame[Result[headHist].tbScore[1][2]]);  
      lv_obj_align(Tb1_3, Scr, LV_ALIGN_CENTER, 40, 20);
    }
  }
  else {

  }
}

/**************************************************************************//**
* Match history initialisation
******************************************************************************/
void InitHistory( void )
{
  headHist = 0;
  tailHist = 0;
  memHist = 0;
  format = matchFormat;
  Result[headHist].server = topServer;
  Result[headHist].tiebreak = false;
  Result[headHist].noAdd = false;
  Result[headHist].chgtEnds = false;
  Result[headHist].set = 0;

  for (int i = 0; i < maxSets; i++)
  {
    Result[headHist].sets[0][i] = 0;
    Result[headHist].sets[1][i] = 0;
    Result[headHist].tbScore[0][i] = 0;
    Result[headHist].tbScore[1][i] = 0;
  }

  Result[headHist].gainSets[0] = 0;
  Result[headHist].gainSets[1] = 0;
  Result[headHist].jeu[0] = 0;
  Result[headHist].jeu[1] = 0;
  Result[headHist].score[0] = 0;
  Result[headHist].score[1] = 0;  
}

/**************************************************************************//**
* Create new step in match history
******************************************************************************/
void CreateHistoryStep( void )
{
  uint16_t prevStep;

  if ( headHist != tailHist ) {
    prevStep = ( headHist + HIST_SIZE - 1 ) % HIST_SIZE;

    Result[headHist].server = Result[prevStep].server;
    Result[headHist].tiebreak = Result[prevStep].tiebreak;
    Result[headHist].noAdd = Result[prevStep].noAdd;
    Result[headHist].chgtEnds = Result[prevStep].chgtEnds;
    Result[headHist].set = Result[prevStep].set;
 
    for (int i = 0; i < maxSets; i++) {
      Result[headHist].sets[0][i] = Result[prevStep].sets[0][i];
      Result[headHist].sets[1][i] = Result[prevStep].sets[1][i];
      Result[headHist].tbScore[0][i] = Result[prevStep].tbScore[0][i];
      Result[headHist].tbScore[1][i] = Result[prevStep].tbScore[1][i];
    }

    Result[headHist].gainSets[0] =   Result[prevStep].gainSets[0];
    Result[headHist].gainSets[1] =   Result[prevStep].gainSets[1];
    Result[headHist].jeu[0] = Result[prevStep].jeu[0];
    Result[headHist].jeu[1] = Result[prevStep].jeu[1];
    Result[headHist].score[0] = Result[prevStep].score[0];
    Result[headHist].score[1] = Result[prevStep].score[1];
  }
}

/**************************************************************************//**
* Select the game on going
******************************************************************************/
void ManageGame( int G, int P ) {
  switch (matchSetType[Result[headHist].set]) {
    case 0:
        exitScore = true;
        break;
    case 1:
        maxGame = 6;
        diffGame = 2;        
        ManageGameAdd (G, P, maxGame, diffGame);
        break;
    case 2:
        maxGame = 6;
        diffGame = 2;        
        ManageGameNoAdd (G, P, maxGame, diffGame);
        break;
    case 3:
        ManageSuperTBreak (G, P, maxGame, diffGame);
        break;
    case 4:
        maxGame = 4;
        diffGame = 2;        
        ManageGameNoAdd (G, P, maxGame, diffGame);
        break;
    case 5:
        maxGame = 3;
        diffGame = 2;        
        ManageGameTMC (G, P, maxGame, diffGame);
        break;
    case 6:
        maxGame = 4;
        diffGame = 2;        
        ManageGameTMC (G, P, maxGame, diffGame);
        break;
    case 7:
        maxGame = 5;
        diffGame = 2;        
        ManageGameTMC (G, P, maxGame, diffGame);
        break;
  }
}

/**************************************************************************//**
* Manage a normal game 
******************************************************************************/
void ManageGameAdd( int G, int P, int max, int diff ) {
  Result[headHist].score[G] += 1;
  Result[headHist].chgtEnds = false;

  if (!Result[headHist].tiebreak)
  {
    // Manage a "normal" game
    // Game won after server advantage
    if (Result[headHist].score[G] == 5) {
      Result[headHist].server = ( Result[headHist].server + 1 ) % 2;
      ManageSet(G, P, max, diff);      
    }    
    // Back to deuce after receiver advantage 
    if (Result[headHist].score[G] == 6)
      Result[headHist].score[G] = Result[headHist].score[P] = 3;     
    // Game won 
    if  ((Result[headHist].score[G] == 4) && (Result[headHist].score[P] < 3)) {
      Result[headHist].server = ( Result[headHist].server + 1 ) % 2;
      ManageSet(G, P, max, diff);      
    }
    // Manage advantage
    if ((Result[headHist].score[G] == 4) && (Result[headHist].score[P] == 3))
      Result[headHist].score[P] = 5; 
  }
  else { 
    // Toebreak management
    // Change ends every 6 points
    if (((Result[headHist].score[G] + Result[headHist].score[P] - 20) % 6) == 0)
      Result[headHist].chgtEnds = true;

    // Swap service pour somme de points impair
    if ((Result[headHist].score[G] + Result[headHist].score[P])%2 == 1)
      Result[headHist].server = ( Result[headHist].server + 1 ) % 2;
    
    // Fin du tiebreak si score gagnant >= 7 points avec 2 points d'écart 
    Result[headHist].tbScore[G][Result[headHist].set] = Result[headHist].score[G];
    Result[headHist].tbScore[P][Result[headHist].set] = Result[headHist].score[P];
    if ((Result[headHist].score[G] >= 17) && ((Result[headHist].score[G] - Result[headHist].score[P]) >= 2))
      ManageSet(G, P, max, diff);
  } 
}

/**************************************************************************//**
* Manage a game with no Add 
******************************************************************************/
void ManageGameNoAdd( int G, int P, int max, int diff) {
  Result[headHist].score[G] += 1;
  Result[headHist].chgtEnds = false;
  if (!Result[headHist].tiebreak) {
    if ((Result[headHist].score[G] == 3) && (Result[headHist].score[P] == 3))
      Result[headHist].noAdd = true;

    // Gain du jeu avec 2 points d'ecart ou avec un point en No Add
    if ((Result[headHist].score[G] == 4) && (Result[headHist].score[P] <= 3)) {
      Result[headHist].server = !Result[headHist].server;
      Result[headHist].noAdd = false;
      ManageSet(G, P, max, diff);      
    }
  }
  else { // Tiebreak
    // Change ends every 6 points
    if (((Result[headHist].score[G] + Result[headHist].score[P] - 20) % 6) == 0)
      Result[headHist].chgtEnds = true;

    // Changement de service pour somme de points impair
    if ((Result[headHist].score[G] + Result[headHist].score[P])%2 == 1)
      Result[headHist].server = ( Result[headHist].server + 1 ) % 2;

    Result[headHist].tbScore[G][Result[headHist].set] = Result[headHist].score[G];
    Result[headHist].tbScore[P][Result[headHist].set] = Result[headHist].score[P];
    if ((Result[headHist].score[G] >= 17) && ((Result[headHist].score[G] - Result[headHist].score[P])>=2))
      ManageSet(G, P, max, diff);
  }
}

/**************************************************************************//**
* Manage a Super TieBreak 
******************************************************************************/
void ManageSuperTBreak( int G, int P, int max, int diff) {
  
  diff = diff;
  max = max;
 
  if (Result[headHist].score[G] == 0) Result[headHist].score[G] = 10;
  if (Result[headHist].score[P] == 0) Result[headHist].score[P] = 10;
  
  Result[headHist].score[G] += 1;  
  Result[headHist].chgtEnds = false;
  Result[headHist].tbScore[G][Result[headHist].set] = Result[headHist].score[G];
  Result[headHist].tbScore[P][Result[headHist].set] = Result[headHist].score[P];
  
  // Changement de service pour somme de points impair
  if ((Result[headHist].score[G] + Result[headHist].score[P])%2 == 1)
    Result[headHist].server = (Result[headHist].server + 1 ) % 2;
  // Changement de terrain tous les 6 points
  if (((Result[headHist].score[G] + Result[headHist].score[P] - 20) % 6) == 0) 
      Result[headHist].chgtEnds = true;
  
  if ((Result[headHist].score[G] >= 20) && ((Result[headHist].score[G] - Result[headHist].score[P]) >= 2)) {
    Result[headHist].score[G] = Result[headHist].score[P] = 0;
    Result[headHist].sets[G][Result[headHist].set] += 1;
    Result[headHist].set +=1;
    Result[headHist].gainSets[G] +=1;
    if (Result[headHist].gainSets[G] > maxSets/2) {
      exitScore = true;
      matchSetType[Result[headHist].set] = 0;
      Result[headHist].set -= 1;
    }
  }   
}

/**************************************************************************//**
* Manage a game specific to TMC
******************************************************************************/
void ManageGameTMC( int G, int P, int max, int diff ) {
  Result[headHist].score[G] += 1;
  Result[headHist].chgtEnds = false;
  if (!Result[headHist].tiebreak) {
    if ((Result[headHist].score[G] == 3) && (Result[headHist].score[P] == 3))
      Result[headHist].noAdd = true;

    if ((Result[headHist].score[G] == 4) && (Result[headHist].score[P] <= 3)) {
      Result[headHist].server = ( Result[headHist].server + 1 ) % 2;
      Result[headHist].noAdd = false;
      ManageSet(G, P, max, diff);      
    }
  }
  else {
    if (((Result[headHist].score[G] + Result[headHist].score[P] - 20) % 6) == 0)
      Result[headHist].chgtEnds = true;

    // Changement de service pour somme de points impair
    if ((Result[headHist].score[G] + Result[headHist].score[P])%2 == 1)
      Result[headHist].server = ( Result[headHist].server + 1 ) % 2;

    Result[headHist].tbScore[G][Result[headHist].set] = Result[headHist].score[G];
    Result[headHist].tbScore[P][Result[headHist].set] = Result[headHist].score[P];
    if ((Result[headHist].score[G] >= 17) && ((Result[headHist].score[G] - Result[headHist].score[P]) >= 2 ))
      ManageSet(G, P, max, diff);
  }
}

/**************************************************************************//**
* Manage a set
******************************************************************************/
void ManageSet( int G, int P, int max, int diff ) {
  
//  diff = diff;
  Result[headHist].score[G] = Result[headHist].score[P] = 0;
  Result[headHist].sets[G][Result[headHist].set] += 1;
  Result[headHist].chgtEnds = false;

  if ((Result[headHist].sets[G][Result[headHist].set] + Result[headHist].sets[P][Result[headHist].set]) % 2 == 1)
    Result[headHist].chgtEnds = true;
  
  // Launch Tie break;
  if ((Result[headHist].sets[G][Result[headHist].set] == max) and Result[headHist].sets[P][Result[headHist].set] == max) {
    Result[headHist].tiebreak = true;
    Result[headHist].score[0] = Result[headHist].score[1] = 10;
    
    memServer = ( Result[headHist].server + 1 ) % 2;
  }
  
  // Ends of set management
  // Normal end: 6 games with a margin of two games over the opponent
  if ((Result[headHist].sets[G][Result[headHist].set] == max) and Result[headHist].sets[P][Result[headHist].set] <= (max-diff)) {
    Result[headHist].gainSets[G] +=1;
    if (Result[headHist].gainSets[G] > maxSets/2) {
      exitScore = true;
      matchSetType[Result[headHist].set] = 0;
    } else
      Result[headHist].set += 1;
  }

  // End of set 7 games to 5 or tiebreak
  if ((Result[headHist].sets[G][Result[headHist].set] == (max + 1)) and Result[headHist].sets[P][Result[headHist].set] == (max - 1)) {
    Result[headHist].gainSets[G] +=1;
    if (Result[headHist].gainSets[G] > maxSets/2) {
      exitScore = true;
      matchSetType[Result[headHist].set] = 0;
    } else {
      Result[headHist].set +=1;
      // Apply a different color to the game score in case of super tiebreak
      if (matchSetType[Result[headHist].set] == 3)
        Result[headHist].tiebreak = true;
    }
//    Result[headHist].set +=1;
  }

  // 
  if ((Result[headHist].sets[G][Result[headHist].set] == (max + 1)) and (Result[headHist].sets[P][Result[headHist].set] == max) and Result[headHist].tiebreak) {
    Result[headHist].gainSets[G] +=1;
    if (Result[headHist].gainSets[G] > maxSets/2) {
      exitScore = true;
      matchSetType[Result[headHist].set] = 0;
    } else {
      Result[headHist].set +=1;
      Result[headHist].server = memServer;
      Result[headHist].tiebreak = false;
    }
  }  
}

/**************************************************************************//**
* Manage the match
******************************************************************************/
void ManageMatch( void ) {
  InitHistory();

//GDV  switch(Result[headHist].format)  
  switch( format )  
  {
    case 1:
        maxSets = 3;
        matchSetType[0] = 1;
        matchSetType[1] = 1;
        matchSetType[2] = 1;
        break;
    case 2:
        maxSets = 3;
        matchSetType[0] = 1;
        matchSetType[1] = 1;
        matchSetType[2] = 3;
        break;
    case 3:
        maxSets = 3;
        matchSetType[0] = 4;
        matchSetType[1] = 4;
        matchSetType[2] = 3;
        break;
    case 4:
        maxSets = 3;
        matchSetType[0] = 2;
        matchSetType[1] = 2;
        matchSetType[2] = 3;
        break;
    case 5:
        maxSets = 3;
        matchSetType[0] = 5;
        matchSetType[1] = 5;
        matchSetType[2] = 3;
        break;
    case 6:
        maxSets = 3;
        matchSetType[0] = 6;
        matchSetType[1] = 6;
        matchSetType[2] = 3;
        break;
    case 7:
        maxSets = 3;
        matchSetType[0] = 7;
        matchSetType[1] = 7;
        matchSetType[2] = 3;
        break;
    case 8:
        maxSets = 3;
        matchSetType[0] = 3;
        matchSetType[1] = 3;
        matchSetType[2] = 3;
        break;
  }
}

