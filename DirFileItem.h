#ifndef DIR_FILE_ITEM_H
#define DIR_FILE_ITEM_H

#include <ListItem.h>
#include <String.h>

class DirFileItem:public BListItem{
   public:
      DirFileItem(BPath,const char *s,bool,bool,bool isQ = false);
      ~DirFileItem();
      virtual void Update(BView*, const BFont *);
      virtual void DrawItem(BView *owner, BRect frame, bool complete = false);
      void MakeMenu(BMenu*);
      bool IsDirectory();
      char* Text();
      void resizeImg();
      void SetName(const char*);
      void SetSize(int);
      BPath GetPath();
   private:
      char *name;
      bool IsDir;
      BBitmap *Img;
      //BEntry entry;
      BPath myPath;
      bool Thumb;
      int Size;
      bool IsQuery;
      bool TextAtBottom;
      int32 FontSize;
      
      
};
#endif