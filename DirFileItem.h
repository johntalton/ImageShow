#ifndef DIR_FILE_ITEM_H
#define DIR_FILE_ITEM_H

#include <ListItem.h>
#include <String.h>

class DirFileItem:public BListItem{
   public:
      DirFileItem(BPath,const char *s,bool,bool);
      ~DirFileItem();
      virtual void Update(BView*, const BFont *);
      virtual void DrawItem(BView *owner, BRect frame, bool complete = false);
      void MakeMenu(BMenu*);
      bool IsDirectory();
      char* Text();
      void Rename();
      void Renamed();
      void resizeImg();
   private:
      char *name;
      bool IsDir;
      BBitmap *Img;
      BEntry entry;
      bool Thumb;
      int Size;
      
      //BView *self;
      
      //BTextControl *rename;
      //bool renameing;
      //bool stoprenameing;
      //bool doingrename;
      
};
#endif