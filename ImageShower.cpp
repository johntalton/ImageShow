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

//#include <stdio.h>

#include "Globals.h"
#include "ImageShower.h"
#include "BitmapDrawer.h"
#include "Addons/ImageFilter.h"
#include "ImageShowWindow.h"

/*******************************************************
*   This is the actual Image displayer. We do all the 
*   drawing and image stuff here.  This is also where
*   all the Translation happens.
*******************************************************/
ImageShower::ImageShower(BRect frame, uint32 mode):
             BView(frame,"ImageShower",mode,B_WILL_DRAW|B_PULSE_NEEDED|B_FRAME_EVENTS){//
   SetViewColor(B_TRANSPARENT_32_BIT); // go tran so we have control over drawing
   img = BTranslationUtils::GetBitmap("Logo");
   redraw=false;
   drawMode = OLD_ASPECT;
   find_directory(B_USER_DIRECTORY, &CurrentPath);// set to home dir
   TmpImgPath.SetTo("");
   //TranserThread = 0;
   Draggin=false;
   HSB = NULL;
   VSB = NULL;
   filters = NULL;
   BGColor.red = 0; BGColor.green = 0; BGColor.blue = 0;
   

   app_info ai;
   be_app->GetAppInfo(&ai);
   BEntry entry(&ai.ref);
  
   BPath path;
   entry.GetPath(&path);
   path.GetParent(&path);
   path.Append("Addons");
   BDirectory directory(path.Path());
   
   ImageFilter *temp = NULL;

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
      }
   }

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
}

/*******************************************************
*   Whooo Draw. Gess what this does.
*******************************************************/
void ImageShower::Draw(BRect frame){
   int i,j;
   float sizeX=0,sizeY=0,scale=0;

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
            //fill all around the bitmap
            if(sizeY > sizeX){
               FillRect(BRect(tmpB.left,tmpB.top,tmpB.right,centerdImag.top-1),B_SOLID_LOW);// Top
               FillRect(BRect(tmpB.left,centerdImag.bottom+1,tmpB.right,tmpB.bottom),B_SOLID_LOW);// Bottom
            }else{
               FillRect(BRect(tmpB.left,centerdImag.top-1,centerdImag.left-1,centerdImag.bottom),B_SOLID_LOW);// Left
               FillRect(BRect(centerdImag.right+1,centerdImag.top-1,tmpB.right,centerdImag.bottom),B_SOLID_LOW);// right
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
            FillRect(BRect(tmpB.left,tmpB.top,tmpB.right,centerdImag.top-1),B_SOLID_LOW);// Top
            FillRect(BRect(tmpB.left,centerdImag.bottom+1,tmpB.right,tmpB.bottom),B_SOLID_LOW);// Bottom
            FillRect(BRect(tmpB.left,centerdImag.top-1,centerdImag.left-1,centerdImag.bottom),B_SOLID_LOW);// Left
            FillRect(BRect(centerdImag.right+1,centerdImag.top-1,tmpB.right,centerdImag.bottom),B_SOLID_LOW);// right
            Sync();
            break;
         case ACTUAL_SIZE: // draw center in real size
            centerdImag.left = (tmpB.Width()/2) - (imgB.Width()/2);
            centerdImag.right=centerdImag.left + imgB.Width();
            centerdImag.top = (tmpB.Height()/2) - (imgB.Height()/2);
            centerdImag.bottom=centerdImag.top + imgB.Height();
             
            DrawBitmapAsync(img, img->Bounds(), centerdImag); 
            FillRect(BRect(tmpB.left,tmpB.top,tmpB.right,centerdImag.top-1),B_SOLID_LOW);// Top
            FillRect(BRect(tmpB.left,centerdImag.bottom+1,tmpB.right,tmpB.bottom),B_SOLID_LOW);// Bottom
            FillRect(BRect(tmpB.left,centerdImag.top-1,centerdImag.left-1,centerdImag.bottom),B_SOLID_LOW);// Left
            FillRect(BRect(centerdImag.right+1,centerdImag.top-1,tmpB.right,centerdImag.bottom),B_SOLID_LOW);// right
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
      MovePenTo(frame.left+5, frame.bottom-2);
      DrawString("The Image Was NULL");
   }
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
  // BMenu *subsubmenu;
	
/*   menuItem = new BMenuItem("About ", new BMessage(B_ABOUT_REQUESTED));
   menu->AddItem(menuItem);
   menuItem = new BMenuItem("Help...", new BMessage(msg_help));
   menu->AddItem(menuItem);
   menu->AddSeparatorItem();
*/

   menu->AddItem(new BMenuItem("Image Menu",NULL));

   menu->AddItem(new BSeparatorItem());
   
   submenu = new BMenu("Edit");
   submenu->AddItem(new BMenuItem("Cut",new BMessage(NO_FUNCTION_YET)));
   submenu->AddItem(new BMenuItem("Copy",new BMessage(NO_FUNCTION_YET)));
   submenu->AddItem(new BSeparatorItem());
   submenu->AddItem(new BMenuItem("Select All",new BMessage(NO_FUNCTION_YET)));
   menu->AddItem(submenu);
 
   submenu = new BMenu("Mode");
   submenu->AddItem(item = new BMenuItem("Actual Size",new BMessage(ACTUAL_SIZE),'C'));
   if(drawMode == ACTUAL_SIZE){ item->SetMarked(true); }
   submenu->AddItem(item = new BMenuItem("New Aspect Ratio",new BMessage(ASPECT),'R'));
   if(drawMode == ASPECT){ item->SetMarked(true); }
   submenu->AddItem(item = new BMenuItem("Aspect Ratio",new BMessage(OLD_ASPECT),'P'));
   if(drawMode == OLD_ASPECT){ item->SetMarked(true); }
   submenu->AddItem(item = new BMenuItem("Stretch",new BMessage(STRETCH),'E'));
   if(drawMode == STRETCH){ item->SetMarked(true); }
   submenu->AddItem(item = new BMenuItem("Resize Win to fit",new BMessage(WINDOW_FIT),'Z'));
   if(drawMode == WINDOW_FIT){ item->SetMarked(true); }
   submenu->AddItem(item = new BMenuItem("Tile image",new BMessage(TILE),'T'));
   if(drawMode == TILE){ item->SetMarked(true); }
   menu->AddItem(submenu);
 
   submenu = new BMenu("Set as Background");
   submenu->AddItem(new BMenuItem("Centered",new BMessage(SET_BG_CENTER)));
   submenu->AddItem(new BMenuItem("Scaled",new BMessage(SET_BG_SCALED)));
   submenu->AddItem(new BMenuItem("Tiled",new BMessage(SET_BG_TILED)));
   menu->AddItem(submenu);
   
   menu->AddItem(new BMenuItem("Toggle FullScreen",new BMessage(FULLSCREEN),'F'));
   menu->AddItem(new BMenuItem("Get Info",new BMessage(GET_IMG_INFO)));
   menu->AddItem(new BMenuItem("Nuke a small country",new BMessage(NO_FUNCTION_YET)));
   ///menu->AddItem(submenu);
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
		}
		if((buttons & B_PRIMARY_MOUSE_BUTTON) && modifiers & B_SHIFT_KEY){
		   here = where;
		 //  EndRectTracking();
		  // BeginRectTracking(BRect(where.x,where.y,where.x,where.y),B_TRACK_RECT_CORNER);
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
   EndRectTracking();
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

         HSB->SetValue(HSB->Value()+((here.x - where.x)));
         VSB->SetValue(VSB->Value()+((here.y - where.y)));
        
      //  printf("mouse is at %f,%f and the images is at %f,%f\n",where.x,where.y,here.x,here.y);
       
        
         here = where;
         //Invalidate();
         
      }     
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
   BBitmap *new_img;
   BMessage *new_msg;
   BMessage SizeMsg(IMAGE_SIZE);
   ImageFilter *temp;
   rgb_color *c = NULL;
   ssize_t s;
   if(img != NULL){ rec = img->Bounds(); }
   BRect us = Bounds();
   
   float x = (rec.Width()-us.Width());
   float y = (rec.Height()-us.Height());
   
   new_img = NULL;
   switch(msg->what){
   case SHOWER_COLOR:
      msg->FindData("RGBColor",B_RGB_COLOR_TYPE,(const void**)&c,&s);
      BGColor = *c;
      Invalidate();
      ((ImageShowWindow*)Window())->ShowerColor = *c;
      break;
   case B_PASTE:
      msg->FindData("RGBColor",B_RGB_COLOR_TYPE,(const void**)&c,&s);
      BGColor = *c;
      Invalidate();
      ((ImageShowWindow*)Window())->ShowerColor = *c;
      
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
      //Find the path to the image. Use a global BPath
      //  So that the thread can get to it and load the
      //  image up. After we find the path - kill andy
      //  current translators and start transing the new
      //  image.
      if(msg->FindFlat("imgpath",&TmpImgPath) != B_OK){
          // printf("No imgPath?\n");
           break;
      }

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
    /*  
      kill_thread(TranserThread);
      printf("Kill and start thread\n");
      TranserThread = spawn_thread(TransIt, "Translate da filz", B_NORMAL_PRIORITY, this);
	  resume_thread(TranserThread);
      */
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
   case RUN_FILTER:
      //Window()->ShowStatusBar();
      
      uint32 id;
      if (msg->FindInt32("filter_id", (int32 *)&id) != B_OK) return;
      temp = filters;
      while(true){
         if (temp == NULL) return;
         if (temp->GetId() == id) break;
         temp = temp->next;
      }
      
      if(temp->GetVer() == 1){
         new_img = temp->Run(img);
         if (new_img == NULL) {
            BAlert *alert = new BAlert(NULL, "Error filtering image!", "OK");
            alert->Go();
            break;
         }
      
         //Window()->HideStatusBar();
      
         delete img;
         img = new BBitmap(new_img);
         Invalidate();
      }
      break;
   default:
      BView::MessageReceived(msg);
      break;
   }
   delete new_img;
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
   BBitmap *new_img;

   // We need to kill the old translator thread and spin off 
   //  a new one here..
   new_img = BTranslationUtils::GetBitmapFile(TmpImgPath.Path());//tmpString

   if(new_img == NULL){
      //Thats not valid image .. oh well
   }else{
      delete img;
      CurrentPath.SetTo(TmpImgPath.Path());//tmpString
      img = new BBitmap(new_img);
      BMessage SizeMsg(IMAGE_SIZE);
      SizeMsg.MakeEmpty();
      if(SizeMsg.AddRect("imgsize",img->Bounds()) == B_OK){ 
         //Parent()->Looper()->PostMessage(&SizeMsg);
         Window()->PostMessage(&SizeMsg);
      }
      FixUPScrollBars();
      if(drawMode == WINDOW_FIT){ MessageReceived(new BMessage(WINDOW_FIT));}
      redraw = true;
   }
   TmpImgPath.Unset(); // Just for good messure
   delete new_img;
   return 0;
}






