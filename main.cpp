/*******************************************************
*   ImageShow©
*
*   Speedy little image viewer. Accepts drag-n-drop, refs
*   from files. double click and open. Fullscreen brows
*   mode, Several draw modes (strech,center,aspect,tile)
*   intuitive GUI layout, the ability to set image as the
*   background of current desktop (Center,strech or tile)
*   Verry stable. I have spent much time tracking down bugs
*   that caused carshes or bad behavior. Have added in many
*   keyboard shourtcuts to help you naviget you way around.
*
*   @author  YNOP (ynop@acm.org)
*   @version beta
*   @date    Sept 18 1999
*******************************************************/
#include "ImageShow.h"
#include "BugOutDef.h"

// Gloable Debug Out
BugOut db("ImageShow");
// Application's signature
const char *APP_SIGNATURE = "application/x-vnd.Abstract-ImageShow";

/*******************************************************
*   Main app. So launch off that ImageShow and lets
*   get down to buizness
*******************************************************/
int main(int argc, char* argv[]){
   ImageShow *app = new ImageShow();

   app->Run();
   delete app;
   return B_NO_ERROR;
}