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
#include "SlideShowWin.h"
#include "YLanguageClass.h"
#include "BugOutDef.h"

extern BugOut db;

/*******************************************************
*   Our wonderful BWindow, ya its kewl like that.
*   we dont do much here but set up the menubar and 
*   let the view take over.  We also nead some message
*   redirection and handling
*******************************************************/
SlideShowWin::SlideShowWin(BWindow *win):BWindow(BRect(350,50,550,150),Language.get("SLIDESHOWWIN"),B_TITLED_WINDOW_LOOK,B_FLOATING_APP_WINDOW_FEEL,B_ASYNCHRONOUS_CONTROLS|B_NOT_RESIZABLE|B_NOT_ZOOMABLE){
   rgb_color LightBlue = { 155,155,250 };
   rgb_color Default = { 51,102,152,255 };
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

   slide = new BSlider(r,"","",new BMessage(SLIDE_SPEED),2,120);
   slide->SetModificationMessage(new BMessage(SLIDE_SPEED));
   Bb->AddChild(slide);
   slide->SetLimitLabels(Language.get("SLIDE_SPEED"), "2");
   slide->SetValue(2);
   slide->SetHashMarks(B_HASH_MARKS_TOP);
   slide->SetHashMarkCount(5);
   slide->SetKeyIncrementValue(1);
   slide->SetBarColor(Default);
   slide->UseFillColor(true, &LightBlue);

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
void SlideShowWin::MessageReceived(BMessage* msg){
   BString value;
   int32 v;
   int32 s;
   switch(msg->what){
   case CHANGE_LANGUAGE:
      break;
   case SLIDE_SPEED:
      value.SetTo("");
      v = slide->Value();
      msg->AddInt32("ShowSlideSpeed",v);
      if(int(v / 60) > 0){
         s = v % 60;
         v /= 60;
         value << (int32)v;
         value.Append("min ");
      }else{
         s = v;
         v = 0;
      }
      if((s != 0)){
         value << (int32)s;
         value.Append("sec");
      }
      if((s == 0) && (v == 0)){
         value.SetTo("Damn fast");
      }
      slide->SetLimitLabels(Language.get("SLIDE_SPEED"),value.String());
      
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
bool SlideShowWin::QuitRequested(){
   Hide();
   return false;
}
