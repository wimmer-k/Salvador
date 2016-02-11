//Global definitions
#define DEFCOLOR   "\033[m"
#define BLUE       "\033[1;34m"
#define YELLOW     "\033[1;33m"
#define GREEN      "\033[1;32m"
#define RED        "\033[1;31m"
#define BLACK      "\033[1;30m"
#define MAGENTA    "\033[1;35m"
#define CYAN       "\033[1;36m"
#define WHITE      "\033[1;37m"
int fpID[6] = {3,5,7,8,9,11};
int fpNr(int id){
  if(id<3 || id>11)
    return -1;
  switch (id){
    case 3:
      return 0;
    case 5:
      return 1;
    case 7:
      return 2;
    case 8:
      return 3;
    case 9:
      return 4;
    case 11:
      return 5;
    default:
      return -1;
  }
}
