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
#include <TranslationUtils.h>
#include <Path.h>
#include <String.h>
#include <Background.h>
#include <Node.h>
#include <FindDirectory.h>
#include <Directory.h>

#include <stdio.h>

#include "Globals.h"
#include "ImageShower.h"
//#include "BitmapDrawer.h"
//#include "Addons/ImageFilter.h"
#include "ImageShowWindow.h"
#include "YLanguageClass.h"
#include "BugOutDef.h"

extern BugOut db;

/*******************************************************
*   This is the actual Image displayer. We do all the 
*   drawing and image stuff here.  This is also where
*   all the Translation happens.
*******************************************************/
ImageShower::ImageShower(BRect frame, uint32 mode):
             BView(frame,"ImageShower",mode,B_WILL_DRAW|B_PULSE_NEEDED|B_FRAME_EVENTS){//
   db.SendMessage("Constructing ImageShower");
   SetViewColor(B_TRANSPARENT_32_BIT); // go tran so we have control over drawing
   img = BTranslationUtils::GetBitmap("Logo");
   undoimg = NULL;
   undoable = false;
   redraw = false;
   drawMode = OLD_ASPECT;
   flipID = rot90ID = mirrorID = rot_90ID = -1;
   find_directory(B_USER_DIRECTORY, &CurrentPath);// set to home dir
   TmpImgPath.SetTo("");
   //TranserThread = 0;
   Draggin=false;
   HSB = NULL;
   VSB = NULL;
//   filters = NULL;
   BGColor.red = 0; BGColor.green = 0; BGColor.blue = 0;
   
   TIDOK = false;
   
   db.SendMessage("Spawning List thread");
   resume_thread(spawn_thread(DoFilterMenu, "Filter ID List", B_NORMAL_PRIORITY, this));
   db.SendMessage("List thread spawned");
   
   TransLock = new BLocker("TranslatorLock");
   db.SendMessage("ImageShower constructed perfectly");
}

/*******************************************************
*   Destroy it all
*******************************************************/
ImageShower::~ImageShower(){
   delete img;
//   delete filters;
   delete TransLock;
}

/*******************************************************
*
*******************************************************/
int32 ImageShower::MakeMenu(){
  /* db.SendMessage("Makeing Addon list");
   app_info ai;
   be_app->GetAppInfo(&ai);
   BEntry entry(&ai.ref);
  
   BPath path;
   entry.GetPath(&path);
   path.GetParent(&path);
   path.Append("Addons");
   BDirectory directory(path.Path());
   
//   ImageFilter *temp = NULL;

   while (directory.GetNextEntry(&entry, true) == B_OK){
      entry.GetPath(&path);
      image_id image = load_add_on(path.Path());
      if (image < 0) continue;
      ImageFilter *(*instantiate_filter)();
      if (get_image_symbol(image, "instantiate_filter", B_SYMBOL_TYPE_TEXT,
         (void **)&instantiate_filter) != B_OK) {
//         printf("get_image_symbol failed for %s\n", path.Path());
         continue;
      }
      ImageFilter *filter = (*instantiate_filter)();
      if (filter != NULL) {
         if (filters == NULL){
            filters = filter;
         }else{
            temp = filters; 
            while (temp->next != NULL){
                temp = temp->next;
            }
            temp->next = filter;
         }
         // Grap a particular filter for later use
         if(!strcmp(filter->GetName(),"Flip Horizontal")){
            flipID = filter->GetId();
         }else if(!strcmp(filter->GetName(),"Flip Vertical")){
            mirrorID = filter->GetId();
         }else if(!strcmp(filter->GetName(),"Rotate CCW 90")){
            rot_90ID = filter->GetId();
         }else if(!strcmp(filter->GetName(),"Rotate CW 90")){
            rot90ID = filter->GetId();
         }
         //end funky grab  
      }
   }
   db.SendMessage("Addon list made");*/
   return B_OK;
}


/*******************************************************
*   We have been targeted
*******************************************************/
void ImageShower::AllAttached(){
   
}

/*******************************************************
*   We have been targeted
*******************************************************/
void ImageShower::TargetedByScrollView(BScrollView *sv){
   HSB = sv->ScrollBar(B_HORIZONTAL);
   VSB = sv->ScrollBar(B_VERTICAL);
   if(VSB){
      VSB->SetRange(0,0);
   }
   if(HSB){
      HSB->SetRange(0,0);
   }
   db.SendMessage("ImageShower Targeted By Scrollbars");
}

/*******************************************************
*   Whooo Draw. Gess what this does.
*******************************************************/
void ImageShower::Draw(BRect frame){
   int i,j;
   float sizeX=0,sizeY=0,scale=0;
   
   BRect f;
   
   SetHighColor(BGColor.red,BGColor.green,BGColor.blue,255);
   SetLowColor(BGColor.red,BGColor.green,BGColor.blue,255);

   Window()->Lock();
  
   if(img != NULL){ 
      BRect centerdImag;
      BRect imgB = img->Bounds();
      BRect tmpB = Bounds();
      
      //SetLowColor(0,0,0,255);
      if(drawMode != ACTUAL_SIZE){
         if(VSB){ VSB->SetRange(0,0); }
         if(HSB){ HSB->SetRange(0,0); } 
      }    
      //SetDrawingMode(B_OP_BLEND); 
      switch(drawMode){
         case WINDOW_FIT:  // Resize window around image
            //NO BREAK just fall thought;
         case ASPECT:  // fit in window best - keep aspect ratio though
            if(imgB.right == 0){
               imgB.right = 1;
            }
            if(imgB.bottom == 0){
               imgB.bottom = 1;
            }
            sizeX = (tmpB.right/imgB.right);
            sizeY = (tmpB.bottom/imgB.bottom);

            if(sizeX > sizeY){
                imgB.bottom *= sizeY;
                imgB.right *= sizeY;
            }else{
               imgB.bottom *= sizeX;
               imgB.right *= sizeX;
            }
            centerdImag.left = (tmpB.Width()/2) - (imgB.Width()/2);
            centerdImag.right=centerdImag.left + imgB.Width();
            centerdImag.top = (tmpB.Height()/2) - (imgB.Height()/2);
            centerdImag.bottom=centerdImag.top + imgB.Height();
            
            DrawBitmapAsync(img, img->Bounds(), centerdImag); 
            
            // This is our new way of doing things with our kewl
            // resize plugins and stuff
            f = centerdImag;
            f.OffsetTo(0,0);
            //DrawBitmapAsync(resizeImg(img,f),centerdImag);
            
            //fill all around the bitmap
            if(!IsPrinting()){
               if(sizeY > sizeX){
                  FillRect(BRect(tmpB.left,tmpB.top,tmpB.right,centerdImag.top-1),B_SOLID_LOW);// Top
                  FillRect(BRect(tmpB.left,centerdImag.bottom+1,tmpB.right,tmpB.bottom),B_SOLID_LOW);// Bottom
               }else{
                  FillRect(BRect(tmpB.left,centerdImag.top-1,centerdImag.left-1,centerdImag.bottom),B_SOLID_LOW);// Left
                  FillRect(BRect(centerdImag.right+1,centerdImag.top-1,tmpB.right,centerdImag.bottom),B_SOLID_LOW);// right
               }
            }
            Sync();
            break;
         case OLD_ASPECT:
            if(imgB.Height() > tmpB.Height()){
               scale = imgB.Height() / tmpB.Height();
               imgB.right = imgB.right / scale;
               imgB.bottom = imgB.bottom / scale;               
            }
            if(imgB.Width() > tmpB.Width()){
               scale = imgB.Width() / tmpB.Width();
               imgB.right = imgB.right / scale;
               imgB.bottom = imgB.bottom / scale;               
            }            
            centerdImag.left = (tmpB.Width()/2) - (imgB.Width()/2);
            centerdImag.right=centerdImag.left + imgB.Width();
            centerdImag.top = (tmpB.Height()/2) - (imgB.Height()/2);
            centerdImag.bottom=centerdImag.top + imgB.Height();
            
            DrawBitmapAsync(img, img->Bounds(), centerdImag); 
            //fill all around the bitmap
            if(!IsPrinting()){
               FillRect(BRect(tmpB.left,tmpB.top,tmpB.right,centerdImag.top-1),B_SOLID_LOW);// Top
               FillRect(BRect(tmpB.left,centerdImag.bottom+1,tmpB.right,tmpB.bottom),B_SOLID_LOW);// Bottom
               FillRect(BRect(tmpB.left,centerdImag.top-1,centerdImag.left-1,centerdImag.bottom),B_SOLID_LOW);// Left
               FillRect(BRect(centerdImag.right+1,centerdImag.top-1,tmpB.right,centerdImag.bottom),B_SOLID_LOW);// right
            }
            Sync();
            break;
         case ACTUAL_SIZE: // draw center in real size
            centerdImag.left = (tmpB.Width()/2) - (imgB.Width()/2);
            centerdImag.right=centerdImag.left + imgB.Width();
            centerdImag.top = (tmpB.Height()/2) - (imgB.Height()/2);
            centerdImag.bottom=centerdImag.top + imgB.Height();
             
            DrawBitmapAsync(img, img->Bounds(), centerdImag); 
            if(!IsPrinting()){
               FillRect(BRect(tmpB.left,tmpB.top,tmpB.right,centerdImag.top-1),B_SOLID_LOW);// Top
               FillRect(BRect(tmpB.left,centerdImag.bottom+1,tmpB.right,tmpB.bottom),B_SOLID_LOW);// Bottom
               FillRect(BRect(tmpB.left,centerdImag.top-1,centerdImag.left-1,centerdImag.bottom),B_SOLID_LOW);// Left
               FillRect(BRect(centerdImag.right+1,centerdImag.top-1,tmpB.right,centerdImag.bottom),B_SOLID_LOW);// right
            }
/*            
            DrawBitmapAsync(img, img->Bounds(), BRect(Offset.x+centerdImag.left,
                                                      Offset.y+centerdImag.top,
                                                      Offset.x+centerdImag.right,
                                                      Offset.y+centerdImag.bottom));
            //fill all around the bitmap
            FillRect(BRect(tmpB.left,tmpB.top,tmpB.right,Offset.y+centerdImag.top-1),B_SOLID_LOW);// Top
            FillRect(BRect(tmpB.left,Offset.y+centerdImag.bottom+1,tmpB.right,tmpB.bottom),B_SOLID_LOW);// Bottom
            FillRect(BRect(tmpB.left,Offset.y+centerdImag.top-1,Offset.x+centerdImag.left-1,Offset.y+centerdImag.bottom),B_SOLID_LOW);// Left
            FillRect(BRect(Offset.x+centerdImag.right+1,Offset.y+centerdImag.top-1,tmpB.right,Offset.y+centerdImag.bottom),B_SOLID_LOW);// right
 */            
            Sync();
            break;
         case STRETCH:  //Streach img to fit window .. funky look
            DrawBitmapAsync(img, img->Bounds(),Bounds()); //Streach
            break;
         case TILE:
            for(j = 0;j < Bounds().bottom;j+=(int)img->Bounds().Height()+1){
               for(i = 0;i < Bounds().right;i+=(int)img->Bounds().Width()+1){
                  DrawBitmapAsync(img, img->Bounds(), BRect(i,j,img->Bounds().right+i,img->Bounds().bottom+j));   
               }
            }
            Sync();
            break;
         case FULLSCREEN:  // full screen (does not go here should | with above)
            redraw = true;
            drawMode = OlddrawMode;
            break;
      }
   }else{ // Image is NULL ..hmm
      FillRect(frame,B_SOLID_LOW);// Top
      SetHighColor(255,255,255);
      MovePenTo(Bounds().left+5, Bounds().bottom-2);
      DrawString(Language.get("NULL_IMAGE"));
   }
   
   //FillRect(BRect(BoxStart,BoxStop));
   
   Window()->Unlock();
}

/*******************************************************
*   If we are being resized. Fix the stuff we need to fix
*******************************************************/
void ImageShower::FrameResized(float,float){
   //redraw = true;
   
   if(drawMode == ACTUAL_SIZE){
      FixUPScrollBars();
   }
   Invalidate();
}

/*******************************************************
*  Pulse keeps us updated so we can refresh the window 
*  real fast and stuff .. beter responce.
*******************************************************/
void ImageShower::Pulse(){
  if(redraw){
      Invalidate(); // dont call Draw() it will look bad!
      redraw=false;
   }
}

/*******************************************************
*   A niffty method for letting the world have our Img
*******************************************************/
BBitmap* ImageShower::GetBitmap(){
   return img;
}

/*******************************************************
*   Makes that nice right click menu that we all love
*   this should consist of the Major things the user 
*   wants to do .. quick and easy.
*******************************************************/
void ImageShower::MakeMenu(BMenu* menu){
   BMenuItem* item;
   BMenu *submenu;

   submenu = new BMenu(Language.get("EDIT"));
   submenu->AddItem(item = new BMenuItem(Language.get("UNDO"),new BMessage(UNDO),'U'));
   if(!undoable){
      item->SetEnabled(false);
   }
   submenu->AddItem(new BSeparatorItem());
   submenu->AddItem(new BMenuItem(Language.get("CUT"),new BMessage(B_CUT)));
   submenu->AddItem(new BMenuItem(Language.get("COPY"),new BMessage(B_COPY)));
   submenu->AddItem(new BMenuItem(Language.get("PASTE"),new BMessage(B_PASTE)));
   submenu->AddItem(new BSeparatorItem());
   submenu->AddItem(new BMenuItem(Language.get("SELECT_ALL"),new BMessage(B_SELECT_ALL)));
   menu->AddItem(submenu);
 
   submenu = new BMenu(Language.get("MODE"));
   submenu->AddItem(item = new BMenuItem(Language.get("ACTUAL_SIZE"),new BMessage(ACTUAL_SIZE),'1'));
   if(drawMode == ACTUAL_SIZE){ item->SetMarked(true); }
   submenu->AddItem(item = new BMenuItem(Language.get("NEW_ASPECT"),new BMessage(ASPECT),'2'));
   if(drawMode == ASPECT){ item->SetMarked(true); }
   submenu->AddItem(item = new BMenuItem(Language.get("ASPECT"),new BMessage(OLD_ASPECT),'3'));
   if(drawMode == OLD_ASPECT){ item->SetMarked(true); }
   submenu->AddItem(item = new BMenuItem(Language.get("STRETCH"),new BMessage(STRETCH),'4'));
   if(drawMode == STRETCH){ item->SetMarked(true); }
   submenu->AddItem(item = new BMenuItem(Language.get("RESIZE_TO_FIT"),new BMessage(WINDOW_FIT),'5'));
   if(drawMode == WINDOW_FIT){ item->SetMarked(true); }
   submenu->AddItem(item = new BMenuItem(Language.get("TILE"),new BMessage(TILE),'6'));
   if(drawMode == TILE){ item->SetMarked(true); }
   menu->AddItem(submenu);
 
   submenu = new BMenu(Language.get("SET_AS_BG"));
   submenu->AddItem(new BMenuItem(Language.get("CENTERED"),new BMessage(SET_BG_CENTER)));
   submenu->AddItem(new BMenuItem(Language.get("SCALED"),new BMessage(SET_BG_SCALED)));
   submenu->AddItem(new BMenuItem(Language.get("TILED"),new BMessage(SET_BG_TILED)));
   menu->AddItem(submenu);
   
   menu->AddItem(new BMenuItem(Language.get("TOGGLE_FULL_SCREEN"),new BMessage(FULLSCREEN),'F'));
   menu->AddItem(new BSeparatorItem());
   menu->AddItem(new BMenuItem(Language.get("GET_INFO"),new BMessage(GET_IMG_INFO)));
   menu->AddItem(new BMenuItem("Nuke a small country",new BMessage(NO_FUNCTION_YET)));

}

/*******************************************************
*   All kinds of good stuff in here. If we click on 
*   the image panel we want to be able to move the
*   image around or if we right click we want a popup
*   menu. Also if we double click we can do stuff,
*******************************************************/
void ImageShower::MouseDown(BPoint where){
	BMessage *currentMsg = Window()->CurrentMessage();
	if (currentMsg->what == B_MOUSE_DOWN) {
		uint32 buttons = 0;
		currentMsg->FindInt32("buttons", (int32 *)&buttons);

		uint32 modifiers = 0;
		currentMsg->FindInt32("modifiers", (int32 *)&modifiers);

		if (buttons & B_SECONDARY_MOUSE_BUTTON) {
			BPopUpMenu *menu = new BPopUpMenu("Image Menu");
			MakeMenu(menu);
			BMenuItem *selected = menu->Go(ConvertToScreen(where));
			if (selected){
				//(selected->Target())->PostMessage(selected->Message()->what);
				if(selected->Message()){
				   Window()->PostMessage(selected->Message()->what);
				}
			}
			delete menu;
		}
		if((buttons & B_PRIMARY_MOUSE_BUTTON) && (modifiers & B_SHIFT_KEY)){
		   here = where;
		   DrawingBox = true;
		   BoxStart = where;
		   BoxStop = BoxStart;
		   //EndRectTracking();
		   //BeginRectTracking(BRect(where.x,where.y,where.x,where.y),B_TRACK_RECT_CORNER);
		}else if((buttons & B_PRIMARY_MOUSE_BUTTON) && !Draggin){
		   Draggin= true; // this is so we can drag
		   here = where;
		   SetMouseEventMask(B_POINTER_EVENTS,B_LOCK_WINDOW_FOCUS);
		}
		
	}
}

/*******************************************************
*   If we unclick then stop dragging or whatever it is 
*   we are doing
*******************************************************/
void ImageShower::MouseUp(BPoint where){
   Draggin = false; // stop following mouse
   if(DrawingBox){
      DrawingBox = false;
      BoxStop = where;
   }
   //EndRectTracking();
 //  BeginRectTracking(BRect(here.x,here.y,where.x,where.y));
}

/*******************************************************
*   Main thing here is the draggin of the image around
*   We track the mouse as we move it so the images moves
*******************************************************/
void ImageShower::MouseMoved(BPoint where,uint32 info,const BMessage *m){
   //if(info == B_EXITED_VIEW){ Draggin = false;}
   if(Draggin){
      // so we want to drag the image around
      if(drawMode == ACTUAL_SIZE){
         //Offset.x += where.x - here.x;
         //Offset.y += where.y - here.y;
         //HSB->SetValue(HSB->Value()+((where.x - here.x)/2));
         //VSB->SetValue(VSB->Value()+((where.y - here.y)/2));

         HSB->SetValue(HSB->Value()+((here.x - where.x)/2));
         VSB->SetValue(VSB->Value()+((here.y - where.y)/2));
        
      //  printf("mouse is at %f,%f and the images is at %f,%f\n",where.x,where.y,here.x,here.y);
       
        
         here = where;
         //Invalidate();
         
      }     
   }else if(DrawingBox){
      BoxStop =where;
      
   }
}


/*******************************************************
*   Here we set the image as the Desktop background
*   we use one of the default modes that Be gives us.
*   that is what is passed in as a pram
*******************************************************/
void ImageShower::SetImgAsBG(uint32 Mode){
   BMessage msg;
   BMessage oldmsg;
   int junk = 0;
   char tmpaddr[5000]; // This is a bad limit we put on ourselfs.
   ssize_t size = 5000;
   int32 workspace;
   const char *tmpString;
   int32 tmpInt32;
   BPoint tmpBPoint;
   bool tmpbool;
   int32 index = 0;
   
   msg.MakeEmpty();
   
   BPath p;
   find_directory(B_DESKTOP_DIRECTORY, &p);
   BNode n(p.Path());
   size = n.ReadAttr(B_BACKGROUND_INFO,B_RAW_TYPE,junk,tmpaddr,size);
   oldmsg.Unflatten(tmpaddr);
   
   /*
   //debug to make shure we attr is right
   if(n.ReadAttr(B_BACKGROUND_INFO,B_RAW_TYPE,junk,addr,size) == size){
         printf("%s",addr);
    }
   */
   
   while(oldmsg.FindInt32(B_BACKGROUND_WORKSPACES,index,&workspace) == B_OK){
      if((uint32)workspace != Window()->Workspaces()){
         oldmsg.FindString(B_BACKGROUND_IMAGE,index,&tmpString);
         msg.AddString(B_BACKGROUND_IMAGE,tmpString);
         
         oldmsg.FindInt32(B_BACKGROUND_MODE,index,&tmpInt32);
         msg.AddInt32(B_BACKGROUND_MODE,tmpInt32);
         
         oldmsg.FindPoint(B_BACKGROUND_ORIGIN,index,&tmpBPoint );
         msg.AddPoint(B_BACKGROUND_ORIGIN, tmpBPoint);
         
         oldmsg.FindBool(B_BACKGROUND_ERASE_TEXT,index,&tmpbool);      
         msg.AddBool(B_BACKGROUND_ERASE_TEXT,tmpbool);      
         
         oldmsg.FindInt32( B_BACKGROUND_WORKSPACES,index, &tmpInt32);
         msg.AddInt32( B_BACKGROUND_WORKSPACES, tmpInt32);
      }else {
         //No old background info .. thats Ok i gess :)
      }
      index++;
   }

   msg.AddString(B_BACKGROUND_IMAGE,CurrentPath.Path());
   switch (Mode){
   case SET_BG_CENTER:
      msg.AddInt32(B_BACKGROUND_MODE,B_BACKGROUND_MODE_CENTERED);
      break;
   case SET_BG_SCALED:
      msg.AddInt32(B_BACKGROUND_MODE,B_BACKGROUND_MODE_SCALED);
      break;
   case SET_BG_TILED: 
      msg.AddInt32(B_BACKGROUND_MODE,B_BACKGROUND_MODE_TILED);
      break;
   }
   msg.AddPoint(B_BACKGROUND_ORIGIN, BPoint(0.0,0.0 ));
   msg.AddBool(B_BACKGROUND_ERASE_TEXT,true);      
   msg.AddInt32( B_BACKGROUND_WORKSPACES, Window()->Workspaces());
   
   size = msg.FlattenedSize(); 
   char* addr = new char[size]; 
   if (msg.Flatten(addr, size) == B_OK) { 
      n.WriteAttr(B_BACKGROUND_INFO, B_RAW_TYPE, 0, addr, size); 
   } 
   delete [] addr; 

   // Smack the Tracker into updateing its background :)
   BMessage reply;
   BMessenger Tracker("application/x-vnd.Be-TRAK");
   reply.MakeEmpty();  
   Tracker.SendMessage(new BMessage(B_RESTORE_BACKGROUND_IMAGE));
   
   
}

/*******************************************************
*   Let the world now what we think the current path is
*******************************************************/
BPath ImageShower::GetPath(){
   return CurrentPath;
}

/*******************************************************
*   And .. here its message received .. thank you thank
*   you - no autographs please .. 
*******************************************************/
void ImageShower::MessageReceived(BMessage *msg){
   entry_ref ref;
   BPath path;
   BRect rec;
   BBitmap *new_img = NULL;
   BMessage *new_msg = NULL;
   BMessage SizeMsg(IMAGE_SIZE);
//   ImageFilter *temp = NULL;
   rgb_color *c = NULL;
   ssize_t s;
   if(img != NULL){ rec = img->Bounds(); }
   BRect us = Bounds();
//   uint32 id;
//   int32 count = 0;
   
   float x = (rec.Width()-us.Width());
   float y = (rec.Height()-us.Height());
   
   switch(msg->what){
   case SHOWER_COLOR:
      msg->FindData("RGBColor",B_RGB_COLOR_TYPE,(const void**)&c,&s);
      BGColor = *c;
      Invalidate();
      ((ImageShowWindow*)Window())->ShowerColor = *c;
      break;
   case B_PASTE:
      if(msg->FindData("RGBColor",B_RGB_COLOR_TYPE,(const void**)&c,&s) == B_OK){
         BGColor = *c;
         Invalidate();
         ((ImageShowWindow*)Window())->ShowerColor = *c;
      }else{
         (new BAlert(NULL,"paste",""))->Go();
      }
      break;
   case B_CUT:
       (new BAlert(NULL,"cut",""))->Go();
      break;
   case B_COPY:
      (new BAlert(NULL,"copy",""))->Go();
      break;
   case B_SELECT_ALL:
      (new BAlert(NULL,"select all",""))->Go();
      break;
   case B_SIMPLE_DATA: // DRAG-N-DROP
      // Look for a ref in the message
      if( msg->FindRef("refs", &ref) == B_OK ){
         BEntry entry(&ref, true);
         if(entry.IsFile() && entry.GetPath(&path)==B_OK){
            // Ok we have a file and all. lets handle it
            new_img = BTranslationUtils::GetBitmapFile(path.Path());
            
            if(new_img == NULL){
               //Thats not valid image .. why did you dorp it ?
            }else{
               CurrentPath.SetTo(path.Path());
               delete img;
               img = BTranslationUtils::GetBitmapFile(path.Path());
               new_msg = new BMessage(IMAGE_SIZE);
               rec = img->Bounds();
               if(new_msg->AddRect("imgsize",rec) != B_OK){ /*some error*/}else{
                  Parent()->MessageReceived(new_msg);
               }
               new_msg = new BMessage(REFS_CHANGE_DIR);
               path.GetParent(&path);
               if(new_msg->AddFlat("imgpath",&path) != B_OK){ /*some error*/}else{
                  Parent()->MessageReceived(new_msg);
               }
               FixUPScrollBars();
               if(drawMode == WINDOW_FIT){ MessageReceived(new BMessage(WINDOW_FIT));}
               redraw = true;
            }
         }else{
            // this must be a directory .. tel Selector about this
            BView::MessageReceived(msg);
         }         
      }else{
         // Call inherited if we didn't handle the message
         BView::MessageReceived(msg);
      }
      break;
   case CHANGE_IMAGE: // WE WERE TOLD TO CHANGE IMAGES
   
     /// new_msg = Looper()->CurrentMessage();
     // while(Looper()->MessageQueue()->FindMessage(CHANGE_IMAGE,pos) != NULL){
     //    delete new_msg;
     //    new_msg = Looper()->DetachCurrentMessage();
     // }
      //if(new_msg->what == CHANGE_IMAGE){
      //   new_msg = DetachCurrentMessage();
      //   delete new_msg;
     // }
      
      //Find the path to the image. Use a global BPath
      //  So that the thread can get to it and load the
      //  image up. After we find the path - kill andy
      //  current translators and start transing the new
      //  image.
      if(msg->FindFlat("imgpath",&TmpImgPath) != B_OK){
          // printf("No imgPath?\n");
           break;
      }
      
      Translator(); // instead of threaded Just call it
      
  /*
     //We do this or do the threading .. hmmm
      new_img = BTranslationUtils::GetBitmapFile(TmpImgPath.Path());//tmpString

      if(new_img == NULL){
         //Thats not valid image .. oh well
      }else{
         delete img;
         CurrentPath.SetTo(TmpImgPath.Path());//tmpString
         img = new BBitmap(new_img);
         rec = img->Bounds();
         SizeMsg.MakeEmpty();
         if(SizeMsg.AddRect("imgsize",rec) == B_OK){ 
            //Parent()->Looper()->PostMessage(&SizeMsg);
            Window()->PostMessage(&SizeMsg);
         }
         FixUPScrollBars();
         if(drawMode == WINDOW_FIT){ MessageReceived(new BMessage(WINDOW_FIT));}
         redraw = true;
      }
      TmpImgPath.Unset(); // Just for good messure
     // Window()->PostMessage(new BMessage(FINISHED));
      */
//    /*  
      //if(TIDOK){
        // wait_for_thread(TranserThread,NULL);
      //kill_thread(TranserThread);
      //}
      
    
     
//      if(TransLock->CountLocks() > 0){ 
//         kill_thread(TransIt);
//         TransLock->Unlock();
//      }
//      TranserThread = spawn_thread(TransIt, "Translate da filz", B_NORMAL_PRIORITY, this);
///	  resume_thread(TranserThread);
	     //wait_for_thread(TranserThread,&stat);

//      */
      break;
   case WINDOW_FIT:// set up the drawing mode
      new_msg = new BMessage(RESIZE_TO_IMAGE);
      new_msg->AddFloat("offx",x);
      new_msg->AddFloat("offy",y);   
      //Parent()->MessageReceived(new_msg); // Pass this one up to Window
      Parent()->Looper()->PostMessage(new_msg);
      drawMode = msg->what;
      redraw = true;
      break;
   case FULLSCREEN:
      new_msg = new BMessage(MAX_SCREEN);
      //Parent()->MessageReceived(new_msg); // Pass this one up to Window
      Parent()->Looper()->PostMessage(new_msg);
      OlddrawMode = drawMode;
  //?    drawMode = msg->what;
      break;
   case ACTUAL_SIZE:
      //fix up scrollbars here;
      FixUPScrollBars();
   case ASPECT:
   case OLD_ASPECT:
   case STRETCH:
   case TILE:
      drawMode = msg->what;
      redraw=true;
      break;
   case SET_BG_CENTER:
   case SET_BG_SCALED:
   case SET_BG_TILED:  
      SetImgAsBG(msg->what);
      break; 
   case UNDO:
      if(undoimg == NULL){ break; }
      delete img;
      img = new BBitmap(undoimg);
      Invalidate();
      delete undoimg;
      undoimg = NULL;
      Window()->PostMessage(new BMessage(UNDO_FALSE));
      undoable = false;
      break;
   case RUN_FILTER:
      // if (msg->FindInt32("filter_id", (int32 *)&id) != B_OK) return;
/*      while(msg->FindInt32("filter_id", count++,(int32 *)&id) == B_OK){
         temp = filters;
         while(true){
            if (temp == NULL) return;
            if (temp->GetId() == id) break;
            temp = temp->next;
         }
         if(temp->GetVer() == 1){
            new_img = temp->Run(img);
            if (new_img == NULL) {
               //BAlert *alert = new BAlert(NULL, Language.get("FILTER_ERROR"),Language.get("OK"));
               //alert->Go();
               (new BAlert(NULL, Language.get("FILTER_ERROR"),Language.get("OK")))->Go();
               break;
            }
            // We are about to delete the old image and create
            // a new image in its place. You know this would be 
            // A fine place to add in undo support
        
            if(undoimg != NULL){ delete undoimg; undoimg = NULL; }
            undoimg = new BBitmap(img);
            // Set the undo menu true
            undoable = true;
            Window()->PostMessage(new BMessage(UNDO_TRUE));
         
            delete img;
            img = new BBitmap(new_img);
         }
      }*/
      Invalidate();
      break;
   case SHOW_LOGO:
      // Just a little niffty show logo thingy :)
      new_img = BTranslationUtils::GetBitmap("Logo");
      if(new_img){
         delete img;
         img = new BBitmap(new_img);    
         redraw = true;  
      }
      break;
   default:
      BView::MessageReceived(msg);
      break;
   }
   if(new_img != NULL){ delete new_img; }
   if(new_msg != NULL){ delete new_msg; }
}

/*******************************************************
*   Fix scrollbars so there range and positions are 
*   corect for the current image and its location.
*******************************************************/
void ImageShower::FixUPScrollBars(){
   float sizeX,sizeY;
   if(VSB){
      if((sizeY = img->Bounds().Height() - Bounds().Height()) > 0){
         VSB->SetRange(-(sizeY/2),(sizeY/2));
      }else{
         VSB->SetRange(0,0);
      }
   }
   if(HSB){
      if((sizeX = img->Bounds().Width() - Bounds().Width()) > 0){
         HSB->SetRange(-(sizeX/2),(sizeX/2));
      }else{
         HSB->SetRange(0,0);
      }
   } 
}

/*******************************************************
*   This is a nice little func that will translate our
*   image. It uses a gloable BPath (TmpImgPath) and 
*   write to a gloable BBitmap (Img).  We do this so 
*   that we can make it a seperate thread and kill it 
*   when we don't want to trans anymore.
*******************************************************/
int32 ImageShower::Translator(){
   TransLock->Lock();
   db.SendMessage("Translating Image");
   Window()->PostMessage(new BMessage(UNDO_FALSE));
   if(undoimg != NULL){ delete undoimg; undoimg = NULL; }
   Window()->PostMessage(new BMessage(WORKING));
   BBitmap *new_img;
   //printf("Entering Transer\n");
   // We need to kill the old translator thread and spin off 
   //  a new one here..
   new_img = BTranslationUtils::GetBitmapFile(TmpImgPath.Path());//tmpString

   if(new_img == NULL){
      //Thats not valid image .. oh well
      // Post a NULL Image message
      Window()->PostMessage(new BMessage(NULL_IMAGE));
     //    why are we posting null image message here ????
      
   }else{
      Looper()->Lock();
      CurrentPath.SetTo(TmpImgPath.Path());//tmpString
      delete img;
      img = new BBitmap(new_img);
      Looper()->Unlock();
      BMessage SizeMsg(IMAGE_SIZE);
      SizeMsg.MakeEmpty();
      if(SizeMsg.AddRect("imgsize",img->Bounds()) == B_OK){ 
         //Parent()->Looper()->PostMessage(&SizeMsg);
         //Window()->Lock();
         Window()->PostMessage(&SizeMsg);
         //Widnow()->Unlock();
      }
      Looper()->Lock();
      FixUPScrollBars();
      if(drawMode == WINDOW_FIT){ MessageReceived(new BMessage(WINDOW_FIT));}
   
      //start of big hack
      int32 mode = 27;
      BMessage dofilter(RUN_FILTER);
      switch(mode){
      case 27:
         break;
      case -28:
         dofilter.AddInt32("filter_id",flipID);
         break;
      case 78:
         dofilter.AddInt32("filter_id",mirrorID);
         break;
      case -68:
         dofilter.AddInt32("filter_id",flipID);
         dofilter.AddInt32("filter_id",mirrorID);
         break;
      case -58:
         dofilter.AddInt32("filter_id",rot90ID);
         break;
      case 108:
         dofilter.AddInt32("filter_id",rot_90ID);
         break;
      case -79:
         dofilter.AddInt32("filter_id",rot90ID);
         dofilter.AddInt32("filter_id",rot90ID);
         break;
      case -109:
         dofilter.AddInt32("filter_id",mirrorID);
         dofilter.AddInt32("filter_id",rot90ID);
         break;
      }
      Window()->PostMessage(&dofilter);
      // end of our big hack
      
      Looper()->Unlock();
      redraw = true;
   }
   TmpImgPath.Unset(); // Just for good messure
   delete new_img;
   //printf("Exiting Transer\n");
   Window()->PostMessage(new BMessage(FINISHED));
   db.SendMessage("Image Translated Just fine");
   TransLock->Unlock();
   
   
   
   return 0;
}

/*******************************************************
*   Resizes the image so its nice and small for thumbs
*******************************************************/
BBitmap* ImageShower::resizeImg(BBitmap* Img,BRect r){
   BBitmap *new_img = new BBitmap(r,B_RGB32,true);
   if(!new_img->IsValid()){
      //This could potentaly cause a problem as we are acutaly inside of a 
      // thread. If we post 2 alerts the app will most likely lock
      (new BAlert(NULL,Language.get("RESIZE_ERROR"),Language.get("OK")))->Go();
      return Img;
   }
   BView  *blah = new BView(r,"drawer",B_FOLLOW_NONE,B_WILL_DRAW);
   
   new_img->AddChild(blah);
   new_img->Lock();
   
   blah->DrawBitmap(Img,Img->Bounds(),blah->Bounds());
   blah->SetHighColor(255,0,0);
  // blah->FillRect(blah->Bounds(),B_SOLID_HIGH);
   blah->DrawString("Resize Exec !",BPoint(100,100));
   
   
   
   blah->Sync();
   new_img->RemoveChild(blah);
   new_img->Unlock();
   
   delete blah;
  // delete Img;
  // Img = new BBitmap(new_img);
   //delete new_img;
   return new_img;
}




