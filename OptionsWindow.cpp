/*******************************************************
*   ImageShow©
*
*   This is hopefully going to be a realy nice image
*   viewer. Not like there are a 100 of them out there
*   or anything.
*
*   @author  YNOP (ynop@acm.org)
*   @version beta
*   @date    Sept 18 1999
*******************************************************/
#include <InterfaceKit.h>
#include <TranslationKit.h>
#include <TranslationUtils.h>

//#include <stdio.h>

#include "Globals.h"
#include "OptionsWindow.h"
#include "YLanguageClass.h"
#include "BugOutDef.h"

extern BugOut db;

rgb_color Black = {0,0,0,0};
rgb_color White = {255,255,255,0};
rgb_color Default = { 51,102,152,255 };
rgb_color LightBlue = { 155,155,250 };
rgb_color green = { 0,255,0,255 };
rgb_color darkgreen = { 0,180,0,255 };

/*******************************************************
*   Our wonderful BWindow, ya its kewl like that.
*   we dont do much here but set up the menubar and 
*   let the view take over.  We also nead some message
*   redirection and handling
*******************************************************/
OptionsWindow::OptionsWindow(BWindow *win):BWindow(BRect(50,50,250,150),Language.get("OPTIONS"),B_TITLED_WINDOW_LOOK,B_FLOATING_APP_WINDOW_FEEL,B_ASYNCHRONOUS_CONTROLS|B_NOT_RESIZABLE|B_NOT_ZOOMABLE){
   BRect r;
   r = Bounds();
   
   theApp = win;
   
   BView *View = new BView(r,"",B_FOLLOW_ALL,0);
   View->SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
   
   r.top -= 1;
   r.left -= 1;
   r.right +=1;
   r.bottom +=1;
   BBox *Bb = new BBox(r,"",B_FOLLOW_ALL_SIDES); 
   View->AddChild(Bb);
   r = Bb->Bounds();
   
   r.InsetBy(3,3);
   
   slide = new BSlider(r,Language.get("THUMB_SIZE"),"",new BMessage(THUMB_SIZE),10,200);
   slide->SetModificationMessage(new BMessage(THUMB_SIZE));
   Bb->AddChild(slide);
   slide->SetLimitLabels(Language.get("THUMB_SIZE"), "");
   slide->SetHashMarks(B_HASH_MARKS_TOP);
   slide->SetHashMarkCount(5);
   slide->SetKeyIncrementValue(1);
   slide->SetBarColor(Default);
   slide->UseFillColor(true, &LightBlue);

   BRect size(10,65,0,0);
   BRadioButton *sixteen = new BRadioButton(size,"","16",new BMessage(SET_TO_16),B_FOLLOW_LEFT);
   sixteen->ResizeToPreferred();
   Bb->AddChild(sixteen);
   
   size.left = 50;
   BRadioButton *thirtytwo = new BRadioButton(size,"","32",new BMessage(SET_TO_32),B_FOLLOW_LEFT);
   thirtytwo->ResizeToPreferred();
   Bb->AddChild(thirtytwo);
   
   size.left = 100;
   BRadioButton *sixtyfour = new BRadioButton(size,"","64",new BMessage(SET_TO_64),B_FOLLOW_LEFT);
   sixtyfour->ResizeToPreferred();
   Bb->AddChild(sixtyfour);
   
   size.left = 150;
   BRadioButton *nintysix = new BRadioButton(size,"","96",new BMessage(SET_TO_96),B_FOLLOW_LEFT);
   nintysix->ResizeToPreferred();
   Bb->AddChild(nintysix);
   
   AddChild(View);
   Run();
}

/*******************************************************
*   To make things a little nicer and more organized
*   we are gona let View be the message handler for 
*   the whole app. All messages that are ours send to
*   View for redistribution.  But we must handle our
*   own BWindow messages or else (crash)
*******************************************************/
void OptionsWindow::MessageReceived(BMessage* msg){
   int s;
   
   switch(msg->what){
   case CHANGE_LANGUAGE:
      slide->SetLimitLabels(Language.get("THUMB_SIZE"), "");
      break;
   case THUMB_SIZE:
      s = slide->Value();
      msg->AddInt32("IconSize",s);
      theApp->PostMessage(msg);
      break;
   case SET_TO_16:
      slide->SetValue(16);
      s = 16;
      msg->what = THUMB_SIZE;
      msg->AddInt32("IconSize",s);
      theApp->PostMessage(msg);
      break;
   case SET_TO_32:
      slide->SetValue(32);
      s = 32;
      msg->what = THUMB_SIZE;
      msg->AddInt32("IconSize",s);
      theApp->PostMessage(msg);
      break;
   case SET_TO_64:
      slide->SetValue(64);
      s = 64;
      msg->what = THUMB_SIZE;
      msg->AddInt32("IconSize",s);
      theApp->PostMessage(msg);
      break;
   case SET_TO_96:
      slide->SetValue(96);
      s = 96;
      msg->what = THUMB_SIZE;
      msg->AddInt32("IconSize",s);
      theApp->PostMessage(msg);
      break;
   default:
      BWindow::MessageReceived(msg);
   }
}

/*******************************************************
*   Someone asked us nicely to quit. I gess we should
*   so clean up. save our setings (position size of win)
*   and tell the main be_app to shut it all down .. bye
*******************************************************/
bool OptionsWindow::QuitRequested(){
   Hide();
   return false;
}
